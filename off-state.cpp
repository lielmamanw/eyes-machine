#include "off-state.hpp"
#include "constants.hpp"

#include <Arduino.h>

OffState::OffState(EyesActionsController& actions)
  : _actions(actions),
    _enterStarted(false),
    _enterComplete(false),
    _blinkPhase(BlinkPhase::EYES_OPEN),
    _blinkIndex(0),
    _phaseStartMs(0),
    _exitStarted(false),
    _exitComplete(false)
{}

void OffState::reset()
{
  _enterStarted = false;
  _enterComplete = false;
  _blinkPhase = BlinkPhase::EYES_OPEN;
  _blinkIndex = 0;
  _phaseStartMs = 0;

  _exitStarted = false;
  _exitComplete = false;
}

void OffState::enter()
{
  if (_enterComplete) return;

  const unsigned long now = millis();

  if (!_enterStarted) {
    _actions.setHorizontal(0.0f);
    _actions.setVertical(0.0f);
    _actions.setOpenness(1.0f);

    _enterStarted = true;
    _blinkPhase = BlinkPhase::EYES_OPEN;
    _blinkIndex = 0;
    _phaseStartMs = now;
    return;
  }

  switch (_blinkPhase)
  {
    case BlinkPhase::EYES_OPEN:
      if (now - _phaseStartMs >= holdDurationMs(_blinkIndex)) {
        _actions.setOpenness(0.0f);
        _blinkPhase = BlinkPhase::EYES_CLOSED;
        _phaseStartMs = now;
      }
      break;

    case BlinkPhase::EYES_CLOSED:
      if (now - _phaseStartMs >= holdDurationMs(_blinkIndex)) {
        _blinkIndex++;
        if (_blinkIndex >= OFF_STATE_TIRED_BLINK_COUNT) {
          _actions.setOpenness(0.0f); // stay closed - the tired blinking is done
          _enterComplete = true;
        } else {
          _actions.setOpenness(1.0f);
          _blinkPhase = BlinkPhase::EYES_OPEN;
          _phaseStartMs = now;
        }
      }
      break;
  }
}

bool OffState::isEnterComplete() const
{
  return _enterComplete;
}

void OffState::execute()
{
  _actions.setOpenness(0.0f);
  _actions.setHorizontal(0.0f);
  _actions.setVertical(0.0f);
}

void OffState::exit()
{
  if (_exitComplete) return;

  const unsigned long now = millis();

  if (!_exitStarted) {
    _actions.setRightEyeOpenness(1.0f); // peek
    _exitStarted = true;
    _phaseStartMs = now;
    return;
  }

  if (now - _phaseStartMs >= OFF_STATE_PEEK_DURATION_MS) {
    _exitComplete = true;
  }
}

bool OffState::isExitComplete() const
{
  return _exitComplete;
}

unsigned long OffState::holdDurationMs(uint8_t blinkIndex) const
{
  unsigned long duration = OFF_STATE_BLINK_BASE_HOLD_MS;
  for (uint8_t i = 0; i < blinkIndex; ++i) {
    duration = static_cast<unsigned long>(duration * OFF_STATE_BLINK_SLOWDOWN_FACTOR);
  }
  return duration;
}
