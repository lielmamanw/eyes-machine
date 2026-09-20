#include "auto-state.hpp"
#include "constants.hpp"
#include "random-utils.hpp"

#include <Arduino.h>

AutoState::AutoState(EyesActionsController& actions)
  : _actions(actions),
    _blinkTimer(BLINK_TIMER_CONFIG),
    _horizontalTimer(HORIZONTAL_SACCADE_TIMER_CONFIG),
    _verticalTimer(VERTICAL_SACCADE_TIMER_CONFIG),
    _blinkPhase(BlinkPhase::IDLE),
    _blinkClosedUntil(0),
    _enterComplete(false)
{}

void AutoState::reset()
{
  _blinkPhase = BlinkPhase::IDLE;
  _blinkClosedUntil = 0;
  _enterComplete = false;
}

void AutoState::enter()
{
  const unsigned long now = millis();

  _actions.setOpenness(1.0f);
  _actions.setHorizontal(0.0f);
  _actions.setVertical(0.0f);

  _blinkTimer.reset(now);
  _horizontalTimer.reset(now);
  _verticalTimer.reset(now);

  _enterComplete = true;
}

bool AutoState::isEnterComplete() const
{
  return _enterComplete;
}

void AutoState::execute()
{
  const unsigned long now = millis();

  updateBlink(now);
  updateHorizontalSaccade(now);
  updateVerticalSaccade(now);
}

void AutoState::exit()
{}

bool AutoState::isExitComplete() const
{
  return true;
}

void AutoState::updateBlink(unsigned long now)
{
  switch (_blinkPhase)
  {
    case BlinkPhase::IDLE:
      if (_blinkTimer.isDue(now)) {
        _actions.setOpenness(0.0f);
        _blinkPhase = BlinkPhase::CLOSED;
        _blinkClosedUntil = now + BLINK_CLOSED_HOLD_MS;
      }
      break;

    case BlinkPhase::CLOSED:
      if (now >= _blinkClosedUntil) {
        _actions.setOpenness(1.0f);
        _blinkPhase = BlinkPhase::IDLE;
        _blinkTimer.reschedule(now);
      }
      break;
  }
}

void AutoState::updateHorizontalSaccade(unsigned long now)
{
  if (_horizontalTimer.tryFire(now)) {
    const float target = sampleClampedGaussian(
      HORIZONTAL_SACCADE_TARGET_MEAN, HORIZONTAL_SACCADE_TARGET_STDDEV,
      AXIS_MIN_VALUE, AXIS_MAX_VALUE);
    _actions.setHorizontal(target);
  }
}

void AutoState::updateVerticalSaccade(unsigned long now)
{
  if (_verticalTimer.tryFire(now)) {
    const float target = sampleClampedGaussian(
      VERTICAL_SACCADE_TARGET_MEAN, VERTICAL_SACCADE_TARGET_STDDEV,
      AXIS_MIN_VALUE, AXIS_MAX_VALUE);
    _actions.setVertical(target);
  }
}
