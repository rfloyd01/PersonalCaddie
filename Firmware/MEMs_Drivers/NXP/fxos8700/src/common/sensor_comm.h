/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file  sensor_comm.h
 * @brief This header contains common definitions for sensor communication interface function.
   This is virtual definition of the sensor communication interface depends on how it connected to MCU peripherals	
*/

#ifndef SENSOR_COMM_H_
#define SENSOR_COMM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NULL 0
/*******************************************************************************
 * Typedefs
 ******************************************************************************/

/*!
 * @brief Sensor Interface Error Type.
 */
typedef struct sensor_comm_handle
{
    void *pComm;
    uint8_t* parm;
}sensor_comm_handle_t;
/*******************************************************************************
 * Constants
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
uint8_t sensor_comm_init(sensor_comm_handle_t *pComHandle);
uint8_t sensor_comm_write(sensor_comm_handle_t *pComHandle, uint16_t offset, uint16_t size, uint8_t *pWritebuffer);
uint8_t sensor_comm_read(sensor_comm_handle_t *pComHandle, uint16_t offset, uint16_t size, uint8_t *pReadbuffer);
#endif /* SENSOR_COMM_H_ */
