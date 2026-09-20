#pragma once

#include "machine-state.hpp"
#include "eyes-actions.controller.hpp"
#include "random-timer.hpp"

// Mimics real-eye idle behavior: blinking and horizontal/vertical saccades,
// each independently timed and sized from its own normal distribution
// (see constants.hpp). No smooth drifting - real eyes snap to a new gaze
// point and hold, so saccades set the target directly rather than easing to it.
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
    enum class BlinkPhase { IDLE, CLOSED };

    void updateBlink(unsigned long now);
    void updateHorizontalSaccade(unsigned long now);
    void updateVerticalSaccade(unsigned long now);

    EyesActionsController& _actions;

    RandomTimer _blinkTimer;
    RandomTimer _horizontalTimer;
    RandomTimer _verticalTimer;

    BlinkPhase _blinkPhase;
    unsigned long _blinkClosedUntil;

    bool _enterComplete;
};
