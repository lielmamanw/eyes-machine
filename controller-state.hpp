#pragma once

#include "machine-state.hpp"
#include "eyes-actions.controller.hpp"

// TODO: real ESP-NOW controlled behavior - a receive callback should feed
// commands into a small buffer here, drained non-blockingly from execute().
class ControllerState : public MachineState
{
  public:
    explicit ControllerState(EyesActionsController& actions);

    void reset() override;

    void enter() override;
    bool isEnterComplete() const override;

    void execute() override;

    void exit() override;
    bool isExitComplete() const override;

  private:
    EyesActionsController& _actions;
};
