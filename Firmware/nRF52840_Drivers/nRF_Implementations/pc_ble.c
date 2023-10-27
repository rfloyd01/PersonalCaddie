#include "pc_ble.h"
#include "app_error.h"
#include "app_timer.h"
#include "ble_sensor_service.h"
#include "SEGGER_RTT.h"

#define DEVICE_NAME                     "Personal Caddie"                       /**< Name of device. Will be included in the advertising data. */
#define APP_ADV_INTERVAL                2400                                    /**< The advertising interval (in units of 0.625 ms. This value corresponds to 1.5s). */

#define APP_ADV_DURATION                BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */
#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

static uint16_t sensor_connection_interval;                                     /**< Variable that holds the desired connection interval (in milliseconds) */

BLE_ADVERTISING_DEF(m_advertising);                                             /**< Advertising module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                         /**< Context for the Queued Write module.*/
NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */  

//Security Parameters
#define SEC_PARAM_BOND                  1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                       /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                      /**< Maximum encryption key size. */

//Connection Parameters
#define SENSOR_CONN_SUP_TIMEOUT         MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(1000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */


static ble_uuid_t m_adv_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}
};

static ble_uuid_t m_sr_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {SENSOR_SERVICE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN}
};

//Pointers
ble_event_handler_t m_ble_event_handlers;
uint16_t*           p_conn_handle; //A pointer to the connection handle for the active connection
volatile bool*      p_notification_done; //A pointer to a bool which lets us know when notification is complete
static uint8_t* p_total_sensor_samples;  //The connection interval needs to change to match the current number of samples we're collecting

void ble_stack_init(ble_event_handler_t* handler_methods, uint16_t* connection_handle, volatile bool* notifications_done, uint8_t* sensor_samples)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    //Populate the m_ble_event_handlers structure with the methods passed
    //into this method
    m_ble_event_handlers.gap_connected_handler = handler_methods->gap_connected_handler;
    m_ble_event_handlers.gap_disconnected_handler = handler_methods->gap_disconnected_handler;

    //Set a reference to the connection handle (the physical variable is in main.c
    //as there are other modules that need access to it)
    p_conn_handle = connection_handle;

    //Set a referene to the notification done boolean in main.c Like the connection
    //handle, there are other modules that reference this variable so we only keep 
    //a pointer to it
    p_notification_done = notifications_done;

    // Initialize Queued Write Module.
    nrf_ble_qwr_init_t        qwr_init = {0};
    
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
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

void advertising_init(void)
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

void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds(); //Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED event
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
    }
}

void conn_params_init(void)
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

uint32_t update_connection_interval(float sensor_odr)
{
    //We use this method to dynamically update the connection interval. This will get
    //called if the ODR of the sensors changes

    //TODO: This method should return an integer which represents the optimal
    //number of samples to store in the data characteristics
    int minimum_interval_required = 1000.0 / sensor_odr * (*p_total_sensor_samples) + 1; //this is in milliseconds, hence the 1000
    minimum_interval_required += (15 - minimum_interval_required % 15); //round up to the nearest 15th millisecond (this is necessary for iOS)

    //Convert from milliseconds to 1.25 millisecond units by dividing by 1.25 (same multiplying by 4/5)
    int mir_125 = minimum_interval_required / 5;
    mir_125 *= 4;

    //Now make the connection interval change request
    ret_code_t err_code = BLE_ERROR_INVALID_CONN_HANDLE;
    ble_gap_conn_params_t new_params;

    new_params.min_conn_interval = mir_125;
    new_params.max_conn_interval = mir_125 + 12; //must be 15 ms (or 12 in 1.25ms units) greater at a minimum to work with Apple Prodcuts
    new_params.slave_latency = SLAVE_LATENCY;
    new_params.conn_sup_timeout = SENSOR_CONN_SUP_TIMEOUT;

    //It's possible that the new parameters won't be applied as the Soft Device
    //is busy doing other stuff. In this case we need to wait until it's no longer
    //busy to apply the update.
    err_code = ble_conn_params_change_conn_params(*p_conn_handle, &new_params);
    while (err_code == NRF_ERROR_BUSY)
    {
        err_code = ble_conn_params_change_conn_params(*p_conn_handle, &new_params);
        SEGGER_RTT_WriteString(0, "SoftDevice Busy, retrying connection interval update.\n");
    }

    //We also need to update the preferred connection parameters, this will ensure that
    //we use the connection interval as specified in the sensor settings array in the case
    //that the connection is lost and re-established.
   
    //TODO: For some reason, the below call to ppcp_set() is working, however,
    //the desired connection interval isn't applied upon reconnection, the 
    //initial connection interval is. Why is this the case? To get around this,
    //just make a call to this update method inside the connection event
    err_code = sd_ble_gap_ppcp_set(&new_params);
    APP_ERROR_CHECK(err_code);

    return err_code;
}

void peer_manager_init(void)
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

static void delete_bonds(void)
{
    ret_code_t err_code;

    //NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

void gap_params_init(float current_sensor_odr)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    //calculate the connection interval based on the ODR of the sensor
    int minimum_interval_required = 1000.0 / current_sensor_odr * (*p_total_sensor_samples) + 1; //this is in milliseconds, hence the 1000
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

void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            SEGGER_RTT_WriteString(0, "Advertising Started (fast).\n");
            break;

        case BLE_ADV_EVT_IDLE:
            SEGGER_RTT_WriteString(0, "Idle Advertising engaged.\n");
            break;

        default:
            break;
    }
}

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

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            SEGGER_RTT_WriteString(0, "Disconnected from the Personal Caddie\n");
            //advertising_mode_start();
            m_ble_event_handlers.gap_disconnected_handler();
            break;

        case BLE_GAP_EVT_CONNECTED:
            *p_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, *p_conn_handle);
            APP_ERROR_CHECK(err_code);
            //connected_mode_start();
            //update_connection_interval(); //confirm that the correct connection interval is being used
            m_ble_event_handlers.gap_connected_handler();
            SEGGER_RTT_WriteString(0, "Connected to the Personal Caddie.\n");
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
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
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            *p_notification_done = true; //set the notification done bool to true to allow more notifications
            break;

        default:
            // No implementation needed.
            break;
    }
}

static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(*p_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}