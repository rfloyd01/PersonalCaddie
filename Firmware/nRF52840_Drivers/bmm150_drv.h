#ifndef FXAS21002_H__
#define FXAS21002_H__

#include <stdint.h>
#include <stdbool.h>
#include "Bosch/bmm150.h"
#include "sensor_settings.h"
#include "sensor_communication.h"

#ifdef __cplusplus
extern "C" {
#else

void bmm150init(imu_communication_t* comm, uint8_t sensors, uint8_t* settings);
#endif

int32_t bmm150_idle_mode_enable(bool init);
int32_t bmm150_active_mode_enable();

void bmm150_get_actual_settings();

//Implementations of Driver Functional Pointers
int8_t bmm150_read_register(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t bmm150_write_register(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
void bmm150_delay(uint32_t period, void *intf_ptr);

int32_t bmm150_get_data(uint8_t* pBuff, uint8_t offset);

#ifdef __cplusplus
}
#endif

#endif // FXAS21002_H__

/** @} */