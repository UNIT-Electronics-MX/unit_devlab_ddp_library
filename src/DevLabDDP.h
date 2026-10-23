#ifndef DEVLAB_DDP_MASTER_H
#define DEVLAB_DDP_MASTER_H

#include <Arduino.h>
#include <Wire.h>
#include <DevLab_I2C_Orchestrator.h>
#include "DevLabDDPProtocol.h"

namespace DevLabDDP {

constexpr uint16_t DEVICE_ANY      = 0x0000U;
constexpr uint16_t DEVICE_TEMT6000 = 0x0100U;
constexpr uint16_t DEVICE_TEMT6000_LEGACY = 0x0002U;
constexpr uint16_t DEVICE_JOYSTICK = 0x0101U;
constexpr uint16_t DEVICE_DS18B20  = 0x0102U;
constexpr uint16_t DEVICE_PIR      = 0x0103U;
constexpr uint16_t DEVICE_SLIDER_POT = 0x0104U;
constexpr uint16_t DEVICE_HX0805   = 0x0105U;
constexpr uint16_t DEVICE_GT36537  = 0x0106U;
constexpr uint16_t DEVICE_DPAD     = 0x0107U;
constexpr uint16_t DEVICE_RELAY    = 0x0108U;
constexpr uint16_t DEVICE_HALL     = 0x0109U;
constexpr uint16_t DEVICE_BUTTON   = 0x0110U;
constexpr uint16_t DEVICE_IR       = 0x0111U;
constexpr uint16_t DEVICE_PHOTOINTERRUPTER = 0x0112U; 
constexpr uint16_t DEVICE_LM35     = 0x0113U;
constexpr uint16_t DEVICE_ENCODER    = 0x0114U;
constexpr uint16_t DEVICE_TOUCH    = 0x0115U;
// WS12XX exposes the slider ID; NeoPixel commands are an extension of it.
constexpr uint16_t DEVICE_WS12XX_NEO = DEVICE_SLIDER_POT;

/* D-Pad button masks. Combine with | and test individual buttons with &. */
constexpr uint8_t DPAD_NONE = 0U;
constexpr uint8_t DPAD_S1 = 1U << 0;
constexpr uint8_t DPAD_S2 = 1U << 1;
constexpr uint8_t DPAD_S3 = 1U << 2;
constexpr uint8_t DPAD_S4 = 1U << 3;

constexpr bool isTemt6000DeviceId(uint16_t deviceId) {
  return deviceId == DEVICE_TEMT6000_LEGACY ||
         deviceId == DEVICE_TEMT6000;
}

constexpr bool deviceIdMatches(uint16_t expectedDeviceId,
                               uint16_t actualDeviceId) {
  return expectedDeviceId == DEVICE_ANY ||
         expectedDeviceId == actualDeviceId ||
         (isTemt6000DeviceId(expectedDeviceId) &&
          isTemt6000DeviceId(actualDeviceId));
}

struct DeviceInfo {
  uint16_t deviceId = 0;
  uint8_t firmwareMajor = 0;
  uint8_t firmwareMinor = 0;
  uint8_t hardwareMajor = 0;
  uint8_t hardwareMinor = 0;
  uint8_t protocolMajor = 0;
  uint8_t protocolMinor = 0;
  uint32_t capabilities = 0;
  bool valid = false;
};

class Master {
 public:
  explicit Master(DevLab_I2C_Orchestrator &orchestrator, uint16_t expectedDeviceId)
      : orchestrator_(orchestrator), expectedDeviceId_(expectedDeviceId) {}

  uint16_t expectedDeviceId() const { return expectedDeviceId_; }

  bool ping(uint8_t address) {
    return orchestrator_.ping(address);
  }

  bool writeByte(uint8_t address, uint8_t value) {
    return orchestrator_.writeCommand(address, value);
  }

  bool readCommand(uint8_t address, uint8_t command, uint8_t *data,
                   uint8_t length, uint16_t processingDelayMs = 5U) {
    return orchestrator_.transact(address, command, data, length, processingDelayMs);
  }

  bool identify(uint8_t address, DeviceInfo &info) {
    info = DeviceInfo{};
    uint8_t bytes[4];
    if (!readCommand(address, CMD_GET_PROTOCOL, bytes, 2U)) return false;
    info.protocolMajor = bytes[0];
    info.protocolMinor = bytes[1];
    if (info.protocolMajor != DEVLAB_PROTOCOL_MAJOR) return false;

    if (!readCommand(address, CMD_GET_DEVICE_ID, bytes, 2U)) return false;
    info.deviceId = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
    if (!readCommand(address, CMD_GET_FIRMWARE_VERSION, bytes, 2U)) return false;
    info.firmwareMajor = bytes[0];
    info.firmwareMinor = bytes[1];
    if (!readCommand(address, CMD_GET_HARDWARE_VERSION, bytes, 2U)) return false;
    info.hardwareMajor = bytes[0];
    info.hardwareMinor = bytes[1];
    if (!readCommand(address, CMD_GET_CAPABILITIES, bytes, 4U)) return false;
    info.capabilities = (uint32_t)bytes[0] |
                        ((uint32_t)bytes[1] << 8) |
                        ((uint32_t)bytes[2] << 16) |
                        ((uint32_t)bytes[3] << 24);
    info.valid = true;
    return true;
  }

  bool matchesExpectedDevice(uint8_t address, DeviceInfo *result = nullptr) {
    DeviceInfo local;
    if (!identify(address, local) ||
        !deviceIdMatches(expectedDeviceId_, local.deviceId)) return false;
    if (result != nullptr) *result = local;
    return true;
  }

  bool getI2cAddress(uint8_t address, uint8_t &reportedAddress) {
    if (!matchesExpectedDevice(address)) return false;
    return readCommand(address, CMD_GET_I2C_ADDR, &reportedAddress, 1U);
  }

  bool setI2cAddress(uint8_t oldAddress, uint8_t newAddress) {
    if (newAddress < 0x08U || newAddress > 0x77U ||
        !matchesExpectedDevice(oldAddress) || ping(newAddress)) return false;
    uint8_t response = 0;
    if (!readCommand(oldAddress, CMD_SET_I2C_ADDR, &response, 1U, 10U) ||
        (response & 0x0FU) != RESP_I2C_ADDR_SET) return false;
    
    if (!orchestrator_.transact(oldAddress, newAddress, &response, 1U, 100U) ||
    (response & 0x0FU) != RESP_I2C_ADDR_SET) return false;
        delay(300U);
    return matchesExpectedDevice(newAddress);
  }

  bool reset(uint8_t address) {
    return matchesExpectedDevice(address) && writeByte(address, CMD_RESET);
  }

  bool readAdc(uint8_t address, uint8_t channel, uint16_t &value) {
    uint8_t bytes[2];
    uint8_t command = channel == 0U ? CMD_READ_ADC0 : CMD_READ_ADC1;
    if (channel > 1U || !matchesExpectedDevice(address) ||
        !readCommand(address, command, bytes, 2U, 30U)) return false;
    value = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
    return true;
  }

  /* Hall-effect module: true while a magnet is detected. The device is
   * verified once by the caller (begin), so each poll is a single read. */
  bool readHall(uint8_t address, bool &detected) {
    uint8_t raw;
    if (!readCommand(address, CMD_READ_GPIO0, &raw, 1U, 2U)) return false;
    detected = raw != 0U;
    return true;
  }

  /* Analog Hall module (DRV5055): raw 12-bit ADC sample, 0..4095. */
  bool readHallRaw(uint8_t address, uint16_t &raw) {
    uint8_t bytes[2];
    if (!readCommand(address, CMD_READ_ADC0, bytes, 2U, 2U)) return false;
    raw = ((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8)) & 0x0FFFU;
    return true;
  }


  /* Current D-Pad state: 0..15, or -1 on I2C/invalid-frame errors.
   * Identify the D-Pad once before polling; each call only reads its buttons.
   * This synchronous read includes processingDelayMs and does not print,
   * cache states, detect edges or require releasing buttons first. */
  int16_t readButtons(uint8_t address, uint16_t processingDelayMs = 2U) {
    uint8_t buttons = 0;
    constexpr uint8_t command = 0x80U;  // D-Pad button snapshot.
    if (!readCommand(address, command, &buttons, 1U, processingDelayMs) ||
        (buttons & 0xF0U) != 0U) return -1;
    return buttons;
  }

  bool readGpio0(uint8_t address, bool &value) {
    uint8_t raw;
    if (!matchesExpectedDevice(address) ||
        !readCommand(address, CMD_READ_GPIO0, &raw, 1U)) return false;
    value = raw != 0U;
    return true;
  }

  /* Relay commands (DDP_CAP_RELAY devices). The acknowledgement carries the
   * response code in bits 0-3 and the relay state in bit 4 (1 = on). */
  bool relayOn(uint8_t address, bool *state = nullptr) {
    return relayCommand(address, CMD_RELAY_ON, state);
  }

  bool relayOff(uint8_t address, bool *state = nullptr) {
    return relayCommand(address, CMD_RELAY_OFF, state);
  }

  bool relayToggle(uint8_t address, bool *state = nullptr) {
    return relayCommand(address, CMD_RELAY_TOGGLE, state);
  }

  bool readRelay(uint8_t address, bool &state) {
    return readGpio0(address, state);
  }

 private:
  bool relayCommand(uint8_t address, uint8_t command, bool *state) {
    uint8_t response = 0;
    if (!matchesExpectedDevice(address) ||
        !readCommand(address, command, &response, 1U, 2U)) return false;
    if (state != nullptr) *state = (response & 0x10U) != 0U;
    return true;
  }

  DevLab_I2C_Orchestrator &orchestrator_;
  uint16_t expectedDeviceId_;
};

inline const char *deviceName(uint16_t deviceId) {
  switch (deviceId) {
    case DEVICE_JOYSTICK: return "joystick";
    case DEVICE_TEMT6000_LEGACY:
    case DEVICE_TEMT6000: return "temt6000";
    case DEVICE_DS18B20:  return "ds18b20";
    case DEVICE_PIR:      return "pir";
    case DEVICE_SLIDER_POT: return "slider-pot/ws12xx-neopixel";
    case DEVICE_HX0805:   return "hx0805-vibration";
    case DEVICE_GT36537:  return "gt36537-light";
    case DEVICE_DPAD:     return "d-pad";
    case DEVICE_BUTTON:   return "button";
    case DEVICE_RELAY:    return "relay";
    case DEVICE_HALL:     return "hall-effect";
    case DEVICE_IR:       return "infrared";
    case DEVICE_LM35:     return "lm35-temperature";
    case DEVICE_PHOTOINTERRUPTER: return "photo-interrupter";
    case DEVICE_ENCODER:  return "encoder";
    case DEVICE_TOUCH:    return "touch";
    default:              return "unknown";
  }
}

inline void printDeviceInfo(Print &out, uint8_t address, const DeviceInfo &info,
                            uint16_t expectedDeviceId) {
  out.print("addr=0x"); if (address < 0x10U) out.print('0'); out.print(address, HEX);
  out.print(" id=0x"); if (info.deviceId < 0x1000U) out.print('0');
  if (info.deviceId < 0x0100U) out.print('0');
  if (info.deviceId < 0x0010U) out.print('0');
  out.print(info.deviceId, HEX);
  out.print(" type="); out.print(deviceName(info.deviceId));
  out.print(" protocol="); out.print(info.protocolMajor); out.print('.'); out.print(info.protocolMinor);
  out.print(" firmware="); out.print(info.firmwareMajor); out.print('.'); out.print(info.firmwareMinor);
  out.print(" hardware="); out.print(info.hardwareMajor); out.print('.'); out.print(info.hardwareMinor);
  out.print(" caps=0x"); out.print(info.capabilities, HEX);
  out.print(" match=");
  out.println(deviceIdMatches(expectedDeviceId, info.deviceId) ? "yes" : "NO");
}

}  // namespace DevLabDDP

#endif
