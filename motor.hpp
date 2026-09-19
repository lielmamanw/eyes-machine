#pragma once

#include <cstdint>
#include <Adafruit_PWMServoDriver.h>

struct MotorLimits
{
  uint16_t minPulse;
  uint16_t maxPulse;
  bool inverted = false;
};

class Motor
{
  public:
    Motor(Adafruit_PWMServoDriver& driver, uint8_t channel, MotorLimits limits);

    void setValue(float value);

  private:
    float validateValue(float value) const;

    Adafruit_PWMServoDriver& _driver;
    uint8_t _channel;
    MotorLimits _limits;
};
