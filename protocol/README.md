# Firmware protocol

This directory owns the C-compatible DDP contract consumed by the PY32
firmwares. Include `include/devlab_protocol.h` from a module's firmware
configuration after defining its `DDP_DEVICE_*` metadata.

The Arduino master library lives separately in
[`../libraries/arduino/DevLabDDP`](../libraries/arduino/DevLabDDP). It is
self-contained for Arduino IDE installation and is checked against this
header by `protocol/check_protocol.py`.
