// BSP mapping functions

#include "BSP.h"
#include "BootLogo.h"
#include "FS2711.hpp"
#include "HUB238.hpp"
#include "I2C_Wrapper.hpp"
#include "Pins.h"
#include "Settings.h"
#include "Setup.h"
#include "TipThermoModel.h"
#include "configuration.h"
#include "history.hpp"
#include "main.hpp"
#include <IRQ.h>

volatile uint16_t    PWMSafetyTimer   = 0;
volatile uint8_t     pendingPWM       = 0;
const uint16_t       powerPWM         = 255;
static const uint8_t holdoffTicks     = 15; // delay of 8 ish ms
static const uint8_t tempMeasureTicks = 15;

uint16_t totalPWM = powerPWM + tempMeasureTicks + holdoffTicks; // htim2.Init.Period, the full PWM cycle

void resetWatchdog() { HAL_IWDG_Refresh(&hiwdg); }
// Lookup table for the NTC
// We dont know exact specs, but it loooks to be roughly a 10K B=4000 NTC
// Stored as ADCReading,Temp in degC
static const uint16_t NTCHandleLookup[] = {
    // ADC Reading , Temp in C
    23931, 0,  //
    23210, 2,  //
    22466, 4,  //
    21703, 6,  //
    20924, 8,  //
    20135, 10, //
    19338, 12, //
    18538, 14, //
    17738, 16, //
    16943, 18, //
    16156, 20, //
    15381, 22, //
    14621, 24, //
    13878, 26, //
    13155, 28, //
    12455, 30, //
    11778, 32, //
    11126, 34, //
    10501, 36, //
    9902,  38, //
    9330,  40, //
    8786,  42, //
    8269,  44, //
    // Extended beyond the measured points using the same NTC model (10K, B~4000, reading ~= 32767*R/(R+13K))
    // so the handle temperature is usable for CJC and for derating when the handle / MOSFET area gets hot.
    7301,  48, //
    6437,  52, //
    5670,  56, //
    4990,  60, //
    4394,  64, //
    3871,  68, //
    3412,  72, //
    3011,  76, //
    2660,  80, //
};

uint16_t getHandleTemperature(uint8_t sample) {
#ifdef TMP36_ADC1_CHANNEL
  int32_t result = getADCHandleTemp(sample);
  // S60 uses 10k NTC resistor
  // For now not doing interpolation
  for (uint32_t i = 0; i < (sizeof(NTCHandleLookup) / (2 * sizeof(uint16_t))); i++) {
    if (result > NTCHandleLookup[(i * 2) + 0]) {
      return NTCHandleLookup[(i * 2) + 1] * 10;
    }
  }
  return 85 * 10;
#else
  return 0; // Not implemented
#endif
}

uint16_t getInputVoltageX10(uint16_t divisor, uint8_t sample) {
  // ADC maximum is 32767 == 3.3V at input == 28.05V at VIN
  // Therefore we can divide down from there
  // Multiplying ADC max by 4 for additional calibration options,
  // ideal term is 467
  uint32_t res = getADCVin(sample);
  res *= 4;
  res /= divisor;
  return res;
}

static void switchToFastPWM(void) {
  // 20Hz
  totalPWM             = powerPWM + tempMeasureTicks + holdoffTicks;
  htim2.Instance->ARR  = totalPWM;
  htim2.Instance->CCR1 = powerPWM + holdoffTicks;
  htim2.Instance->CCR4 = powerPWM;
  htim2.Instance->PSC  = 1500;
}

#ifdef TIP_CURRENT_LIMIT_CHOP
/*
 * Output stage drive for irons without an inductor (S99):
 *
 *  TIM2 (~20 Hz) is the power envelope: the tip is driven for pendingPWM of the powerPWM ticks, then the
 *  output is forced off for the holdoff + ADC measurement window. This is what the PID controls, exactly
 *  like the TS100 / Pinecil.
 *
 *  TIM4 (fast, user selectable frequency) only chops *inside* the envelope on-time, and only at the duty
 *  needed to keep the average tip current within what the supply can deliver (I_limit / I_tip). If the supply
 *  can deliver the full tip current, TIM4 sits at 100 % and the MOSFET only switches at 20 Hz.
 *
 *  Hard switching 8 A at 11 kHz through the weak gate drive is what overheats the MOSFET, so this keeps the
 *  number of switching events proportional to the delivered power, and to zero for supplies that don't need it.
 */
static const uint16_t tipChopPeriodTicks       = 64 + 1;                 // TIM4 ARR + 1
static const uint16_t tipChopPrescalers[]      = {10, 20, 40, 80};       // 8 MHz / (PSC+1) / 65 -> 11.2k, 5.9k, 3.0k, 1.5k Hz
static volatile uint16_t tipChopCompare        = tipChopPeriodTicks;     // TIM4 CCR3 while the envelope is on; ARR+1 == solid on
static uint16_t          tipChopDutyX256       = 256;

static void applyTipChopPrescaler(void) {
  uint8_t idx = getSettingValue(SettingsOptions::TipChopFrequency);
  if (idx >= (sizeof(tipChopPrescalers) / sizeof(tipChopPrescalers[0]))) {
    idx = 0;
  }
  if (htim4.Instance->PSC != tipChopPrescalers[idx]) {
    htim4.Instance->PSC = tipChopPrescalers[idx];
    htim4.Instance->EGR = TIM_EGR_UG; // Load the new prescaler now rather than at next update
  }
}

uint32_t getTipChopFrequencyHzX10() {
  uint8_t idx = getSettingValue(SettingsOptions::TipChopFrequency);
  if (idx >= (sizeof(tipChopPrescalers) / sizeof(tipChopPrescalers[0]))) {
    idx = 0;
  }
  return (8000000UL * 10) / ((uint32_t)(tipChopPrescalers[idx] + 1) * tipChopPeriodTicks);
}

uint16_t getTipChopDutyX256() {
  // Tip current at the present input voltage
  uint32_t voltageX10   = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
  uint32_t tipRx10      = getTipResistanceX10();
  uint32_t tipCurrentx100 = 0;
  if (tipRx10 > 0 && voltageX10 > 0) {
    tipCurrentx100 = (voltageX10 * 100) / tipRx10;
  }

  // Current the supply is allowed to deliver
  uint32_t limitx100 = 0;
#if POW_PD_EXT == 2
  if (!getIsPoweredByDCIN() && FS2711::has_run_selection()) {
    limitx100 = FS2711::source_currentx100();
  }
#endif
  // The user power limit is also treated as a supply limit (this is how DC users describe their supply)
  const uint32_t userLimitW = getSettingValue(SettingsOptions::PowerLimit);
  if (userLimitW && voltageX10) {
    uint32_t userLimitx100 = (userLimitW * 1000) / voltageX10;
    if (limitx100 == 0 || userLimitx100 < limitx100) {
      limitx100 = userLimitx100;
    }
  }

  uint32_t dutyX256 = 256;
  if (limitx100 && tipCurrentx100 > limitx100) {
    dutyX256 = (limitx100 * 256) / tipCurrentx100;
    if (dutyX256 < 8) {
      dutyX256 = 8; // ~3 %, keeps the output alive so the tip still gets measured meaningfully
    }
  }
  tipChopDutyX256 = dutyX256;

  uint16_t compare = tipChopPeriodTicks;
  if (dutyX256 < 256) {
    compare = (dutyX256 * tipChopPeriodTicks) / 256;
    if (compare < 1) {
      compare = 1;
    }
  }
  tipChopCompare = compare;
  applyTipChopPrescaler();
  return tipChopDutyX256;
}

void setTipPWM(const uint8_t pulse, const bool shouldUseFastModePWM) {
  (void)shouldUseFastModePWM;
  PWMSafetyTimer = 20; // This is decremented in the handler for PWM so that the tip pwm is
                       // disabled if the PID task is not scheduled often enough.
  pendingPWM = pulse;
}
// These are called by the HAL after the corresponding events from the system
// timers.

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  // Period has elapsed
  if (htim->Instance == TIM2) {
    // Start of a new envelope period
    if (PWMSafetyTimer) {
      PWMSafetyTimer--;
    }
    // We decrement this safety value so that lockups in the
    // scheduler will not cause the PWM to become locked in an
    // active driving state.
    if (PWMSafetyTimer == 0 || pendingPWM == 0) {
      htim2.Instance->CCR4 = 0;
      htim4.Instance->CCR3 = 0;
    } else {
      htim2.Instance->CCR4 = pendingPWM;     // Envelope on-time, always < holdoff / ADC trigger point
      htim4.Instance->CCR3 = tipChopCompare; // Chop only as hard as the supply needs
    }
  } else if (htim->Instance == TIM1) {
    // STM uses this for internal functions as a counter for timeouts
    HAL_IncTick();
  }
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
  // End of the envelope on-time
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
    htim4.Instance->CCR3 = 0;
  }
}

#else /* !TIP_CURRENT_LIMIT_CHOP : original always-chop scheme (S60 / S60P / T55) */

void setTipPWM(const uint8_t pulse, const bool shouldUseFastModePWM) {
  PWMSafetyTimer = 20; // This is decremented in the handler for PWM so that the tip pwm is
                       // disabled if the PID task is not scheduled often enough.
  pendingPWM = pulse;
}
// These are called by the HAL after the corresponding events from the system
// timers.

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  // Period has elapsed
  if (htim->Instance == TIM2) {
    // we want to turn on the output again
    PWMSafetyTimer--;
    // We decrement this safety value so that lockups in the
    // scheduler will not cause the PWM to become locked in an
    // active driving state.
    // While we could assume this could never happen, its a small price for
    // increased safety
    if (PWMSafetyTimer == 0) {
      htim4.Instance->CCR3 = 0;
    } else {
      htim4.Instance->CCR3 = pendingPWM / 4;
    }
  } else if (htim->Instance == TIM1) {
    // STM uses this for internal functions as a counter for timeouts
    HAL_IncTick();
  }
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
  // This was a when the PWM for the output has timed out
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
    // HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
    htim4.Instance->CCR3 = 0;
  }
}
#endif /* TIP_CURRENT_LIMIT_CHOP */

void unstick_I2C() {}

uint8_t getButtonA() { return HAL_GPIO_ReadPin(KEY_A_GPIO_Port, KEY_A_Pin) == GPIO_PIN_RESET ? 1 : 0; }
uint8_t getButtonB() { return HAL_GPIO_ReadPin(KEY_B_GPIO_Port, KEY_B_Pin) == GPIO_PIN_RESET ? 1 : 0; }

void BSPInit(void) { switchToFastPWM(); }

void reboot() { NVIC_SystemReset(); }

void delay_ms(uint16_t count) { HAL_Delay(count); }

bool isTipDisconnected() {

  uint16_t tipDisconnectedThres = TipThermoModel::getTipMaxInC() - 5;
  uint32_t tipTemp              = TipThermoModel::getTipInC();
  return tipTemp > tipDisconnectedThres;
}

void    setStatusLED(const enum StatusLED state) {}
uint8_t preStartChecks() {
#if POW_PD_EXT == 1
  if (!hub238_has_run_selection() && (xTaskGetTickCount() < TICKS_SECOND * 5)) {
    return 0;
  }
  // We check if we are in a "Limited" mode; where we have to run the PWM really fast
  // Where as if we are on 9V for example, the tip resistance is enough
  uint16_t voltage     = hub238_source_voltage();
  uint16_t currentx100 = hub238_source_currentX100();
#endif
#if POW_PD_EXT == 2
  if (!FS2711::has_run_selection() && (xTaskGetTickCount() < TICKS_SECOND * 5)) {
    return 0;
  }
  uint16_t voltage     = FS2711::source_voltage();
  uint16_t currentx100 = FS2711::source_currentx100();
#endif

#ifdef TIP_CURRENT_LIMIT_CHOP
  (void)voltage;
  (void)currentx100;
  getTipChopDutyX256(); // Latch chop duty + prescaler for the negotiated supply before heating starts
#else
  uint16_t thresholdResistancex10 = ((voltage * 1000) / currentx100) + 5;

  if (getTipResistanceX10() <= thresholdResistancex10) {
    // We are limited by resistance, not our current limiting, we can slow down PWM to avoid audible noise
    htim4.Instance->PSC = 50; // 10 -> 500 removes audible noise
  }
#endif

  return 1; // We are done now
}
uint64_t getDeviceID() {
  //
  return HAL_GetUIDw0() | ((uint64_t)HAL_GetUIDw1() << 32);
}

uint8_t getTipResistanceX10() {
#ifdef COPPER_HEATER_COIL

  // TODO
  //! Warning, must never return 0.
  TemperatureType_t measuredTemperature = TipThermoModel::getTipInC(false);
  if (measuredTemperature < 25) {
    return 50; // Start assuming under spec to soft-start
  }

  // Assuming a temperature rise of 0.00393 per deg c over 20C

  uint32_t scaler = 393 * (measuredTemperature - 20);

  return TIP_RESISTANCE + ((TIP_RESISTANCE * scaler) / 100000);
#else
  uint8_t user_selected_tip = getUserSelectedTipResistance();
  if (user_selected_tip == 0) {
    return TIP_RESISTANCE; // Auto mode
  }
  return user_selected_tip;
#endif
}
bool    isTipShorted() { return false; }
uint8_t preStartChecksDone() { return 1; }

uint16_t getTipThermalMass() { return TIP_THERMAL_MASS; }
uint16_t getTipInertia() { return TIP_THERMAL_INERTIA; }

void setBuzzer(bool on) {}

void showBootLogo(void) { BootLogo::handleShowingLogo((uint8_t *)FLASH_LOGOADDR); }

#ifdef CUSTOM_MAX_TEMP_C
TemperatureType_t getCustomTipMaxInC() { return MAX_TEMP_C; }
#endif
