#pragma once

#include "machine-state.hpp"
#include "eyes-actions.controller.hpp"

// TODO: real autonomous behavior - random eye movement/blinking via a normal
// distribution, sampled on this state's own timing, not the hardware layer's.
class AutoState : public MachineState
{
  public:
    explicit AutoState(EyesActionsController& actions);

    void reset() override;

    void enter() override;
    bool isEnterComplete() const override;

    void execute() override;

    void exit() override;
    bool isExitComplete() const override;

  private:
    EyesActionsController& _actions;
};
