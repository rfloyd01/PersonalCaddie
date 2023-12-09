#include <stdint.h>
#include <string.h>
#include "ble_sensor_service.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "sdk_macros.h"

/**@brief Function for handling the Write event.
 *
 * @param[in] p_ss       Sensor Service structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
static void on_write(ble_sensor_service_t * p_ss, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (   (p_evt_write->handle == p_ss->settings_handles.value_handle)
        && (p_ss->setting_write_handler != NULL))
    {
        p_ss->setting_write_handler(p_ble_evt->evt.gap_evt.conn_handle, p_ss, p_evt_write->data);
    }
}

void ble_sensor_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_sensor_service_t * p_ss = (ble_sensor_service_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTS_EVT_WRITE:
            on_write(p_ss, p_ble_evt);
            break;

        default:
            // Consider adding a read handler here
            break;
    }
}

uint32_t ble_sensor_service_init(ble_sensor_service_t * p_ss, const ble_sensor_service_init_t * p_ss_init, const uint8_t settings_length)
{
    uint32_t              err_code;
    ble_uuid_t            ble_uuid;

    // Initialize service structure.
    p_ss->setting_write_handler = p_ss_init->setting_write_handler;

    // Add service.
    ble_uuid128_t base_uuid = {SENSOR_SERRVICE_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_ss->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_ss->uuid_type;
    ble_uuid.uuid = SENSOR_SERVICE_UUID;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_ss->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add Data characteristics
    err_code = ble_sensor_service_data_char_add(p_ss);
    VERIFY_SUCCESS(err_code);

    // Add Settings characteristic.
    err_code = ble_sensor_service_settings_char_add(p_ss, settings_length);
    VERIFY_SUCCESS(err_code);

    // Add Available Sesnors characteristic
    return ble_sensor_service_available_sensors_char_add(p_ss);
}

uint32_t ble_sensor_service_data_char_add(ble_sensor_service_t * p_ss)
{
    ble_add_char_params_t add_small_char_params, add_med_char_params, add_large_char_params;

    //There are three different characteristics for holding sensor data, a small, medium and 
    //large characteristic. The reason for this is that the number of samples we send to the 
    //front end during each connection interval will vary with the sensor ODR. If a single
    //data characteristic is used that's sized for the largest possible case, then there will 
    //be a lot of unneccessary bits that get sent when the ODR is reduced. The longer the
    //antenna is on, the more current draw there is which we want to minimize.

    // Add Small Data characteristic.
    memset(&add_small_char_params, 0, sizeof(add_small_char_params));
    add_small_char_params.uuid              = SMALL_DATA_CHARACTERISTIC_UUID;
    add_small_char_params.uuid_type         = p_ss->uuid_type;
    add_small_char_params.init_len          = SMALL_DATA_CHARACTERISTIC_SIZE * sizeof(uint8_t); 
    add_small_char_params.max_len           = SMALL_DATA_CHARACTERISTIC_SIZE * sizeof(uint8_t);
    add_small_char_params.char_props.read   = 1;
    add_small_char_params.char_props.notify = 1;

    add_small_char_params.read_access       = SEC_OPEN;
    add_small_char_params.cccd_write_access = SEC_OPEN;

    uint32_t err_code = characteristic_add(p_ss->service_handle,
                                  &add_small_char_params,
                                  &p_ss->data_handles[0]);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Medium Data characteristic.
    memset(&add_med_char_params, 0, sizeof(add_med_char_params));
    add_med_char_params.uuid              = MEDIUM_DATA_CHARACTERISTIC_UUID;
    add_med_char_params.uuid_type         = p_ss->uuid_type;
    add_med_char_params.init_len          = MEDIUM_DATA_CHARACTERISTIC_SIZE * sizeof(uint8_t);
    add_med_char_params.max_len           = MEDIUM_DATA_CHARACTERISTIC_SIZE * sizeof(uint8_t);
    add_med_char_params.char_props.read   = 1;
    add_med_char_params.char_props.notify = 1;

    add_med_char_params.read_access       = SEC_OPEN;
    add_med_char_params.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_ss->service_handle,
                                  &add_med_char_params,
                                  &p_ss->data_handles[1]);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Large Data characteristic.
    memset(&add_large_char_params, 0, sizeof(add_large_char_params));
    add_large_char_params.uuid              = LARGE_DATA_CHARACTERISTIC_UUID;
    add_large_char_params.uuid_type         = p_ss->uuid_type;
    add_large_char_params.init_len          = LARGE_DATA_CHARACTERISTIC_SIZE * sizeof(uint8_t);
    add_large_char_params.max_len           = LARGE_DATA_CHARACTERISTIC_SIZE * sizeof(uint8_t);
    add_large_char_params.char_props.read   = 1;
    add_large_char_params.char_props.notify = 1;

    add_large_char_params.read_access       = SEC_OPEN;
    add_large_char_params.cccd_write_access = SEC_OPEN;

    return characteristic_add(p_ss->service_handle,
                                  &add_large_char_params,
                                  &p_ss->data_handles[2]);
}

uint32_t ble_sensor_service_settings_char_add(ble_sensor_service_t * p_ss, const uint8_t settings_length)
{
    ble_add_char_params_t add_char_params;

    // Add Settings characteristic.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = SETTINGS_CHARACTERISTIC_UUID;
    add_char_params.uuid_type         = p_ss->uuid_type;
    add_char_params.init_len          = settings_length * sizeof(uint8_t);
    add_char_params.max_len           = settings_length * sizeof(uint8_t);
    add_char_params.char_props.read   = 1;
    add_char_params.char_props.write  = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;

    uint32_t err_code = characteristic_add(p_ss->service_handle,
                                  &add_char_params,
                                  &p_ss->settings_handles);

    return err_code;
}

uint32_t ble_sensor_service_available_sensors_char_add(ble_sensor_service_t * p_ss)
{
    ble_add_char_params_t add_char_params;

    //Add Available Sensors characteristic. This is a read only
    //characteristic that holds 20 bytes. The first 10 bytes
    //represent the sensors available on the internal TWI bus and
    //the second 10 bytes represent availble sensors on the 
    //external TWI bus.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = AVAILABLE_SENSORS_CHAR_UUID;
    add_char_params.uuid_type         = p_ss->uuid_type;
    add_char_params.init_len          = 20 * sizeof(uint8_t);
    add_char_params.max_len           = 20 * sizeof(uint8_t);
    add_char_params.char_props.read   = 1;

    add_char_params.read_access       = SEC_OPEN;

    uint32_t err_code = characteristic_add(p_ss->service_handle,
                                  &add_char_params,
                                  &p_ss->available_handle);

    return err_code;
}