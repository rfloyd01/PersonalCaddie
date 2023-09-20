/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file fxos8700_config.c
 * @brief The fxos8700_driver_config.c file contains definitions for FXOS8700 sensor configurations.
 * @see   https://www.nxp.com/files-static/sensors/doc/data_sheet/FXOS8700CQ.pdf
 */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "fxos8700_driver.h"
#include "fxos8700_config.h"
#include "sensor_common.h"

/*-------------------------------------------------------------------
//Macros
-------------------------------------------------------------------*/

/*! @def    FXOS8700 FF_MT freefall & Pulse detection counter register threshold values
 *  @brief  FXOS8700 FF_MT freefall & Pulse detection counter register threshold values.
 *          These values have been derived based on the Application Note AN4070(similar
 *          threshold values are applied for FXOS8700).
 *  @see    http://cache.freescale.com/files/sensors/doc/app_note/AN4070.pdf
 */
#define FF_A_FFMT_THS 0x18  /* FreeFall Threshold Value. */
#define MT_A_FFMT_THS 0x15  /* Motion Threshold Value. */
#define A_FFMT_COUNT  0x08  /* Freefall/motion debounce count value. */
#define PL_COUNT      0x15  /* Debounce count value. */
#define ASLP_COUNTER  0x07  /* Auto Sleep after ~5s. */

#define PULSE_THX     0x28  /* X-axis pulse threshold value. */
#define PULSE_THY     0x28  /* Y-axis pulse threshold value. */
#define PULSE_THZ     0x28  /* Z-axis pulse threshold value. */
#define PULSE_TL      0x30  /* Time limit value for pulse detection. */
#define PULSE_LT      0x50  /* Latency time for second pulse detection. */
#define PULSE_WT      0x78  /* Window time for second pulse detection. */

#define RAW_MAG_DATA_SIZE          (6)
#define FXOS8700_MAG_CNTPERUT      (10)  //counts per uT factor
#define FXOS8700_MAG_VECM_DET_FLAG (2)
#define MAG_VEC_THS_MSB            (0x81)
#define MAG_VEC_THS_LSB            (0x90)
#define MAG_VEC_CNT                (0x10)

//-----------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------
/*! --------------------------------------------------------------------------*/
/*! FXOS8700 Register Write List for various modes                            */
/*! --------------------------------------------------------------------------*/

/*! @brief Register Configuration to put FXOS8700 sensor in operating mode as Active */
const registerwritelist_t gFxos8700ActiveModeConfig[] = {
    /*! Set FXOS8700 CTRL_REG1 Register "active" bit-field to put sensor in Active mode. */
    {FXOS8700_CTRL_REG1, FXOS8700_CTRL_REG1_ACTIVE_ACTIVE_MODE, FXOS8700_CTRL_REG1_ACTIVE_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to put FXOS8700 sensor in operating mode as STANDBY */
const registerwritelist_t gFxos8700StandbyModeConfig[] = {
    /*! Reset FXOS8700 CTRL_REG1 Register "active" bit-field to put sensor in StandBy mode. */
    {FXOS8700_CTRL_REG1, FXOS8700_CTRL_REG1_ACTIVE_STANDBY_MODE, FXOS8700_CTRL_REG1_ACTIVE_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure FXOS8700 for reading Accel+Mag samples in Interrupt mode */
const registerwritelist_t gFxos8700HybridInterruptConfig[] = {
    /*! Configure FXOS8700 CTRL_REG3 Register "ipol" & "pp_od" bit-fields to set INT polarity to ActiveHigh
	    and output mode selection as push-pull. */
    {FXOS8700_CTRL_REG3, FXOS8700_CTRL_REG3_IPOL_ACTIVE_HIGH | FXOS8700_CTRL_REG3_PP_OD_PUSH_PULL,
                         FXOS8700_CTRL_REG3_IPOL_MASK | FXOS8700_CTRL_REG3_PP_OD_MASK},
    /*! Set FXOS8700 CTRL_REG4 Register "int_en_drdy" bit-field to enable data-ready interrupt. */
    {FXOS8700_CTRL_REG4, FXOS8700_CTRL_REG4_INT_EN_DRDY_EN, FXOS8700_CTRL_REG4_INT_EN_DRDY_MASK},
    /*! Set FXOS8700 CTRL_REG5 Register "int_cfg_drdy" bit-field to route data-ready interrupt to INT1. */
    {FXOS8700_CTRL_REG5, FXOS8700_CTRL_REG5_INT_CFG_DRDY_INT1 | FXOS8700_CTRL_REG5_INT_CFG_DRDY_MASK},
    /*! Set FXOS8700 M_CTRL_REG1 Register "m_acal" & "m_hms[1:0]" bit-fields to enable auto-calibration
	    and configure sensor for hybrid mode. */
    {FXOS8700_M_CTRL_REG1, FXOS8700_M_CTRL_REG1_M_ACAL_EN | FXOS8700_M_CTRL_REG1_M_HMS_HYBRID_MODE,
                           FXOS8700_M_CTRL_REG1_M_ACAL_MASK | FXOS8700_M_CTRL_REG1_M_HMS_MASK},
    /*! Configure FXOS8700 M_CTRL_REG2 Register "hyb_autoinc_mode" & "m_rst_cnt[1:0]" bit-fields to enable hybrid
	    auto-increment mode and disable auto magetic reset. */
    {FXOS8700_M_CTRL_REG2, FXOS8700_M_CTRL_REG2_M_AUTOINC_HYBRID_MODE | FXOS8700_M_CTRL_REG2_M_RST_CNT_DISABLE,
                           FXOS8700_M_CTRL_REG2_M_AUTOINC_MASK | FXOS8700_M_CTRL_REG2_M_RST_CNT_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure FXOS8700 for reading Accel samples in Interrupt mode */
const registerwritelist_t gFxos8700AccelInterruptConfig[] = {
    /*! Configure FXOS8700 CTRL_REG3 Register "ipol" & "pp_od" bit-fields to set INT polarity to ActiveHigh
	    and output mode selection as push-pull. */
    {FXOS8700_CTRL_REG3, FXOS8700_CTRL_REG3_IPOL_ACTIVE_HIGH | FXOS8700_CTRL_REG3_PP_OD_PUSH_PULL,
                         FXOS8700_CTRL_REG3_IPOL_MASK | FXOS8700_CTRL_REG3_PP_OD_MASK},
    /*! Set FXOS8700 CTRL_REG4 Register "int_en_drdy" bit-field to enable data-ready interrupt. */
    {FXOS8700_CTRL_REG4, FXOS8700_CTRL_REG4_INT_EN_DRDY_EN, FXOS8700_CTRL_REG4_INT_EN_DRDY_MASK},
    /*! Set FXOS8700 CTRL_REG5 Register "int_cfg_drdy" bit-field to route data-ready interrupt to INT1. */
    {FXOS8700_CTRL_REG5, FXOS8700_CTRL_REG5_INT_CFG_DRDY_INT1 | FXOS8700_CTRL_REG5_INT_CFG_DRDY_MASK},
    /*! Set FXOS8700 M_CTRL_REG1 Register "m_hms[1:0]" bit-fields to configure sensor for Accel only mode. */
    {FXOS8700_M_CTRL_REG1, FXOS8700_M_CTRL_REG1_M_HMS_ACCEL_ONLY, FXOS8700_M_CTRL_REG1_M_HMS_MASK},
    /*! Reset FXOS8700 M_CTRL_REG2 Register "hyb_autoinc_mode" bit-field to disable Hybrid auto-increment mode. */
    {FXOS8700_M_CTRL_REG2, FXOS8700_M_CTRL_REG2_M_AUTOINC_ACCEL_ONLY_MODE, FXOS8700_M_CTRL_REG2_M_AUTOINC_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure FXOS8700 for reading Accel 8-bit samples in Polling mode */
const registerwritelist_t gFxos87008bitAccelPollConfig[] = {
    /*! Set FXOS8700 M_CTRL_REG1 Register "m_hms[1:0]" bit-fields to configure sensor for Accel only mode. */
    {FXOS8700_M_CTRL_REG1, FXOS8700_M_CTRL_REG1_M_HMS_ACCEL_ONLY, FXOS8700_M_CTRL_REG1_M_HMS_MASK},
    /*! Reset FXOS8700 M_CTRL_REG2 Register "hyb_autoinc_mode" bit-field to disable Hybrid auto-increment mode. */
    {FXOS8700_M_CTRL_REG2, FXOS8700_M_CTRL_REG2_M_AUTOINC_ACCEL_ONLY_MODE, FXOS8700_M_CTRL_REG2_M_AUTOINC_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure FXOS8700 for reading Accel samples in Polling mode */
const registerwritelist_t gFxos8700AccelPollConfig[] = {
    /*! Set FXOS8700 M_CTRL_REG1 Register "m_hms[1:0]" bit-fields to configure sensor for Accel only mode. */
    {FXOS8700_M_CTRL_REG1, FXOS8700_M_CTRL_REG1_M_HMS_ACCEL_ONLY, FXOS8700_M_CTRL_REG1_M_HMS_MASK},
    /*! Reset FXOS8700 M_CTRL_REG2 Register "hyb_autoinc_mode" bit-field to disable Hybrid auto-increment mode. */
    {FXOS8700_M_CTRL_REG2, FXOS8700_M_CTRL_REG2_M_AUTOINC_ACCEL_ONLY_MODE, FXOS8700_M_CTRL_REG2_M_AUTOINC_MASK},
    __END_WRITE_DATA__};

/*! Prepare the register write list to configure FXOS8700 for reading Accel + Mag samples in polling/hybrid mode. */
const registerwritelist_t gFxos8700HybridPollConfig[] = {

    /*! Configure FXOS8700 to enable auto-sleep, low power in sleep, high res in wake. */
    {FXOS8700_XYZ_DATA_CFG, FXOS8700_XYZ_DATA_CFG_FS_4G_0P488,FXOS8700_XYZ_DATA_CFG_FS_MASK},
	 /*! Configure FXOS8700 to enable auto-sleep, low power in sleep, high res in wake. */
	{FXOS8700_CTRL_REG2, FXOS8700_CTRL_REG2_MODS_HIGH_RES | FXOS8700_CTRL_REG2_SLPE_EN | FXOS8700_CTRL_REG2_SMODS_LOW_POWER,
	                          FXOS8700_CTRL_REG2_MODS_MASK | FXOS8700_CTRL_REG2_SLPE_MASK | FXOS8700_CTRL_REG2_SMODS_MASK},
    /*! Set FXOS8700 M_CTRL_REG1 Register "m_acal" & "m_hms[1:0]" bit-fields to enable auto-calibration
        and configure sensor for hybrid mode. */
	{FXOS8700_M_CTRL_REG1, FXOS8700_M_CTRL_REG1_M_RST_EN | FXOS8700_M_CTRL_REG1_M_HMS_HYBRID_MODE | FXOS8700_M_CTRL_REG1_M_OS_OSR7,
			FXOS8700_M_CTRL_REG1_M_RST_MASK | FXOS8700_M_CTRL_REG1_M_HMS_MASK | FXOS8700_M_CTRL_REG1_M_OS_MASK},
    /*! Reset FXOS8700 M_CTRL_REG2 Register "hyb_autoinc_mode" bit-field to disable Hybrid auto-increment mode. */
    {FXOS8700_M_CTRL_REG2, FXOS8700_M_CTRL_REG2_M_AUTOINC_HYBRID_MODE,
                           FXOS8700_M_CTRL_REG2_M_AUTOINC_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure FXOS8700 for Accel samples in Fifo mode */
const registerwritelist_t gFxos8700AccelFifoConfig[] = {
    /*! Configure FXOS8700 F_SETUP Register "f_mode[1:0]" bit-fields to enable FIFO mode and set watermark*/
    {FXOS8700_F_SETUP, FXOS8700_F_SETUP_F_MODE_FIFO_STOP_OVF | (FIFO_SIZE << FXOS8700_F_SETUP_F_WMRK_SHIFT),
     FXOS8700_F_SETUP_F_MODE_MASK | FXOS8700_F_SETUP_F_WMRK_MASK},
    /*! Set FXOS8700 M_CTRL_REG1 Register "m_hms[1:0]" bit-fields to configure sensor for Accel only mode. */
    {FXOS8700_M_CTRL_REG1, FXOS8700_M_CTRL_REG1_M_HMS_ACCEL_ONLY, FXOS8700_M_CTRL_REG1_M_HMS_MASK},
    /*! Reset FXOS8700 M_CTRL_REG2 Register "hyb_autoinc_mode" bit-field to disable Hybrid auto-increment mode. */
    {FXOS8700_M_CTRL_REG2, FXOS8700_M_CTRL_REG2_M_AUTOINC_ACCEL_ONLY_MODE, FXOS8700_M_CTRL_REG2_M_AUTOINC_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure FXOS8700 for Orientation-detect Mode. */
const registerwritelist_t gFxos8700OrientDetectConfig[] = {
    {FXOS8700_PL_CFG, FXOS8700_PL_CFG_DBCNTM_CLEAR_MODE | FXOS8700_PL_CFG_PL_EN_ENABLE,
     FXOS8700_PL_CFG_PL_EN_MASK | FXOS8700_PL_CFG_DBCNTM_MASK},
    {FXOS8700_CTRL_REG4, FXOS8700_CTRL_REG4_INT_EN_LNDPRT_EN | FXOS8700_CTRL_REG4_INT_EN_ASLP_EN,
     FXOS8700_CTRL_REG4_INT_EN_LNDPRT_MASK | FXOS8700_CTRL_REG4_INT_EN_ASLP_MASK},
    {FXOS8700_CTRL_REG3, FXOS8700_CTRL_REG3_WAKE_LNDPRT_EN, FXOS8700_CTRL_REG3_WAKE_LNDPRT_MASK},
    {FXOS8700_CTRL_REG2, FXOS8700_CTRL_REG2_SLPE_EN, FXOS8700_CTRL_REG2_SLPE_MASK},
    {FXOS8700_CTRL_REG1, FXOS8700_CTRL_REG1_DR_HYBRID_3P125_HZ | FXOS8700_CTRL_REG1_ACTIVE_ACTIVE_MODE,
     FXOS8700_CTRL_REG1_DR_MASK | FXOS8700_CTRL_REG1_ACTIVE_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure FXOS8700 for Motion-detect Mode. */
const registerwritelist_t gFxos8700MotiontDetectConfig[] = {
    {FXOS8700_A_FFMT_THS, MT_A_FFMT_THS, FXOS8700_A_FFMT_THS_THS_MASK}, /* Threshold */
    {FXOS8700_A_FFMT_CFG, FXOS8700_A_FFMT_CFG_OAE_MOTION | FXOS8700_A_FFMT_CFG_ZEFE_RAISE_EVENT |
                              FXOS8700_A_FFMT_CFG_YEFE_RAISE_EVENT | FXOS8700_A_FFMT_CFG_XEFE_RAISE_EVENT,
     FXOS8700_A_FFMT_CFG_OAE_MASK | FXOS8700_A_FFMT_CFG_ZEFE_MASK | FXOS8700_A_FFMT_CFG_YEFE_MASK |
         FXOS8700_A_FFMT_CFG_XEFE_MASK},
    {FXOS8700_CTRL_REG4, FXOS8700_CTRL_REG4_INT_EN_FFMT_EN, FXOS8700_CTRL_REG4_INT_EN_FFMT_MASK},
    {FXOS8700_CTRL_REG1, FXOS8700_CTRL_REG1_DR_HYBRID_0P7813_HZ | FXOS8700_CTRL_REG1_ACTIVE_ACTIVE_MODE,
     FXOS8700_CTRL_REG1_DR_MASK | FXOS8700_CTRL_REG1_ACTIVE_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure FXOS8700 for Free-fall Mode. */
const registerwritelist_t gFxos8700FreefallDetectConfig[] = {
    {FXOS8700_A_FFMT_COUNT, A_FFMT_COUNT, 0}, /* Debounce Counter */
    {FXOS8700_A_FFMT_THS, FF_A_FFMT_THS | FXOS8700_A_FFMT_THS_DBCNTM_MASK,
     FXOS8700_A_FFMT_THS_THS_MASK | FXOS8700_A_FFMT_THS_DBCNTM_MASK}, /* Threshold */
    {FXOS8700_A_FFMT_CFG,
     FXOS8700_A_FFMT_CFG_OAE_FREEFALL | FXOS8700_A_FFMT_CFG_ELE_EN | FXOS8700_A_FFMT_CFG_ZEFE_RAISE_EVENT |
         FXOS8700_A_FFMT_CFG_YEFE_RAISE_EVENT | FXOS8700_A_FFMT_CFG_XEFE_RAISE_EVENT,
     FXOS8700_A_FFMT_CFG_OAE_MASK | FXOS8700_A_FFMT_CFG_ELE_MASK | FXOS8700_A_FFMT_CFG_ZEFE_MASK |
         FXOS8700_A_FFMT_CFG_YEFE_MASK | FXOS8700_A_FFMT_CFG_XEFE_MASK},
    {FXOS8700_CTRL_REG4, FXOS8700_CTRL_REG4_INT_EN_FFMT_EN, FXOS8700_CTRL_REG4_INT_EN_FFMT_MASK},
    {FXOS8700_CTRL_REG1, FXOS8700_CTRL_REG1_DR_HYBRID_100_HZ, FXOS8700_CTRL_REG1_DR_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure FXOS8700 for Double-tap pulse detection Mode. */
const registerwritelist_t gFxos8700DoubleTapDetectConfig[] = {
    /*! HP_FILTER_CUTOFF to be set to 0x00 */
    {FXOS8700_HP_FILTER_CUTOFF, FXOS8700_HP_FILTER_CUTOFF_PULSE_HPF_BYP_EN, FXOS8700_HP_FILTER_CUTOFF_PULSE_HPF_BYP_MASK},
    /*! Enable double-pulse event on X, Y & Z axis */
    {FXOS8700_PULSE_CFG, FXOS8700_PULSE_CFG_PLS_XDPEFE_EN | FXOS8700_PULSE_CFG_PLS_YDPEFE_EN | FXOS8700_PULSE_CFG_PLS_ZDPEFE_EN,
     FXOS8700_PULSE_CFG_PLS_XDPEFE_MASK | FXOS8700_PULSE_CFG_PLS_YDPEFE_MASK | FXOS8700_PULSE_CFG_PLS_ZDPEFE_MASK},
     /*! Set thresholds to be used by the system to start the pulse-event detection procedure */
    {FXOS8700_PULSE_THSX, PULSE_THX, FXOS8700_PULSE_THSX_PLS_THSX_MASK},
    {FXOS8700_PULSE_THSY, PULSE_THY, FXOS8700_PULSE_THSY_PLS_THSY_MASK},
    {FXOS8700_PULSE_THSZ, PULSE_THZ, FXOS8700_PULSE_THSZ_PLS_THSZ_MASK},
     /*! Set Pulse time limit threshold to PULSE_TL */
    {FXOS8700_PULSE_TMLT, PULSE_TL, FXOS8700_PULSE_THSZ_PLS_THSZ_MASK},
     /*! Set Pulse latency time threshold to PULSE_LT */
    {FXOS8700_PULSE_LTCY, PULSE_LT, FXOS8700_PULSE_THSZ_PLS_THSZ_MASK},
     /*! Set Pulse window time threshold to PULSE_WT */
    {FXOS8700_PULSE_WIND, PULSE_WT, FXOS8700_PULSE_THSZ_PLS_THSZ_MASK},
    /*! Configure FXOS8700 in standby mode, normal read and noise mode, 800Hz ODR and auto-wake frequency of 50Hz */
    {FXOS8700_CTRL_REG1, FXOS8700_CTRL_REG1_ACTIVE_STANDBY_MODE | FXOS8700_CTRL_REG1_F_READ_NORMAL | FXOS8700_CTRL_REG1_LNOISE_NORMAL | FXOS8700_CTRL_REG1_DR_SINGLE_800_HZ | FXOS8700_CTRL_REG1_ASLP_RATE_50_HZ,
     FXOS8700_CTRL_REG1_ACTIVE_MASK | FXOS8700_CTRL_REG1_F_READ_MASK | FXOS8700_CTRL_REG1_LNOISE_MASK | FXOS8700_CTRL_REG1_DR_MASK | FXOS8700_CTRL_REG1_ASLP_RATE_MASK},
    /*! Configure FXOS8700 in low power wake mode */
    {FXOS8700_CTRL_REG2, FXOS8700_CTRL_REG2_MODS_LOW_POWER, FXOS8700_CTRL_REG2_MODS_MASK},
    __END_WRITE_DATA__};

const registerwritelist_t gFxos8700MagVecmConfig[] = {
    /*! System and Control registers. */
    /*! Configure the FXOS8700 CTRL_REG1 to set 50Hz sampling rate. */
    {FXOS8700_CTRL_REG1, FXOS8700_CTRL_REG1_DR_SINGLE_50_HZ, FXOS8700_CTRL_REG1_DR_MASK},
    /*! Configure the FXOS8700 CTRL_REG3 to set interrupt polarity to active high and push-pull configuration. */
    {FXOS8700_CTRL_REG3, FXOS8700_CTRL_REG3_IPOL_ACTIVE_HIGH | FXOS8700_CTRL_REG3_PP_OD_PUSH_PULL,
                         FXOS8700_CTRL_REG3_IPOL_MASK | FXOS8700_CTRL_REG3_PP_OD_MASK},
    /*! Configure the FXOS8700 CTRL_REG4 to enable data ready event interrupt. */
    {FXOS8700_CTRL_REG4, FXOS8700_CTRL_REG4_INT_EN_DRDY_EN,
                         FXOS8700_CTRL_REG4_INT_EN_DRDY_MASK},
    /*! Configure the FXOS8700 CTRL_REG5 to route data ready event interrupt to INT2 pin. */
    {FXOS8700_CTRL_REG5, FXOS8700_CTRL_REG5_INT_CFG_DRDY_INT2, FXOS8700_CTRL_REG5_INT_CFG_DRDY_MASK},
    /*! Configure the FXOS8700 M_CTRL_REG1 to make MAG only active, set OSR = 32. */
	{FXOS8700_M_CTRL_REG1, FXOS8700_M_CTRL_REG1_M_HMS_MAG_ONLY|FXOS8700_M_CTRL_REG1_M_OS_OSR7,
			               FXOS8700_M_CTRL_REG1_M_HMS_MASK|FXOS8700_M_CTRL_REG1_M_OS_MASK},
    /*! Configure the FXOS8700 M_VECM_CFG to:
     *  Enable event latch,
     *  Set to not update initial reference,
     *  Enable magnetometer vector-magnitude detection function,
     *  Enable magnetometer vector-magnitude event interrupt,
     */
	{FXOS8700_M_VECM_CFG, FXOS8700_M_VECM_CFG_M_VECM_ELE_EN|FXOS8700_M_VECM_CFG_M_VECM_INITM_OUT|FXOS8700_M_VECM_CFG_M_VECM_UPDM_EN|FXOS8700_M_VECM_CFG_A_VECM_EN_EN|FXOS8700_M_VECM_CFG_M_VECM_INT_EN_EN|FXOS8700_M_VECM_CFG_M_VECM_INIT_CFG_INT1,
			              FXOS8700_M_VECM_CFG_M_VECM_ELE_MASK|FXOS8700_M_VECM_CFG_M_VECM_INITM_MASK|FXOS8700_M_VECM_CFG_M_VECM_UPDM_MASK|FXOS8700_M_VECM_CFG_A_VECM_EN_MASK|FXOS8700_M_VECM_CFG_M_VECM_INT_EN_MASK|FXOS8700_M_VECM_CFG_M_VECM_INIT_CFG_MASK},
    /*! Configure the FXOS8700 M_VECM_THS to set magnetometer vector-magnitude threshold to ~40uT (value = 0x190). */
	{FXOS8700_M_VECM_THS_MSB, MAG_VEC_THS_MSB, FXOS8700_M_VECM_THS_MSB_M_VECM_THS_MASK},
	{FXOS8700_M_VECM_THS_LSB, MAG_VEC_THS_LSB, FXOS8700_M_VECM_THS_MSB_M_VECM_THS_MASK},
    /*! Configure the FXOS8700 M_VECM_THS to set magnetometer vector-magnitude count to 16. */
	{FXOS8700_M_VECM_CNT, MAG_VEC_CNT, FXOS8700_M_VECM_THS_MSB_M_VECM_THS_MASK},

    __END_WRITE_DATA__};

/*! --------------------------------------------------------------------------*/
/*! FXOS8700 Register Read List                                               */
/*! --------------------------------------------------------------------------*/

/*! @brief Read FXOS8700 STATUS Register */
const registerreadlist_t gFxos8700ReadStatus[] = {
	{.readFrom = FXOS8700_STATUS, .numBytes = 1},
	__END_READ_DATA__};

/*! @brief Read register list for FXOS8700 to read accel + mag samples */
const registerreadlist_t gFxos8700ReadAccelMag[] = {
	{.readFrom = FXOS8700_OUT_X_MSB, .numBytes = FXOS8700_ACCEL_MAG_DATA_SIZE},
    __END_READ_DATA__};

/*! @brief Read register list for FXOS8700 to read Accel 14bit samples */
const registerreadlist_t gFxos8700ReadAccel[] = {
	{.readFrom = FXOS8700_OUT_X_MSB, .numBytes = FXOS8700_ACCEL_DATA_SIZE},
    __END_READ_DATA__};

/*! @brief Read register list for FXOS8700 to read Accel 14bit samples */
const registerreadlist_t gFxos8700ReadAccelFifo[] = {
	{.readFrom = FXOS8700_OUT_X_MSB, .numBytes = FXOS8700_ACCEL_DATA_SIZE * FIFO_SIZE},
    __END_READ_DATA__};

/*! @brief Read register list for FXOS8700 to read Accel 8bit samples */
const registerreadlist_t gFxos8700ReadAccel8bit[] = {
	{.readFrom = FXOS8700_OUT_X_MSB, .numBytes = FXOS8700_ACCEL_8BITDATA_SIZE},
    __END_READ_DATA__};

/*! @brief Read register list for FXOS8700 to read Mag samples */
const registerreadlist_t gFxos8700ReadMag[] = {
    {.readFrom = FXOS8700_M_OUT_X_MSB, .numBytes = FXOS8700_MAG_DATA_SIZE},
    __END_READ_DATA__};

/*! @brief Read register list for FXOS8700 to read Free-Fall/Motion Status Register. */
const registerreadlist_t gFxos8700ReadFFMTSrc[] = {
	{.readFrom = FXOS8700_A_FFMT_SRC, .numBytes = 1},
	__END_READ_DATA__};

/*! @brief Read register list for FXOS8700 to read Interrupt Source Register. */
const registerreadlist_t gFxos8700ReadINTSrc[] = {
	{.readFrom = FXOS8700_INT_SOURCE, .numBytes = 1},
	__END_READ_DATA__};

/*! @brief Read register list for FXOS8700 to read Portrait/Landscape Status Register. */
const registerreadlist_t gFxos8700ReadPLStatus[] = {
	{.readFrom = FXOS8700_PL_STATUS, .numBytes = 1},
	__END_READ_DATA__};

/*! @brief Read register list for FXOS8700 to read status bits for the pulse detection function. */
const registerreadlist_t gFxos8700ReadPulseSrc[] = {
	{.readFrom = FXOS8700_PULSE_SRC, .numBytes = 1},
	__END_READ_DATA__};

/*!@brief Read register list for the Mag Vecm Magitude Data */
const registerreadlist_t gFxos8700MagVecmRead[] =
{
	{.readFrom = FXOS8700_M_OUT_X_MSB, .numBytes = RAW_MAG_DATA_SIZE},
	{.readFrom =  FXOS8700_M_INT_SRC, .numBytes = 1},
	__END_READ_DATA__
};
//-----------------------------------------------------------------------
// local functions/ variables
//-----------------------------------------------------------------------