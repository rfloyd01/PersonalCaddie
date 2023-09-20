/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file fxos8700_config.h
 * @brief The fxos8700_driver_config.h file contains definitions for FXOS8700 sensor configurations.
 * @see   https://www.nxp.com/files-static/sensors/doc/data_sheet/FXOS8700CQ.pdf
 */

#ifndef FXOS8700_CONFIG_H_
#define FXOS8700_CONFIG_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "common/sensor_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @def    FIFO_SIZE
 *  @brief  The watermark value configured for FXOS8700 FIFO Buffer.
 */
#define FIFO_SIZE 16
/*******************************************************************************
 * Typedefs
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

extern const registerwritelist_t gFxos8700ActiveModeConfig[];
extern const registerwritelist_t gFxos8700StandbyModeConfig[];
extern const registerwritelist_t gFxos8700HybridInterruptConfig[];
extern const registerwritelist_t gFxos8700AccelInterruptConfig[];
extern const registerwritelist_t gFxos87008bitAccelPollConfig[];
extern const registerwritelist_t gFxos8700AccelPollConfig[];
extern const registerwritelist_t gFxos8700HybridPollConfig[];
extern const registerwritelist_t gFxos8700AccelFifoConfig[];
extern const registerwritelist_t gFxos8700OrientDetectConfig[];
extern const registerwritelist_t gFxos8700MotiontDetectConfig[];
extern const registerwritelist_t gFxos8700FreefallDetectConfig[];
extern const registerwritelist_t gFxos8700DoubleTapDetectConfig[];
extern const registerwritelist_t gFxos8700MagVecmConfig[];

extern const registerreadlist_t gFxos8700ReadStatus[];
extern const registerreadlist_t gFxos8700ReadAccelMag[];
extern const registerreadlist_t gFxos8700ReadAccel[];
extern const registerreadlist_t gFxos8700ReadAccelFifo[];
extern const registerreadlist_t gFxos8700ReadAccel8bit[];
extern const registerreadlist_t gFxos8700ReadMag[];
extern const registerreadlist_t gFxos8700ReadFFMTSrc[];
extern const registerreadlist_t gFxos8700ReadINTSrc[];
extern const registerreadlist_t gFxos8700ReadPLStatus[];
extern const registerreadlist_t gFxos8700ReadPulseSrc[];
extern const registerreadlist_t gFxos8700MagVecmRead[];
/*******************************************************************************
 * APIs Prototype
 ******************************************************************************/



#endif /* FXOS8700_CONFIG_H_ */