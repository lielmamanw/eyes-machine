#include "eyes-machine.hpp"

EyesMachine::EyesMachine(const MachineMode mode)
  : _mode(mode),
    _actionsController(),
    _offState(_actionsController),
    _autoState(_actionsController),
    _controllerState(_actionsController),
    _activeState(&_offState),
    _pendingState(nullptr),
    _phase(StatePhase::ENTER)
{}

void EyesMachine::begin()
{
  randomSeed(esp_random());
  _actionsController.begin();
}

void EyesMachine::runInLoop()
{
  switch (_phase)
  {
    case StatePhase::ENTER:
      _activeState->enter();
      if (_activeState->isEnterComplete()) {
        _phase = StatePhase::EXECUTE;
      }
      break;

    case StatePhase::EXECUTE:
      _activeState->execute();
      break;

    case StatePhase::EXIT:
      _activeState->exit();
      if (_activeState->isExitComplete()) {
        _activeState = _pendingState;
        _pendingState = nullptr;
        _activeState->reset();
        _phase = StatePhase::ENTER;
      }
      break;
  }
}

EyesMachine::MachineMode EyesMachine::setMode(const MachineMode mode)
{
  constexpr uint8_t RGB_PIN = 48;

  _mode = (mode == MachineMode::UNREACHABLE_MODE) ? MachineMode::OFF : mode;
  const RGBColor modeColor = EyesMachineModeColors[static_cast<uint8_t>(_mode)];
  rgbLedWrite(RGB_PIN, modeColor.r, modeColor.g, modeColor.b);

  requestState(stateForMode(_mode));

  return _mode;
}

EyesMachine::MachineMode EyesMachine::setNextMode()
{
  MachineMode currentMode = _mode;
  int currentModeAsNumber = static_cast<int>(currentMode);
  MachineMode nextMode = static_cast<MachineMode>(currentModeAsNumber + 1);
  return setMode(nextMode);
}

MachineState* EyesMachine::stateForMode(MachineMode mode)
{
  switch (mode)
  {
    case MachineMode::AUTO: return &_autoState;
    case MachineMode::CONTROLLER: return &_controllerState;
    case MachineMode::OFF:
    default: return &_offState;
  }
}

void EyesMachine::requestState(MachineState* nextState)
{
  if (nextState == _activeState || nextState == _pendingState) return;

  _pendingState = nextState;
  _phase = StatePhase::EXIT;
}