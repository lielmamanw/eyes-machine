#include "controller-state.hpp"

ControllerState::ControllerState(EyesActionsController& actions) : _actions(actions)
{}

void ControllerState::reset()
{}

void ControllerState::enter()
{}

bool ControllerState::isEnterComplete() const
{
  return true;
}

void ControllerState::execute()
{}

void ControllerState::exit()
{}

bool ControllerState::isExitComplete() const
{
  return true;
}
