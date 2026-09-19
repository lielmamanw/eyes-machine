#pragma once

#include "constants.hpp"
#include "eyes-actions.controller.hpp"
#include "machine-state.hpp"
#include "off-state.hpp"
#include "auto-state.hpp"
#include "controller-state.hpp"

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
    enum class StatePhase { ENTER, EXECUTE, EXIT };

    MachineState* stateForMode(MachineMode mode);
    void requestState(MachineState* nextState);

    MachineMode _mode;
    EyesActionsController _actionsController;

    OffState _offState;
    AutoState _autoState;
    ControllerState _controllerState;

    MachineState* _activeState;
    MachineState* _pendingState;
    StatePhase _phase;
};