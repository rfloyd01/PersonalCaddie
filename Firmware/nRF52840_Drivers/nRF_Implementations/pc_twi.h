#ifndef PC_TWI_H__
#define PC_TWI_H__

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_twi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
The PersonalCaddie_twi files hold all information necessary for communication 
over the TWI peripheral of the nRF52840. This includes initialization and 
turning on/off of two TWI buses, methods for reading/writing IMU sensors, handler 
methods, as well as a method for scanning both TWI buses for connected sensors.
*/

//Handler methods
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context, uint8_t twi_bus);
void internal_twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);
void external_twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);

//Init and De-init method
void twi_init();
void enable_twi_bus(int instance_id);
void disable_twi_bus(int instance_id);

//Get methods
const nrf_drv_twi_t* get_internal_twi_bus();
const nrf_drv_twi_t* get_external_twi_bus();
int get_internal_twi_bus_id();
int get_external_twi_bus_id();

//Reading/Writing Methods
void twi_address_scan(uint8_t* addresses, uint8_t* device_count, nrf_drv_twi_t const * bus);
int32_t sensor_read_register(void *bus, uint8_t add, uint8_t reg, uint8_t *bufp, uint16_t len);
int32_t sensor_write_register(void *bus, uint8_t add, uint8_t reg, const uint8_t *bufp);

#ifdef __cplusplus
}
#endif

#endif // PC_TWI_H