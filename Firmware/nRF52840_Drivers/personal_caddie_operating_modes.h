#ifndef PERSONAL_CADDIE_OPERATING_MODES_H__
#define PERSONAL_CADDIE_OPERATING_MODES_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum
  {
      ADVERTISING_MODE   = 0x00,
      CONNECTED_MODE     = 0x01,
      SENSOR_IDLE_MODE   = 0x02,
      SENSOR_ACTIVE_MODE = 0x03,
  } personal_caddie_operating_mode_t;

#ifdef __cplusplus
}
#endif

#endif // PERSONAL_CADDIE_OPERATING_MODES_H__

/** @} */