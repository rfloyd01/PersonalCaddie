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
#include "custom_services.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
uint32_t data_service_init(ble_service_t * p_data_service, ble_gatts_char_handles_t ** p_data_characteristics)
{

    uint32_t err_code;
    p_data_service->service_uuid.uuid = DATA_SERVICE_BLE_UUID;
    ble_uuid128_t base_uuid = PERSONAL_CADDIE_BASE_UUID;
    p_data_service->service_base_uuid = base_uuid; //If I try to set p_data_service->service_base_uuid directly to PERSONAL_CADDIE_BASE_UUID I get an error, why is this?

    //err_code = sd_ble_uuid_vs_add(&p_data_service->service_base_uuid, &p_data_service->service_uuid.type);
    err_code = sd_ble_uuid_vs_add(&p_data_service->service_base_uuid, &p_data_service->service_uuid.type);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &p_data_service->service_uuid, &p_data_service->service_handle);
    APP_ERROR_CHECK(err_code);
	
    // Print messages to Segger Real Time Terminal
    // UNCOMMENT THE FOUR LINES BELOW AFTER INITIALIZING THE SERVICE OR THE EXAMPLE WILL NOT COMPILE.
    SEGGER_RTT_WriteString(0, "Executing data_service_init().\n"); // Print message to RTT to the application flow
    SEGGER_RTT_printf(0, "Service UUID: 0x%#04x\n", p_data_service->service_uuid.uuid); // Print service UUID should match definition DATA_SERVICE_BLE_UUID
    SEGGER_RTT_printf(0, "Service UUID type: 0x%#02x\n", p_data_service->service_uuid.type); // Print UUID type. Should match BLE_UUID_TYPE_VENDOR_BEGIN. Search for BLE_UUID_TYPES in ble_types.h for more info
    SEGGER_RTT_printf(0, "Service handle: 0x%#04x\n", p_data_service->service_handle); // Print out the service handle. Should match service handle shown in MCP under Attribute values

    //Add custom characteristics to the data service
    err_code = data_characteristics_init(p_data_service, p_data_characteristics);
    APP_ERROR_CHECK(err_code);
}

void settings_service_init(ble_service_t * p_settings_service)
{
    uint32_t err_code;
    ble_uuid128_t base_uuid = PERSONAL_CADDIE_BASE_UUID;
    p_settings_service->service_uuid.uuid = SETTINGS_SERVICE_BLE_UUID;
    p_settings_service->service_base_uuid = base_uuid; //If I try to set p_data_service->service_base_uuid directly to PERSONAL_CADDIE_BASE_UUID I get an error, why is this?

    err_code = sd_ble_uuid_vs_add(&p_settings_service->service_base_uuid, &p_settings_service->service_uuid.type);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &p_settings_service->service_uuid, &p_settings_service->service_handle);
    APP_ERROR_CHECK(err_code);
	
    // Print messages to Segger Real Time Terminal
    // UNCOMMENT THE FOUR LINES BELOW AFTER INITIALIZING THE SERVICE OR THE EXAMPLE WILL NOT COMPILE.
    SEGGER_RTT_WriteString(0, "Executing settings_service_init().\n"); // Print message to RTT to the application flow
    SEGGER_RTT_printf(0, "Service UUID: 0x%#04x\n", p_settings_service->service_uuid.uuid); // Print service UUID should match definition DATA_SERVICE_BLE_UUID
    SEGGER_RTT_printf(0, "Service UUID type: 0x%#02x\n", p_settings_service->service_uuid.type); // Print UUID type. Should match BLE_UUID_TYPE_VENDOR_BEGIN. Search for BLE_UUID_TYPES in ble_types.h for more info
    SEGGER_RTT_printf(0, "Service handle: 0x%#04x\n", p_settings_service->service_handle); // Print out the service handle. Should match service handle shown in MCP under Attribute values
}

uint32_t data_characteristics_init(ble_service_t * p_data_service, ble_gatts_char_handles_t ** p_data_characteristics)
{
    //p_data_characteristics contains three pointers, one each to the accelerometer, gyroscope and magnetometer
    //characteristic handles. All three characteristics are set up in this method.

    //ALl three characteristics will have the same parameters so initialize them with the same parameter variable
    uint32_t err_code;
    ble_add_char_params_t characteristic_parameters;
    uint8_t bytes_per_sample  =  6; //There are 6 bytes for a single sample of X, Y and Z data from each sensor

    //ble_add_char_user_desc_t characteristic_description; //Allows for the addition of a description to the characteristic
    //uint8_t test_string[] = {'a', 'c', 'c', ' ', 'd', 'a', 't', 'a'};
    //characteristic_description.p_char_user_desc = test_string;
    //characteristic_description.max_size = 0x4; //allocate 4-bytes for the description string

    // Initialize Accelerometer Characteristic and add it to the data service.
    memset(&characteristic_parameters, 0, sizeof(characteristic_parameters));
    characteristic_parameters.uuid              = ACC_DATA_CHARACTERISTIC_UUID;
    characteristic_parameters.uuid_type         = p_data_service->service_uuid.type;
    characteristic_parameters.init_len          = SAMPLE_SIZE * SENSOR_SAMPLES * sizeof(uint8_t); //initial length of the characteristic value, want to broadcast all acc values so start with 6 * uint8_t
    characteristic_parameters.max_len           = SAMPLE_SIZE * SENSOR_SAMPLES * sizeof(uint8_t); //maximum length of the characteristic value, want to broadcast all acc values so start with 6 * uint8_t
    characteristic_parameters.char_props.read   = 1;
    characteristic_parameters.char_props.notify = 1;
    characteristic_parameters.read_access       = SEC_OPEN;
    characteristic_parameters.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_data_service->service_handle, &characteristic_parameters, p_data_characteristics[0]);

    // Initialize Gyroscope Characteristic and add it to the data service.
    characteristic_parameters.uuid               = GYR_DATA_CHARACTERISTIC_UUID;
    err_code = characteristic_add(p_data_service->service_handle, &characteristic_parameters, p_data_characteristics[1]);

    // Initialize Magnetometer Characteristic and add it to the data service.
    characteristic_parameters.uuid               = MAG_DATA_CHARACTERISTIC_UUID;
    err_code = characteristic_add(p_data_service->service_handle, &characteristic_parameters, p_data_characteristics[2]);

    return err_code;
}