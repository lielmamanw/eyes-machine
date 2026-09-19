#include "eyes-machine.hpp"

EyesMachine::EyesMachine(const MachineMode mode) : _mode(mode), _actionsController()
{}

void EyesMachine::begin()
{
  _actionsController.begin();
}

void EyesMachine::runInLoop()
{
  _actionsController.setOpenness(0);
  delay(1000);
  _actionsController.setOpenness(1);
  delay(1000);
}

EyesMachine::MachineMode EyesMachine::setMode(const MachineMode mode)
{
  constexpr uint8_t RGB_PIN = 48;
  
  _mode = (mode == MachineMode::UNREACHABLE_MODE) ? MachineMode::OFF : mode;
  const RGBColor modeColor = EyesMachineModeColors[static_cast<uint8_t>(_mode)];
  rgbLedWrite(RGB_PIN, modeColor.r, modeColor.g, modeColor.b);
  
  return _mode;
}

EyesMachine::MachineMode EyesMachine::setNextMode()
{
  MachineMode currentMode = _mode;
  int currentModeAsNumber = static_cast<int>(currentMode);
  MachineMode nextMode = static_cast<MachineMode>(currentModeAsNumber + 1);
  return setMode(nextMode);
}