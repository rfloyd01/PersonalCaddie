#ifndef PC_BLE_H__
#define PC_BLE_H__

#include <stdint.h>
#include <stdbool.h>

#include "ble_advertising.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "nrf_sdh.h"
#include "nrf_ble_qwr.h"
#include "ble_conn_params.h"
#include "nrf_ble_gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
This PersonalCaddie_BLE files hold all methods needed for initializing
the BLE stack. Most of these methods were just copied and pasted directly
from examples in the nRF SDK, but they've been moved to this location to 
reduce clutter in main.c
*/

// Forward declaration of the ble_event_handler_t type.
typedef struct ble_event_handler_s ble_event_handler_t;

typedef void (*pc_handler_t) (void); //Function pointer for methods to be passed to BLE handler

//Struct to hold function pointers for BLE Event handler
struct ble_event_handler_s
{
    pc_handler_t gap_connected_handler;   /**< This method will get called when a connection is first made. */
    pc_handler_t gap_disconnected_handler;   /**< This method will get called when a connection is lost. */
};

//BLE Stack Methods
void ble_stack_init(ble_event_handler_t* handler_methods, uint16_t* connection_handle, volatile bool* notifications_done, uint8_t* sensor_samples);

//Advertising/Connection Methods
void advertising_init(void);
void advertising_start(bool erase_bonds);
void conn_params_init(void);
uint32_t update_connection_interval(float sensor_odr);

//Peer/Bonding Methods
void peer_manager_init(void);
static void delete_bonds(void);

//GAP and GATT Methods
void gap_params_init(float current_sensor_odr);
void gatt_init(void);

//Handlers
static void on_adv_evt(ble_adv_evt_t ble_adv_evt);
static void pm_evt_handler(pm_evt_t const * p_evt);
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context);
static void nrf_qwr_error_handler(uint32_t nrf_error);
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt);
static void conn_params_error_handler(uint32_t nrf_error);

#ifdef __cplusplus
}
#endif

#endif // PC_BLE_H