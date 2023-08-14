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
#include "ble_sensor_service.h"
#include "personal_caddie_operating_modes.h"

//Bluetooth Parameters
#define DEVICE_NAME                     "Personal Caddie"                       /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "FloydInc."                             /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                2400                                    /**< The advertising interval (in units of 0.625 ms. This value corresponds to 1.5s). */

#define APP_ADV_DURATION                BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */
#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

static uint16_t current_connection_interval = 0;                                /**< Keeps track of the connection interval length (in milliseconds) */
static uint16_t sici = 1980;                                                    /**< The desired connection interval in sensor idle mode (in milliseconds) */
static uint16_t saci = 150;                                                     /**< The desired connection interval in sensor active mode (in milliseconds) */

#define SENSOR_IDLE_MIN_CONN_INTERVAL   MSEC_TO_UNITS(sici - 15, UNIT_1_25_MS)  /**< Minimum acceptable connection interval in sensor idle mode (1.95 seconds). */
#define SENSOR_IDLE_MAX_CONN_INTERVAL   MSEC_TO_UNITS(sici + 15, UNIT_1_25_MS)  /**< Maximum acceptable connection interval in sensor idle mode (2.05 seconds). */
#define SENSOR_IDLE_CONN_SUP_TIMEOUT    MSEC_TO_UNITS(6000, UNIT_10_MS)        /**< Connection supervisory timeout (4 seconds). */
#define SENSOR_ACTIVE_MIN_CONN_INTERVAL MSEC_TO_UNITS(saci - 15, UNIT_1_25_MS)  /**< Minimum acceptable connection interval in sensor active mode (0.125 seconds). */
#define SENSOR_ACTIVE_MAX_CONN_INTERVAL MSEC_TO_UNITS(saci + 15, UNIT_1_25_MS)  /**< Maximum acceptable connection interval in sensor active mode (0.25 seconds). */
#define SENSOR_ACTIVE_CONN_SUP_TIMEOUT  MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */
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
#define TWI_INSTANCE_ID     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID     1
#endif

const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
volatile bool m_xfer_done = false; //Indicates if operation on TWI has ended.

//IMU Sensor Parameters
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
static lsm9ds1_id_t whoamI;
static int measurements_taken = 0;

static int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t write_mag(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t read_mag(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void tx_com( uint8_t *tx_buffer, uint16_t len );
static int32_t get_IMU_data(uint8_t offset);
static int32_t get_MAG_data(uint8_t offset);

static personal_caddie_operating_mode_t current_operating_mode = ADVERTISING_MODE; /**< The chip starts in advertising mode*/

//Pin Setup
#define SENSOR_POWER_PIN        NRF_GPIO_PIN_MAP(1, 11)                         /**< Pin used to power external sensors (mapped to D2 on BLE 33)*/
#define SCL_PIN                 NRF_GPIO_PIN_MAP(1, 14)                         /**< Pin used for external TWI clock (mapped to D6 on BLE 33) */
#define SDA_PIN                 NRF_GPIO_PIN_MAP(1, 15)                         /**< Pin used for external TWI data (mapped to D4 on BLE 33) */
#define BLE_33_SENSOR_POWER_PIN NRF_GPIO_PIN_MAP(0, 22)                         /**< Pin used to power external sensors (mapped to D2 on BLE 33)*/
#define BLE_33_SCL_PIN          NRF_GPIO_PIN_MAP(0, 15)                         /**< Pin used for internal TWI clock for BLE33*/
#define BLE_33_SDA_PIN          NRF_GPIO_PIN_MAP(0, 14)                         /**< Pin used for internal TWI data for BLE33*/
#define BLE_33_PULLUP           NRF_GPIO_PIN_MAP(1, 0)                          /**< Pullup resistors on BLE 33 sense have separate power source*/
#define BLE_33_GREEN_LED        NRF_GPIO_PIN_MAP(1, 9)                          /**< Green LED Indicator on BLE 33 sense*/
#define BLE_33_RED_LED          NRF_GPIO_PIN_MAP(0, 24)                         /**< Red LED Indicator on BLE 33 sense*/
#define BLE_33_BLUE_LED         NRF_GPIO_PIN_MAP(0, 6)                          /**< Blue LED Indicator on BLE 33 sense*/
#define BLE_33_DARK_GREEN_LED   NRF_GPIO_PIN_MAP(0, 16)                         /**< Dark LED Indicator on BLE 33 sense*/
static volatile uint8_t active_led = BLE_33_BLUE_LED;                           /**< Variable used to keep track of which LED to turn on/off*/

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

/**@brief Function for the LSM9DS1 initialization.
 *
 * @details Initializes the LSM9DS1 accelerometer, gyroscope and magnetometer
 */
static void sensors_init(void)
{
    //first reset the sensor settings array
    for (int i = 0; i < SENSOR_SETTINGS_LENGTH; i++) sensor_settings[i] = 0;

    //configure the power pin and pullup resistor pins for the the sensors
    nrf_gpio_cfg_output(BLE_33_PULLUP); //send power to BLE 33 Sense pullup resistors (they aren't connected to VDD)
    nrf_gpio_cfg(BLE_33_SENSOR_POWER_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0H1, NRF_GPIO_PIN_NOSENSE);
    
    //Handle the initialization of individual sensors
    lsm9ds1_init(&lsm9ds1_imu, &lsm9ds1_mag, sensor_settings, SENSOR_SETTINGS_LENGTH, &m_twi, &m_xfer_done);

    //after sensor initialization is complete we attempt to communicate 
    //with each sensor just to ensure that it's there and working properly.
    //To do so, turn on the TWI bus and turn on the power pin(s) and pullup resistors
    nrf_drv_twi_enable(&m_twi);
    nrf_gpio_pin_set(BLE_33_PULLUP);
    nrf_gpio_pin_set(BLE_33_SENSOR_POWER_PIN);
    nrf_delay_ms(50); //slight delay so sensors have time to power on

    //attempt to read the WHOAMI register for each sensor
    uint32_t ret = lsm9ds1_dev_id_get(&lsm9ds1_mag, &lsm9ds1_imu, &whoamI);
    if (whoamI.imu == LSM9DS1_IMU_ID) SEGGER_RTT_WriteString(0, "LSM9DS1 discovered.\n");
    else SEGGER_RTT_WriteString(0, "Error: Couldn't find LSM9DS1.\n");

    //regardless of whether or not the LSM9DS1 is found, disable the power pins and TWI bus
    nrf_gpio_pin_clear(BLE_33_PULLUP);
    nrf_gpio_pin_clear(BLE_33_SENSOR_POWER_PIN);
    nrf_drv_twi_disable(&m_twi);

    //After all sensors have been initialized, update the sensor settings characteristic to 
    //reflect the sensor settings array
    ble_gatts_value_t settings;\
    settings.len = SENSOR_SETTINGS_LENGTH;
    settings.p_value = sensor_settings;
    uint32_t err_code = sd_ble_gatts_value_set(m_conn_handle, m_ss.settings_handles.value_handle, &settings);
    APP_ERROR_CHECK(err_code);
}

static void characteristic_update_and_notify()
{
    
    ble_gatts_hvx_params_t acc_notify_params, gyr_notify_params, mag_notify_params;
    memset(&acc_notify_params, 0, sizeof(acc_notify_params));
    memset(&gyr_notify_params, 0, sizeof(gyr_notify_params));
    memset(&mag_notify_params, 0, sizeof(mag_notify_params));

    uint16_t data_characteristic_size = SENSOR_SAMPLES * SAMPLE_SIZE; //not really sure why the size needs to be passed in as a reference

    //Setup accelerometer notification first
    acc_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    acc_notify_params.handle = m_ss.data_handles[0].value_handle;
    acc_notify_params.p_data = acc_characteristic_data;
    acc_notify_params.p_len  = &data_characteristic_size;
    acc_notify_params.offset = 0;

    //Setup gyroscope notification second
    gyr_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    gyr_notify_params.handle = m_ss.data_handles[1].value_handle;
    gyr_notify_params.p_data = gyr_characteristic_data;
    gyr_notify_params.p_len  = &data_characteristic_size;
    gyr_notify_params.offset = 0;

    //Setup magnetometer notification third
    mag_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    mag_notify_params.handle = m_ss.data_handles[2].value_handle;
    mag_notify_params.p_data = mag_characteristic_data;
    mag_notify_params.p_len  = &data_characteristic_size;
    mag_notify_params.offset = 0;

    sd_ble_gatts_hvx(m_conn_handle, &acc_notify_params);
    sd_ble_gatts_hvx(m_conn_handle, &gyr_notify_params);
    sd_ble_gatts_hvx(m_conn_handle, &mag_notify_params);
}

static void data_reading_timer_handler(void * p_context)
{
    //uint32_t time_stamp = nrf_drv_timer_capture(&my_timer, NRF_TIMER_CC_CHANNEL1);
    //float clock_conversion = 1.0 / 16000000.0;
    //float actual_time = time_stamp * clock_conversion;
    //SEGGER_RTT_printf(0, "TWI initiated at %u ticks of 16MHz.\n", time_stamp);

    //Everytime the data reading timer goes off we take sensor readings and then 
    //update the appropriate characteristic values.
    int measurements_taken = 0;
    while (measurements_taken < SENSOR_SAMPLES)
    {
        get_IMU_data(SAMPLE_SIZE * measurements_taken);
        get_MAG_data(SAMPLE_SIZE * measurements_taken);
        measurements_taken++;
    }

    //after the samples are read, update the characteristics and notify
    characteristic_update_and_notify();
}

static void led_timer_handler(void * p_context)
{
    //This timer causes the currently active LED to light up. A separate timer is used to 
    //turn the LED back off after a set time period
    nrf_gpio_pin_clear(active_led);
    nrf_drv_timer_clear(&LED_ON_TIMER); //reset the LED-on timer
    nrf_drv_timer_enable(&LED_ON_TIMER); //turn on the LED_on timer, the handler for this timer will turn the LED back off
    //uint32_t timer_val = nrf_drv_timer_capture(&LED_ON_TIMER, NRF_TIMER_CC_CHANNEL1);
}

void led_on_timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    //This handler gets called when an LED is turned to the active state. Simply turn the 
    //LED back off when this handler is called and then turn the LED-on timer off.
    nrf_gpio_pin_set(active_led);
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

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = SENSOR_IDLE_MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = SENSOR_IDLE_MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = SENSOR_IDLE_CONN_SUP_TIMEOUT;

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
    //In this mode, the TWI bus is active and power is going to the LSM9DS1 but the 
    //sensor is put into sleep mode. A red LED blinking in time with the connection 
    //interval shows when this mode is active.
    if (current_operating_mode == SENSOR_IDLE_MODE) return; //no need to change anything if already in idle mode

    active_led = BLE_33_RED_LED; //swap to the red LED
    ret_code_t err_code;

    if (current_operating_mode == SENSOR_ACTIVE_MODE)
    {
        //If we're transitioning from active to idle mode we need to stop the data collection timer
        //and then put the sensor into sleep mode. Once this is done we change the connection interval 
        //back to 2 seconds as we won't be communicating with the central as often (for a short while).
        err_code = app_timer_stop(m_data_reading_timer); //Even if the timer isn't actively on it's ok to call this method
        lsm9ds1_idle_mode_enable(&lsm9ds1_imu, &lsm9ds1_mag);

        //the connection interval should be less than 2 seconds, but still check to make sure
        if (current_connection_interval < SENSOR_IDLE_MIN_CONN_INTERVAL)
        {
            ret_code_t err_code = BLE_ERROR_INVALID_CONN_HANDLE;
            ble_gap_conn_params_t new_params;

            new_params.min_conn_interval = SENSOR_IDLE_MIN_CONN_INTERVAL;
            new_params.max_conn_interval = SENSOR_IDLE_MAX_CONN_INTERVAL;
            new_params.slave_latency = SLAVE_LATENCY;
            new_params.conn_sup_timeout = SENSOR_IDLE_CONN_SUP_TIMEOUT;

            err_code = ble_conn_params_change_conn_params(m_conn_handle, &new_params);

            if (err_code != NRF_SUCCESS)
            {
                SEGGER_RTT_WriteString(0, "Couldn't update connection interval.\n");
            }
        }

        //the LED is deactivated during data collection so turn the LED back on
        err_code = app_timer_start(m_led_timer, LED_DELAY, NULL); //Even if the timer is already on it's ok to call this method
    }
    else 
    {
        //If we aren't in sensor active or idle mode, it means that we arrived here from connection mode.
        //This means that both the TWI bus and LSM9DS1 need to be turned on. No need to put the LSM9DS1 
        //into sleep mode as this happens automatically upon power up.

        // Leave the below two lines commented for future reference
        //uint32_t enabled = m_twi.u.twim.p_twim->ENABLE;
        //if (enabled == 0) nrf_drv_twi_enable(&m_twi);

        //turn on the TWI bus
        nrf_drv_twi_enable(&m_twi);

        //Power the sensor and the pullup resistors to the SCL and SDA line
        nrf_gpio_pin_set(BLE_33_SENSOR_POWER_PIN);
        nrf_gpio_pin_set(BLE_33_PULLUP);
    }

    current_operating_mode = SENSOR_IDLE_MODE; //set the current operating mode to idle
}

static void sensor_active_mode_start()
{
    if (current_operating_mode != SENSOR_IDLE_MODE) return; //we can only go to active mode from idle mode

    //All of the settings for the sensor should have been set already, we just need to activate them
    //and then start the data collection timer. We also disable the LED to save on power
    app_timer_stop(m_led_timer); //disable the led by turning of it's timer

    //turn on the sensors by applying the current settings in the settings array
    lsm9ds1_active_mode_enable(&lsm9ds1_imu, &lsm9ds1_mag);

    //After turning on the sensor, request that the connection interval be changed
    //to match the ODR of the sensor * the number of samples being collected.
    float sensor_odr = lsm9ds1_odr_calculate(sensor_settings[ODR + GYR_START], sensor_settings[ODR + MAG_START]);

    if (sensor_odr != 0)
    {
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
        new_params.conn_sup_timeout = SENSOR_ACTIVE_CONN_SUP_TIMEOUT;

        err_code = ble_conn_params_change_conn_params(m_conn_handle, &new_params);

        if (err_code != NRF_SUCCESS)
        {
            SEGGER_RTT_WriteString(0, "Couldn't update connection interval.\n");
        }

        //Once the settings have been applied, restart the data collection timer.
        //The timer needs to be converted from ms to 'ticks' which match the frequency of 
        //the app timer. Normally this is done with a Macro but that can't be accessed here 
        //so the macro code is copied here
        uint64_t A = minimum_interval_required * (uint64_t)APP_TIMER_CLOCK_FREQ;
        uint64_t B = 1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1);
        uint32_t data_timer_delay = (((A) + ((B) / 2)) / (B));
        app_timer_start(m_data_reading_timer, data_timer_delay, NULL);
    }
    
    current_operating_mode = SENSOR_ACTIVE_MODE; //set the current operating mode to idle
}

static void connected_mode_start()
{
    //We'll other be put into this mode from advertising mode (in which case all we need to do is 
    //change the color of the blinking LED, or from sensor_idle mode, in which case we need to 
    //disable the power going to the sensor and the TWI bus.
    if (current_operating_mode == SENSOR_IDLE_MODE)
    {
        //The chip is currently in sensor idle mode so we need to power off the sensor and disable the TWI bus
        //Disable power to the sensor and the pullup resistors to the SCL and SDA line
        nrf_gpio_pin_clear(BLE_33_SENSOR_POWER_PIN);
        nrf_gpio_pin_clear(BLE_33_PULLUP);

        //Turn off the TWI bus
        nrf_drv_twi_disable(&m_twi);
    }

    //change the color of the blinking LED
    active_led = BLE_33_DARK_GREEN_LED; //swap to the red LED
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
            active_led = BLE_33_BLUE_LED;
            break;
    }

    current_operating_mode = ADVERTISING_MODE; //set the current operating mode to idle
    
    //finally, we need to terminate the existing connection
    //TODO: Implement this feature
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
            sensor_idle_mode_start();
            for (int i = 1; i < SENSOR_SETTINGS_LENGTH; i++) sensor_settings[i] = *(settings_state + i);
            break;
        case 4:
            sensor_idle_mode_start();
            sensor_settings[*(settings_state + 1)] = *(settings_state + 2);
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

    err_code = ble_sensor_service_init(&m_ss, &init);
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
            NRF_LOG_INFO("Fast advertising.");
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
            current_connection_interval = p_ble_evt->evt.gap_evt.params.connected.conn_params.max_conn_interval;
            SEGGER_RTT_WriteString(0, "Connected to the Personal Caddie.\n");
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            current_connection_interval = p_ble_evt->evt.gap_evt.params.conn_param_update.conn_params.min_conn_interval * 5 / 4;
            SEGGER_RTT_printf(0, "Connection Interval updated to : %u milliseconds.\n", current_connection_interval);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
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

    NRF_LOG_INFO("Erase bonds!");

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
            //NRF_LOG_INFO("Successfully reached slave address 0x%x.\n", p_event->xfer_desc.address);
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_TXRX | p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            {
                //uncomment below line after the data_handler function has been written
                //data_handler(m_data);
            }
            break;
        case NRF_DRV_TWI_EVT_ADDRESS_NACK:
            //NRF_LOG_INFO("Address NACK received while trying to reach slave address 0x%x.\n", p_event->xfer_desc.address);
            break;
        case NRF_DRV_TWI_EVT_DATA_NACK:
            //NRF_LOG_INFO("Data NACK received while trying to reach slave address 0x%x.\n", p_event->xfer_desc.address);
            break;
        default:
            //NRF_LOG_INFO("Something other than Event Done was achieved.");
            break;
    }

    NRF_LOG_PROCESS();
    m_xfer_done = true; //this must be set to true before another TWI transaction can occur
}

void  twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_lsm9ds1_config = {
       .scl                = BLE_33_SCL_PIN,
       .sda                = BLE_33_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    //A handler method is necessary to enable non-blocking mode. TXRX operations can only be carried 
    //out when non-blocking mode is enabled so a handler is needed here.
    err_code = nrf_drv_twi_init(&m_twi, &twi_lsm9ds1_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);
}

static void leds_init()
{
    //configure the pins for the BLE 33 Sense on-board red, green and blue tri-colored LED.
    nrf_gpio_cfg_output(BLE_33_RED_LED);
    nrf_gpio_cfg_output(BLE_33_BLUE_LED);
    nrf_gpio_cfg_output(BLE_33_DARK_GREEN_LED);

    //these LEDs are situated backwards from what you would expect so the pin needs to be 
    //pulled high for the LEDs to turn off
    nrf_gpio_pin_set(BLE_33_RED_LED);
    nrf_gpio_pin_set(BLE_33_BLUE_LED);
    nrf_gpio_pin_set(BLE_33_DARK_GREEN_LED);
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
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    peer_manager_init();
    twi_init();
    sensors_init();
    leds_init();
  
    NRF_LOG_FLUSH(); //flush out all logs called during initialization

    // Start execution.
    NRF_LOG_INFO("Personal Caddie Initialized");
    NRF_LOG_PROCESS();
    application_timers_start();

    advertising_start(erase_bonds);

    // Enter main loop.
    for (;;)
    {
        idle_state_handle(); //puts CPU into sleep and waits for an even signal to wake it up
    }
}

static void tx_com( uint8_t *tx_buffer, uint16_t len )
{
    int x = 5;
}

static int32_t get_IMU_data(uint8_t offset)
{
    //Reads the raw accelerometer and magnetometer data and stores it in their respective arrays
    int ret = lsm9ds1_read_reg(&lsm9ds1_imu, LSM9DS1_OUT_X_L_G, gyr_characteristic_data + offset, 6);

    if (ret == 0)
    {
        //SEGGER_RTT_printf(0, "Base Characteristic Address: 0x%#08x\n", acc_characteristic_data);
        //SEGGER_RTT_printf(0, "Address after offset: 0x%#08x\n", acc_characteristic_data + offset);
        ret = lsm9ds1_read_reg(&lsm9ds1_imu, LSM9DS1_OUT_X_L_XL, acc_characteristic_data + offset, 6);
    }

    return ret;
}

static int32_t get_MAG_data(uint8_t offset)
{
    //Reads the raw magnetometer data and stores it in its respective array
    int ret = lsm9ds1_read_reg(&lsm9ds1_mag, LSM9DS1_OUT_X_L_M, mag_characteristic_data + offset, 6);

    return ret;
}

/**
 * @}
 */
