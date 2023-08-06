/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdint.h>
#include <string.h>
#include "sensor_service.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

/**@brief Function for handling the Write event.
 *
 * @param[in] p_ss       Sensor Service structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
static void on_write(ble_sensor_service_t * p_ss, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (   (p_evt_write->handle == p_ss->settings_handles.value_handle)
        && (p_evt_write->len == 1)
        && (p_ss->setting_write_handler != NULL))
    {
        p_ss->setting_write_handler(p_ble_evt->evt.gap_evt.conn_handle, p_ss, p_evt_write->data[0]);
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

uint32_t ble_sensor_service_init(ble_sensor_service_t * p_ss, const ble_sensor_service_init_t * p_ss_init)
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
    return ble_sensor_service_settings_char_add(p_ss);
}

uint32_t ble_sensor_service_data_char_add(ble_sensor_service_t * p_ss)
{
    ble_add_char_params_t add_acc_char_params, add_gyr_char_params, add_mag_char_params;

    // Add Accelerometer Data characteristic.
    memset(&add_acc_char_params, 0, sizeof(add_acc_char_params));
    add_acc_char_params.uuid              = ACC_DATA_CHARACTERISTIC_UUID;
    add_acc_char_params.uuid_type         = p_ss->uuid_type;
    add_acc_char_params.init_len          = SAMPLE_SIZE * SENSOR_SAMPLES * sizeof(uint8_t);
    add_acc_char_params.max_len           = SAMPLE_SIZE * SENSOR_SAMPLES * sizeof(uint8_t);
    add_acc_char_params.char_props.read   = 1;
    add_acc_char_params.char_props.notify = 1;

    add_acc_char_params.read_access       = SEC_OPEN;
    add_acc_char_params.cccd_write_access = SEC_OPEN;

    uint32_t err_code = characteristic_add(p_ss->service_handle,
                                  &add_acc_char_params,
                                  &p_ss->data_handles[0]);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Gyroscope Data characteristic.
    memset(&add_gyr_char_params, 0, sizeof(add_gyr_char_params));
    add_gyr_char_params.uuid              = GYR_DATA_CHARACTERISTIC_UUID;
    add_gyr_char_params.uuid_type         = p_ss->uuid_type;
    add_gyr_char_params.init_len          = SAMPLE_SIZE * SENSOR_SAMPLES * sizeof(uint8_t);
    add_gyr_char_params.max_len           = SAMPLE_SIZE * SENSOR_SAMPLES * sizeof(uint8_t);
    add_gyr_char_params.char_props.read   = 1;
    add_gyr_char_params.char_props.notify = 1;

    add_gyr_char_params.read_access       = SEC_OPEN;
    add_gyr_char_params.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_ss->service_handle,
                                  &add_gyr_char_params,
                                  &p_ss->data_handles[1]);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Magnetometer Data characteristic.
    memset(&add_mag_char_params, 0, sizeof(add_mag_char_params));
    add_mag_char_params.uuid              = MAG_DATA_CHARACTERISTIC_UUID;
    add_mag_char_params.uuid_type         = p_ss->uuid_type;
    add_mag_char_params.init_len          = SAMPLE_SIZE * SENSOR_SAMPLES * sizeof(uint8_t);
    add_mag_char_params.max_len           = SAMPLE_SIZE * SENSOR_SAMPLES * sizeof(uint8_t);
    add_mag_char_params.char_props.read   = 1;
    add_mag_char_params.char_props.notify = 1;

    add_mag_char_params.read_access       = SEC_OPEN;
    add_mag_char_params.cccd_write_access = SEC_OPEN;

    return characteristic_add(p_ss->service_handle,
                                  &add_mag_char_params,
                                  &p_ss->data_handles[2]);
}

uint32_t ble_sensor_service_settings_char_add(ble_sensor_service_t * p_ss)
{
    ble_add_char_params_t add_char_params;

    // Add Settings characteristic.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = SETTINGS_CHARACTERISTIC_UUID;
    add_char_params.uuid_type         = p_ss->uuid_type;
    add_char_params.init_len          = sizeof(uint16_t);
    add_char_params.max_len           = sizeof(uint16_t);
    add_char_params.char_props.read   = 1;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;

    return characteristic_add(p_ss->service_handle,
                                  &add_char_params,
                                  &p_ss->settings_handles);
}