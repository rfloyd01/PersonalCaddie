#ifndef BLE_PC_SERVICE_H__
#define BLE_PC_SERVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief   Macro for defining a ble_sensor_service instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_PC_SERVICE_DEF(_name)                                          \
static ble_pc_service_t _name;                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                        \
                     2,                                                    \
                     ble_pc_service_on_ble_evt, &_name)

#define PC_SERRVICE_UUID_BASE        {0xF9, 0xF8, 0xA9, 0x3D, 0x6C, 0x8D, 0x4A, 0x4A, \
                                          0x84, 0x32, 0x79, 0x26, 0x00, 0x00, 0x30, 0x2B} 
#define PC_SERVICE_UUID                0xBF40
#define ERROR_CHARACTERISTIC_UUID      0xBF41

// Forward declaration of the ble_pc_service_t type.
typedef struct ble_pc_service_s ble_pc_service_t;

/**@brief Sensor Service structure. This structure contains various status information for the service. */
struct ble_pc_service_s
{
    uint16_t                    service_handle;                         /**< Handle of Sensor Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t    error_handle;                           /**< Handle related to the Error Characteristic. */
    uint8_t                     uuid_type;                              /**< UUID type for the Sensor Service. */
};


/**@brief Function for initializing the Sensor Service.
 *
 * @param[out] p_pc       Personal Caddie Service structure. This structure must be supplied by
 *                        the application. It is initialized by this function and will later
 *                        be used to identify this particular service instance.
 *
 * @retval NRF_SUCCESS If the service was initialized successfully. Otherwise, an error code is returned.
 */
uint32_t ble_pc_service_init(ble_pc_service_t * p_pc);

/**@brief Function for adding the Error Characteristic.
 *
 * @param[out] p_pc       Personal Caddie structure. This structure must be supplied by
 *                        the application. It is initialized by this function and will later
 *                        be used to identify this particular service instance.
 *
 * @retval NRF_SUCCESS If the characteristics were initialized successfully. Otherwise, an error code is returned.
 */
uint32_t ble_pc_service_error_char_add(ble_pc_service_t * p_pc);

/**@brief Function for handling the application's BLE stack events.
 *
 * @details This function handles all events from the BLE stack that are of interest to the Personal Caddie Service.
 *
 * @param[in] p_ble_evt  Event received from the BLE stack.
 * @param[in] p_context  Sensor Service structure.
 */
void ble_pc_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);


#ifdef __cplusplus
}
#endif

#endif // BLE_PC_SERVICE_H__

/** @} */