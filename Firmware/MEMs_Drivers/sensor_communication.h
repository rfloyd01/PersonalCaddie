#ifndef SENSOR_COMMUNICATION_H
#define SENSOR_COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "nrf_drv_twi.h"

typedef int32_t(*update_ptr)(uint8_t);
typedef int32_t(*read_register_ptr)(void*, uint8_t, uint8_t, uint8_t*, uint16_t);
typedef int32_t(*write_register_ptr)(void*, uint8_t, uint8_t, const uint8_t*);

typedef int32_t(*stmdev_write_ptr)(void*, uint8_t, const uint8_t*, uint16_t);
    typedef int32_t(*stmdev_read_ptr)(void*, uint8_t, uint8_t*, uint16_t);

//a struct that holds info for communication with an individual sensor
typedef struct
{
    uint8_t               address;         //i2c slave address of sensor
    nrf_drv_twi_t const * twi_bus;         //pointer to the twi bus used for communication
    update_ptr            update_settings; //pointer to function that updates sensor settings
    read_register_ptr     read_register;   //pointer to method for reading sensor registers
    write_register_ptr    write_register;  //pointer to method for writing sensor registers
    void*                 other;           //an extra customizable pointer
} sensor_communication_t;

//a struct that holds info for communication with multiple sensors
typedef struct
{
    sensor_communication_t acc_comm;
    sensor_communication_t gyr_comm;
    sensor_communication_t mag_comm;
} imu_communication_t;

#endif /* SENSOR_COMMUNICATION_H */