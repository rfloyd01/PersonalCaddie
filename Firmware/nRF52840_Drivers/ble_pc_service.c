#include <stdint.h>
#include <string.h>
#include "ble_pc_service.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "sdk_macros.h"

void ble_pc_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    //Blank for now but may add code in the future
}

uint32_t ble_pc_service_init(ble_pc_service_t * p_pc)
{
    uint32_t              err_code;
    ble_uuid_t            ble_uuid;

    //Add vendor specific uuid.
    ble_uuid128_t base_uuid = {PC_SERRVICE_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_pc->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_pc->uuid_type;
    ble_uuid.uuid = PC_SERVICE_UUID;

    //Add the service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_pc->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add Error characteristics
    return ble_pc_service_error_char_add(p_pc);
}

uint32_t ble_pc_service_error_char_add(ble_pc_service_t * p_pc)
{
    ble_add_char_params_t add_err_char_params;

    //Adds a characteristic which holds error codes. Any time an error is encountered
    //in the firmware (SoftDevice and HardFaults excluded), we put the error code in
    //this characteristic and notify the front end that we've encountered an issue. We
    //only notify one error at a time so the characteristic itself is small, just 4 bytes
    //(which is long enough to hold the 0xDEADBEEF error).

    memset(&add_err_char_params, 0, sizeof(add_err_char_params));
    add_err_char_params.uuid              = ERROR_CHARACTERISTIC_UUID;
    add_err_char_params.uuid_type         = p_pc->uuid_type;
    add_err_char_params.init_len          = 4 * sizeof(uint8_t); 
    add_err_char_params.max_len           = 4* sizeof(uint8_t);
    add_err_char_params.char_props.read   = 1;
    add_err_char_params.char_props.notify = 1;

    add_err_char_params.read_access       = SEC_OPEN;
    add_err_char_params.cccd_write_access = SEC_OPEN;

    return characteristic_add(p_pc->service_handle,
                                  &add_err_char_params,
                                  &p_pc->error_handle);
}