#ifndef BLE_SENSOR_SERVICE_H__
#define BLE_SENSOR_SERVICE_H__

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
#define BLE_SENSOR_SERVICE_DEF(_name)                                          \
static ble_sensor_service_t _name;                                                       \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                      \
                     2,                                                                  \
                     ble_sensor_service_on_ble_evt, &_name)

#define SENSOR_SERRVICE_UUID_BASE        {0xF9, 0xF8, 0xA9, 0x3D, 0x6C, 0x8D, 0x4A, 0x4A, \
                                          0x84, 0x32, 0x79, 0x26, 0x00, 0x00, 0x30, 0x2B} 
#define SENSOR_SERVICE_UUID               0xBF34
#define SETTINGS_CHARACTERISTIC_UUID      0xBF35
#define ACC_DATA_CHARACTERISTIC_UUID      0xBF36
#define GYR_DATA_CHARACTERISTIC_UUID      0xBF36
#define MAG_DATA_CHARACTERISTIC_UUID      0xBF36

#define SENSOR_SAMPLES 10                    /**< The number of full sensor samples to read into data characteristics */
#define SAMPLE_SIZE     6                    /**< The size (in bytes) of a full sensor sample reading (includes x, y and z axes) */

// Forward declaration of the ble_sensor_service_t type.
typedef struct ble_sensor_service_s ble_sensor_service_t;

typedef void (*ble_sensor_service_setting_write_handler_t) (uint16_t conn_handle, ble_sensor_service_t * p_ss, uint16_t new_state);

/** @brief Sensor Service init structure. This structure contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_sensor_service_setting_write_handler_t setting_write_handler; /**< Event handler to be called when the Settings Characteristic is written. */
} ble_sensor_service_init_t;

/**@brief Sensor Service structure. This structure contains various status information for the service. */
struct ble_sensor_service_s
{
    uint16_t                    service_handle;                         /**< Handle of Sensor Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t    data_handles[3];                           /**< Handles related to the Data Characteristic. */
    ble_gatts_char_handles_t    settings_handles;                       /**< Handles related to the Settings Characteristic. */
    uint8_t                     uuid_type;                              /**< UUID type for the Sensor Service. */
    ble_sensor_service_setting_write_handler_t setting_write_handler;   /**< Event handler to be called when the Settings Characteristic is written. */
};


/**@brief Function for initializing the Sensor Service.
 *
 * @param[out] p_ss       Sensor Service structure. This structure must be supplied by
 *                        the application. It is initialized by this function and will later
 *                        be used to identify this particular service instance.
 * @param[in] p_ss_init   Information needed to initialize the service.
 *
 * @retval NRF_SUCCESS If the service was initialized successfully. Otherwise, an error code is returned.
 */
uint32_t ble_sensor_service_init(ble_sensor_service_t * p_ss, const ble_sensor_service_init_t * p_ss_init);

/**@brief Function for adding the Data Characteristics.
 *
 * @param[out] p_ss       Sensor Service structure. This structure must be supplied by
 *                        the application. It is initialized by this function and will later
 *                        be used to identify this particular service instance.
 *
 * @retval NRF_SUCCESS If the characteristics were initialized successfully. Otherwise, an error code is returned.
 */
uint32_t ble_sensor_service_data_char_add(ble_sensor_service_t * p_ss);

/**@brief Function for adding the Settings Characteristic.
 *
 * @param[out] p_ss       Sensor Service structure. This structure must be supplied by
 *                        the application. It is initialized by this function and will later
 *                        be used to identify this particular service instance.
 *
 * @retval NRF_SUCCESS If the characteristics were initialized successfully. Otherwise, an error code is returned.
 */
uint32_t ble_sensor_service_settings_char_add(ble_sensor_service_t * p_ss);


/**@brief Function for handling the application's BLE stack events.
 *
 * @details This function handles all events from the BLE stack that are of interest to the Sensor Service.
 *
 * @param[in] p_ble_evt  Event received from the BLE stack.
 * @param[in] p_context  Sensor Service structure.
 */
void ble_sensor_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);


#ifdef __cplusplus
}
#endif

#endif // BLE_SENSOR_SERVICE_H__

/** @} */