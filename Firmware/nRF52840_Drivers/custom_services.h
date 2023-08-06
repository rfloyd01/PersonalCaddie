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
 
#ifndef CUSTOM_SERVICES_H__
#define CUSTOM_SERVICES_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

//UUID Definitions
#define PERSONAL_CADDIE_BASE_UUID            {0xF9, 0xF8, 0xA9, 0x3D, 0x6C, 0x8D, 0x4A, 0x4A, 0x84, 0x32, 0x79, 0x26, 0x00, 0x00, 0x30, 0x2B} // 128-bit base UUID for the personal caddie, randomly generated
#define DATA_SERVICE_BLE_UUID                0xABCD // Just a random, but recognizable value
#define SETTINGS_SERVICE_BLE_UUID            0xABCE
#define OLD_ACC_DATA_CHARACTERISTIC_UUID         0x0034
#define OLD_GYR_DATA_CHARACTERISTIC_UUID         0x0035
#define OLD_MAG_DATA_CHARACTERISTIC_UUID         0x0036

//Other Definitions
#define OLD_SENSOR_SAMPLES 10                    /**< The number of samples to read into data characteristics */
#define OLD_SAMPLE_SIZE     6                    /**< The size (in bytes) of a full sensor sample reading */

// Forward declaration of the ble_custom_service_t type.
typedef struct ble_custom_service_s ble_custom_service_t;

typedef void (*custom_service_handler_t) (uint16_t conn_handle, ble_custom_service_t * p_custom_service, uint8_t new_state);

/** @brief Custom Service init structure. This structure contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    custom_service_handler_t service_handler; /**< Event handler to be called when the LED Characteristic is written. */
} ble_custom_service_init_t;

//A struct for holding information about custom made services
struct ble_custom_service_s
{
    uint16_t                 service_handle;     /**< Handle of Our Service (as provided by the BLE stack). */
    ble_uuid_t               service_uuid;       /**< Information about the service uuid */
    ble_uuid128_t            service_base_uuid;  /**< Information about the custom 128-bit base uuid */
    custom_service_handler_t service_handler;    /**< A handler to be called if required */
};

/**@brief Function for initializing our new service.
 *
 * @param[in]   p_data_service       Pointer to Our Service structure.
 */
uint32_t data_service_init(ble_custom_service_t * p_data_service, ble_gatts_char_handles_t ** p_data_characteristics);

void settings_service_init(ble_custom_service_t * p_settings_service);

//characteristic creation functions
uint32_t data_characteristics_init(ble_custom_service_t * p_data_service, ble_gatts_char_handles_t ** p_data_characteristics);

#endif  /* _ CUSTOM_SERVICES_H__ */