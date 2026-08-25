#pragma once

#include <Arduino.h>
#include <Wire.h>

inline bool devlabWaitForI2cLineHigh(uint8_t pin, uint32_t timeoutUs)
{
  const uint32_t started = micros();
  while (!digitalRead(pin)) {
    if ((uint32_t)(micros() - started) >= timeoutUs) return false;
    delayMicroseconds(10);
  }
  return true;
}

inline bool devlabClearI2cBus(uint8_t sdaPin, uint8_t sclPin)
{
  pinMode(sdaPin, INPUT_PULLUP);
  pinMode(sclPin, INPUT_PULLUP);
  delay(2);
  if (!devlabWaitForI2cLineHigh(sclPin, 20000)) return false;

  for (uint8_t pulse = 0; pulse < 18 && !digitalRead(sdaPin); ++pulse) {
    digitalWrite(sclPin, LOW);
    pinMode(sclPin, OUTPUT);
    delayMicroseconds(5);
    pinMode(sclPin, INPUT_PULLUP);
    if (!devlabWaitForI2cLineHigh(sclPin, 2000)) return false;
    delayMicroseconds(5);
  }

  digitalWrite(sclPin, LOW);
  pinMode(sclPin, OUTPUT);
  digitalWrite(sdaPin, LOW);
  pinMode(sdaPin, OUTPUT);
  delayMicroseconds(5);
  pinMode(sclPin, INPUT_PULLUP);
  if (!devlabWaitForI2cLineHigh(sclPin, 2000)) {
    pinMode(sdaPin, INPUT_PULLUP);
    return false;
  }
  delayMicroseconds(5);
  pinMode(sdaPin, INPUT_PULLUP);
  delayMicroseconds(5);
  return digitalRead(sdaPin) && digitalRead(sclPin);
}

inline bool devlabBeginI2cBusRecovered(
    TwoWire &bus, uint8_t sdaPin, uint8_t sclPin,
    uint32_t frequency, uint32_t timeoutMs, bool restart = false)
{
  if (restart) {
    bus.end();
    delay(2);
  }
  if (!devlabClearI2cBus(sdaPin, sclPin)) return false;

#if defined(ARDUINO_ARCH_RP2040)
  bus.setSDA(sdaPin);
  bus.setSCL(sclPin);
  bus.begin();
#elif defined(ARDUINO_ARCH_ESP32)
  bus.begin(sdaPin, sclPin);
#else
#error "Unsupported architecture. Use RP2040/RP2350 or ESP32."
#endif

  bus.setClock(frequency);
  bus.setTimeout(timeoutMs);
  return true;
}
