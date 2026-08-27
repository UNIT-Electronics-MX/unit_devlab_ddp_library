# UNIT DevLab DDP Library

Arduino master library for the **DevLab Device Protocol (DDP)**, and the shared,
versioned contract that UNIT DevLab I2C firmware and its master applications
build on.

A DDP master discovers nodes on the I2C bus, validates their protocol version,
device ID and capability bitmap, and only then performs device-specific reads,
address changes or resets.

## Installation

**Arduino IDE** — *Sketch → Include Library → Manage Libraries*, search for
`DevLabDDP`. Or download this repository as a ZIP and use
*Sketch → Include Library → Add .ZIP Library*.

**arduino-cli**

```bash
arduino-cli lib install DevLabDDP
```

**From source**, compiling a sketch against a local checkout:

```bash
arduino-cli compile \
  --fqbn esp32:esp32:esp32s3 \
  --libraries /path/to/parent-of-this-repo \
  path/to/sketch
```

## Layout

```text
src/DevLabDDPProtocol.h      DDP contract: command map, device IDs, capabilities
src/DevLabDDP.h              Master class and DeviceInfo
src/DevLabDDPConsole.h       Serial console front-end for a Master
src/DevLabI2CBusRecovery.h   bus unblock helpers (standalone)
```

`DevLabDDPProtocol.h` is a plain C header shared by both sides of the bus. The
PY32 firmwares include it after defining their `DDP_DEVICE_*` metadata, which
activates the slave-side response builder; the Arduino master includes it
without those macros and gets only the command map. One file, one source of
truth for both.

## Quick start

```cpp
#include <Wire.h>
#include <DevLabDDP.h>

DevLabDDP::Master master(Wire, DevLabDDP::DEVICE_JOYSTICK);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  DevLabDDP::DeviceInfo info;
  if (master.identify(0x28, info)) {
    DevLabDDP::printDeviceInfo(Serial, 0x28, info, master.expectedDeviceId());
  }
}

void loop() {
  uint16_t value;
  if (master.readAdc(0x28, 0, value)) {
    Serial.println(value);
  }
  delay(100);
}
```

Pass `DEVICE_ANY` as the expected device ID to build a universal master that
accepts any registered DDP node.

## API

`DevLabDDP::Master`

| Method | Purpose |
| --- | --- |
| `ping(address)` | bare I2C ACK check |
| `identify(address, info)` | fill a `DeviceInfo` from the device info block |
| `matchesExpectedDevice(address, info = nullptr)` | identify and enforce the expected device ID |
| `readAdc(address, channel, value)` | 16-bit analog read |
| `readGpio0(address, value)` | digital read |
| `getI2cAddress(address, reported)` | ask the node for its own address |
| `setI2cAddress(oldAddress, newAddress)` | persist a new address, then re-verify |
| `reset(address)` | soft reset |

Address changes and resets are rejected when the node does not match the
master's expected device ID.

Known device IDs are exposed as `DEVICE_ANY`, `DEVICE_JOYSTICK`,
`DEVICE_TEMT6000`, `DEVICE_DS18B20`, `DEVICE_PIR` and `DEVICE_WS12XX_NEO`.
`deviceName(id)` maps them to printable names.

## Serial console

`DevLabDDPConsole.h` wraps a `Master` in an interactive console. Construct it
with a `Master`, a `Stream` and a read handler, call `begin()` in `setup()` and
`poll()` in `loop()`.

Commands: `scan`, `info`, `read`, `addr`, `setaddr`, `reset`, `help`.
I2C addresses are entered in hexadecimal.

## Using it as a dependency

Libraries built on top of DDP declare it in their own `library.properties`:

```properties
depends=DevLabDDP (>=1.0.0)
```

## License

See [LICENSE](LICENSE).
