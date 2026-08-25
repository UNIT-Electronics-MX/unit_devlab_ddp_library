# UNIT DevLab DDP Library

Shared, versioned dependency for UNIT DevLab I2C firmware and compatible
master applications.

```text
protocol/include/             C-compatible DDP contract for PY32 firmware
libraries/arduino/DevLabDDP/  self-contained Arduino master library
```

Firmware projects include `protocol/include/devlab_protocol.h`. Arduino
projects pass `--libraries libraries/arduino` to `arduino-cli` or install the
`DevLabDDP` directory through Arduino IDE.

Run `make check` before releasing a version; it verifies that the Arduino
library embeds exactly the same DDP command map as the firmware contract.
