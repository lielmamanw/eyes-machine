#include "motor.hpp"
#include "constants.hpp"

Motor::Motor(Adafruit_PWMServoDriver& driver, uint8_t channel, MotorLimits limits)
  : _driver(driver), _channel(channel), _limits(limits)
{}

void Motor::setValue(float value)
{
  const float ratio = validateValue(value);
  const float steeredRatio = _limits.inverted ? (1.0f - ratio) : ratio;
  const float range = static_cast<float>(_limits.maxPulse - _limits.minPulse);
  const uint16_t pulse = _limits.minPulse + static_cast<uint16_t>(steeredRatio * range);
  _driver.setPWM(_channel, 0, pulse);
}

float Motor::validateValue(float value) const
{
  if (value < NORMALIZED_MIN_VALUE) return NORMALIZED_MIN_VALUE;
  if (value > NORMALIZED_MAX_VALUE) return NORMALIZED_MAX_VALUE;
  return value;
}
