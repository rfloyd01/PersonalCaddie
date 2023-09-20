#include "fxos8700.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

//set up settings variables
static imu_communication_t* imu_comm;
static uint8_t*             p_sensor_settings;
static fxos8700_driver_t    sensor_driver;
static sensor_comm_handle_t fxos_com;

//TODO: Should add fxas21002 stuff here in the future

void fxos8700init(imu_communication_t* comm, uint8_t sensors, uint8_t* settings)
{
    //create a pointer to an array which holds settings for the sensor
    p_sensor_settings = settings;

    //set up communication with the chip. The FXOS driver comes with their
    //own communication struct which we must utilize to communicate with the
    //chip. If both the FXOS acc and mag ar in use then the built-in communication
    //struct will default to the magnetometer.
    imu_comm = comm;

    //initialize read/write methods, address, and default settings for acc
    if (sensors & 0b001)
    {
        //Set up the communication struct needed to use the FXOS8700 driver using my own communication struct
        fxos_com.pComm = (void*)(&comm->acc_comm);
        sensor_driver.pComHandle = & fxos_com; 
        fxos8700_driver_t* tester = &sensor_driver;
        
        //Apply default settings for the acc
        update_sensor_setting(p_sensor_settings + ACC_START, FS_RANGE, FXOS8700_XYZ_DATA_CFG_FS_4G_0P488); //accelerometer full scale range (+/- 4 g)

        update_sensor_setting(p_sensor_settings + ACC_START, ODR, FXOS8700_ODR_SINGLE_100_HZ); //accelerometer ODR (100 Hz)
        update_sensor_setting(p_sensor_settings + ACC_START, POWER, FXOS8700_ACCEL_NORMAL); //accelerometer Power (normal power mode)

        update_sensor_setting(p_sensor_settings + ACC_START, FILTER_SELECTION, FXOS8700_XYZ_DATA_CFG_HPF_OUT_DISABLE); //accelerometer filter selection (high pass filter disabled)
        update_sensor_setting(p_sensor_settings + ACC_START, HIGH_PASS_FILTER, FXOS8700_HP_FILTER_CUTOFF_SEL_DISABLE); //accelerometer high pass filter setting (frequency selection disabled)

        //After setting default settings, attempt to read the whoAmI register
        uint8_t whoamI;
        uint8_t ret = sensor_comm_read(sensor_driver.pComHandle, FXOS8700_WHO_AM_I, 1, &whoamI);
        if (whoamI == FXOS8700_WHO_AM_I_PROD_VALUE) SEGGER_RTT_WriteString(0, "FXOS8700 Acc discovered.\n");
        else SEGGER_RTT_WriteString(0, "Error: Couldn't find FXOS8700 Acc.\n");
    }

    //initialize read/write methods, address, and default settings for mag
    if (sensors & 0b100)
    {
        //Set up the communication struct needed to use the FXOS8700 driver using my own communication struct
        sensor_driver.pComHandle = (void*)(&comm->mag_comm);
        //lsm9ds1_mag.read_reg = lsm9ds1_read_mag;
        //lsm9ds1_mag.write_reg = lsm9ds1_write_mag;
        //MAG_Address = comm->mag_comm.address;

        //Apply default settings for the mag
        //update_sensor_setting(p_sensor_settings + MAG_START, FS_RANGE, LSM9DS1_4Ga); //magnetometer full scale range (+/- 4 Gauss)

        //update_sensor_setting(p_sensor_settings + MAG_START, ODR, LSM9DS1_MAG_LP_40Hz); //magnetomer ODR and Power (40 Hz, magnetometer in low power mode)
        //update_sensor_setting(p_sensor_settings + MAG_START, POWER, LSM9DS1_MAG_LP_40Hz); //magnetomer ODR and Power (40 Hz, magnetometer in low power mode)

        ////After setting default settings, attempt to read the whoAmI register
        //uint32_t ret = lsm9ds1_dev_id_get(&lsm9ds1_mag, NULL, &whoamI);
        //if (whoamI.mag == LSM9DS1_MAG_ID) SEGGER_RTT_WriteString(0, "LSM9DS1 Mag discovered.\n");
        //else SEGGER_RTT_WriteString(0, "Error: Couldn't find LSM9DS1 Mag.\n");
    }
}