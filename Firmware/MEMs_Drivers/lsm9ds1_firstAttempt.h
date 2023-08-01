#ifndef LSM9DS1_H
#define LSM9DS1_H

#ifdef __cplusplus
extern "C" {
#endif

/*Sensor Register Definitions
* All of the definitions below are for mapping the sensor registers
*/

//Sensor Addresses
#define LSM9DS1_ADDR             0x68U
#define LSM9DS1_SPI_ADDR         0x68U //used when the chip is in SPI communication mode
#define LSM9DS1_ADDR_M           0x3DU

/* =========================================================================================================================== */
/* ================                           Accelerometer and Gyroscope Registers                           ================ */
/* =========================================================================================================================== */

//Interrupt Status and Configuration Registers
#define LSM9DS1_INT_GEN_CFG_XL    0x06  //Accelerometer interrupt generator configuration register
#define LSM9DS1_INT_GEN_THS_X_XL  0x07  //Accelerometer interrupt threshold register X-axis
#define LSM9DS1_INT_GEN_THS_Y_XL  0x08  //Accelerometer interrupt threshold register Y-axis
#define LSM9DS1_INT_GEN_THS_Z_XL  0x09  //Accelerometer interrupt threshold register Z-axis
#define LSM9DS1_INT_GEN_DUR_XL    0x0A  //Accelerometer interrupt duration register
#define LSM9DS1_INT1_CTRL         0x0C  //INT1_A/G pin control register
#define LSM9DS1_INT2_CTRL         0x0D  //INT2_A/G pin control register
#define LSM9DS1_INT_GEN_SRC_G     0x14  //Gyroscopeinterrupt source register. 
#define LSM9DS1_INT_GEN_SRC_XL    0x26  //Accelerometer interrupt source register
#define LSM9DS1_INT_GEN_CFG_G     0x30  //Gyroscope interrupt generator configuration register
#define LSM9DS1_INT_GEN_THS_XH_G  0x31  //Gyroscope x-axis interrupt generator threshold register (high). The value is expressed as a 15-bit word in two’s complement with low
#define LSM9DS1_INT_GEN_THS_XL_G  0x32  //Gyroscope x-axis interrupt generator threshold register (low). The value is expressed as a 15-bit word in two’s complement with high
#define LSM9DS1_INT_GEN_THS_YH_G  0x33  //Gyroscope y-axis interrupt generator threshold register (high). The value is expressed as a 15-bit word in two’s complement with low
#define LSM9DS1_INT_GEN_THS_YL_G  0x34  //Gyroscope y-axis interrupt generator threshold register (low). The value is expressed as a 15-bit word in two’s complement with high
#define LSM9DS1_INT_GEN_THS_ZH_G  0x35  //Gyroscope z-axis interrupt generator threshold register (high). The value is expressed as a 15-bit word in two’s complement with low
#define LSM9DS1_INT_GEN_THS_ZL_G  0x36  //Gyroscope z-axis interrupt generator threshold register (low). The value is expressed as a 15-bit word in two’s complement with high
#define LSM9DS1_INT_GEN_DUR_G     0x37  //Gyroscope interrupt generator duration register

//Control Registers
#define LSM9DS1_CTRL_REG1_G       0x10  //Control Register 1 (Gyroscope): Bandwidth selection, Full scale range selection, ODR selection
#define LSM9DS1_CTRL_REG2_G       0x11  //Control Register 2 (Gyroscope): Output selection, Interrupt selection
#define LSM9DS1_CTRL_REG3_G       0x12  //Control Register 3 (Gyroscope): Low-power mode enable, High-pass filter enable, High-pass filter frq. selection
#define LSM9DS1_ORIENT_CFG_G      0x13  //Gyroscope sign and oritentation register (allows for inversion of axis data and swapping of axes)
#define LSM9DS1_CTRL_REG4         0x1E  //Control Register 4: Gyroscope axis output enable, Latched interrupt, 4D option
#define LSM9DS1_CTRL_REG5_XL      0x1f  //Control Register 5 (Accelerometer): Accelerometer axis output enable, Data decimation
#define LSM9DS1_CTRL_REG6_XL      0x20  //Control Register 6 (Accelerometer): ODR selection, Power mode selection, Full scale range selection, Bandwidth selection, Anti-aliasing filter selection
#define LSM9DS1_CTRL_REG7_XL      0x21  //Control Register 7 (Accelerometer): High resolution mode enable, Digital filter cutoff frequency selection, Filtered data selection, High pass filter enable
#define LSM9DS1_CTRL_REG8         0x22  //Control Register 8: Reboot, Block data update, Interrupt level, Push-pull / open-drain selection, SPI mode selection, Auto register increment during multi-read, Big/Little endian, reset
#define LSM9DS1_CTRL_REG9         0x23  //Control Register 9: Enable FIFO, FIFO Threshold level, I2C Disable, Data ready enable, Gyroscope sleep enable
#define LSM9DS1_CTRL_REG10        0x24  //Control Register 10: Sense self test enable

//Data Output Registers
#define LSM9DS1_OUT_TEMP_L        0x15  //Temperature data output register. L and H registers together express a 16-bit word in two’s complement right - justified
#define LSM9DS1_OUT_TEMP_H        0x16  //Temperature data output register. L and H registers together express a 16-bit word in two’s complement right - justified
#define LSM9DS1_OUT_X_L_G         0x18  //Gyroscope x-axis angular rate output register (low byte). The value is expressed as a 16-bit word in two’s complement with high byte
#define LSM9DS1_OUT_X_H_G         0x19  //Gyroscope x-axis angular rate output register (high byte). The value is expressed as a 16-bit word in two’s complement with low byte
#define LSM9DS1_OUT_Y_L_G         0x1A  //Gyroscope y-axis angular rate output register (low byte). The value is expressed as a 16-bit word in two’s complement with high byte
#define LSM9DS1_OUT_Y_H_G         0x1B  //Gyroscope y-axis angular rate output register (high byte). The value is expressed as a 16-bit word in two’s complement with low byte
#define LSM9DS1_OUT_Z_L_G         0x1C  //Gyroscope z-axis angular rate output register (low byte). The value is expressed as a 16-bit word in two’s complement with high byte
#define LSM9DS1_OUT_Z_H_G         0x1D  //Gyroscope z-axis angular rate output register (high byte). The value is expressed as a 16-bit word in two’s complement with low byte
#define LSM9DS1_OUT_X_L_XL        0x28  //Accelerometer x-axis angular rate output register (low byte). The value is expressed as a 16-bit word in two’s complement with high byte
#define LSM9DS1_OUT_X_H_XL        0x29  //Accelerometer x-axis angular rate output register (high byte). The value is expressed as a 16-bit word in two’s complement with low byte
#define LSM9DS1_OUT_Y_L_XL        0x2A  //Accelerometer y-axis angular rate output register (low byte). The value is expressed as a 16-bit word in two’s complement with high byte
#define LSM9DS1_OUT_Y_H_XL        0x2B  //Accelerometer y-axis angular rate output register (high byte). The value is expressed as a 16-bit word in two’s complement with low byte
#define LSM9DS1_OUT_Z_L_XL        0x2C  //Accelerometer z-axis angular rate output register (low byte). The value is expressed as a 16-bit word in two’s complement with high byte
#define LSM9DS1_OUT_Z_H_XL        0x2D  //Accelerometer z-axis angular rate output register (high byte). The value is expressed as a 16-bit word in two’s complement with low byte

//FIFO Options
#define LSM9DS1_FIFO_CTRL         0x2E  //FIFO Control Register: Threshold level setting, Mode selection
#define LSM9DS1_FIFO_SRC          0x2F  //FIFO Status Register: Number of unread samples, Overrun status, Threshold status

//Status Registers
#define LSM9DS1_STATUS_REG1       0x17  //Status Register 1: New data available, Interrupt output signal, Inactivity interrupt signal, Boot status signal
#define LSM9DS1_STATUS_REG2       0x27  //Status Register 2: Same as status register 1

//Other Registers
#define LSM9DS1_ACT_THS           0x04  //Activity threshold register
#define LSM9DS1_ACT_DUR           0x05  //Inactivity duration register
#define LSM9DS1_REFERENCE_G       0x0B  //Angular rate sensor reference value register for digital high-pass filter (r/w)
#define LSM9DS1_WHO_AM_I          0x0F  //Register containing the slave address of the device

/* =========================================================================================================================== */
/* ================                                   Magnetometer Registers                                  ================ */
/* =========================================================================================================================== */

//Interrupt Status and Configuration Registers
#define LSM9DS1_INT_CFG_M         0x30
#define LSM9DS1_INT_SRC_M         0x31
#define LSM9DS1_INT_THS_L_M       0x32
#define LSM9DS1_INT_THS_H_M       0x33

//Control Registers
#define LSM9DS1_CTRL_REG1_M       0x20
#define LSM9DS1_CTRL_REG2_M       0x21
#define LSM9DS1_CTRL_REG3_M       0x22
#define LSM9DS1_CTRL_REG4_M       0x23
#define LSM9DS1_CTRL_REG5_M       0x24

//Data Output Registers
#define LSM9DS1_OUT_X_L_M         0x28
#define LSM9DS1_OUT_X_H_M         0x29
#define LSM9DS1_OUT_Y_L_M         0x2A
#define LSM9DS1_OUT_Y_H_M         0x2B
#define LSM9DS1_OUT_Z_L_M         0x2C
#define LSM9DS1_OUT_Z_H_M         0x2D

//Status Registers
#define LSM9DS1_STATUS_REG_M      0x27

//Offset Registers
#define LSM9DS1_OFFSET_X_REG_L_M  0x05
#define LSM9DS1_OFFSET_X_REG_H_M  0x06
#define LSM9DS1_OFFSET_Y_REG_L_M  0x07
#define LSM9DS1_OFFSET_Y_REG_H_M  0x08
#define LSM9DS1_OFFSET_Z_REG_L_M  0x09
#define LSM9DS1_OFFSET_Z_REG_H_M  0x0A

//Other Registers
#define LSM9DS1_WHO_AM_I_M        0x0F

#ifdef __cplusplus
}
#endif


//All of the below information is relavent for the C++ Application side
//uint8_t acc_settings[18]; //See info on each byte below
//uint8_t gyr_settings[18]; //See info on each byte below
//uint8_t mag_settings[18]; //See info on each byte below

/*
 * ACC Settings array options
 * Byte 0 (ODR Settings): 0x000 (off), 0x001 (10 Hz or 14.9Hz if gryo on), 0x010 (50 Hz or 59.5 Hz if gyro on), 0x011 (119 Hz), 0x100 (238 Hz), 0x101 (476 Hz), 0x110 (952 Hz)
 * Byte 1 (Fullscale Range Settings): 0x00 (+/- 2G), 0x01 (+/- 16G), 0x10 (+/- 4G), 0x11 (+/- 8G)
 * Byte 2 (Filter Settings): 0x000 (Auto mode, no extra filters), 0x001 (Auto mode, HPF on), 0x010 (Auto mode, LPF2 on), 0x100 (Manual mode, no extra filters), 0x101 (Manual mode, HPF on), 0x110 (Manual mode, LPF2 on)
 * Byte 3 (High Pass Filter Frequency): 0x00 (ODR/50), 0x01 (ODR/100), 0x10 (ODR/9), 0x11 (ODR/400)
 * Byte 4 (Low Pass Filter Frequency): 0x00 (408 Hz), 0x01 (211 Hz), 0x10 (105 Hz), 0x11 (50 Hz)
 * Byte 5 (Power/Mode Settings): 0x0 (Accelerometer only), 0x1 (Accelerometer and Gyroscope active)
 * Bytes 6-17 are currently not used and set at 0x00
 */

 /*
 * GYR Settings array options
 * Byte 0 (ODR Settings): 0x000 (off), 0x001 (14.9Hz), 0x010 (59.5 Hz), 0x011 (119 Hz), 0x100 (238 Hz), 0x101 (476 Hz), 0x110 (952 Hz)
 * Byte 1 (Fullscale Range Settings): 0x00 (+/- 245 deg/s), 0x01 (+/- 500 deg/s), 0x11 (+/- 2000 deg/s)
 * Byte 2 (Filter Settings [These are confusing in the documentation and should be verified]): 0x000 (LPF1 Only), 0x010 (LPF1 and LPF2), 0x101 (LPF1 and HPF), 0x111 (LPF1, LPF2 and HPF)
 * Byte 3 (High Pass Filter Frequency): 0x0000 (1Hz/4Hz/8Hz/15Hz/30Hz/57Hz ascending with ODR),
 *                                      0x0001 (.5Hz/2Hz/4Hz/8Hz/15Hz/30Hz ascending with ODR),
 *                                      0x0010 (.2Hz/1Hz/2Hz/4Hz/8Hz/15Hz ascending with ODR),
 *                                      0x0011 (.1Hz/.5Hz/1Hz/2Hz/4Hz/8Hz ascending with ODR),
 *                                      0x0100 (.05Hz/.2Hz/.5Hz/1Hz/2Hz/4Hz ascending with ODR),
 *                                      0x0101 (.02Hz/.1Hz/.2Hz/.5Hz/1Hz/2Hz ascending with ODR),
 *                                      0x0110 (.01Hz/.05Hz/.1Hz/.2Hz/.5Hz/1Hz ascending with ODR),
 *                                      0x0111 (.005Hz/.02Hz/.05Hz/.1Hz/.2Hz/.5Hz ascending with ODR),
 *                                      0x1000 (.002Hz/.01Hz/.02Hz/.05Hz/.1Hz/.2Hz ascending with ODR),
 *                                      0x1001 (.001Hz/.005Hz/.01Hz/.02Hz/.05Hz/.1Hz ascending with ODR)
 * Byte 4 (Low Pass Filter Frequency): 0x00 (0Hz/16Hz/14Hz/14Hz/21Hz/33Hz ascending with ODR),
 *                                     0x01 (0Hz/16Hz/31Hz/29Hz/28Hz/40Hz ascending with ODR),
 *                                     0x10 (0Hz/16Hz/31Hz/63Hz/57Hz/58Hz ascending with ODR),
 *                                     0x11 (0Hz/16Hz/31Hz/78Hz/100Hz/100Hz ascending with ODR)
 * Byte 5 (Power/Mode Settings): 0x0 (Normal Operating Mode), 0x1 (Low Power Mode)
 * Bytes 6-17 are currently not used and set at 0x00
 */

 /*
 * MAG Settings array options
 * Byte 0 (ODR Settings): 0x0000 (0.625 Hz), 0x0001 (1.25 Hz), 0x0010 (2.5 Hz), 0x0011 (5 Hz), 0x0100 (10 Hz), 0x0101 (20 Hz), 0x0110 (40 Hz), 0x0111 (80 Hz), 0x1000 (Fast ODR Mode, not sure what this entails, need to test)
 * Byte 1 (Fullscale Range Settings): 0x00 (+/- 4 Gauss), 0x01 (+/- 8 Gauss), 0x10 (+/- 12 Gauss), 0x11 (+/- 16 Gauss)
 * Byte 2 (Filter Settings): 0x0 (Currently no filter settings to adjust)
 * Byte 3 (High Pass Filter Frequency): 0x0 (Currently no filter settings to adjust)
 * Byte 4 (Low Pass Filter Frequency): 0x0 (Currently no filter settings to adjust)
 * Byte 5 (Power/Mode Settings): 0x000000 (Low Power XYZ, Continuous Conversion), 0x000100 (Low Power XY, Medium Performance Z, Continuous Conversion), 0x001000 (Low Power XY, High Performance Z, Continuous Conversion), 0x001100 (Low Power XY, Ultra Performance Z, Continuous Conversion),
 *                               0x000001 (Low Power XYZ, Single Conversion), 0x000101 (Low Power XY, Medium Performance Z, Single Conversion), 0x001001 (Low Power XY, High Performance Z, Single Conversion), 0x001101 (Low Power XY, Ultra Performance Z, Single Conversion),
 *                               0x010000 (Medium Performance XY, Low Power Z, Continuous Conversion), 0x010100 (Medium Performance XY, Medium Performance Z, Continuous Conversion), 0x011000 (Medium Performance XY, High Performance Z, Continuous Conversion), 0x011100 (Medium Performance XY, Ultra Performance Z, Continuous Conversion),
 *                               0x010001 (Medium Performance XY, Low Power Z, Single Conversion), 0x010101 (Medium Performance XY, Medium Performance Z, Single Conversion), 0x011001 (Medium Performance XY, High Performance Z, Single Conversion), 0x011101 (Medium Performance XY, Ultra Performance Z, Single Conversion),
 *                               0x100000 (High Performance XY, Low Power Z, Continuous Conversion), 0x100100 (High Performance XY, Medium Performance Z, Continuous Conversion), 0x101000 (High Performance XY, High Performance Z, Continuous Conversion), 0x101100 (High Performance XY, Ultra Performance Z, Continuous Conversion),
 *                               0x100001 (High Performance XY, Low Power Z, Single Conversion), 0x100101 (High Performance XY, Medium Performance Z, Single Conversion), 0x101001 (High Performance XY, High Performance Z, Single Conversion), 0x101101 (High Performance XY, Ultra Performance Z, Single Conversion),
 *                               0x110000 (Ultra Performance  XY, Low Power Z, Continuous Conversion), 0x110100 (Ultra Performance  XY, Medium Performance Z, Continuous Conversion), 0x111000 (Ultra Performance  XY, High Performance Z, Continuous Conversion), 0x111100 (Ultra Performance  XY, Ultra Performance Z, Continuous Conversion),
 *                               0x110001 (Ultra Performance  XY, Low Power Z, Single Conversion), 0x110101 (Ultra Performance  XY, Medium Performance Z, Single Conversion), 0x111001 (Ultra Performance  XY, High Performance Z, Single Conversion), 0x111101 (Ultra Performance  XY, Ultra Performance Z, Single Conversion),
 * Bytes 6-17 are currently not used and set at 0x00
 */

 /*
  * Some notes on the sensor settings array. The GYROSCOPE_FILTER_SETTINGS variable is a combination of a few different registers. Its 8 bits look
  * like this: [00000ABB] where the A bit represents if the hi-pass filter is enabled and BB bits represent which filters get used. The
  * ACCELEROMETER_FILTER_SETTINGS variable is also a combination of a few different registers. Its 8 bits look like this: [00000ABC] where the A
  * bit represents if the anti-aliasing filter is set to custom or auto, the B bit represents if the LPF2 is on or off and the C bit represents which
  * filter(s) get utilized for the final output value. The MAGNETOMETER_ODR variable is a combination of both the ODR and whether or not FastODR mode is
  * enabled. Its 8 bits look like this: [0000AAAB] where the AAA bits represents the ODR and B represents FastODR mode. Finally, the MAGNETOMETER_POWER_MODE
  * variable is a combination of a few registers. Its 8 bits
  * look like this [00AABBCC] where the AA bits represent the power mode for the X and Y axes, the BB bits represent the power mode for the Z axis, and the CC
  * bits represent whether the magnetometer is in continous, single, or off mode.
  */