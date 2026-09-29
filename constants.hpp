#pragma once

#include <cstdint>

#include "motor.hpp"
#include "random-timer.hpp"

struct RGBColor
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

constexpr RGBColor EyesMachineModeColors[] =
{
  {255, 0, 0}, // Red
  {0, 255, 0}, // Green
  {0, 0, 255} // Blue
};

// --- Motors / PCA9685 ---

constexpr uint16_t SERVO_PWM_FREQUENCY_HZ = 50;

enum class MotorChannel
{
  RIGHT_LOWER_LID,
  RIGHT_UPPER_LID,
  X_AXIS,
  Y_AXIS,
  LEFT_LOWER_LID,
  LEFT_UPPER_LID
};

constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 20;

// Motor::setValue() normalized input range
constexpr float NORMALIZED_MIN_VALUE = 0.0f;
constexpr float NORMALIZED_MAX_VALUE = 1.0f;

// EyesActionsController public API ranges
constexpr float AXIS_MIN_VALUE = -1.0f;
constexpr float AXIS_MAX_VALUE = 1.0f;

constexpr float OPENNESS_MIN_VALUE = 0.0f;
constexpr float OPENNESS_MAX_VALUE = 1.0f;

// Per-motor calibration (min pulse, max pulse, inverted) - tuned on the real rig.
constexpr MotorLimits X_AXIS_LIMITS = { 260, 400, true };
constexpr MotorLimits Y_AXIS_LIMITS = { 300, 400, false };
constexpr MotorLimits RIGHT_UPPER_LID_LIMITS = { 240, 350, false };
constexpr MotorLimits RIGHT_LOWER_LID_LIMITS = { 250, 350, true };
constexpr MotorLimits LEFT_UPPER_LID_LIMITS = { 250, 350, true };
constexpr MotorLimits LEFT_LOWER_LID_LIMITS = { 310, 430, false };

// --- OffState timing (tired-blink enter animation + one-eye peek on exit) ---
constexpr uint8_t OFF_STATE_TIRED_BLINK_COUNT = 3;
constexpr unsigned long OFF_STATE_BLINK_BASE_HOLD_MS = 150UL;
constexpr float OFF_STATE_BLINK_SLOWDOWN_FACTOR = 1.6f;
constexpr unsigned long OFF_STATE_PEEK_DURATION_MS = 2000UL;

// --- AutoState behavior (each action has its own timing + magnitude distribution) ---

// How long a blink stays closed before reopening.
constexpr unsigned long BLINK_CLOSED_HOLD_MS = 120UL;

constexpr RandomTimerConfig BLINK_TIMER_CONFIG = { 4000.0f, 1500.0f, 1500UL };
constexpr RandomTimerConfig HORIZONTAL_SACCADE_TIMER_CONFIG = { 2500.0f, 900.0f, 600UL };
constexpr RandomTimerConfig VERTICAL_SACCADE_TIMER_CONFIG = { 3000.0f, 1000.0f, 700UL };

// Saccade target position distributions, in the same -1..1 space as setHorizontal/setVertical.
constexpr float HORIZONTAL_SACCADE_TARGET_MEAN = 0.0f;
constexpr float HORIZONTAL_SACCADE_TARGET_STDDEV = 0.5f;

constexpr float VERTICAL_SACCADE_TARGET_MEAN = 0.0f;
constexpr float VERTICAL_SACCADE_TARGET_STDDEV = 0.35f;

// --- AutoState gimmicks ---

// Wink: occasionally blink only one eye instead of both.
constexpr float WINK_PROBABILITY = 0.12f;

// Blink synced with a saccade: occasionally blink at the same moment the gaze jumps.
constexpr float SACCADE_SYNCED_BLINK_PROBABILITY = 0.25f;

// Idle micro-jitter: tiny continuous nudges around the current gaze point between saccades.
constexpr RandomTimerConfig JITTER_TIMER_CONFIG = { 200.0f, 60.0f, 80UL };
constexpr float JITTER_STDDEV = 0.03f;

// Energy: a slow random walk (0.5 = lazy, 1.0 = normal, 1.5 = excited) that speeds up
// or slows down blink & saccade rate via RandomTimer's speedScale.
constexpr RandomTimerConfig ENERGY_TIMER_CONFIG = { 8000.0f, 2000.0f, 4000UL };
constexpr float ENERGY_STEP_STDDEV = 0.15f;
constexpr float ENERGY_MIN = 0.5f;
constexpr float ENERGY_MAX = 1.5f;
constexpr float ENERGY_DEFAULT = 1.0f;