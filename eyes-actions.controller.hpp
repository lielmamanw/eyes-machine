#pragma once

#include <cstdint>
#include <Arduino.h>

class EyesActionsController
{
  public:
    EyesActionsController();

    void setXaxis(int8_t val);
    void setYaxis(int8_t val);
    void setEyelids(int8_t val);
};