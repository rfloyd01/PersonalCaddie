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

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "lsm9ds1_reg.h"
#include "custom_services.h"

//Bluetooth Parameters
#define DEVICE_NAME                     "Personal_Caddie"                       /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "FloydInc."                             /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                2400                                    /**< The advertising interval (in units of 0.625 ms. This value corresponds to 1.5s). */

#define APP_ADV_DURATION                BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */
#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
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

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

static ble_os_t m_data_service;                                                 /**< Represents handle to custom data service. */

//static uint8_t m_data_service_uuid;
//static uint16_t m_data_service_handle;                                          /**<handle for data service. */
//#define DATA_UUID_SERVICE     0x1526 //uuid for data service
//#define DATA_UUID_CHAR        0x1527 //uuid for data characteristic
//#define DATA_SERVICE_UUID_BASE        {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, \
//                                       0xDE, 0xEF, 0x12, 0x69, 0x00, 0x00, 0x00, 0x00}

//TWI Parameters
#if TWI0_ENABLED
#define TWI_INSTANCE_ID     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID     1
#endif

static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
static volatile bool m_xfer_done = false; //Indicates if operation on TWI has ended.

//IMU Sensor Parameters
static stmdev_ctx_t lsm9ds1_imu;                                                /**< LSM9DS1 accelerometer/gyroscope instance. */
static stmdev_ctx_t lsm9ds1_mag;                                                /**< LSM9DS1 magnetometer instance. */

static int16_t data_raw_accelerometer[3];
static int16_t data_raw_gyroscope[3];
static int16_t data_raw_magnetometer[3];
static float acceleration_g[3];
static float angular_rate_dps[3];
static float magnetic_field_gauss[3];
static lsm9ds1_id_t whoamI;
static lsm9ds1_status_t reg;
static uint8_t rst;
static uint8_t tx_buffer[1000];
static bool lsm9ds1_register_auto_increment = true;                            /**register auto increment function for multiple byte reads of LSM9DS1 chip **/

static int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t write_mag(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t read_mag(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void tx_com( uint8_t *tx_buffer, uint16_t len );
static int32_t get_IMU_data();
static int32_t get_MAG_data();

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

// YOUR_JOB: Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}
};

static ble_uuid_t m_sr_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {DATA_SERVICE_BLE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN}
};


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


/**@brief Function for handling Peer Manager events.
 *
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
static void lsm9ds1_init(void)
{
    //initialize read/write methods for acc./gyro.
    lsm9ds1_imu.read_reg = read_imu;
    lsm9ds1_imu.write_reg = write_imu;

    //initialize read/write methods for mag.
    lsm9ds1_mag.read_reg = read_mag;
    lsm9ds1_mag.write_reg = write_mag;

    //before trying to reach the LSM9DS1 via TWI we need to actually power it on.
    //The BLE33 sense board has multiple sensor tied to the same power pin so high drive 
    //mode needs to be enabled to ensure enough current gets to the LSM9DS1

    //uncomment to use external sensors
    //nrf_gpio_cfg_output(SENSOR_POWER_PIN);
    //nrf_gpio_pin_set(SENSOR_POWER_PIN);

    nrf_gpio_cfg_output(BLE_33_PULLUP); //send power to BLE 33 Sense pullup resistors (they aren't connected to VDD)
    nrf_gpio_pin_set(BLE_33_PULLUP);    //send power to BLE 33 Sense pullup resistors (they aren't connected to VDD)
    nrf_gpio_cfg(BLE_33_SENSOR_POWER_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0H1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_set(BLE_33_SENSOR_POWER_PIN);
    nrf_delay_ms(50); //slight delay so the chip has time to power on

    //confirm that the sensor is powered on, read the whoami register and then turn
    //on one of the board leds after a successful read
    uint32_t ret = lsm9ds1_dev_id_get(&lsm9ds1_mag, &lsm9ds1_imu, &whoamI);
    if (whoamI.imu == LSM9DS1_IMU_ID)
    {
        nrf_gpio_cfg_output(BLE_33_RED_LED); //Pin needs to be set low to turn led on
    }

    //set the odr
    ret = lsm9ds1_imu_data_rate_set(&lsm9ds1_imu, LSM9DS1_IMU_119Hz);
    ret = lsm9ds1_mag_data_rate_set(&lsm9ds1_mag, LSM9DS1_MAG_MP_80Hz);

    //set the full scale ranges
    ret = lsm9ds1_gy_full_scale_set(&lsm9ds1_imu, LSM9DS1_2000dps); // +/-2000 deg/s
    ret = lsm9ds1_xl_full_scale_set(&lsm9ds1_imu, LSM9DS1_4g);      // +/- 4 g
    ret = lsm9ds1_mag_full_scale_set(&lsm9ds1_mag, LSM9DS1_4Ga);    // +/- 4 gauss
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

    // Create timers.

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
       ret_code_t err_code;
       err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
       APP_ERROR_CHECK(err_code); */
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

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

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


/**@brief Function for handling the YYY Service events.
 * YOUR_JOB implement a service handler function depending on the event the service you are using can generate
 *
 * @details This function will be called for all YY Service events which are passed to
 *          the application.
 *
 * @param[in]   p_yy_service   YY Service structure.
 * @param[in]   p_evt          Event received from the YY Service.
 *
 *
static void on_yys_evt(ble_yy_service_t     * p_yy_service,
                       ble_yy_service_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_YY_NAME_EVT_WRITE:
            APPL_LOG("[APPL]: charact written with value %s. ", p_evt->params.char_xx.value.p_str);
            break;

        default:
            // No implementation needed.
            break;
    }
}
*/

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    ret_code_t         err_code;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize custom services
    data_service_init(&m_data_service);
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
    /* YOUR_JOB: Start your timers. below is an example of how to start a timer.
       ret_code_t err_code;
       err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
       APP_ERROR_CHECK(err_code); */

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
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
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
            NRF_LOG_INFO("Disconnected.");
            // LED indication will be changed when advertising starts.
            break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
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
    //init.advdata.include_appearance      = true;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    //original code when there was only one service
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

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
    //I'm not positive on this yet but it seems like any error codes on TWI line are sent to this handler
    //when an error occurs somewhere incode, err_code is always coming back as NRF_SUCCESS even though this handler is
    //receiving an Address NACK. Need a good way to handle these situations

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
            NRF_LOG_INFO("Address NACK received while trying to reach slave address 0x%x.\n", p_event->xfer_desc.address);
            break;
        case NRF_DRV_TWI_EVT_DATA_NACK:
            NRF_LOG_INFO("Data NACK received while trying to reach slave address 0x%x.\n", p_event->xfer_desc.address);
            break;
        default:
            NRF_LOG_INFO("Something other than Event Done was achieved.");
            break;
    }
    //NRF_LOG_FLUSH();
    NRF_LOG_PROCESS();
    m_xfer_done = true;
}

void  twi_init (void)
{
    ret_code_t err_code;

    //const nrf_drv_twi_config_t twi_lsm9ds1_config = {
    //   .scl                = SCL_PIN,
    //   .sda                = SDA_PIN,
    //   .frequency          = NRF_DRV_TWI_FREQ_400K,
    //   .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
    //   .clear_bus_init     = false
    //};

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

    nrf_drv_twi_enable(&m_twi);
}

/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;

    // Initialize.
    log_init();
    timers_init();
    buttons_leds_init(&erase_bonds);
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    peer_manager_init();
    twi_init();

    //Once all nRF and BLE initializations are complete, turn on the on-board LED to show
    //that the BLE 33 sense is recieving power
    nrf_gpio_cfg_output(BLE_33_GREEN_LED);
    nrf_gpio_pin_set(BLE_33_GREEN_LED);

    lsm9ds1_init();
    NRF_LOG_FLUSH(); //flush out all logs called during initialization

    // Start execution.
    NRF_LOG_INFO("Template example started.");
    NRF_LOG_PROCESS();
    application_timers_start();

    advertising_start(erase_bonds);

    // Enter main loop.
    for (;;)
    {
        idle_state_handle(); //might need to comment this out, I think CPU goes to sleep here and is only woken up from an event trigger
        get_IMU_data();
        get_MAG_data();
    }
}

static int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    //The steps for a single byte read are as follows:
    //  1. nRF52840 sends start signal by pulling the SDA line low
    //  2. nRF52840 sends 7-bit slave address followed by a 0 (write) bit
    //  3. LSM9DS1 responds with an Acknowledge
    //  4. nRF52840 sends the 8-bit address of the register it wants to read
    //  5. LSM9DS1 responds with an Acknowledge
    //  6. nRF52840 sends a repeated start signal
    //  7. nRG52840 sends 7-bit slave address followed by a 1 (read) bit
    //  8. LSM9DS1 responds with an Acknowledge
    //  9. LSM9DS1 sends the requested data
    // 10. nRF52840 responds with a non-Acknowledge and then sends the stop signal
    //Since reading the LSM9DS1 requires a repeated start signal the NRF_DRV_TWI_XFER_TXRX transfer type is used
    ret_code_t err_code = 0;

    //the primary buffer only holds the address of the register we want to read and therefore only has a length of 1 byte
    //the secondary buffer is where the data will get read into
    const nrf_drv_twi_xfer_desc_t lsm9ds1_imu_read = {
        .address = (LSM9DS1_IMU_I2C_ADD_H >> 1),
        .primary_length = 1,
        .secondary_length = (uint8_t)len,
        .p_primary_buf = &reg,
        .p_secondary_buf = bufp,
        .type =  NRF_DRV_TWI_XFER_TXRX};

    m_xfer_done = false; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)

    do
    {
        err_code = nrf_drv_twi_xfer(&m_twi, &lsm9ds1_imu_read, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (m_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}
static int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    ret_code_t err_code = 0;

    uint8_t register_and_data[2] = {reg, bufp[0]};

    //the primary buffer will hold the register address, as well as the value to write into it.
    //for a single write operation, the secondary buffer has no use in a single write command
    const nrf_drv_twi_xfer_desc_t lsm9ds1_imu_write = {
        .address = (LSM9DS1_IMU_I2C_ADD_H >> 1),
        .primary_length = 2,
        .secondary_length = 0,
        .p_primary_buf = register_and_data,
        .p_secondary_buf = NULL,
        .type =  NRF_DRV_TWI_XFER_TX};

    m_xfer_done = false; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)

    do
    {
        err_code = nrf_drv_twi_xfer(&m_twi, &lsm9ds1_imu_write, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (m_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}
static int32_t read_mag(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    ret_code_t err_code = 0;

    //the logic for handling multiple byte reads must be handled in software for the magnetometer, whereas, the accelerometer
    //and gyroscope can do it via hardware
    const nrf_drv_twi_xfer_desc_t lsm9ds1_mag_read = {
        .address = (((uint8_t)lsm9ds1_register_auto_increment << 7) | (LSM9DS1_MAG_I2C_ADD_H >> 1)),
        .primary_length = 1,
        .secondary_length = (uint8_t)len,
        .p_primary_buf = &reg,
        .p_secondary_buf = bufp,
        .type =  NRF_DRV_TWI_XFER_TXRX};

    m_xfer_done = false; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)

    do
    {
        err_code = nrf_drv_twi_xfer(&m_twi, &lsm9ds1_mag_read, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (m_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}
static int32_t write_mag(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    ret_code_t err_code = 0;

    uint8_t register_and_data[2] = {reg, bufp[0]};

    //the primary buffer will hold the register address, as well as the value to write into it.
    //for a single write operation, the secondary buffer has no use in a single write command
    const nrf_drv_twi_xfer_desc_t lsm9ds1_mag_write = {
        .address = (LSM9DS1_MAG_I2C_ADD_H >> 1),
        .primary_length = 2,
        .secondary_length = 0,
        .p_primary_buf = register_and_data,
        .p_secondary_buf = NULL,
        .type =  NRF_DRV_TWI_XFER_TX};

    m_xfer_done = false; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)

    do
    {
        err_code = nrf_drv_twi_xfer(&m_twi, &lsm9ds1_mag_write, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (m_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}

static void tx_com( uint8_t *tx_buffer, uint16_t len )
{
    int x = 5;
}

static int32_t get_IMU_data()
{
    //Reads the raw accelerometer and magnetometer data and stores it in their respective arrays
    int ret = 0; 

    if (lsm9ds1_register_auto_increment)
    {
        ret = lsm9ds1_angular_rate_raw_get(&lsm9ds1_imu, data_raw_gyroscope);

        if (ret == 0)
        {
            ret = lsm9ds1_acceleration_raw_get(&lsm9ds1_imu, data_raw_accelerometer);

            if (ret == 0)
            {
                //this is temporary to make sure things are working as intended, convert the raw data to actual data.
                //actual raw data conversion will happen on the application side
                for (int i = 0; i < 3; i++)
                {
                    angular_rate_dps[i] = lsm9ds1_from_fs2000dps_to_mdps(data_raw_gyroscope[i]) / 1000.0;
                    acceleration_g[i]   = lsm9ds1_from_fs4g_to_mg(data_raw_accelerometer[i]) / 1000.0; 
                }
            }
        }
    }
    else
    {
        //implement this at some point
    }
    
    

    return ret;
}

static int32_t get_MAG_data()
{
    //Reads the raw magnetometer data and stores it in its respective array
    int ret = 0;
    
    if (lsm9ds1_register_auto_increment)
    {
        ret = lsm9ds1_magnetic_raw_get(&lsm9ds1_mag, data_raw_magnetometer);

        if (ret == 0)
            {
                //this is temporary to make sure things are working as intended, convert the raw data to actual data.
                //after testing, actual raw data conversion will happen on the application side
                for (int i = 0; i < 3; i++)
                {
                    magnetic_field_gauss[i] = 1000 * lsm9ds1_from_fs4gauss_to_mG(data_raw_magnetometer[i]);
                }
            }
    }
    else
    {
        //implement this at some point
    }

    return ret;
}

/**
 * @}
 */
