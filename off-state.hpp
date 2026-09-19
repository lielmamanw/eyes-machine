#pragma once

#include <cstdint>

#include "machine-state.hpp"
#include "eyes-actions.controller.hpp"

class OffState : public MachineState
{
  public:
    explicit OffState(EyesActionsController& actions);

    void reset() override;

    void enter() override;
    bool isEnterComplete() const override;

    void execute() override;

    void exit() override;
    bool isExitComplete() const override;

  private:
    enum class BlinkPhase { EYES_OPEN, EYES_CLOSED };

    unsigned long holdDurationMs(uint8_t blinkIndex) const;

    EyesActionsController& _actions;

    bool _enterStarted;
    bool _enterComplete;
    BlinkPhase _blinkPhase;
    uint8_t _blinkIndex;
    unsigned long _phaseStartMs;

    bool _exitStarted;
    bool _exitComplete;
};
