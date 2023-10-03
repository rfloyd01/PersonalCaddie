/**
 * Copyright (c) 2014 - 2020, Nordic Semiconductor ASA
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
/** @file
 *
 * @defgroup ble_sdk_app_template_main main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file.
 *
 * This file contains a template for creating a new application. It has the code necessary to wakeup
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_delay.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "sensorsim.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "SEGGER_RTT.h"

#include "lsm9ds1.h"
#include "fxos8700.h"
#include "fxas21002.h"
#include "ble_sensor_service.h"
#include "personal_caddie_operating_modes.h"

//Bluetooth Parameters
#define DEVICE_NAME                     "Personal Caddie"                       /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "FloydInc."                             /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                2400                                    /**< The advertising interval (in units of 0.625 ms. This value corresponds to 1.5s). */

#define APP_ADV_DURATION                BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */
#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

static uint16_t sensor_connection_interval;                                     /**< Variable that holds the desired connection interval (in milliseconds) */

#define SENSOR_CONN_SUP_TIMEOUT         MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(1000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                  1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                       /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                      /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                         /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                             /**< Advertising module instance. */
BLE_SENSOR_SERVICE_DEF(m_ss);                                                   /**< Sensor Service instance. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

//BLE Advertising Data
static ble_uuid_t m_adv_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}
};

static ble_uuid_t m_sr_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {SENSOR_SERVICE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN}
};

//TWI Parameters
#if TWI0_ENABLED
#define INTERNAL_TWI_INSTANCE_ID     0
#endif

#if TWI1_ENABLED
#define EXTERNAL_TWI_INSTANCE_ID     1
#endif

const nrf_drv_twi_t m_twi_internal = NRF_DRV_TWI_INSTANCE(INTERNAL_TWI_INSTANCE_ID);
const nrf_drv_twi_t m_twi_external = NRF_DRV_TWI_INSTANCE(EXTERNAL_TWI_INSTANCE_ID);

volatile bool m_xfer_internal_done = false; //Indicates if operation on the internal TWI bus has ended.
volatile bool m_xfer_external_done = false; //Indicates if operation on the external TWI bus has ended.

volatile bool m_data_ready  = false;        //Indicates when all characteristics have been filled with new data and we're ready to send it to the client
volatile bool m_notification_done = false;  //Indicates when the current data notification has complete

static int m_twi_internal_bus_status;                                                    /**< lets us know the status of the internal TWI bus after each communication attempt on the bus */
static int m_twi_external_bus_status;                                                    /**< lets us know the status of the external TWI bus after each communication attempt on the bus */

static int measurements_taken = 0;                                              /**< keeps track of how many IMU measurements have taken in the given connection interval. */

//IMU Sensor Parameters
static uint8_t default_sensors[3] = {FXOS8700_ACC, FXAS21002_GYR, FXOS8700_MAG};/**< Default sensors that are attempted to be initialized first. */
//static uint8_t default_sensors[3] = {LSM9DS1_ACC, LSM9DS1_GYR, LSM9DS1_MAG};  /**< Default sensors that are attempted to be initialized first. */
static bool sensors_initialized[3] = {false, false, false};                     /**< Keep track of which sensors are currently initialized */
static uint8_t internal_sensors[10];                                            /**< An array for holding the addresses of sensors on the internal TWI line */
static uint8_t external_sensors[10];                                            /**< An array for holding the addresses of sensors on the external TWI line */

static uint8_t internal_sensors_found = 0;
static uint8_t external_sensors_found = 0;

static int32_t sensor_read_register(void *bus, uint8_t add, uint8_t reg, uint8_t *bufp, uint16_t len); //function declaration for reading sensor registers
static int32_t sensor_write_register(void *bus, uint8_t add, uint8_t reg, const uint8_t *bufp); //function declaration for writing sensor registers

static stmdev_ctx_t lsm9ds1_imu;                                                /**< LSM9DS1 accelerometer/gyroscope instance. */
static stmdev_ctx_t lsm9ds1_mag;                                                /**< LSM9DS1 magnetometer instance. */

APP_TIMER_DEF(m_data_reading_timer);                                            /**< A timer used for collecting data from the LSM9DS1 (interrupts are disabled so a timer is needed). */
APP_TIMER_DEF(m_led_timer);                                                     /**< A timer used for LED timing */
#define NEXT_MEASUREMENT_DELAY          APP_TIMER_TICKS(1500)                   /**< Defines the delay between Sensor Measurments (1500 milliseconds). */
#define LED_DELAY                       APP_TIMER_TICKS(2000)                   /**< Defines the delay between LED blinks */
const nrf_drv_timer_t LED_ON_TIMER =    NRF_DRV_TIMER_INSTANCE(1);              /**< Timer to keep track of how long LEDs are turned on during a blink */

static uint8_t acc_characteristic_data[SENSOR_SAMPLES * SAMPLE_SIZE];
static uint8_t gyr_characteristic_data[SENSOR_SAMPLES * SAMPLE_SIZE];
static uint8_t mag_characteristic_data[SENSOR_SAMPLES * SAMPLE_SIZE];
uint8_t sensor_settings[SENSOR_SETTINGS_LENGTH];                               /**< An array represnting the IMU sensor settings */

static int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t write_mag(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t read_mag(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void tx_com( uint8_t *tx_buffer, uint16_t len );
static int32_t get_IMU_data(uint8_t offset);
static int32_t get_MAG_data(uint8_t offset);
static float current_sensor_odr = 59.5;         //keeps track of the current sensor ODR to let us know if we need to update the connection interval

static personal_caddie_operating_mode_t current_operating_mode = ADVERTISING_MODE; /**< The chip starts in advertising mode*/

//Pin Setup
#define USE_EXTERNAL_SENSORS      false                                           /**< Let's the BLE33 know tuat external sensors are being used*/
#define USE_EXTERNAL_LEDS         false                                           /**< Let's the BLE33 know that external LEDs are being used*/
#define EXTERNAL_SENSOR_POWER_PIN NRF_GPIO_PIN_MAP(1, 10)                         /**< Pin used to power external sensors (mapped to RX on BLE 33)*/
#define EXTERNAL_SCL_PIN          NRF_GPIO_PIN_MAP(0, 21)                         /**< Pin used for external TWI clock (mapped to D8 on BLE 33) */
#define EXTERNAL_SDA_PIN          NRF_GPIO_PIN_MAP(0, 23)                         /**< Pin used for external TWI data (mapped to D7 on BLE 33) */
//#define EXTERNAL_SCL_PIN          NRF_GPIO_PIN_MAP(0, 26)                       /**< Pin used for external TWI clock (mapped to D8 on BLE 33) */
//#define EXTERNAL_SDA_PIN          NRF_GPIO_PIN_MAP(0, 27)                       /**< Pin used for external TWI data (mapped to D10 on BLE 33) */
#define EXTERNAL_RED_LED          NRF_GPIO_PIN_MAP(1, 15)                         /**< Pin used for powering an external red LED (mapped to D4 on BLE 33)*/
#define EXTERNAL_BLUE_LED         NRF_GPIO_PIN_MAP(1, 13)                         /**< Pin used for powering an external blue LED (mapped to D5 on BLE 33)*/
#define EXTERNAL_GREEN_LED        NRF_GPIO_PIN_MAP(1, 14)                         /**< Pin used for powering an external green LED (mapped to D6 on BLE 33)*/
#define BLE_33_SENSOR_POWER_PIN   NRF_GPIO_PIN_MAP(0, 22)                         /**< Pin used to power BLE33 onboard sensors*/
#define BLE_33_SCL_PIN            NRF_GPIO_PIN_MAP(0, 15)                         /**< Pin used for internal TWI clock for BLE33*/
#define BLE_33_SDA_PIN            NRF_GPIO_PIN_MAP(0, 14)                         /**< Pin used for internal TWI data for BLE33*/
#define BLE_33_PULLUP             NRF_GPIO_PIN_MAP(1, 0)                          /**< Pullup resistors on BLE 33 sense have separate power source*/
#define BLE_33_BAD_GREEN_LED      NRF_GPIO_PIN_MAP(1, 9)                          /**< low efficiency Green LED Indicator on BLE 33 sense*/
#define BLE_33_RED_LED            NRF_GPIO_PIN_MAP(0, 24)                         /**< Red LED Indicator on BLE 33 sense*/
#define BLE_33_BLUE_LED           NRF_GPIO_PIN_MAP(0, 6)                          /**< Blue LED Indicator on BLE 33 sense*/
#define BLE_33_GREEN_LED          NRF_GPIO_PIN_MAP(0, 16)                         /**< Green LED Indicator on BLE 33 sense*/
static volatile uint8_t active_led = USE_EXTERNAL_LEDS ? EXTERNAL_BLUE_LED : BLE_33_BLUE_LED;                             /**< Variable used to keep track of which LED to turn on/off*/

imu_communication_t imu_comm;                                                          /**< Holds information on how to communicate with each sensor*/

static void advertising_start(bool erase_bonds);

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Peer Manager events                   
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            advertising_start(false);
            break;

        default:
            break;
    }
}

static void twi_address_scan(uint8_t* addresses, uint8_t* device_count, nrf_drv_twi_t const * bus)
{
    //This method scans for all possible TWI addresses on the current bus. If an
    //address is found it gets added to the given array of addresses and the cound
    //of device gets incremented.
    uint8_t sample_data;

    for (uint8_t add = 0; add <= 127; add++)
    {
        ret_code_t err_code;
        
        //set the m_xfer_done bool to false given the current TWI bus
        uint8_t instance = ((nrf_drv_twi_t const*)bus)->inst_idx;
        volatile bool * m_xfer_done;
        if (instance == INTERNAL_TWI_INSTANCE_ID) m_xfer_done = &m_xfer_internal_done; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)
        else m_xfer_done = &m_xfer_external_done;

        *m_xfer_done = false;

        do
        {
            err_code = nrf_drv_twi_rx(bus, add, &sample_data, sizeof(sample_data));
        } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
        while (*m_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

        //If a sensor was found so we add it to the list, get the bus status from the 
        //current twi bus and add the current address if we get an ACK
        int* m_twi_bus_status;
        if (instance == INTERNAL_TWI_INSTANCE_ID) m_twi_bus_status = &m_twi_internal_bus_status;
        else m_twi_bus_status = &m_twi_external_bus_status;

        if (*m_twi_bus_status == NRF_DRV_TWI_EVT_DONE)
        {
            //We've found a TWI address, before adding it to the array though make sure that
            //it's an IMU sensor. The BLE 33 Sense comes with some other sensors on board
            //which we (for now at least) don't care about.
            bool add_to_list = false;
            for (int i = ACC_SENSOR; i <= MAG_SENSOR; i++)
            {
                int stop;
                if (i == ACC_SENSOR) stop = ACC_MODEL_END;
                else if (i == GYR_SENSOR) stop = GYR_MODEL_END;
                else stop = MAG_MODEL_END;

                //iterate through all sensor models of type i
                for (int j = 0; j < stop; j++)
                {
                    uint8_t valid_sensor_address_l = get_sensor_low_address(i, j);
                    uint8_t valid_sensor_address_h = get_sensor_high_address(i, j);

                    if (add == valid_sensor_address_l || add == valid_sensor_address_h)
                    {
                        addresses[(*device_count)++] = add;
                        add_to_list = true;
                        break;
                    }
                }

                if (add_to_list) break;
            }
        }
    }
}

static void disable_twi_bus(int instance_id)
{
    //this method disables a currently enabled twi bus, as well as shuts
    //off power to any gpio pins that it requires.
    if (instance_id == INTERNAL_TWI_INSTANCE_ID)
    {
        nrf_gpio_pin_clear(BLE_33_PULLUP);
        nrf_gpio_pin_clear(BLE_33_SENSOR_POWER_PIN);

        nrf_drv_twi_disable(&m_twi_internal); //disable the current bus
    }
    else
    {
        nrf_gpio_pin_clear(EXTERNAL_SENSOR_POWER_PIN);

        nrf_drv_twi_disable(&m_twi_external); //disable the current bus
    }
}

static void enable_twi_bus(int instance_id)
{
    //This method enables the given twi bus instance, as well as turns on any 
    //necessary power and pullup resistor pins. Before enabling, make sure the
    //current instance isn't already enabled as this will cause an error.
    if (instance_id == INTERNAL_TWI_INSTANCE_ID)
    {
        if (!m_twi_internal.u.twim.p_twim->ENABLE)
        {
            //Send power to the internal sensors and the pullup resistor,
            //also make sure that power for the external sensors is off
            nrf_gpio_pin_set(BLE_33_PULLUP);
            nrf_gpio_pin_set(BLE_33_SENSOR_POWER_PIN);

            nrf_drv_twi_enable(&m_twi_internal); //enable the bus
        }
    }
    else
    {
        if (!m_twi_external.u.twim.p_twim->ENABLE)
        {
            //Send power to the external sensors. also make sure that power,
            //afor the internal sensors and the pullup resistor is off
            nrf_gpio_pin_set(EXTERNAL_SENSOR_POWER_PIN);

            nrf_drv_twi_enable(&m_twi_external); //enable the bus
        }
    }

    nrf_delay_ms(50); //slight delay so sensors have time to power on
}

static void sensor_communication_init(sensor_type_t type, uint8_t model, uint8_t address, nrf_drv_twi_t const * bus)
{
    switch (type)
    {
        case ACC_SENSOR:
            imu_comm.acc_comm.address = address;
            imu_comm.acc_comm.twi_bus = bus;
            imu_comm.acc_comm.read_register = sensor_read_register;
            imu_comm.acc_comm.write_register = sensor_write_register;
            if (model == LSM9DS1_ACC)
            {
                imu_comm.acc_comm.update_settings = lsm9ds1_acc_apply_setting;
                imu_comm.acc_comm.get_data = lsm9ds1_get_acc_data;
            }
            else if (model == FXOS8700_ACC)
            {
                imu_comm.acc_comm.update_settings = fxos8700_acc_apply_setting;
                imu_comm.acc_comm.get_data = fxos8700_get_acc_data;
            }
            break;
        case GYR_SENSOR:
            imu_comm.gyr_comm.address = address;
            imu_comm.gyr_comm.twi_bus = bus;
            imu_comm.gyr_comm.read_register = sensor_read_register;
            imu_comm.gyr_comm.write_register = sensor_write_register;
            if (model == LSM9DS1_GYR)
            {
                imu_comm.gyr_comm.update_settings = lsm9ds1_gyr_apply_setting;
                imu_comm.gyr_comm.get_data = lsm9ds1_get_gyr_data;
            }
            else if (model == FXAS21002_GYR)
            {
                imu_comm.gyr_comm.update_settings = NULL;
                imu_comm.gyr_comm.get_data = fxas21002_get_gyr_data;
            }
            break;
        case MAG_SENSOR:
            imu_comm.mag_comm.address = address;
            imu_comm.mag_comm.twi_bus = bus;
            imu_comm.mag_comm.read_register = sensor_read_register;
            imu_comm.mag_comm.write_register = sensor_write_register;
            if (model == LSM9DS1_MAG)
            {
                imu_comm.mag_comm.update_settings = lsm9ds1_mag_apply_setting;
                imu_comm.mag_comm.get_data = lsm9ds1_get_mag_data;
            }
            else if (model == FXOS8700_MAG)
            {
                imu_comm.mag_comm.update_settings = fxos8700_mag_apply_setting;
                imu_comm.mag_comm.get_data = fxos8700_get_mag_data;
            }
            break;
    }
}

static float sensor_odr_calculate()
{
    //Calculates the highest ODR of the three active sensors. The highest ODR
    //value will dictate the appropriate connection interval for the BLE connection.
    float current_highest_odr = 0.0;

    for (int i = 0; i < 3; i++)
    {
        if (imu_comm.sensor_model[i] == LSM9DS1_ACC)
        {
            float new_odr = lsm9ds1_odr_calculate(sensor_settings, imu_comm.sensor_model[0], imu_comm.sensor_model[1], imu_comm.sensor_model[2], i);
            if (new_odr > current_highest_odr) current_highest_odr = new_odr;
        }
        else if (imu_comm.sensor_model[i] == FXOS8700_ACC)
        {
            float new_odr;

            if (i == GYR_SENSOR) new_odr = fxas21002_odr_calculate(imu_comm.sensor_model[1], sensor_settings[GYR_START + ODR]);
            else new_odr = fxos8700_odr_calculate(imu_comm.sensor_model[0], imu_comm.sensor_model[2], sensor_settings[ACC_START + ODR], sensor_settings[MAG_START + ODR]);
            
            if (new_odr > current_highest_odr) current_highest_odr = new_odr;
        }
    }

    return current_highest_odr;
}

static void default_sensor_select()
{
    //Check that all sensors have been initialized after scanning the internal and external bus.
    //If not, then we're going to need to choose a non-default sensor. The first priority is to 
    //pick a sensor on the same chip (i.e. same model and TWI bus). If this can't work for 
    //whatever reason then try to select a sensor of the same model but on the other bus. If this
    //doesn't work, then select the first option available.
    if (!sensors_initialized[0] || !sensors_initialized[1] || !sensors_initialized[2])
    {
        for (int i = ACC_SENSOR; i <= MAG_SENSOR; i++)
        {
            if (sensors_initialized[i]) continue;

            //We'll need to put the new model number into the sensor settings array
            //so figure out the right place to do so.
            int setting_location;
            if (i == ACC_SENSOR) setting_location = ACC_START + SENSOR_MODEL;
            else if (i == GYR_SENSOR) setting_location = GYR_START + SENSOR_MODEL;
            else setting_location = MAG_START + SENSOR_MODEL;

            for (int j = 0; j < 3; j++)
            {
                if (!sensors_initialized[j]) continue;

                //Get the addresses for sensor i from sensor j's manufacturer
                uint8_t new_sensor_address_l = get_sensor_low_address(i, default_sensors[j]);
                uint8_t new_sensor_address_h = get_sensor_high_address(i, default_sensors[j]);

                //figure out which TWI bus sensor j is on
                nrf_drv_twi_t const * known_sensor_bus;
                if (j == 0) known_sensor_bus = imu_comm.acc_comm.twi_bus;
                else if (j == 1) known_sensor_bus = imu_comm.gyr_comm.twi_bus;
                else if (j == 2) known_sensor_bus = imu_comm.mag_comm.twi_bus;

                uint8_t* primary_search = internal_sensors, *secondary_search = external_sensors;
                int primary_stop = internal_sensors_found, secondary_stop = external_sensors_found;
                if (known_sensor_bus->inst_idx == 1)
                {
                    primary_search = external_sensors;
                    secondary_search = internal_sensors;
                    primary_stop = external_sensors_found;
                    secondary_stop = internal_sensors_found;
                }

                //NOTE: In the below loops, we can only initialize sensor i using sensor j's model number
                //because currently all sensor manufacturers I use have an acc, gyr and
                //mag so the model numbers line up (i.e. lsm9ds1_acc = 0, lsm9ds1_gyr = 0).
                //If I start using chips that don't have all three sensors then the 
                //model numbers will no longer line up and the below loops won't be safe anymore.

                //Search sensor j's TWI bus for sensor i.
                for (int k = 0; k < primary_stop; k++)
                {
                    if (primary_search[k] == new_sensor_address_l || primary_search[k] == new_sensor_address_h)
                    {
                        sensor_communication_init(i, default_sensors[j], primary_search[k], known_sensor_bus);
                        sensors_initialized[i] = true;
                        sensor_settings[setting_location] = default_sensors[j];
                        default_sensors[i] = default_sensors[j];
                        break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                    }
                }

                if (sensors_initialized[i]) break; //move onto the next sensor

                //If we were external on sensor j's TWI bus, search the other TWI bus
                for (int k = 0; k < secondary_stop; k++)
                {
                    if (secondary_search[k] == new_sensor_address_l || secondary_search[k] == new_sensor_address_h)
                    {
                        sensor_communication_init(i, default_sensors[j], secondary_search[k], known_sensor_bus);
                        sensors_initialized[i] = true;
                        sensor_settings[setting_location] = default_sensors[j];
                        default_sensors[i] = default_sensors[j];
                        break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                    }
                }

                if (sensors_initialized[i]) break; //move onto the next sensor
            }

            if (sensors_initialized[i]) continue; //if this sensor has been initialized then move on to the next one.

            //If we haven't initialized the sensor yet then it means we can't find one of
            //the appropriate model on either bus, so we need to just initialize the first
            //one possible.
            int stop;
            if (i == ACC_SENSOR) stop = ACC_MODEL_END;
            else if (i == GYR_SENSOR) stop = GYR_MODEL_END;
            else stop = MAG_MODEL_END;

            //search the external bus first
            for (int k = 0; k < external_sensors_found; k++)
            {
                //loopt through all models of the current sensor type
                for (int l = 0; l < stop; l++)
                {
                    uint8_t valid_sensor_address_l = get_sensor_low_address(i, l);
                    uint8_t valid_sensor_address_h = get_sensor_high_address(i, l);

                    if (external_sensors[k] == valid_sensor_address_l || external_sensors[k] == valid_sensor_address_h)
                    {
                        sensor_communication_init(i, l, external_sensors[k], &m_twi_external);
                        sensors_initialized[i] = true;
                        sensor_settings[setting_location] = l;
                        default_sensors[i] = l;
                        break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                    }
                }
            }

            if (sensors_initialized[i]) continue; //move onto the next sensor

            //and finally search the internal bus
            for (int k = 0; k < internal_sensors_found; k++)
            {
                //loopt through all models of the current sensor type
                for (int l = 0; l < stop; l++)
                {
                    uint8_t valid_sensor_address_l = get_sensor_low_address(i, l);
                    uint8_t valid_sensor_address_h = get_sensor_high_address(i, l);

                    if (internal_sensors[k] == valid_sensor_address_l || internal_sensors[k] == valid_sensor_address_h)
                    {
                        sensor_communication_init(i, l, internal_sensors[k], &m_twi_internal);
                        sensors_initialized[i] = true;
                        sensor_settings[setting_location] = l;
                        default_sensors[i] = l;
                        break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                    }
                }
            }

            //If we haven't found a suitable sensor yet then there's something wrong. Print
            //an error statement
            if (!sensors_initialized[i]) SEGGER_RTT_WriteString(0, "Couldn't find any sensors, check board connections.\n");
        }
    }
}


/**@brief Function for the LSM9DS1 initialization.
 *
 * @details Initializes the LSM9DS1 accelerometer, gyroscope and magnetometer
 */
static void sensors_init(bool discovery)
{
    //When first turning on the Personal Caddie we need to scan both the internal
    //and external TWI bus to see what sensors are available so we can populate some
    //arrays with this information. Subsequent calls to this method don't require this.
    //Regardless of whether or not we enter this method in discovery mode, both TWI buses
    //need to be enabled.
    enable_twi_bus(INTERNAL_TWI_INSTANCE_ID);
    enable_twi_bus(EXTERNAL_TWI_INSTANCE_ID);
    
    if (discovery)
    {
        //First reset the sensor settings array, everything goes to zero except
        //the sensor models which are set to the default sensors.
        for (int i = 0; i < SENSOR_SETTINGS_LENGTH; i++) sensor_settings[i] = 0;
        sensor_settings[ACC_START + SENSOR_MODEL] = default_sensors[0];
        sensor_settings[GYR_START + SENSOR_MODEL] = default_sensors[1];
        sensor_settings[MAG_START + SENSOR_MODEL] = default_sensors[2];

        //Then initialize the internal and external sensors arrays so that each element
        //has a value of 0xFF. This value alerts the front end that no sensor is present.
        for (int i = 0; i < 10; i++)
        {
            internal_sensors[i] = 0xff;
            external_sensors[i] = 0xff;
        }

        //Initiate the TWI bus scan, looking for sensors
        twi_address_scan(internal_sensors, &internal_sensors_found, &m_twi_internal); //Scan the internal TWI bus
        twi_address_scan(external_sensors, &external_sensors_found, &m_twi_external); //Then scan the external bus

        //We also need to populate the available sensors characteristic with all the sensors
        //that were found on the internal and external TWI buses. This will allow the front
        //end application to dynamically choose which sensors to use.
        ble_gatts_value_t internal_available_sensors, external_available_sensors;

        internal_available_sensors.len = 10;
        internal_available_sensors.p_value = internal_sensors;
        internal_available_sensors.offset = 0;

        external_available_sensors.len = 10;
        external_available_sensors.p_value = external_sensors;
        external_available_sensors.offset = 10;

        uint32_t err_code = sd_ble_gatts_value_set(m_conn_handle, m_ss.available_handle.value_handle, &internal_available_sensors);
        APP_ERROR_CHECK(err_code);

        err_code = sd_ble_gatts_value_set(m_conn_handle, m_ss.available_handle.value_handle, &external_available_sensors);
        APP_ERROR_CHECK(err_code);
    }

    //Scan the external sensors to see if any of them match the default sensors that were
    //set in the settings array. If not, then scan the internal sensors to see if any of
    //them are there. If one of the default sensors isn't found on either bus then a
    //different one will need to be selected.
    if (external_sensors_found > 0)
    {
        for (int i = 0; i < 3; i++)
        {
            uint8_t default_sensor_address_l = get_sensor_low_address(i, default_sensors[i]);
            uint8_t default_sensor_address_h = get_sensor_high_address(i, default_sensors[i]);

            for (int j = 0; j < external_sensors_found; j++)
            {
                if (external_sensors[j] == default_sensor_address_l || external_sensors[j] == default_sensor_address_h)
                {
                    sensor_communication_init(i, default_sensors[i], external_sensors[j], &m_twi_external);
                    sensors_initialized[i] = true;
                    break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                }
            }
        }
    }

    //See if all sensors were initialized on the external bus, if not, then initialized sensors
    //from internal bus
    if (!sensors_initialized[0] || !sensors_initialized[1] || !sensors_initialized[2])
    {
        for (int i = 0; i < 3; i++)
        {
            if (sensors_initialized[i]) continue; //this sensor was initialized on the external bus so skip it
            uint8_t default_sensor_address_l = get_sensor_low_address(i, default_sensors[i]);
            uint8_t default_sensor_address_h = get_sensor_high_address(i, default_sensors[i]);

            for (int j = 0; j < internal_sensors_found; j++)
            {
                if (internal_sensors[j] == default_sensor_address_l || internal_sensors[j] == default_sensor_address_h)
                {
                    sensor_communication_init(i, default_sensors[i], internal_sensors[j], &m_twi_internal);
                    sensors_initialized[i] = true;
                    break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                }
            }
        }
    }

    //If any of the three sensors haven't been initialized yet, use the below
    //function to pick a different default sensor(s).
    default_sensor_select();

    //Handle the initialization of individual sensors. Currently the model for each sensor type
    //matches the enums for the other sensor models (i.e. lsm9ds1 acc/gyrmag all have enum values of 0x00).
    //We take advantage of this fact to see how many sensors from each model need to be initialized. This
    //may not hold up as more sensors get added, but I'll address that when the time comes.
    for (int i = 0; i < ACC_MODEL_END; i++)
    {
        int sensors = 0;
        for (int j = 0; j < 3; j++)
        {
            if (default_sensors[j] == i)
            {
                imu_comm.sensor_model[j] = i;
                sensors |= (1 << j);
            }
        }

        if (sensors != 0)
        {
            switch(i)
            {
                case LSM9DS1_ACC:
                    lsm9ds1_init(&imu_comm, sensors, sensor_settings);
                    break;
                case FXOS8700_ACC:
                    fxos8700init(&imu_comm, sensors, sensor_settings);
                    fxas21002init(&imu_comm, sensors, sensor_settings);
                    break;
            }
        }
    }

    //regardless of whether or not any sensors are found, disable the power pins and TWI bus
    disable_twi_bus(INTERNAL_TWI_INSTANCE_ID);
    disable_twi_bus(EXTERNAL_TWI_INSTANCE_ID);

    //After all sensors have been initialized, update the sensor settings characteristic to 
    //reflect the sensor settings array. Furthermore, we also set the connection interval 
    //to be equal to the highest sensor ODR (to the nearest 15th millisecond)
    ble_gatts_value_t settings;
    settings.len = SENSOR_SETTINGS_LENGTH;
    settings.p_value = sensor_settings;
    settings.offset = 0;
    uint32_t err_code = sd_ble_gatts_value_set(m_conn_handle, m_ss.settings_handles.value_handle, &settings);
    APP_ERROR_CHECK(err_code);
}

static void characteristic_update_and_notify()
{
    
    ble_gatts_hvx_params_t acc_notify_params, gyr_notify_params, mag_notify_params;
    memset(&acc_notify_params, 0, sizeof(acc_notify_params));
    memset(&gyr_notify_params, 0, sizeof(gyr_notify_params));
    memset(&mag_notify_params, 0, sizeof(mag_notify_params));

    uint16_t acc_data_characteristic_size = SENSOR_SAMPLES * SAMPLE_SIZE;
    uint16_t gyr_data_characteristic_size = SENSOR_SAMPLES * SAMPLE_SIZE;
    uint16_t mag_data_characteristic_size = SENSOR_SAMPLES * SAMPLE_SIZE;

    //Setup accelerometer notification first
    acc_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    acc_notify_params.handle = m_ss.data_handles[0].value_handle;
    acc_notify_params.p_data = acc_characteristic_data;
    acc_notify_params.p_len  = &acc_data_characteristic_size;
    acc_notify_params.offset = 0;

    //Setup gyroscope notification second
    gyr_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    gyr_notify_params.handle = m_ss.data_handles[1].value_handle;
    gyr_notify_params.p_data = gyr_characteristic_data;
    gyr_notify_params.p_len  = &gyr_data_characteristic_size;
    gyr_notify_params.offset = 0;

    //Setup magnetometer notification third
    mag_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    mag_notify_params.handle = m_ss.data_handles[2].value_handle;
    mag_notify_params.p_data = mag_characteristic_data;
    mag_notify_params.p_len  = &mag_data_characteristic_size;
    mag_notify_params.offset = 0;

    //Set the notifcation_done boolean to false and start notifications.
    //The boolean will be set to true in the BLE handler when the notification
    //is complete, alerting us to send out the next notification.


    m_notification_done = false;
    uint32_t ret = sd_ble_gatts_hvx(m_conn_handle, &acc_notify_params); //acc data notification
    ret = sd_ble_gatts_hvx(m_conn_handle, &gyr_notify_params); //gyr data notification
    ret = sd_ble_gatts_hvx(m_conn_handle, &mag_notify_params); //mag data notification

    while (!m_notification_done) {} //wait for the three notifications to go out before continuing
    //APP_ERROR_CHECK(ret); //Uncommenting this can help debug notification errors
}

static void data_reading_timer_handler(void * p_context)
{
    //Everytime the data reading timer goes off we take sensor readings and then 
    //update the appropriate characteristic values. The timer should go off SENSOR_SAMPLES times
    //every connection interval
    imu_comm.acc_comm.get_data(acc_characteristic_data, SAMPLE_SIZE * measurements_taken);
    imu_comm.gyr_comm.get_data(gyr_characteristic_data, SAMPLE_SIZE * measurements_taken);
    imu_comm.mag_comm.get_data(mag_characteristic_data, SAMPLE_SIZE * measurements_taken);
    
    measurements_taken++;

    //after all the samples are read, update the characteristics and notify
    if ( measurements_taken == SENSOR_SAMPLES)
    {
        //SEGGER_RTT_WriteString(0, "Sending the following bytes to GYR characteristic:\n");
        //for (int i = 0; i < SENSOR_SAMPLES; i++)
        //{
        //    for (int j = 0; j < SAMPLE_SIZE; j++)
        //    {
        //        SEGGER_RTT_printf(0, "0x%#01x ", *(gyr_characteristic_data + i * SAMPLE_SIZE + j));
        //    }
        //    SEGGER_RTT_WriteString(0, "\n");
        //}
        //SEGGER_RTT_WriteString(0, "\n");

        m_data_ready = true; //flags the main loop to broadcast data notifications
        measurements_taken = 0; //reset the data counter
    }
}

static void led_timer_handler(void * p_context)
{
    //This timer causes the currently active LED to light up. A separate timer is used to 
    //turn the LED back off after a set time period
    if (USE_EXTERNAL_LEDS) nrf_gpio_pin_set(active_led);
    else nrf_gpio_pin_clear(active_led);

    nrf_drv_timer_clear(&LED_ON_TIMER); //reset the LED-on timer
    nrf_drv_timer_enable(&LED_ON_TIMER); //turn on the LED_on timer, the handler for this timer will turn the LED back off
    //uint32_t timer_val = nrf_drv_timer_capture(&LED_ON_TIMER, NRF_TIMER_CC_CHANNEL1);
}

void led_on_timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    //This handler gets called when an LED is turned to the active state. Simply turn the 
    //LED back off when this handler is called and then turn the LED-on timer off.
    if (USE_EXTERNAL_LEDS) nrf_gpio_pin_clear(active_led);
    else nrf_gpio_pin_set(active_led);
    nrf_drv_timer_disable(&LED_ON_TIMER);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create a non-ble timer to help keep track of LED on time
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.frequency = NRF_TIMER_FREQ_16MHz;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;

    err_code = nrf_drv_timer_init(&LED_ON_TIMER, &timer_cfg, led_on_timer_handler);
    APP_ERROR_CHECK(err_code);

    //TODO: Lower the led on timer when done testing
    uint32_t led_on_time = nrf_drv_timer_ms_to_ticks(&LED_ON_TIMER, 25); //the LEDs will only be on for 2 milliseconds while blinking
    nrf_drv_timer_extended_compare(&LED_ON_TIMER, NRF_TIMER_CC_CHANNEL0, led_on_time, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true); //calls the led_on_timer_handler

    // Create custom timers.
    app_timer_create(&m_data_reading_timer, APP_TIMER_MODE_REPEATED, data_reading_timer_handler);
    app_timer_create(&m_led_timer, APP_TIMER_MODE_REPEATED, led_timer_handler);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

    //calculate the connection interval based on the ODR of the sensor
    //TODO: It would be better to move this into the sensor_init() method
    //      but for some reason I waas getting an error when putting it there.
    float sensor_odr = sensor_odr_calculate();

    int minimum_interval_required = 1000.0 / sensor_odr * SENSOR_SAMPLES + 1; //this is in milliseconds, hence the 1000
    minimum_interval_required += (15 - minimum_interval_required % 15); //round up to the nearest 15th millisecond

    //Convert from milliseconds to 1.25 millisecond units by dividing by 1.25 (same multiplying by 4/5)
    int mir_125 = minimum_interval_required / 5;
    mir_125 *= 4;

    gap_conn_params.min_conn_interval = mir_125;
    gap_conn_params.max_conn_interval = mir_125 + 12; //must be 15 ms (or 12 in 1.25ms units) greater at a minimum
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = SENSOR_CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

//Functions for updating sensor power modes and settings
static void sensor_idle_mode_start()
{ 
    //In this mode, the appropriate TWI bus(es) is active and power is going to the sensor(s) but the 
    //sensor is put into sleep mode. A red LED blinking in time with the connection 
    //interval shows when this mode is active.
    if (current_operating_mode == SENSOR_IDLE_MODE) return; //no need to change anything if already in idle mode

    //swap to the red LED
    if (USE_EXTERNAL_LEDS) active_led = EXTERNAL_RED_LED;
    else active_led = BLE_33_RED_LED;
    ret_code_t err_code;

    if (current_operating_mode == SENSOR_ACTIVE_MODE)
    {
        //If we're transitioning from active to idle mode we need to stop the data collection timer
        //and then put the sensor into sleep mode.
        err_code = app_timer_stop(m_data_reading_timer); //Even if the timer isn't actively on it's ok to call this method
        
        //Put all sensors into idle mode, any of the sensors that are active
        //will be properly initialized
        lsm9ds1_idle_mode_enable();
        fxos8700_idle_mode_enable();
        fxas21002_idle_mode_enable();

        //the LED is deactivated during data collection so turn it back on
        err_code = app_timer_start(m_led_timer, LED_DELAY, NULL); //Even if the timer is already on it's ok to call this method
    }
    else 
    {
        //If we aren't in sensor active or idle mode, it means that we arrived here from connection mode.
        //This means that both the TWI bus and sensors need to be turned on. There are two reasons why we'd
        //be turning on the sensors, either we're about to take data readings, or, we want to update the
        //sensor settings. In both cases we need to turn on the TWI bus to enable communication, but
        //before doing so we need to see if a call from the front end to use a different sensor has been made.

        uint8_t new_sensors = 0;

        SEGGER_RTT_printf(0, "Sensors in settings array: %d, %d, %d\n", sensor_settings[ACC_START + SENSOR_MODEL], sensor_settings[GYR_START + SENSOR_MODEL], sensor_settings[MAG_START + SENSOR_MODEL]);

        if (imu_comm.sensor_model[ACC_SENSOR] != sensor_settings[ACC_START + SENSOR_MODEL])
        {
            new_sensors |= 0b001;
            default_sensors[ACC_SENSOR] = sensor_settings[ACC_START + SENSOR_MODEL];
            sensors_initialized[ACC_SENSOR] = false;
        }

        if (imu_comm.sensor_model[GYR_SENSOR] != sensor_settings[GYR_START + SENSOR_MODEL])
        {
            new_sensors |= 0b010;
            default_sensors[GYR_SENSOR] = sensor_settings[GYR_START + SENSOR_MODEL];
            sensors_initialized[GYR_SENSOR] = false;
        }
        
        if (imu_comm.sensor_model[MAG_SENSOR] != sensor_settings[MAG_START + SENSOR_MODEL])
        {
            new_sensors |= 0b100;
            default_sensors[MAG_SENSOR] = sensor_settings[MAG_START + SENSOR_MODEL];
            sensors_initialized[MAG_SENSOR] = false;
        }

        //Initialize any new sensors that need it
        if (new_sensors) sensors_init(false);

        //turn on any TWI buses that are needed by the current sensors
        enable_twi_bus(imu_comm.acc_comm.twi_bus->inst_idx);
        enable_twi_bus(imu_comm.gyr_comm.twi_bus->inst_idx);
        enable_twi_bus(imu_comm.mag_comm.twi_bus->inst_idx);        
    }

    current_operating_mode = SENSOR_IDLE_MODE; //set the current operating mode to idle
}

static void sensor_active_mode_start()
{
    if (current_operating_mode != SENSOR_IDLE_MODE) return; //we can only go to active mode from idle mode

    //All of the settings for the sensor should have been set already, we just need to activate them
    //and then start the data collection timer. We also disable the LED to save on power
    app_timer_stop(m_led_timer); //disable the led by turning of it's timer

    //DEBUG: Turn on timer to see how offten measurements are being taken
    //nrf_drv_timer_clear(&LED_ON_TIMER); //reset the LED-on timer
    //nrf_drv_timer_enable(&LED_ON_TIMER); //turn on the LED_on timer, the handler for this timer will turn the LED back off
    
    //Put all sensors into idle mode, any of the sensors that are active
    //will be properly initialized
    lsm9ds1_active_mode_enable();
    fxos8700_active_mode_enable();
    fxas21002_active_mode_enable();

    //start data acquisition by turning on the data timer The timer needs to be converted from ms
    //to 'ticks' which match the frequency of the app timer. Normally this is done with a Macro but
    //that can't be accessed here so the macro code is copied here
    float sensor_odr = sensor_odr_calculate();
    int reading_timer_milliseconds = 1000.0 / sensor_odr + 1; //this is how often (in milliseconds) we need to take a sensor reading

    uint64_t A = reading_timer_milliseconds * (uint64_t)APP_TIMER_CLOCK_FREQ;
    uint64_t B = 1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1);
    uint32_t data_timer_delay = (((A) + ((B) / 2)) / (B));
    app_timer_start(m_data_reading_timer, data_timer_delay, NULL);
    
    current_operating_mode = SENSOR_ACTIVE_MODE; //set the current operating mode to idle
}

static void connected_mode_start()
{
    //We'll other be put into this mode from advertising mode (in which case all we need to do is 
    //change the color of the blinking LED, or from sensor_idle mode, in which case we need to 
    //disable the power going to the sensor and the TWI bus.
    if (current_operating_mode == SENSOR_IDLE_MODE)
    {
        //The chip is currently in sensor idle mode so we need to power off the sensors and disable any active TWI bus
        disable_twi_bus(imu_comm.acc_comm.twi_bus->inst_idx);
        disable_twi_bus(imu_comm.gyr_comm.twi_bus->inst_idx);
        disable_twi_bus(imu_comm.mag_comm.twi_bus->inst_idx);
    }

    //change the color of the blinking LED to green
    if (USE_EXTERNAL_LEDS) active_led = EXTERNAL_GREEN_LED;
    else active_led = BLE_33_GREEN_LED;
    current_operating_mode = CONNECTED_MODE; //set the current operating mode to idle
}

static void advertising_mode_start()
{
    //There are two ways to get into advertising mode, either by turning the device on,
    //or by losing a connection to a central device (whether this is intentional or 
    //accidental). In the latter case, we need to disable any sensors or peripherals
    //to save on power.

    switch (current_operating_mode)
    {
        case ADVERTISING_MODE:
            return; //no changes need to be made so leave the method
        case SENSOR_ACTIVE_MODE:
            sensor_idle_mode_start();
            //break statement purposely omitted here
        case SENSOR_IDLE_MODE:
            connected_mode_start();
            //break statement purposely omitted here
        default:
            if (USE_EXTERNAL_LEDS) active_led = EXTERNAL_BLUE_LED;
            else active_led = BLE_33_BLUE_LED;
            break;
    }

    current_operating_mode = ADVERTISING_MODE; //set the current operating mode to idle
    
    //finally, we need to terminate the existing connection
    //TODO: Implement this feature
}

void update_connection_interval()
{
    //When changing the settings for the IMU, we take a look to see if the ODR for the sensors
    //has changed. If so, we alter the current connection interval to get all data to the 
    //app in a timely manner.
    float sensor_odr = sensor_odr_calculate();
    if (sensor_odr != current_sensor_odr)
    {
        //if the odr has changed then we update the connection interval to match it
        int minimum_interval_required = 1000.0 / sensor_odr * SENSOR_SAMPLES + 1; //this is in milliseconds, hence the 1000
        minimum_interval_required += (15 - minimum_interval_required % 15); //round up to the nearest 15th millisecond

        //Convert from milliseconds to 1.25 millisecond units by dividing by 1.25 (same multiplying by 4/5)
        int mir_125 = minimum_interval_required / 5;
        mir_125 *= 4;

        //Now make the connection interval change request
        ret_code_t err_code = BLE_ERROR_INVALID_CONN_HANDLE;
        ble_gap_conn_params_t new_params;

        new_params.min_conn_interval = mir_125;
        new_params.max_conn_interval = mir_125 + 12; //must be 15 ms (or 12 in 1.25ms units) greater at a minimum
        new_params.slave_latency = SLAVE_LATENCY;
        new_params.conn_sup_timeout = SENSOR_CONN_SUP_TIMEOUT;

        err_code = ble_conn_params_change_conn_params(m_conn_handle, &new_params);

        if (err_code != NRF_SUCCESS) SEGGER_RTT_WriteString(0, "Couldn't update connection interval.\n");
        else current_sensor_odr = sensor_odr;
    }
}

/**@brief Function for handling write events to the Sensor Settings Characteristic.
 *
 * @param[in] p_lbs     Instance of Sensor Settings Service to which the write applies.
 * @param[in] led_state Written/desired state for the sensor settings.
 */
static void sensor_settings_write_handler(uint16_t conn_handle, ble_sensor_service_t * p_ss, const uint8_t* settings_state)
{
    //Different things can happen depending on what gets written to the settings characteristic
    switch (*settings_state)
    {
        case 0:
            advertising_mode_start();
            break;
        case 1:
            connected_mode_start();
            break;
        case 2:
            sensor_idle_mode_start();
            break;
        case 3:
            for (int i = 1; i < SENSOR_SETTINGS_LENGTH; i++) sensor_settings[i] = *(settings_state + i);
            sensor_idle_mode_start();
            update_connection_interval();
            break;
        case 4:
            sensor_settings[*(settings_state + 1)] = *(settings_state + 2);
            sensor_idle_mode_start();
            break;
        case 5:
            sensor_active_mode_start();
            break;
    }

    //SEGGER_RTT_printf(0, "Byte %u = 0x%x\n", i, *(settings_state + i));
    
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    ret_code_t                err_code;
    nrf_ble_qwr_init_t        qwr_init = {0};
    ble_sensor_service_init_t init  = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize sensor service
    init.setting_write_handler = sensor_settings_write_handler;

    err_code = ble_sensor_service_init(&m_ss, &init, SENSOR_SETTINGS_LENGTH);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
 */
static void application_timers_start(void)
{
    //a timer that let's us know when to read the next data sample from the Sensor
    //app_timer_start(m_data_reading_timer, NEXT_MEASUREMENT_DELAY, NULL);
    app_timer_start(m_led_timer, LED_DELAY, NULL); //blinks the green led along with the advertising interval
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            SEGGER_RTT_WriteString(0, "Fast advertising.");
            //err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING); //turn off LED indication for now
            //APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            //NRF_LOG_INFO("Disconnected.");
            SEGGER_RTT_WriteString(0, "Disconnected from the Personal Caddie\n");
            advertising_mode_start();
            break;

        case BLE_GAP_EVT_CONNECTED:
            //NRF_LOG_INFO("Connected.");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            connected_mode_start();
            SEGGER_RTT_WriteString(0, "Connected to the Personal Caddie.\n");
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            //NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            sensor_connection_interval = p_ble_evt->evt.gap_evt.params.conn_param_update.conn_params.min_conn_interval * 5 / 4;
            SEGGER_RTT_printf(0, "Connection Interval updated to : %u milliseconds.\n", sensor_connection_interval);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            //NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            //NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            m_notification_done = true; //set the notification done bool to true to allow more notifications
            //SEGGER_RTT_WriteString(0, "Notification complete.\n");
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    //NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated when button is pressed.
 */
static void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break; // BSP_EVENT_SLEEP

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break; // BSP_EVENT_DISCONNECT

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break; // BSP_EVENT_KEY_0

        default:
            break;
    }
}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = true;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    //Services included in advertising data
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    //Services included in scan response data
    init.srdata.uuids_complete.uuid_cnt = sizeof(m_sr_uuids) / sizeof(m_sr_uuids[0]);
    init.srdata.uuids_complete.p_uuids = m_sr_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**@brief Function for starting advertising.
 */
static void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED event
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);

        APP_ERROR_CHECK(err_code);
    }
}

void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    //A handler that gets called when events happen on the TWI bus. I'm currently not really using this 
    //method for anything as my sensor reading functions work on a timer. I should keep this function though.
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            //SEGGER_RTT_printf(0, "Successfully reached slave address 0x%x.\n", p_event->xfer_desc.address);
            //if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_TXRX | p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            //{
            //    //uncomment below line after the data_handler function has been written
            //    //data_handler(m_data);
            //}
            break;
        case NRF_DRV_TWI_EVT_ADDRESS_NACK:
            //SEGGER_RTT_printf(0, "Address NACK received while trying to reach slave address 0x%x.\n", p_event->xfer_desc.address);
            break;
        case NRF_DRV_TWI_EVT_DATA_NACK:
            //SEGGER_RTT_printf(0, "Data NACK received while trying to reach slave address 0x%x.\n", p_event->xfer_desc.address);
            break;
        case NRFX_TWI_EVT_OVERRUN:
            SEGGER_RTT_printf(0, "Overrun event while trying to reach slave address 0x%x.\n", p_event->xfer_desc.address);
            break;
        case NRFX_TWI_EVT_BUS_ERROR:
            SEGGER_RTT_printf(0, "Bus Error received while trying to reach slave address 0x%x.\n", p_event->xfer_desc.address);
            break;
        default:
            SEGGER_RTT_WriteString(0, "Something other than Event Done was achieved.\n");
            break;
    }
}

void internal_twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    m_twi_internal_bus_status = p_event->type;
    twi_handler(p_event, p_context);
    m_xfer_internal_done = true;
}

void external_twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    m_twi_external_bus_status = p_event->type;
    twi_handler(p_event, p_context);
    m_xfer_external_done = true;
}

void  twi_init (void)
{
    ret_code_t err_code;

    //There are two separate TWI buses that are used here. One for communication with
    //sensors that are on the current board, and one for communication with sensors that
    //are external to the board.
    const nrf_drv_twi_config_t twi_internal_config = {
       .scl                = BLE_33_SCL_PIN,
       .sda                = BLE_33_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = true
        };

    const nrf_drv_twi_config_t twi_external_config = {
       .scl                = EXTERNAL_SCL_PIN,
       .sda                = EXTERNAL_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = true
        };

    //A handler method is necessary to enable non-blocking mode. TXRX operations can only be carried 
    //out when non-blocking mode is enabled so a handler is needed here.
    err_code = nrf_drv_twi_init(&m_twi_internal, &twi_internal_config, internal_twi_handler, NULL);
    err_code = nrf_drv_twi_init(&m_twi_external, &twi_external_config, external_twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    //After initializing the buses, configure the necessary gpio pins to use these buses
    nrf_gpio_cfg_output(BLE_33_PULLUP); //send power to BLE 33 Sense pullup resistors (they aren't connected to VDD)
    nrf_gpio_cfg(BLE_33_SENSOR_POWER_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0H1, NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(EXTERNAL_SENSOR_POWER_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0H1, NRF_GPIO_PIN_NOSENSE);
}

static void leds_init()
{
    //configure the pins for the BLE 33 Sense on-board red, green and blue tri-colored LED.

    if (USE_EXTERNAL_LEDS)
    {
        nrf_gpio_cfg_output(EXTERNAL_RED_LED);
        nrf_gpio_cfg_output(EXTERNAL_BLUE_LED);
        nrf_gpio_cfg_output(EXTERNAL_GREEN_LED);
    }
    else
    {
        nrf_gpio_cfg_output(BLE_33_RED_LED);
        nrf_gpio_cfg_output(BLE_33_BLUE_LED);
        nrf_gpio_cfg_output(BLE_33_GREEN_LED);

        //these LEDs are situated backwards from what you would expect so the pin needs to be 
        //pulled high for the LEDs to turn off
        nrf_gpio_pin_set(BLE_33_RED_LED);
        nrf_gpio_pin_set(BLE_33_BLUE_LED);
        nrf_gpio_pin_set(BLE_33_GREEN_LED);
    }
    
}

/**@brief Function for application main entry.
 */
int main(void)
 {
    bool erase_bonds;

    // Initialize.
    log_init();
    timers_init();
    power_management_init();
    ble_stack_init();
    gatt_init();
    services_init();
    twi_init();
    sensors_init(true);
    gap_params_init();
    advertising_init();
    conn_params_init();
    peer_manager_init();
    leds_init();
  
    //NRF_LOG_FLUSH(); //flush out all logs called during initialization

    // Start execution.
    //NRF_LOG_INFO("Personal Caddie Initialized");
    //NRF_LOG_PROCESS();
    application_timers_start();

    advertising_start(erase_bonds);

    int adv_num = 0;

    // Enter main loop.
    for (;;)
    {
        idle_state_handle(); //puts CPU into sleep and waits for an event signal to wake it up

        //after the CPU get's woken up (at a minimum this should happen once every
        //connection interval), see if the data ready flag has been set to true and
        //if so, send out data notifications).
        if (m_data_ready)
        {
            characteristic_update_and_notify();
            m_data_ready = false;
            //SEGGER_RTT_printf(0, "Sending notification %d\n", ++adv_num);
        }
    }
}

static int32_t sensor_read_register(void *bus, uint8_t add, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    //This method allows for the reading of a sensor register(s). In most cases only a single register 
    //will be read, however, some sensors have a register auto-increment method so if the input read 
    //length is greater than 1 multiple registers can be read with a single command.
    ret_code_t err_code = 0;

    const nrf_drv_twi_xfer_desc_t sensor_read = {
        .address = add,
        .primary_length = 1,
        .secondary_length = (uint8_t)len,
        .p_primary_buf = &reg,
        .p_secondary_buf = bufp,
        .type =  NRF_DRV_TWI_XFER_TXRX};

    //set the m_xfer_done bool to false given the current TWI bus
    uint8_t instance = ((nrf_drv_twi_t const*)bus)->inst_idx;
    volatile bool * m_xfer_done;
    if (instance == INTERNAL_TWI_INSTANCE_ID) m_xfer_done = &m_xfer_internal_done; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)
    else m_xfer_done = &m_xfer_external_done;

    *m_xfer_done = false;

    do
    {
        err_code = nrf_drv_twi_xfer((nrf_drv_twi_t const*)bus, &sensor_read, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (*m_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}
static int32_t sensor_write_register(void *bus, uint8_t add, uint8_t reg, const uint8_t *bufp)
{
    //This method is for writing data to a single register for a sensor
    ret_code_t err_code = 0;

    uint8_t register_and_data[2] = {reg, bufp[0]};

    //the primary buffer will hold the register address, as well as the value to write into it.
    //for a single write operation, the secondary buffer has no use in a single write command
    const nrf_drv_twi_xfer_desc_t sensor_write = {
        .address = add,
        .primary_length = 2,
        .secondary_length = 0,
        .p_primary_buf = register_and_data,
        .p_secondary_buf = NULL,
        .type =  NRF_DRV_TWI_XFER_TX};

    //set the m_xfer_done bool to false given the current TWI bus
    uint8_t instance = ((nrf_drv_twi_t const*)bus)->inst_idx;
    volatile bool * m_xfer_done;
    if (instance == INTERNAL_TWI_INSTANCE_ID) m_xfer_done = &m_xfer_internal_done; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)
    else m_xfer_done = &m_xfer_external_done;

    *m_xfer_done = false;

    do
    {
        err_code = nrf_drv_twi_xfer((nrf_drv_twi_t const*)bus, &sensor_write, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (*m_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}

/**
 * @}
 */
