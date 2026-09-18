#pragma once

#include <cstdint>
#include <Arduino.h>

class EyesMachine
{
  public:
    enum class MachineMode
    {
      AUTO,
      CONTROLLER
    };

    EyesMachine(const MachineMode mode = MachineMode::AUTO);

    MachineMode setMode(const MachineMode mode);
    MachineMode setNextMode();

  private:
    MachineMode _mode;
};