/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file  fxos8700_driver.c
 * @brief This file implements sensor interface module for FXOS8700, the 6-axis sensor
 *        with integrated linear accelerometer and magnetometer.
*/

/*******************************************************************************
 * Includes
 ******************************************************************************/
/* Component Lib Includes */
#include "fxos8700_driver.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*! @brief  The function to Initialize FXOS8700 sensor communication interface.
 */
uint8_t fxos8700_init(fxos8700_driver_t *pDriver)
{
  /* Initialize the sensor driver handler and interfaces */
    if(NULL == pDriver){
        return SENSOR_INVALIDPARAM_ERR;
    }
    sensor_comm_init(pDriver->pComHandle);
	return SENSOR_SUCCESS;
}


/*! @brief  The local function to set operating mode for the FXOS8700 sensor.
*/
uint8_t fxos8700_set_mode(fxos8700_driver_t *pDriver, fxos8700_mode_type_t sensorMode)
{
    uint8_t status = SENSOR_SUCCESS;

    /* Check for bad address. */
    if (NULL == pDriver)
    {
        return SENSOR_BAD_ADDRESS;
    }

    switch (sensorMode)
    {
    case FXOS8700_STANDBY_MODE:
        /*! Apply Register Configuration to set FXOS8700 into Standby mode. */
        status = sensor_burst_write(pDriver->pComHandle, gFxos8700StandbyModeConfig);
        if (SENSOR_SUCCESS != status)
        {
            return status;
        }

        break;
    case FXOS8700_ACTIVE_MODE:
        /*! Apply Register Configuration to set FXOS8700 into Active mode. */
        status = sensor_burst_write(pDriver->pComHandle, gFxos8700ActiveModeConfig);
        if (SENSOR_SUCCESS != status)
        {
            return status;
        }
        break;
    default:
        status = SENSOR_INVALIDPARAM_ERR;
        break;
    }
    return status;
}

/*! @brief fxos8700_read_reg
 */
uint8_t fxos8700_read_reg(fxos8700_driver_t *pDriver, uint16_t address, uint16_t nByteToRead, uint8_t *pReadBuffer)
{
	if((NULL == pDriver) || (NULL == pReadBuffer))
	{
		return SENSOR_INVALIDPARAM_ERR;
	}
	sensor_comm_read(pDriver->pComHandle, address, nByteToRead, pReadBuffer);
	return SENSOR_SUCCESS;
}

/*! @brief fxos8700_write_reg
 */
uint8_t fxos8700_write_reg(fxos8700_driver_t *pDriver, uint16_t address, uint16_t nByteToWrite, uint8_t *pWriteBuffer)
{

	if((NULL == pDriver) || (NULL == pWriteBuffer))
	{
		return SENSOR_INVALIDPARAM_ERR;
	}
	sensor_comm_write(pDriver->pComHandle, address, nByteToWrite, pWriteBuffer);
	return SENSOR_SUCCESS;
}

/*! @brief  The interface function to read FXOS8700 sensor data.
 */
uint8_t fxos8700_read_data(fxos8700_driver_t *pDriver, fxos8700_data_type_t dataType, fxos8700_data_t* pDataBuffer)
{
    uint8_t status;
    uint8_t dr_status = 0;
    uint8_t data[FXOS8700_ACCEL_DATA_SIZE * FIFO_SIZE];

    /* Check for bad address and invalid params. */
    if ((NULL == pDataBuffer) || (NULL == pDriver))
    {
        return SENSOR_BAD_ADDRESS;
    }

    switch (dataType)
    {
    case FXOS8700_ACCEL_14BIT_DATAREAD:
        /* Read the FXOS8700 status register and wait till new data is ready*/

        //TODO: The data ready register was taking much longer than the actual
        //ODR to update, as a result a single data read was taking close to 40 milliseconds
        //I've commented out waiting for the data ready register but should look into 
        //again in the future

        //status = fxos8700_read_reg(pDriver, FXOS8700_STATUS, 1, &dr_status);
        //if (SENSOR_SUCCESS != status)
        //{
        //    return status;
        //}
        
        //while (0 == (dr_status & FXOS8700_DR_STATUS_ZYXDR_MASK))
        //{
        //    status = fxos8700_read_reg(pDriver, FXOS8700_STATUS, 1, &dr_status);
        //    if (SENSOR_SUCCESS != status)
        //    {
        //        return status;
        //    }
        //}

        /* Received DataReady event, Read the FXOS8700 Accel samples*/
        status = sensor_burst_read(pDriver->pComHandle, gFxos8700ReadAccel, data);
        if (SENSOR_SUCCESS != status)
        {
            return status;
        }

        /*! Convert the raw sensor data to signed 16-bit container. */
        pDataBuffer->accel[0] = ((int16_t)data[0] << 8) | data[1];
        pDataBuffer->accel[0] /= 4;
        pDataBuffer->accel[1] = ((int16_t)data[2] << 8) | data[3];
        pDataBuffer->accel[1] /= 4;
        pDataBuffer->accel[2] = ((int16_t)data[4] << 8) | data[5];
        pDataBuffer->accel[2] /= 4;

        break;
    case FXOS8700_ACCEL_14BIT_FIFO_DATAREAD:
        /* Read the FXOS8700 status register and wait till new data is ready*/
                    status = fxos8700_read_reg(pDriver, FXOS8700_STATUS, 1, &dr_status);
        if (SENSOR_SUCCESS != status)
        {
            return status;
        }
                    while (0 == (dr_status & FXOS8700_F_STATUS_F_WMRK_FLAG_MASK))
                    {
                            status = fxos8700_read_reg(pDriver, FXOS8700_STATUS, 1, &dr_status);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }
                    }

        /* Received DataReady event, Read the FXOS8700 Accel samples*/
                    status = sensor_burst_read(pDriver->pComHandle, gFxos8700ReadAccelFifo, data);
                    if (SENSOR_SUCCESS != status)
                    {
                            return status;
                    }

            for (uint8_t i = 0; i < FIFO_SIZE; i++)
            {
                            /*! Convert the raw sensor data to signed 16-bit container. */
                            pDataBuffer->accel[i*3 + 0] = ((int16_t)data[i * FXOS8700_ACCEL_DATA_SIZE + 0] << 8) | data[i * FXOS8700_ACCEL_DATA_SIZE + 1];
                            pDataBuffer->accel[i*3 + 0] /= 4;
                            pDataBuffer->accel[i*3 + 1] = ((int16_t)data[i * FXOS8700_ACCEL_DATA_SIZE + 2] << 8) | data[i * FXOS8700_ACCEL_DATA_SIZE + 3];
                            pDataBuffer->accel[i*3 + 1] /= 4;
                            pDataBuffer->accel[i*3 + 2] = ((int16_t)data[i * FXOS8700_ACCEL_DATA_SIZE + 4] << 8) | data[i * FXOS8700_ACCEL_DATA_SIZE + 5];
                            pDataBuffer->accel[i*3 + 2] /= 4;
            }

                    break;
            case FXOS8700_ACCEL_8BIT_DATAREAD:

        /* Read the FXOS8700 status register and wait till new data is ready*/
                    status = fxos8700_read_reg(pDriver, FXOS8700_STATUS, 1, &dr_status);
        if (SENSOR_SUCCESS != status)
        {
            return status;
        }
                    while (0 == (dr_status & FXOS8700_DR_STATUS_ZYXDR_MASK))
                    {
                            status = fxos8700_read_reg(pDriver, FXOS8700_STATUS, 1, &dr_status);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }
                    }

        /* Received DataReady event, Read the FXOS8700 Accel samples*/
                    status = sensor_burst_read(pDriver->pComHandle, gFxos8700ReadAccel8bit, data);
                    if (SENSOR_SUCCESS != status)
                    {
                            return status;
                    }

                    /*! Convert the raw sensor data to signed 16-bit container. */
                    pDataBuffer->accel[0] = ((int16_t)data[0]);
                    pDataBuffer->accel[1] = ((int16_t)data[1]);
                    pDataBuffer->accel[2] = ((int16_t)data[2]);

                    break;
    case FXOS8700_MAG_DATAREAD:
        /* Read the FXOS8700 status register and wait till new data is ready*/
        //TODO: The data ready register was taking much longer than the actual
        //ODR to update, as a result a single data read was taking close to 40 milliseconds
        //I've commented out waiting for the data ready register but should look into 
        //again in the future
        //while (dr_status != 0xFF)
        //{
        //    status = fxos8700_read_reg(pDriver, FXOS8700_M_DR_STATUS, 1, &dr_status);
        //    if (SENSOR_SUCCESS != status)
        //    {
        //        return status;
        //    }
        //}

        status = sensor_burst_read(pDriver->pComHandle, gFxos8700ReadMag, data);
        if (SENSOR_SUCCESS != status)
        {
            return status;
        }

        /*! Convert the raw sensor data to signed 16-bit container. */
        pDataBuffer->mag[0] = ((int16_t)data[0] << 8) | data[1];
        pDataBuffer->mag[1] = ((int16_t)data[2] << 8) | data[3];
        pDataBuffer->mag[2] = ((int16_t)data[4] << 8) | data[5];

        break;
    case FXOS8700_ACCEL_MAG_HYBRID:

        /* Read the FXOS8700 status register and wait till new data is ready*/
                    status = fxos8700_read_reg(pDriver, FXOS8700_STATUS, 1, &dr_status);
        if (SENSOR_SUCCESS != status)
        {
            return status;
        }
                    while (0 == (dr_status & FXOS8700_DR_STATUS_ZYXDR_MASK))
                    {
                            status = fxos8700_read_reg(pDriver, FXOS8700_STATUS, 1, &dr_status);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }
                    }

                    status = sensor_burst_read(pDriver->pComHandle, gFxos8700ReadAccelMag, data);
                    if (SENSOR_SUCCESS != status)
                    {
                            return status;
                    }

                    /*! Convert the raw sensor data to signed 16-bit container. */
                    pDataBuffer->accel[0] = (int16_t)((data[0] << 8) | data[1]);
                    pDataBuffer->accel[0] /= 4;
                    pDataBuffer->accel[1] = (int16_t)((data[2] << 8) | data[3]);
                    pDataBuffer->accel[1] /= 4;
                    pDataBuffer->accel[2] = (int16_t)((data[4] << 8) | data[5]);
                    pDataBuffer->accel[2] /= 4;

                    pDataBuffer->mag[0] = (int16_t)((data[6] << 8) | data[7]);
                    pDataBuffer->mag[1] = (int16_t)((data[8] << 8) | data[9]);
                    pDataBuffer->mag[2] = (int16_t)((data[10] << 8) | data[11]);

                    break;
    default:
        status = SENSOR_INVALIDPARAM_ERR;

        break;
    }
    return status;
}

/*! @brief  The interface function to read FXOS8700 sensor events.
 */
uint8_t fxos8700_read_event(fxos8700_driver_t *pDriver, fxos8700_event_type_t eventType, uint8_t* eventVal)
{
	uint8_t status;
    uint8_t eventStatus;

	(* eventVal) = FXOS8700_NO_EVENT_DETECTED;

	switch (eventType)
	{
		case FXOS8700_FREEFALL:

			status = sensor_burst_read(pDriver->pComHandle, gFxos8700ReadFFMTSrc, &eventStatus);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            if (0 == (eventStatus & FXOS8700_A_FFMT_SRC_EA_MASK))
            { /* Return, if new event is not detected. */
              return SENSOR_INVALIDPARAM_ERR;
            }

            (* eventVal) = FXOS8700_FREEFALL_DETECTED;

			break;
		case FXOS8700_MOTION:

			status = sensor_burst_read(pDriver->pComHandle, gFxos8700ReadFFMTSrc, &eventStatus);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            if (0x80 == (eventStatus & FXOS8700_A_FFMT_SRC_EA_MASK))
            { /*! Motion event has been detected. */
            	(* eventVal) = FXOS8700_MOTION_DETECTED;
            }

			break;
		case FXOS8700_DOUBLETAP:

			status = sensor_burst_read(pDriver->pComHandle, gFxos8700ReadPulseSrc, &eventStatus);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            if (0x01 == (eventStatus & FXOS8700_PULSE_SRC_PLS_SRC_DPE_MASK))
            { /*! Double-Tap event has been detected. */
            	(* eventVal) = FXOS8700_DOUBLETAP_DETECTED;
            }

			break;
		case FXOS8700_ORIENTATION:

			status = sensor_burst_read(pDriver->pComHandle, gFxos8700ReadPLStatus, &eventStatus);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			if (((eventStatus & FXOS8700_PL_STATUS_NEWLP_MASK) == 0x80) &&
				((eventStatus & FXOS8700_PL_STATUS_LO_MASK) == 0x00))
			{
				uint8_t lp_orient = eventStatus & FXOS8700_PL_STATUS_LAPO_MASK;
				switch(lp_orient)
				{
					case 0x00:
						(* eventVal) = FXOS8700_PORTRAIT_UP;
						break;
					case 0x02:
						(* eventVal) = FXOS8700_PORTRAIT_DOWN;
						break;
					case 0x04:
						(* eventVal) = FXOS8700_LANDSCAPE_RIGHT;
						break;
					case 0x06:
						(* eventVal) = FXOS8700_LANDSCAPE_LEFT;
						break;
					default:
					    break;
				}
			}

			if (((eventStatus & FXOS8700_PL_STATUS_NEWLP_MASK) == 0x80) &&
				((eventStatus & FXOS8700_PL_STATUS_LO_MASK) == 0x40))
			{
				uint8_t bf_orient = eventStatus & FXOS8700_PL_STATUS_BAFRO_MASK;
				switch(bf_orient)
				{
					case 0x00:
						(* eventVal) = FXOS8700_FRONT_SIDE;
						break;
					case 0x01:
						(* eventVal) = FXOS8700_BACK_SIDE;
						break;
					default:
					    break;
				}
			}

			break;
        default:
            status = SENSOR_INVALIDPARAM_ERR;
            break;
	}
	return status;
}

/*! @brief  The interface function to apply FXOS8700 Accel configuration.
 */
uint8_t fxos8700_configure_accel(fxos8700_driver_t *pDriver, fxos8700_odr_t odr, fxos8700_power_mode_t powerMode, fxos8700_accel_config_type_t pConfig)
{
	uint8_t status = SENSOR_SUCCESS;

	/* Check for bad address. */
	if (NULL == pDriver)
	{
	    return SENSOR_BAD_ADDRESS;
	}

	/*! Prepare the register write list to configure FXOS8700 for required ODR and power mode. */
	registerwritelist_t fxos8700OdrSmodConfig[] = {
		/*! Configure FXOS8700 CTRL_REG1 Register "dr[2:0]" bit-fields to set ODR value. */
		{FXOS8700_CTRL_REG1, odr, FXOS8700_CTRL_REG1_DR_MASK},
		/*! Configure FXOS8700 CTRL REG2 Register "smods[1:0]" bit-fields to set power mode. */
		{FXOS8700_CTRL_REG2, powerMode, FXOS8700_CTRL_REG2_SMODS_MASK},
		__END_WRITE_DATA__};

	switch (pConfig)
	{
	case FXOS8700_ACCEL_8BIT_READ_POLL_MODE:
            /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for required ODR and SMOD */
            status = sensor_burst_write(pDriver->pComHandle, fxos8700OdrSmodConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for reading Accel 8-bit samples in Polling mode */
            status = sensor_burst_write(pDriver->pComHandle, gFxos87008bitAccelPollConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Set FXOS8700 into Active mode.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Successfully applied sensor configuration. */

            break;
	case FXOS8700_ACCEL_14BIT_READ_POLL_MODE:
            /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for required ODR and SMOD */
            status = sensor_burst_write(pDriver->pComHandle, fxos8700OdrSmodConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for reading Accel samples in Polling mode */
            status = sensor_burst_write(pDriver->pComHandle, gFxos8700AccelPollConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Set FXOS8700 into Active mode.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Successfully applied sensor configuration. */

            break;
	case FXOS8700_ACCEL_14BIT_READ_FIFO_MODE:
            /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for required ODR and SMOD */
            status = sensor_burst_write(pDriver->pComHandle, fxos8700OdrSmodConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for reading Accel samples in FIFO mode */
            status = sensor_burst_write(pDriver->pComHandle, gFxos8700AccelFifoConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Set FXOS8700 into Active mode.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Successfully applied sensor configuration. */

            break;
	case FXOS8700_ACCEL_14BIT_READ_INT_MODE:
            /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for required ODR and SMOD */
            status = sensor_burst_write(pDriver->pComHandle, fxos8700OdrSmodConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for reading Accel samples in INT mode */
            status = sensor_burst_write(pDriver->pComHandle, gFxos8700AccelInterruptConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Set FXOS8700 into Active mode.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Successfully applied sensor configuration. */

            break;
        default:
            status = SENSOR_INVALIDPARAM_ERR;

            break;
	}
    return status;
}

/*! @brief  The interface function to apply FXOS8700 Mag configuration.
 */
uint8_t fxos8700_configure_mag(fxos8700_driver_t *pDriver, fxos8700_odr_t odr, fxos8700_mag_config_type_t pConfig)
{
	uint8_t status = SENSOR_SUCCESS;

	/* Check for bad address. */
	if (NULL == pDriver)
	{
	    return SENSOR_BAD_ADDRESS;
	}

	/*! Prepare the register write list to configure FXOS8700 for required ODR. */
	registerwritelist_t fxos8700OdrSmodConfig[] = {
		/*! Configure FXOS8700 CTRL_REG1 Register "dr[2:0]" bit-fields to set ODR value. */
		{FXOS8700_CTRL_REG1, odr, FXOS8700_CTRL_REG1_DR_MASK},
		__END_WRITE_DATA__};

	switch (pConfig)
	{
	case FXOS8700_MAG_READ_POLLING_MODE:
            /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for required ODR and SMOD */
            status = sensor_burst_write(pDriver->pComHandle, fxos8700OdrSmodConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for reading Accel 8-bit samples in Polling mode */
            status = sensor_burst_write(pDriver->pComHandle, gFxos8700MagPollConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Set FXOS8700 into Active mode.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Successfully applied sensor configuration. */

            break;
	case FXOS8700_MAG_READ_INT_MODE:
            /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for required ODR and SMOD */
            status = sensor_burst_write(pDriver->pComHandle, fxos8700OdrSmodConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure FXOS8700 for reading Mag samples in INT mode */
            status = sensor_burst_write(pDriver->pComHandle, gFxos8700MagInterruptConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Set FXOS8700 into Active mode.*/
            status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Successfully applied sensor configuration. */

            break;
        default:
            status = SENSOR_INVALIDPARAM_ERR;

            break;
	}
    return status;
}

/*! @brief  The interface function to apply FXOS8700 in Hybrid mode configuration.
 */
uint8_t fxos8700_configure_hybrid(fxos8700_driver_t *pDriver, fxos8700_hybrid_odr_t odr, fxos8700_hybrid_config_type_t pConfig)
{
	uint8_t status = SENSOR_SUCCESS;

	/* Check for bad address. */
	if (NULL == pDriver)
	{
	    return SENSOR_BAD_ADDRESS;
	}

	/*! Prepare the register write list to configure FXOS8700 for required ODR and power mode. */
	registerwritelist_t fxos8700OdrSmodConfig[] = {
		/*! Configure FXOS8700 CTRL_REG1 Register "dr[2:0]" bit-fields to set ODR value. */
		{FXOS8700_CTRL_REG1, odr, FXOS8700_CTRL_REG1_DR_MASK},
		__END_WRITE_DATA__};

	switch (pConfig)
	{
		case FXOS8700_HYBRID_READ_POLL_MODE:

		    /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
			status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Apply Register Configuration to configure FXOS8700 for required ODR */
			status = sensor_burst_write(pDriver->pComHandle, fxos8700OdrSmodConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure FXOS8700 for reading Accel+Mag samples in Hybrid Poll mode */
			status = sensor_burst_write(pDriver->pComHandle, gFxos8700HybridPollConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set FXOS8700 into Active mode.*/
			status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
		case FXOS8700_HYBRID_READ_INT_MODE:

		    /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
			status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Apply Register Configuration to configure FXOS8700 for required ODR */
			status = sensor_burst_write(pDriver->pComHandle, fxos8700OdrSmodConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure FXOS8700 for reading Accel+Mag samples in Hybrid INT mode */
			status = sensor_burst_write(pDriver->pComHandle, gFxos8700HybridInterruptConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set FXOS8700 into Active mode.*/
			status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
        default:
            status = SENSOR_INVALIDPARAM_ERR;

            break;
	}
    return status;
}

/*! @brief  The interface function to apply embedded functionality configuration for FXOS8700 sensor.
 */
uint8_t fxos8700_set_embedded_function(fxos8700_driver_t *pDriver, fxos8700_embedded_func_config_type_t configMode)
{
	uint8_t status = SENSOR_SUCCESS;

	/* Check for bad address. */
	if (NULL == pDriver)
	{
	    return SENSOR_BAD_ADDRESS;
	}

	switch (configMode)
	{
		case FXOS8700_FREEFALL_DETECTION_MODE:

		    /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
			status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure FXOS8700 for Freefall detection mode */
			status = sensor_burst_write(pDriver->pComHandle, gFxos8700FreefallDetectConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set FXOS8700 into Active mode.*/
			status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
		case FXOS8700_MOTION_DETECTION_MODE:

		    /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
			status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure FXOS8700 for Motion detection mode */
			status = sensor_burst_write(pDriver->pComHandle, gFxos8700MotiontDetectConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set FXOS8700 into Active mode.*/
			status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
		case FXOS8700_ORIENT_DETECTION_MODE:

		    /*! Set FXOS8700 into standby mode so that configuration can be applied.*/
			status = fxos8700_set_mode(pDriver, FXOS8700_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure FXOS8700 for Orientation detection mode */
			status = sensor_burst_write(pDriver->pComHandle, gFxos8700OrientDetectConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set FXOS8700 into Active mode.*/
			status = fxos8700_set_mode(pDriver, FXOS8700_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
        default:
            status = SENSOR_INVALIDPARAM_ERR;

            break;
	}
	return status;
}

/*! @brief  The interface function to apply interrupt configuration for FXOS8700 sensor.
 */
uint8_t fxos8700_config_interrupt(fxos8700_driver_t *pDriver, fxos8700_interrupt_config_t *pConfig)
{
    if(NULL == pDriver)
	{
		return SENSOR_INVALIDPARAM_ERR;
	}
	uint8_t ctrlReg[3];
	uint8_t status = SENSOR_SUCCESS;


	/* Read the CTRL_REG3 and preserve the existing configuration bits of the control registers other than interrupt configuration bits. */
	status = sensor_comm_read(pDriver->pComHandle, FXOS8700_CTRL_REG3, 1, ctrlReg);
	if(status != SENSOR_SUCCESS)
	{
	    return status;
	}
	/* Update the Ctrl reg with polarity and open drain/push pull. */
	//ctrlReg[0] |= (*pConfig)& (FXOS8700_CTRL_REG3_PP_OD_MASK | FXOS8700_CTRL_REG3_IPOL_MASK));

	/* Enable the desired interrupt sources. */
	//ctrlReg[1] = pConfig->intSources;

	/* configure the interrupt routing */
	//ctrlReg[2]  = pConfig->int1_2 ;


	/* configure the interrupts sources with desired pin configuration setting for fxos8700 */
	status = sensor_comm_write(pDriver->pComHandle, FXOS8700_CTRL_REG3, 3, ctrlReg);
	if(status != SENSOR_SUCCESS)
	{
	    return status;
	}

    return status;
}

/*! @brief  The interface function to disable interrupt FXOS8700 sensor.
 */
uint8_t fxos8700_disable_interrupt(fxos8700_driver_t *pDriver, fxos8700_interrupt_source_t intSource)
{
	uint8_t status;

    if(NULL == pDriver)
	{
		return SENSOR_INVALIDPARAM_ERR;
	}
	uint8_t ctrlReg;
	status = sensor_comm_read(pDriver->pComHandle, FXOS8700_CTRL_REG4, 1, &ctrlReg);
	if(status != SENSOR_SUCCESS)
	{
	    return status;
	}
	ctrlReg &= ~intSource;

	/* Disable the interrupt sources configured */
	status = sensor_comm_write(pDriver->pComHandle, FXOS8700_CTRL_REG4, 1, &ctrlReg);
	if(status != SENSOR_SUCCESS)
	{
	    return status;
	}
    return status;
}
