#ifndef DEVLAB_DDP_CONSOLE_H
#define DEVLAB_DDP_CONSOLE_H

#include "DevLabDDP.h"

namespace DevLabDDP {

using ReadDeviceHandler = bool (*)(Master &, uint8_t, Print &);

class Console {
 public:
  Console(Master &master, Stream &serial, ReadDeviceHandler readDevice)
      : master_(master), serial_(serial), readDevice_(readDevice) {}

  void begin() {
    serial_.println("DevLab DDP master 1.0");
    if (master_.expectedDeviceId() == DEVICE_ANY) {
      serial_.println("Expected device ID: any registered DDP device");
    } else {
      serial_.print("Expected device ID: 0x");
      serial_.println(master_.expectedDeviceId(), HEX);
    }
    printHelp();
  }

  void poll() {
    while (serial_.available()) {
      char c = (char)serial_.read();
      if (c == '\r' || c == '\n') {
        if (line_.length() != 0U) {
          process(line_);
          line_ = "";
        }
      } else if (line_.length() < 80U) {
        line_ += c;
      } else {
        line_ = "";
      }
    }
  }

  void scan() {
    uint8_t found = 0;
    for (uint8_t address = 0x08U; address <= 0x77U; ++address) {
      if (!master_.ping(address)) continue;
      ++found;
      DeviceInfo info;
      if (master_.identify(address, info)) {
        printDeviceInfo(serial_, address, info, master_.expectedDeviceId());
      } else {
        serial_.print("addr=0x"); printHexByte(address);
        serial_.println(" protocol=legacy/non-DDP");
      }
    }
    serial_.print("found="); serial_.println(found);
  }

 private:
  static bool parseAddress(const String &text, uint8_t &address) {
    char *end = nullptr;
    long parsed = strtol(text.c_str(), &end, 16);
    if (end == text.c_str() || *end != '\0' || parsed < 0x08L || parsed > 0x77L) return false;
    address = (uint8_t)parsed;
    return true;
  }

  static uint8_t split(String line, String words[], uint8_t maximum) {
    line.trim();
    uint8_t count = 0;
    int start = 0;
    while (start < (int)line.length() && count < maximum) {
      while (start < (int)line.length() && line[start] == ' ') ++start;
      if (start >= (int)line.length()) break;
      int end = line.indexOf(' ', start);
      if (end < 0) end = line.length();
      words[count++] = line.substring(start, end);
      start = end + 1;
    }
    return count;
  }

  void printHexByte(uint8_t value) {
    if (value < 0x10U) serial_.print('0');
    serial_.print(value, HEX);
  }

  void printHelp() {
    serial_.println("Commands:");
    serial_.println("  scan");
    serial_.println("  info <addr_hex>");
    serial_.println("  read <addr_hex>");
    serial_.println("  addr <addr_hex>");
    serial_.println("  setaddr <old_hex> <new_hex>");
    serial_.println("  reset <addr_hex>");
    serial_.println("  help");
  }

  void process(String line) {
    line.trim();
    line.toLowerCase();
    String words[3];
    uint8_t count = split(line, words, 3U);
    if (count == 0U) return;
    if (words[0] == "help" || words[0] == "?") { printHelp(); return; }
    if (words[0] == "scan") { scan(); return; }

    uint8_t address;
    if (count < 2U || !parseAddress(words[1], address)) {
      serial_.println("ERROR invalid address");
      return;
    }
    if (words[0] == "info") {
      DeviceInfo info;
      if (!master_.identify(address, info)) serial_.println("ERROR not a DDP device");
      else printDeviceInfo(serial_, address, info, master_.expectedDeviceId());
      return;
    }
    if (words[0] == "read") {
      if (readDevice_ == nullptr || !readDevice_(master_, address, serial_))
        serial_.println("ERROR identity mismatch or read failed");
      return;
    }
    if (words[0] == "addr") {
      uint8_t reported;
      if (!master_.getI2cAddress(address, reported)) {
        serial_.println("ERROR identity mismatch or read failed");
      } else {
        serial_.print("address=0x"); printHexByte(reported); serial_.println();
      }
      return;
    }
    if (words[0] == "reset") {
      serial_.println(master_.reset(address) ? "OK" : "ERROR identity mismatch or reset failed");
      return;
    }
    if (words[0] == "setaddr") {
      uint8_t newAddress;
      if (count < 3U || !parseAddress(words[2], newAddress)) {
        serial_.println("ERROR invalid new address");
      } else {
        serial_.println(master_.setI2cAddress(address, newAddress)
                            ? "OK" : "ERROR identity mismatch, collision or write failed");
      }
      return;
    }
    serial_.println("ERROR unknown command");
  }

  Master &master_;
  Stream &serial_;
  ReadDeviceHandler readDevice_;
  String line_;
};

}  // namespace DevLabDDP

#endif
