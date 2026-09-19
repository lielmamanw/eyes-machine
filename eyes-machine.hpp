#pragma once

#include "constants.hpp"
#include "eyes-actions.controller.hpp"

#include <cstdint>
#include <Arduino.h>

class EyesMachine
{
  public:
    enum class MachineMode
    {
      OFF,
      AUTO,
      CONTROLLER,
      UNREACHABLE_MODE
    };

    EyesMachine(const MachineMode mode = MachineMode::AUTO);

    void begin();
    void runInLoop();

    MachineMode setMode(const MachineMode mode);
    MachineMode setNextMode();

  private:
    MachineMode _mode;
    EyesActionsController _actionsController;
};