# DevLabDDP

Biblioteca Arduino autocontenida para maestros del DevLab Device Protocol
(DDP) sobre I2C.

## Instalación

Copie la carpeta `DevLabDDP` completa dentro de `Arduino/libraries` o
comprímala como ZIP e instálela desde Arduino IDE. No es necesario copiar
ningún archivo del directorio padre `protocol`.

```cpp
#include <Wire.h>
#include <DevLabDDP.h>

DevLabDDP::Master device(Wire, DevLabDDP::DEVICE_TEMT6000);
```

`DevLabDDPProtocol.h` contiene el mapa DDP 1.0 incluido en la propia
biblioteca. `DevLabI2CBusRecovery.h` ofrece recuperación opcional del bus
para ESP32 y RP2040/RP2350.

## Dispositivos registrados

| Constante | Device ID |
|---|---:|
| `DEVICE_JOYSTICK` | `0x0101` |
| `DEVICE_TEMT6000` | `0x0102` |
| `DEVICE_DS18B20` | `0x0103` |
| `DEVICE_PIR` | `0x0104` |
| `DEVICE_WS12XX_NEO` | `0x0400` |

Licencia y condiciones de publicación: consulte el repositorio del proyecto.
