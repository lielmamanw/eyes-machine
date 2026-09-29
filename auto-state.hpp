#pragma once

#include "machine-state.hpp"
#include "eyes-actions.controller.hpp"
#include "random-timer.hpp"

// Mimics real-eye idle behavior: blinking and horizontal/vertical saccades,
// each independently timed and sized from its own normal distribution
// (see constants.hpp). No smooth drifting - real eyes snap to a new gaze
// point and hold, so saccades set the target directly rather than easing to it.
//
// Gimmicks layered on top: occasional single-eye wink instead of a full blink,
// a blink synced to a saccade (real eyes often blink while changing gaze),
// continuous micro-jitter around the current gaze point, and a slow-drifting
// "energy" level that speeds up or slows down blink/saccade rate over time.
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
    enum class BlinkKind { FULL, LEFT_WINK, RIGHT_WINK };

    void triggerBlink(unsigned long now);
    void updateBlink(unsigned long now);
    void updateHorizontalSaccade(unsigned long now);
    void updateVerticalSaccade(unsigned long now);
    void updateJitter(unsigned long now);
    void updateEnergy(unsigned long now);

    EyesActionsController& _actions;

    RandomTimer _blinkTimer;
    RandomTimer _horizontalTimer;
    RandomTimer _verticalTimer;
    RandomTimer _jitterTimer;
    RandomTimer _energyTimer;

    BlinkPhase _blinkPhase;
    BlinkKind _blinkKind;
    unsigned long _blinkClosedUntil;

    float _horizontalTarget; // last saccade target - jitter wobbles around this, not the raw output
    float _verticalTarget;

    float _energy; // scales blink/saccade RandomTimer intervals; see constants.hpp

    bool _enterComplete;
};
