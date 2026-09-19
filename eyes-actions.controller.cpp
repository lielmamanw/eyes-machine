#include "eyes-actions.controller.hpp"
#include "constants.hpp"

EyesActionsController::EyesActionsController()
  : _driver(),
    _xAxis(_driver, static_cast<uint8_t>(MotorChannel::X_AXIS), X_AXIS_LIMITS),
    _yAxis(_driver, static_cast<uint8_t>(MotorChannel::Y_AXIS), Y_AXIS_LIMITS),
    _rightUpperLid(_driver, static_cast<uint8_t>(MotorChannel::RIGHT_UPPER_LID), RIGHT_UPPER_LID_LIMITS),
    _rightLowerLid(_driver, static_cast<uint8_t>(MotorChannel::RIGHT_LOWER_LID), RIGHT_LOWER_LID_LIMITS),
    _leftUpperLid(_driver, static_cast<uint8_t>(MotorChannel::LEFT_UPPER_LID), LEFT_UPPER_LID_LIMITS),
    _leftLowerLid(_driver, static_cast<uint8_t>(MotorChannel::LEFT_LOWER_LID), LEFT_LOWER_LID_LIMITS)
{}

void EyesActionsController::begin()
{
  Wire.begin(SDA_PIN, SCL_PIN); 
  _driver.begin();
  _driver.setPWMFreq(SERVO_PWM_FREQUENCY_HZ);
}

void EyesActionsController::setHorizontal(float value)
{
  const float clamped = validateVal(value, AXIS_MIN_VALUE, AXIS_MAX_VALUE);
  _xAxis.setValue((clamped + 1.0f) / 2.0f);
}

void EyesActionsController::setVertical(float value)
{
  const float clamped = validateVal(value, AXIS_MIN_VALUE, AXIS_MAX_VALUE);
  _yAxis.setValue((clamped + 1.0f) / 2.0f);
}

void EyesActionsController::setOpenness(float value)
{
  setRightEyeOpenness(value);
  setLeftEyeOpenness(value);
}

void EyesActionsController::setRightEyeOpenness(float value)
{
  const float clamped = validateVal(value, OPENNESS_MIN_VALUE, OPENNESS_MAX_VALUE);
  _rightUpperLid.setValue(clamped);
  _rightLowerLid.setValue(clamped);
}

void EyesActionsController::setLeftEyeOpenness(float value)
{
  const float clamped = validateVal(value, OPENNESS_MIN_VALUE, OPENNESS_MAX_VALUE);
  _leftUpperLid.setValue(clamped);
  _leftLowerLid.setValue(clamped);
}

float EyesActionsController::validateVal(float value, float minVal, float maxVal) const
{
  if (value < minVal) return minVal;
  if (value > maxVal) return maxVal;
  return value;
}
