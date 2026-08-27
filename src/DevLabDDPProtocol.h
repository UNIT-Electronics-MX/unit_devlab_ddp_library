#ifndef DEVLAB_PROTOCOL_H
#define DEVLAB_PROTOCOL_H

#include <stdint.h>

/* DevLab Device Protocol (DDP) version. */
#define DEVLAB_PROTOCOL_MAJOR 1U
#define DEVLAB_PROTOCOL_MINOR 0U

/* Command blocks. */
#define DDP_BLOCK_DEVICE_INFO  0x00U
#define DDP_BLOCK_CONFIGURATION 0x20U
#define DDP_BLOCK_DIGITAL_IO   0x40U
#define DDP_BLOCK_ANALOG       0x60U
#define DDP_BLOCK_SENSOR_DATA  0x80U
#define DDP_BLOCK_ACTUATORS    0xA0U
#define DDP_BLOCK_CALIBRATION  0xC0U
#define DDP_BLOCK_RESERVED     0xE0U
#define DDP_BLOCK_SYSTEM       0xF0U

/* 0x00-0x1F: information, identical on every DDP device. */
#define CMD_GET_DEVICE_ID        0x00U /* uint16 little-endian */
#define CMD_GET_FIRMWARE_VERSION 0x01U /* [major, minor] */
#define CMD_GET_HARDWARE_VERSION 0x02U /* [major, minor] */
#define CMD_GET_CAPABILITIES     0x03U /* uint32 little-endian */
#define CMD_GET_PROTOCOL         0x04U /* [major, minor] */

/* 0x20-0x3F: persistent and bus configuration. */
#define CMD_GET_I2C_ADDR          0x20U /* current 7-bit address */
#define CMD_SET_I2C_ADDR          0x21U /* next write is new address */
#define CMD_SAVE_CONFIG           0x22U
#define CMD_RESET_FACTORY         0x23U
#define CMD_GET_I2C_STATUS        0x24U /* 0=default/UID, 1=Flash */

/* 0x40-0x5F: logical digital I/O. Pin mapping is device-specific. */
#define CMD_READ_GPIO0            0x40U
#define CMD_READ_GPIO1            0x41U
#define CMD_WRITE_GPIO0           0x42U /* next write is 0 or 1 */
#define CMD_WRITE_GPIO1           0x43U /* next write is 0 or 1 */

/* 0x60-0x7F: generic analog channels. Reads return uint16 little-endian. */
#define CMD_READ_ADC0             0x60U
#define CMD_READ_ADC1             0x61U
#define CMD_SET_ADC_AVERAGING     0x62U /* next write is sample count */
#define CMD_GET_ADC_AVERAGING     0x63U

/* 0xA0-0xBF: actuators. */
#define CMD_RELAY_OFF             0xA0U
#define CMD_RELAY_ON              0xA1U
#define CMD_RELAY_TOGGLE          0xA2U
#define CMD_SET_TOGGLE_TIME       0xA3U /* next write is units of 25 ms */
#define CMD_GET_TOGGLE_TIME       0xA4U

/* WS12XX/NeoPixel actuator extension (device ID 0x0400). */
#define CMD_NEO_SET_PIXEL         0xB0U /* next: index, R, G, B */
#define CMD_NEO_FILL              0xB1U /* next: R, G, B */
#define CMD_NEO_CLEAR             0xB2U
#define CMD_NEO_SHOW              0xB3U
#define CMD_NEO_SET_HUE           0xB4U /* next: index, hue(0..191), bright(0..2) */
#define CMD_NEO_BRIGHTNESS        0xB5U /* next: global brightness 0..255 */
#define CMD_NEO_CCT_MODE          0xB6U /* next: 0 warm, 1 cool */
#define CMD_NEO_GET_INTENSITY     0xB7U /* response: ADC mapped to 0..255 */
#define CMD_NEO_RGB_COLOR         0xB8U /* next: R, G, B; ADC controls brightness */

/* 0xF0-0xFF: system and diagnostics. */
#define CMD_RESET                 0xF0U
#define CMD_WATCHDOG_RESET        0xF1U
#define CMD_GET_RESET_INFO        0xF2U
#define CMD_DISABLE_NRST          0xF3U
#define CMD_CHECK_NRST            0xF4U

/* Capability bitmap returned by CMD_GET_CAPABILITIES. */
#define DDP_CAP_I2C_CONFIG       (1UL << 0)
#define DDP_CAP_DIGITAL_INPUT    (1UL << 1)
#define DDP_CAP_DIGITAL_OUTPUT   (1UL << 2)
#define DDP_CAP_ANALOG_INPUT     (1UL << 3)
#define DDP_CAP_SENSOR_DATA      (1UL << 4)
#define DDP_CAP_RELAY            (1UL << 5)
#define DDP_CAP_CALIBRATION      (1UL << 6)
#define DDP_CAP_WATCHDOG         (1UL << 7)
#define DDP_CAP_PERSISTENT_CONFIG (1UL << 8)

/* Status bytes for new DDP operations. Unchanged legacy commands retain their
 * historical response byte during the v1 migration. Data reads return data. */
#define DDP_STATUS_OK             0x00U
#define DDP_STATUS_UNKNOWN_CMD    0x01U
#define DDP_STATUS_INVALID_VALUE  0x02U
#define DDP_STATUS_BUSY           0x03U
#define DDP_STATUS_NOT_SUPPORTED  0x04U
#define DDP_STATUS_IO_ERROR       0x05U

/* Legacy common wire values accepted during the DDP v1 transition. */
#define CMD_LEGACY_PA4_DIGITAL      0x07U
#define CMD_LEGACY_PB0_DIGITAL      0x06U
#define CMD_LEGACY_PA0_DIGITAL      0x09U
#define CMD_LEGACY_DISABLE_NRST     0x30U
#define CMD_LEGACY_CHECK_NRST       0x31U
#define CMD_LEGACY_INIT_PB0         0x32U
#define CMD_LEGACY_GET_RESET_INFO   0x33U
#define CMD_LEGACY_SET_I2C_ADDR     0x3DU
#define CMD_LEGACY_RESET_FACTORY    0x3EU
#define CMD_LEGACY_GET_I2C_STATUS   0x3FU
#define CMD_LEGACY_RELAY_TOGGLE     0xA6U
#define CMD_LEGACY_SET_TOGGLE_TIME  0xA7U
#define CMD_LEGACY_GET_TOGGLE_TIME  0xA8U
#define CMD_LEGACY_SET_ADC_AVERAGING 0xDCU
#define CMD_LEGACY_GET_ADC_AVERAGING 0xDDU
#define CMD_LEGACY_RESET            0xFEU
#define CMD_LEGACY_WATCHDOG_RESET   0xFFU

/* Temporary Flash-test commands, outside DDP. */
#define CMD_LEGACY_SAVE_DATA 0x3AU
#define CMD_LEGACY_READ_DATA 0x3BU
#define RESPONSE_DATA   0x3CU

/* Legacy response values retained until the response format is versioned. */
#define RESP_RELAY_OFF        0x00U
#define RESP_RELAY_ON         0x01U
#define RESP_RELAY_TOGGLE     0x06U
#define RESP_TOGGLE_TIME_SET  0x07U
#define RESP_I2C_ERROR        0x08U
#define RESP_PA4_DIGITAL      0x09U
#define RESP_WATCHDOG_OK      0x0AU
#define RESP_CMD_UNKNOWN      0x0BU
#define RESP_ADC_AVERAGING_SET 0x0CU
#define RESP_I2C_ADDR_SET     0x0DU
#define RESP_FACTORY_RESET    0x0EU
#define RESP_I2C_FROM_FLASH   0x0FU
#define RESP_I2C_FROM_UID     0x0AU
#define RESP_NRST_DISABLED    0x10U
#define RESP_NRST_GPIO        0x11U
#define RESP_NRST_ACTIVE      0x12U
#define RESP_PB0_DIGITAL      0x13U
#define RESP_PB0_INITIALIZED  0x14U
#define RESP_RESET_OK         0x0FU

#define RESET_CAUSE_POWER_ON  0x00U
#define RESET_CAUSE_PIN       0x01U
#define RESET_CAUSE_SOFTWARE  0x02U
#define RESET_CAUSE_IWDG      0x03U
#define RESET_CAUSE_WWDG      0x04U
#define RESET_CAUSE_OBL       0x05U
#define RESET_CAUSE_UNKNOWN   0xFFU

#if defined(DDP_DEVICE_ID) || defined(DDP_FIRMWARE_MAJOR) || \
    defined(DDP_FIRMWARE_MINOR) || defined(DDP_HARDWARE_MAJOR) || \
    defined(DDP_HARDWARE_MINOR) || defined(DDP_DEVICE_CAPABILITIES)
#ifndef DDP_DEVICE_ID
#error "Define DDP_DEVICE_ID before including devlab_protocol.h"
#endif
#ifndef DDP_FIRMWARE_MAJOR
#error "Define DDP_FIRMWARE_MAJOR before including devlab_protocol.h"
#endif
#ifndef DDP_FIRMWARE_MINOR
#error "Define DDP_FIRMWARE_MINOR before including devlab_protocol.h"
#endif
#ifndef DDP_HARDWARE_MAJOR
#error "Define DDP_HARDWARE_MAJOR before including devlab_protocol.h"
#endif
#ifndef DDP_HARDWARE_MINOR
#error "Define DDP_HARDWARE_MINOR before including devlab_protocol.h"
#endif
#ifndef DDP_DEVICE_CAPABILITIES
#error "Define DDP_DEVICE_CAPABILITIES before including devlab_protocol.h"
#endif

static inline uint8_t DDP_BuildDeviceInfoResponse(uint8_t command,
                                                  uint8_t response[4])
{
  switch (command) {
    case CMD_GET_DEVICE_ID:
      response[0] = (uint8_t)(DDP_DEVICE_ID & 0xFFU);
      response[1] = (uint8_t)((DDP_DEVICE_ID >> 8) & 0xFFU);
      return 2U;
    case CMD_GET_FIRMWARE_VERSION:
      response[0] = DDP_FIRMWARE_MAJOR;
      response[1] = DDP_FIRMWARE_MINOR;
      return 2U;
    case CMD_GET_HARDWARE_VERSION:
      response[0] = DDP_HARDWARE_MAJOR;
      response[1] = DDP_HARDWARE_MINOR;
      return 2U;
    case CMD_GET_CAPABILITIES:
      response[0] = (uint8_t)(DDP_DEVICE_CAPABILITIES & 0xFFUL);
      response[1] = (uint8_t)((DDP_DEVICE_CAPABILITIES >> 8) & 0xFFUL);
      response[2] = (uint8_t)((DDP_DEVICE_CAPABILITIES >> 16) & 0xFFUL);
      response[3] = (uint8_t)((DDP_DEVICE_CAPABILITIES >> 24) & 0xFFUL);
      return 4U;
    case CMD_GET_PROTOCOL:
      response[0] = DEVLAB_PROTOCOL_MAJOR;
      response[1] = DEVLAB_PROTOCOL_MINOR;
      return 2U;
    default:
      return 0U;
  }
}
#endif /* device-side metadata and response builder */

#endif
