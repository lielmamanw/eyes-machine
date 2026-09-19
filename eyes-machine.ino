#include "eyes-machine.hpp"

#define BUTTON_PIN 0
#define DATA_COMMUNICATION_CHANNEL 9600

EyesMachine machine;

void setup() {
  machine.begin();
  machine.setMode(EyesMachine::MachineMode::OFF);
  Serial.begin(DATA_COMMUNICATION_CHANNEL);

  // Configure Button Pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  changeMachineModeButtonEvent();
  machine.runInLoop();
}

void changeMachineModeButtonEvent()
{
  constexpr uint8_t PRESSED_BUTTON_DELAY_MS = 250;
  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) {
    machine.setNextMode();
    delay(PRESSED_BUTTON_DELAY_MS);
  }
}