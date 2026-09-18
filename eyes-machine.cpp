#include "eyes-machine.hpp"

EyesMachine::EyesMachine(const MachineMode mode) : _mode(mode)
{}

EyesMachine::MachineMode EyesMachine::setMode(const MachineMode mode)
{
  constexpr uint8_t RGB_PIN = 48; 
  
  _mode = mode;
  switch(_mode)
  {
    case MachineMode::AUTO:
      rgbLedWrite(RGB_PIN, 255, 0, 0);
      break;
    case MachineMode::CONTROLLER:
      rgbLedWrite(RGB_PIN, 0, 0, 255);
      break;
    default:
      _mode = setMode(MachineMode::AUTO);
      break;
  }
  return _mode;
}

EyesMachine::MachineMode EyesMachine::setNextMode()
{
  MachineMode currentMode = _mode;
  int currentModeAsNumber = static_cast<int>(currentMode);
  MachineMode nextMode = static_cast<MachineMode>(currentModeAsNumber + 1);
  Serial.printf("Next mode: %d\n", nextMode);
  return setMode(nextMode);
}