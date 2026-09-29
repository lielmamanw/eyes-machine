#include "auto-state.hpp"
#include "constants.hpp"
#include "random-utils.hpp"

#include <Arduino.h>

AutoState::AutoState(EyesActionsController& actions)
  : _actions(actions),
    _blinkTimer(BLINK_TIMER_CONFIG),
    _horizontalTimer(HORIZONTAL_SACCADE_TIMER_CONFIG),
    _verticalTimer(VERTICAL_SACCADE_TIMER_CONFIG),
    _jitterTimer(JITTER_TIMER_CONFIG),
    _energyTimer(ENERGY_TIMER_CONFIG),
    _blinkPhase(BlinkPhase::IDLE),
    _blinkKind(BlinkKind::FULL),
    _blinkClosedUntil(0),
    _horizontalTarget(0.0f),
    _verticalTarget(0.0f),
    _energy(ENERGY_DEFAULT),
    _enterComplete(false)
{}

void AutoState::reset()
{
  _blinkPhase = BlinkPhase::IDLE;
  _blinkKind = BlinkKind::FULL;
  _blinkClosedUntil = 0;
  _horizontalTarget = 0.0f;
  _verticalTarget = 0.0f;
  _energy = ENERGY_DEFAULT;
  _enterComplete = false;
}

void AutoState::enter()
{
  const unsigned long now = millis();

  _actions.setOpenness(1.0f);
  _actions.setHorizontal(0.0f);
  _actions.setVertical(0.0f);

  _horizontalTarget = 0.0f;
  _verticalTarget = 0.0f;

  _blinkTimer.reset(now);
  _horizontalTimer.reset(now);
  _verticalTimer.reset(now);
  _jitterTimer.reset(now);
  _energyTimer.reset(now);

  _enterComplete = true;
}

bool AutoState::isEnterComplete() const
{
  return _enterComplete;
}

void AutoState::execute()
{
  const unsigned long now = millis();

  updateEnergy(now);
  updateBlink(now);
  updateHorizontalSaccade(now);
  updateVerticalSaccade(now);
  updateJitter(now);
}

void AutoState::exit()
{}

bool AutoState::isExitComplete() const
{
  return true;
}

void AutoState::triggerBlink(unsigned long now)
{
  if (_blinkPhase != BlinkPhase::IDLE) return; // already mid-blink, don't restart

  if (sampleUniform01() < WINK_PROBABILITY) {
    _blinkKind = (sampleUniform01() < 0.5f) ? BlinkKind::LEFT_WINK : BlinkKind::RIGHT_WINK;
    if (_blinkKind == BlinkKind::LEFT_WINK) {
      _actions.setLeftEyeOpenness(0.0f);
    } else {
      _actions.setRightEyeOpenness(0.0f);
    }
  } else {
    _blinkKind = BlinkKind::FULL;
    _actions.setOpenness(0.0f);
  }

  _blinkPhase = BlinkPhase::CLOSED;
  _blinkClosedUntil = now + BLINK_CLOSED_HOLD_MS;
}

void AutoState::updateBlink(unsigned long now)
{
  switch (_blinkPhase)
  {
    case BlinkPhase::IDLE:
      if (_blinkTimer.isDue(now)) {
        triggerBlink(now);
      }
      break;

    case BlinkPhase::CLOSED:
      if (now >= _blinkClosedUntil) {
        switch (_blinkKind)
        {
          case BlinkKind::LEFT_WINK: _actions.setLeftEyeOpenness(1.0f); break;
          case BlinkKind::RIGHT_WINK: _actions.setRightEyeOpenness(1.0f); break;
          case BlinkKind::FULL:
          default: _actions.setOpenness(1.0f); break;
        }
        _blinkPhase = BlinkPhase::IDLE;
        _blinkTimer.reschedule(now, _energy);
      }
      break;
  }
}

void AutoState::updateHorizontalSaccade(unsigned long now)
{
  if (_horizontalTimer.tryFire(now, _energy)) {
    _horizontalTarget = sampleClampedGaussian(
      HORIZONTAL_SACCADE_TARGET_MEAN, HORIZONTAL_SACCADE_TARGET_STDDEV,
      AXIS_MIN_VALUE, AXIS_MAX_VALUE);
    _actions.setHorizontal(_horizontalTarget);

    if (sampleUniform01() < SACCADE_SYNCED_BLINK_PROBABILITY) {
      triggerBlink(now);
    }
  }
}

void AutoState::updateVerticalSaccade(unsigned long now)
{
  if (_verticalTimer.tryFire(now, _energy)) {
    _verticalTarget = sampleClampedGaussian(
      VERTICAL_SACCADE_TARGET_MEAN, VERTICAL_SACCADE_TARGET_STDDEV,
      AXIS_MIN_VALUE, AXIS_MAX_VALUE);
    _actions.setVertical(_verticalTarget);

    if (sampleUniform01() < SACCADE_SYNCED_BLINK_PROBABILITY) {
      triggerBlink(now);
    }
  }
}

void AutoState::updateJitter(unsigned long now)
{
  if (_jitterTimer.tryFire(now)) {
    const float jitteredHorizontal = sampleClampedGaussian(
      _horizontalTarget, JITTER_STDDEV, AXIS_MIN_VALUE, AXIS_MAX_VALUE);
    const float jitteredVertical = sampleClampedGaussian(
      _verticalTarget, JITTER_STDDEV, AXIS_MIN_VALUE, AXIS_MAX_VALUE);

    _actions.setHorizontal(jitteredHorizontal);
    _actions.setVertical(jitteredVertical);
  }
}

void AutoState::updateEnergy(unsigned long now)
{
  if (_energyTimer.tryFire(now)) {
    _energy += sampleGaussian(0.0f, ENERGY_STEP_STDDEV);

    if (_energy < ENERGY_MIN) _energy = ENERGY_MIN;
    if (_energy > ENERGY_MAX) _energy = ENERGY_MAX;
  }
}
