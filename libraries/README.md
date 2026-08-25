# Compatible libraries

Libraries for host/master platforms are kept separately from firmware-facing
DDP headers. The Arduino implementation is
[`arduino/DevLabDDP`](arduino/DevLabDDP), which can be copied directly into an
Arduino libraries directory.

Its protocol map is verified against
[`../protocol/include/devlab_protocol.h`](../protocol/include/devlab_protocol.h).
