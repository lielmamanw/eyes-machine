#include "auto-state.hpp"

AutoState::AutoState(EyesActionsController& actions) : _actions(actions)
{}

void AutoState::reset()
{}

void AutoState::enter()
{}

bool AutoState::isEnterComplete() const
{
  return true;
}

void AutoState::execute()
{}

void AutoState::exit()
{}

bool AutoState::isExitComplete() const
{
  return true;
}
