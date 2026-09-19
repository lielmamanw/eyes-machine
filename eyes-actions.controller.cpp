#include "eyes-actions.controller.hpp"

EyesActionsController::EyesActionsController()
{}

void EyesActionsController::setXaxis(int8_t val)
{
  Serial.printf("setXaxis");
}

void EyesActionsController::setYaxis(int8_t val)
{
  Serial.printf("setYaxis");
}

void EyesActionsController::setEyelids(int8_t val)
{
  Serial.printf("setZaxis");
}