/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file  fxos8700.h
 * @brief This header contains definitions and metadata required for sensor interface module
 *        for FXOS8700
*/
#ifndef FXOS8700_DRIVER_H_
#define FXOS8700_DRIVER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fxos8700_regdef.h"
#include "common/sensor_common.h"
#include "fxos8700_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NUM_AXES                       (3U)
#define FXOS8700_ACCEL_DATA_SIZE       (6U)   /* 2 byte X,Y,Z ACCEL */
#define FXOS8700_ACCEL_8BITDATA_SIZE   (3U)   /* 1 byte (MSB) X,Y,Z ACCEL */
#define FXOS8700_MAG_DATA_SIZE         (6U)   /* 2 byte X,Y,Z MAG */
#define FXOS8700_ACCEL_MAG_DATA_SIZE   (FXOS8700_ACCEL_DATA_SIZE + FXOS8700_MAG_DATA_SIZE)

/*******************************************************************************
 * Typedefs
 ******************************************************************************/

/*!
 * @brief FXOS8700 Sensor Embedded Functionality Configurations
 * @see   Section#12: https://www.nxp.com/files-static/sensors/doc/data_sheet/FXOS8700CQ.pdf
 */
typedef enum fxos8700_embedded_func_config_type
{
    /*!< Auto-wake/sleep mode */
    FXOS8700_AUTOWAKE_SLEEP             = 1U,  /*!< FXOS8700 Register Configuration to configure sensor for Auto-wake/sleep mode. */

    /*!< Accelerometer freefall and motion event detection */
    FXOS8700_FREEFALL_DETECTION_MODE    = 2U, /*!< FXOS8700 Register Configuration to configure sensor for Freefall detection mode. */
    FXOS8700_MOTION_DETECTION_MODE      = 3U, /*!< FXOS8700 Register Configuration to configure sensor for Motion detectionmode. */

    /*!< Transient detection */
    FXOS8700_TRANSIENT_DETECTION_MODE   = 4U, /*!< FXOS8700 Register Configuration to configure sensor for Transient detection mode. */

    /*!< Pulse detection */
    FXOS8700_PULSE_DETECTION_MODE       = 5U, /*!< FXOS8700 Register Configuration to configure sensor for Pulse detection mode. */

    /*!< Orientation detection */
    FXOS8700_ORIENT_DETECTION_MODE      = 6U, /*!< FXOS8700 Register Configuration to configure sensor for detecting change in orientation. */

    /*!< Acceleration vector-magnitude detection */
    FXOS8700_ACCEL_VM_DETECTION_MODE    = 7U, /*!< FXOS8700 Register Configuration to configure sensor for acceleration vector-magnitude detection mode. */

    /*!< Magnetic vector-magnitude detection */
    FXOS8700_MAG_VM_DETECTION_MODE      = 8U, /*!< FXOS8700 Register Configuration to configure sensor for magnectic vector-magnitude detection mode. */

    /*!< Magnetic threshold detection */
    FXOS8700_MAG_THS_DETECTION_MODE     = 9U, /*!< FXOS8700 Register Configuration to configure sensor for magnectic threshold detection mode. */

    /*!< Magnetic min/max detection */
    FXOS8700_MAG_MINMAX_DETECTION_MODE  = 10U, /*!< FXOS8700 Register Configuration to configure sensor for magnectic min/max detection mode. */
	
    FXOS8700_EMBEDDED_FUNCT_CONFIG_END

} fxos8700_embedded_func_config_type_t;

/*!
 * @brief FXOS8700 Accel Configurations
 */
typedef enum fxos8700_accel_config_type
{
	/*!< 8-bit or 14-bit accelerometer data. */
    FXOS8700_ACCEL_8BIT_READ_POLL_MODE  = 0U,  /*!< FXOS8700 Register Configuration to configure sensor for reading Accel 8-bit samples in polling mode. */
    FXOS8700_ACCEL_14BIT_READ_POLL_MODE = 1U,  /*!< FXOS8700 Register Configuration to configure sensor for reading Accel 14-bit samples in polling mode. */
    FXOS8700_ACCEL_14BIT_READ_FIFO_MODE = 2U,  /*!< FXOS8700 Register Configuration to configure sensor for reading Accel 14-bit samples in FIFO mode. */
    FXOS8700_ACCEL_14BIT_READ_INT_MODE  = 3U,  /*!< FXOS8700 Register Configuration to configure sensor for reading Accel 14-bit samples in Interrupt mode. */
    FXOS8700_ACCEL_CONFIG_END

} fxos8700_accel_config_type_t;

/*!
 * @brief FXOS8700 Mag Configurations
 */
typedef enum fxos8700_mag_config_type
{
    /*!< Magnetometer data read */
    FXOS8700_MAG_READ_POLLING_MODE      = 4U, /*!< FXOS8700 Register Configuration to configure sensor for reading Mag samples in polling mode. */
    FXOS8700_MAG_READ_INT_MODE          = 5U, /*!< FXOS8700 Register Configuration to configure sensor for reading Mag samples in Interrupt mode. */

} fxos8700_mag_config_type_t;

/*!
 * @brief FXOS8700 Mag Configurations
 */
typedef enum fxos8700_hybrid_config_type
{
    /*!< Hybrid mode */
    FXOS8700_HYBRID_READ_POLL_MODE      = 6U, /*!< FXOS8700 Register Configuration to configure sensor for reading Accel+Mag samples in polling mode. */
    FXOS8700_HYBRID_READ_INT_MODE       = 7U, /*!< FXOS8700 Register Configuration to configure sensor for reading Accel+Mag samples in Interrupt mode. */
    FXOS8700_HYBRID_CONFIG_END

} fxos8700_hybrid_config_type_t;

/*!
 * @brief FXOS8700 Sensor Data Type
 */
typedef enum fxos8700_data_type
{
    FXOS8700_ACCEL_14BIT_DATAREAD       = 0U, /*!< Accelerometer data read in 14-bit mode. */
    FXOS8700_ACCEL_14BIT_FIFO_DATAREAD  = 1U,
    FXOS8700_ACCEL_8BIT_DATAREAD        = 2U, /*!< Accelerometer data read in 8-bit mode. */
    FXOS8700_ACCEL_MAG_HYBRID           = 3U, /*!< Hybrid mode, both accelerometer and magnetometer sensors are active. */
    FXOS8700_MAG_DATAREAD               = 4U, /*!< Magnetometer data read.*/
} fxos8700_data_type_t;

/*!
 * @brief FXOS8700 Sensor Event Type
 */
typedef enum fxos8700_event_type
{
    FXOS8700_FREEFALL                   = 0U, /*!< Freefall detection. */
    FXOS8700_MOTION                     = 1U, /*!< Motion detection. */
    FXOS8700_TRANSIENT                  = 2U, /*!< Transient detection. */
    FXOS8700_DOUBLETAP                  = 3U, /*!< Double Tap Pulse detection. */
    FXOS8700_ORIENTATION                = 4U, /*!< Orientation change detection. */
    FXOS8700_ACCEL_VECTOR_MAGNITUDE     = 5U, /*!< Acceleration vector-magnitude detection.*/
    FXOS8700_MAG_VECTOR_MAGNITUDE       = 6U, /*!< Magnetic vector-magnitude detection.*/
    FXOS8700_MAG_THRESHOLD              = 7U, /*!< Magnetic threshold detection.*/
    FXOS8700_MAG_MINMAX                 = 8U, /*!< Magnetic min/max detection.*/
} fxos8700_event_type_t;

/*!
 * @brief FXOS8700 Sensor Event Status Type
 */
typedef enum fxos8700_event_status_type
{
    FXOS8700_NO_EVENT_DETECTED          = 0U,  /*!< No event detected. */
    FXOS8700_FREEFALL_DETECTED          = 1U,  /*!< Freefall event detected. */
    FXOS8700_MOTION_DETECTED            = 2U,  /*!< Motion event detected.*/
    FXOS8700_TRANSIENT_DETECTED         = 3U,  /*!< Transient event detected.*/
    FXOS8700_DOUBLETAP_DETECTED         = 4U,  /*!< Double-Tap Pulse event detected. */
    FXOS8700_PORTRAIT_UP                = 5U,  /*!< Orientation: Portrait UP detected*/
    FXOS8700_PORTRAIT_DOWN              = 6U,  /*!< Orientation: Portrait Down detected*/
    FXOS8700_LANDSCAPE_RIGHT            = 7U,  /*!< Orientation: Landscape Right detected*/
    FXOS8700_LANDSCAPE_LEFT             = 8U,  /*!< Orientation: Landscape Left detected*/
    FXOS8700_FRONT_SIDE                 = 9U,  /*!< Orientation: Front Side detected*/
    FXOS8700_BACK_SIDE                  = 10U, /*!< Orientation: Back Side detected*/
    FXOS8700_ACCEL_VM_DETECTED          = 11U, /*!< Accel vector-magnitude event detected. */
    FXOS8700_MAG_VM_DETECTED            = 12U, /*!< Mag vector-magnitude event detected. */
    FXOS8700_MAG_THS_DETECTED           = 13U, /*!< Magnetic threshold event detected.*/
    FXOS8700_MAG_MIN_DETECTED           = 14U, /*!< Magnetic min detected.*/
    FXOS8700_MAG_MAX_DETECTED           = 15U, /*!< Magnetic max detected.*/
    FXOS8700_FIFO_WTRMRK_DETECTED       = 16U, /*!< FIFO watermark event detected. */
} fxos8700_event_status_type_t;

/*!
 * @brief FXOS8700 Sensor Mode Type
 */
typedef enum fxos8700_mode_type
{
    FXOS8700_STANDBY_MODE               = 0U, /*!< Standby Mode. */
    FXOS8700_ACTIVE_MODE                = 1U, /*!< Active Mode. */
    FXOS8700_MODE_END
} fxos8700_mode_type_t;

/*!
 * @brief This structure defines the fxos8700 raw accel + mag data buffer.
 */
typedef struct
{
    //uint32_t timestamp;                   /*! The time, this sample was recorded.  */ [TBD: Removed]
    int16_t  accel[NUM_AXES*FIFO_SIZE];     /*!< The accel data */
    int16_t  mag[NUM_AXES];                 /*!< The mag data */
} fxos8700_data_t;

/*!
 * @brief This structure defines the fxos8700 comm handle.
 */
typedef struct fxos8700_driver
{
    sensor_comm_handle_t *pComHandle;
}fxos8700_driver_t;

/*!
 * @brief fxos8700 interrupt configuration parameters
 */
typedef struct fxos8700_interrupt_config
{
	uint8_t                 pp_od : 1;   /*!<  - Push-Pull/Open Drain selection on interrupt pad for INT1/INT2
                                               0: Push-pull (default)
											   1: Open-drain.	*/
	uint8_t                 ipol  : 1;   /*!<  - Interrupt polarity ACTIVE high, or ACTIVE low for INT1/INT2.
	                                           0: Active low (default)
											   1: Active high. */
	uint8_t                 reserved: 5;
	FXOS8700_CTRL_REG4_t    intSources;  /*!<  Sources to be configured.
	                                           0: to a specific source field bit -disable the interrupt
											   1: to a specific source field bit -Enable the interrupt
											   eg. int_en_ff_mt bit to zero disable the interrupt, int_en_ff_mt bit to 1 enable the interrupt. */
	FXOS8700_CTRL_REG5_t    int1_2;      /*!< INT1 or INT2 Routing configuration for specified source
	                                           0: to a bit configures interrupt for specified source to INT2 pin
											   1: to a bit configures interrupt for specified source to INT1 pin
											   eg. int_cfg_pulse bit to '0' configures pulse interrupt to INT2, int_cfg_pulse bit to '1' configures pulse interrupt to INT1 */

}fxos8700_interrupt_config_t;

/*!
 * @brief fxos8700 interrupt sources
 */
typedef enum fxos8700_interrupt_source
{
    FXOS8700_DRDY       = 0x01,
    FXOS8700_SRC_ASLP   = 0x02,
    FXOS8700_SRC_FFMT   = 0x04,
    FXOS8700_SRC_PULSE  = 0x08,
    FXOS8700_SRC_LNDPRT = 0x10,
    FXOS8700_SRC_TRANS  = 0x20,
    FXOS8700_FIFO       = 0x40,
    FXOS8700_ASLP       = 0x80,
}fxos8700_interrupt_source_t;

/*!
 * @brief fxos8700 power mode
 */
typedef enum fxos8700_power_mode
{
    FXOS8700_ACCEL_NORMAL             = 0x00,  /*!< Normal Power Mode.*/
    FXOS8700_ACCEL_LOWNOISE_LOWPOWER  = 0x08,  /*!< Low Noise and Low Power Mode.*/
    FXOS8700_ACCEL_HIGHRESOLUTION     = 0x10,  /*!< High Resolution via OSR.*/
    FXOS8700_ACCEL_LOWPOWER           = 0x18,  /*!< Low Power Mode .*/
    FXOS8700_OFF                      = 0xff,
}fxos8700_power_mode_t;

/*!
 * @brief fxos8700 Output Data Rate
 */
typedef enum fxos8700_odr
{
    FXOS8700_ODR_SINGLE_800_HZ      = 0x00,
    FXOS8700_ODR_SINGLE_400_HZ      = 0x08,
    FXOS8700_ODR_SINGLE_200_HZ      = 0x10,
    FXOS8700_ODR_SINGLE_100_HZ      = 0x18,
    FXOS8700_ODR_SINGLE_50_HZ       = 0x20,
    FXOS8700_ODR_SINGLE_12P5_HZ     = 0x28,
    FXOS8700_ODR_SINGLE_6P25_HZ     = 0x30,
    FXOS8700_ODR_SINGLE_1P5625_HZ   = 0x38,
    FXOS8700_ODR_SINGLE_OFF         = 0xff,
}fxos8700_odr_t;

/*!
 * @brief fxos8700 Hybrid mode Output Data Rate
 */
typedef enum fxos8700_hybrid_odr
{
    FXOS8700_ODR_HYBRID_400_HZ      = 0x00,
    FXOS8700_ODR_HYBRID_200_HZ      = 0x08,
    FXOS8700_ODR_HYBRID_100_HZ      = 0x10,
    FXOS8700_ODR_HYBRID_50_HZ       = 0x18,
    FXOS8700_ODR_HYBRID_25_HZ       = 0x20,
    FXOS8700_ODR_HYBRID_6P25_HZ     = 0x28,
    FXOS8700_ODR_HYBRID_3P125_HZ    = 0x30,
    FXOS8700_ODR_HYBRID_0P7813_HZ   = 0x38,
    FXOS8700_ODR_HYBRID_OFF         = 0xff,
}fxos8700_hybrid_odr_t;

/*******************************************************************************
 * Constants
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * APIs Prototype
 ******************************************************************************/

/*! @brief       The interface function to initialize the FXOS8700 sensor comm.
 *  @details     This function initialize the FXOS8700 sensor communication interface.
 *  @param[in]   fxos8700_driver_t *pComHandle, the pointer to the FXOS8700 driver handle.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_init(fxos8700_driver_t *pDriver);

/*! @brief       The interface function to set FXOS8700 sensor mode.
 *  @details     This function set required FXOS8700 sensor mode.
 *  @param[in]   fxos8700_driver_t *pDriver, the pointer to the fxos8700 comm handle.
 *  @param[in]   fxos8700_mode_type_t sensorMode, FXOS8700 sensor mode that user want to set to.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_set_mode(fxos8700_driver_t *pDriver, fxos8700_mode_type_t sensorMode);

/*! @brief       The interface function to generically read a fxos8700 sensor register.
 *  @details     This function read a fxos8700 sensor register.
 *  @param[in]   fxos8700_driver_t *pDriver, the pointer to the FXOS8700 driver handle.
 *  @param[in]   address - Address from the register to read.
 *  @param[in]   nByteToRead - number of byte to read.
 *  @param[out]  pReadBuffer - a pointer to read buffer to to store the requested data read.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_read_reg(fxos8700_driver_t *pDriver, uint16_t address, uint16_t nByteToRead, uint8_t *pReadBuffer);

/*! @brief       The interface function to generically write to a fxos8700 sensor register.
 *  @details     This function write to a fxos8700 sensor registers.
 *  @param[in]   fxos8700_driver_t *pDriver, the pointer to the FXOS8700 driver handle.
 *  @param[in]   pWriteAddress - Address from the register to write.
 *  @param[out]  pWriteBuffer - a pointer to write buffer having value to write.
 *  @param[in]   nByteToWrite - number of byte to write.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_write_reg(fxos8700_driver_t *pDriver, uint16_t address, uint16_t nByteToWrite, uint8_t *pWriteBuffer);

/*! @brief       The interface function to set and configure fxos8700 sensor embedded functions.
 *  @details     This function configures the fxos8700 sensor with the required embedded configuration.
                 User can configure multiple embedded function using the single call.
 *  @param[in]   fxos8700_driver_t *pDriver - the pointer to the FXOS8700 driver handle.
 *  @param[in]   configType - types of embedded function to be configured.
 *  @return      returns the status of the operation.
 */
 
uint8_t fxos8700_set_embedded_function(fxos8700_driver_t *pDriver, fxos8700_embedded_func_config_type_t configType);

/*! @brief       The interface function to configure fxos8700 accel
 *  @details     This function configure the accel with desired configuration.
 *  @param[in]   fxos8700_driver_t *pDriver - the pointer to the FXOS8700 driver handle.
 *  @param[in]   ODR - ODR to be configured
 *  @param[in]   pConfig, the pointer to the acceleration configuration.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_configure_accel(fxos8700_driver_t *pDriver, fxos8700_odr_t odr, fxos8700_power_mode_t powerMode, fxos8700_accel_config_type_t pConfig);

/*! @brief       The interface function to configure fxos8700 mag
 *  @details     This function configure the mag with desired configuration.
 *  @param[in]   fxos8700_driver_t *pDriver - the pointer to the FXOS8700 driver handle.
 *  @param[in]   ODR - ODR to be configured
 *  @param[in]   pConfig, the pointer to the acceleration configuration.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_configure_mag(fxos8700_driver_t *pDriver, fxos8700_odr_t odr, fxos8700_mag_config_type_t pConfig);

/*! @brief       The interface function to configure fxos8700 in hybrid mode
 *  @details     This function configure the fxos8700 with desired configuration in hybrid mode.
 *  @param[in]   fxos8700_driver_t *pDriver - the pointer to the FXOS8700 driver handle.
 *  @param[in]   ODR - ODR to be configured
 *  @param[in]   pConfig, the pointer to the acceleration configuration.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_configure_hybrid(fxos8700_driver_t *pDriver, fxos8700_hybrid_odr_t odr, fxos8700_hybrid_config_type_t pConfig);

/*! @brief       The interface function to configure fxos8700 interrupt controller for desired sources.
 *  @details     This function configure the fxos8700 interrupts for desired sources.It is possible that multiple source can be configured in same INT1 or INT2 pin
                 thus one or more functional blocks can assert an interrupt pin simultaneously; therefore a host application responding to an interrupt should read the INT_SOURCE register to determine the source(s) of the interrupt(s)
                 this function allows to configure single or multiple interrupt sources using single call.
				 IMPORTANT NOTE:
				 It is important to understand that application developers should handle the ISR handle at MCU level. this function just configure sensor interrupt mode only.
 *  @param[in]   fxos8700_driver_t *pDriver - the pointer to the FXOS8700 driver handle.
 *  @param[in]   pConfig  - Configuration data for the interrupt mode
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_config_interrupt(fxos8700_driver_t *pDriver, fxos8700_interrupt_config_t *pConfig);

/*! @brief       The interface function to disable specified interrupt source/sources
 *  @details     This function allow the disable the multiple source using single call.
 *  @param[in]   fxos8700_driver_t *pDriver - the pointer to the FXOS8700 driver handle.
 *  @param[in]   ODR - ODR to be configured
 *  @param[in]   pConfig, the pointer to the acceleration configuration.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_disable_interrupt(fxos8700_driver_t *pDriver, fxos8700_interrupt_source_t intSource);

/*! @brief       The interface function to read FXOS8700 sensor data.
 *  @details     This function reads the FXOS8700 sensor output data.
 *  @param[in]   fxos8700_driver_t *pDriver, the pointer to the FXOS8700 driver handle.
 *  @param[in]   dataType - The FXOS8700 sensor data type to be read.
 *  @param[in]   fxos8700_data_t* pDataBuffer, the pointer to the data buffer to store FXOS8700 sensor data output.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_read_data(fxos8700_driver_t *pDriver, fxos8700_data_type_t dataType, fxos8700_data_t* pDataBuffer);

/*! @brief       The interface function to read FXOS8700 sensor events.
 *  @details     This function reads the FXOS8700 sensor events.
 *  @param[in]   fxos8700_driver_t *pDriver, the pointer to the FXOS8700 driver handle.
 *  @param[in]   eventType - The FXOS8700 sensor event type to be read.
 *  @param[in]   eventVal, the pointer to the event value/status storage.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_read_event(fxos8700_driver_t *pDriver, fxos8700_event_type_t eventType, uint8_t* eventVal);

/*! @brief       The interface function to de-initialize the FXOS8700 sensor.
 *  @details     This function de-initialize the FXOS8700 sensor.
 *  @param[in]   fxos8700_driver_t *pDriver, the pointer to the FXOS8700 driver handle.
 *  @return      returns the status of the operation.
 */
uint8_t fxos8700_deinit(fxos8700_driver_t *pDriver);

#endif /* FXOS8700_DRIVER_H_ */
