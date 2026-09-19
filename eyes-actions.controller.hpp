#pragma once

#include <cstdint>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#include "motor.hpp"
#include "constants.hpp"

class EyesActionsController
{
  public:
    EyesActionsController();

    void begin();

    void setHorizontal(float value);       // -1.0 (left)  .. 1.0 (right)
    void setVertical(float value);         // -1.0 (down)  .. 1.0 (up)
    void setOpenness(float value);         // both eyes,  0.0 (closed) .. 1.0 (open)
    void setRightEyeOpenness(float value); // 0.0 (closed) .. 1.0 (open)
    void setLeftEyeOpenness(float value);  // 0.0 (closed) .. 1.0 (open)

  private:
    float validateVal(float value, float minVal, float maxVal) const;

    Adafruit_PWMServoDriver _driver;

    Motor _xAxis;
    Motor _yAxis;
    Motor _rightUpperLid;
    Motor _rightLowerLid;
    Motor _leftUpperLid;
    Motor _leftLowerLid;
};
