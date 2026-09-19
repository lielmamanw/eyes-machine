#pragma once

#include <cstdint>

#include "motor.hpp"

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