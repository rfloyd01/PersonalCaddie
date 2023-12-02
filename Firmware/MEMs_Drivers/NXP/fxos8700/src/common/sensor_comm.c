/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file sensor_comm.c
 * @brief The sensor_comm.c file implements the sensor communication virtual interface. 
   User application needs to implement/call underlying SDK communication interfaces such i2c/SPI. 
   This is the SDK agnostic layer for the sensor for communication.
 */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "sensor_comm.h"
#include "sensor_communication.h"

//-----------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------
uint8_t sensor_comm_init(sensor_comm_handle_t *pComHandle)
{
    //I have my own custom init function so didn't end up populating this one with anything.
    return 0;
}

uint8_t sensor_comm_write(sensor_comm_handle_t *pComHandle, uint16_t offset, uint16_t size, uint8_t *pWritebuffer)
{
    //A method for writing to registers of the FXOS8700. This is basically a pass through function that utilizes my 
    //own TWI write method.

    //Note* - Looking at calls to this function from the driver, it seems that the offset variable is actually the address of 
    //the register that we want to write. Not sure why the name is offset, or why it's a 16 bit number instead of 8-bit
    sensor_communication_t* sensor_comm = (sensor_communication_t*)pComHandle->pComm; //import my custom communication struct
    return sensor_comm->write_register((void*)sensor_comm->twi_bus, sensor_comm->address, (uint8_t)offset, pWritebuffer, 1);
}

uint8_t sensor_comm_read(sensor_comm_handle_t *pComHandle, uint16_t offset, uint16_t size, uint8_t *pReadbuffer)
{
    //A method for writing to registers of the FXOS8700. This is basically a pass through function that utilizes my 
    //own TWI write method.

    //Note* - Looking at calls to this function from the driver, it seems that the offset variable is actually the address of 
    //the register that we want to write. Not sure why the name is offset, or why it's a 16 bit number instead of 8-bit
    sensor_communication_t* sensor_comm = (sensor_communication_t*)pComHandle->pComm; //import my custom communication struct
    return sensor_comm->read_register((void*)sensor_comm->twi_bus, sensor_comm->address, (uint8_t)offset, pReadbuffer, size);
}
