#include "fxos8700.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "sensor_settings.h"
#include "personal_caddie_operating_modes.h"

//set up settings variables
static imu_communication_t* imu_comm;
static uint8_t*             p_sensor_settings;
static fxos8700_driver_t    sensor_driver;
static sensor_comm_handle_t fxos_com;

void fxos8700init(imu_communication_t* comm, uint8_t* settings)
{
    //create a pointer to an array which holds settings for the sensor
    p_sensor_settings = settings;

    //set up communication with the chip. The FXOS driver comes with their
    //own communication struct which we must utilize to communicate with the
    //chip. If both the FXOS acc and mag ar in use then the built-in communication
    //struct will default to the magnetometer.
    imu_comm = comm;

    //initialize read/write methods, address, and default settings for acc
    if (imu_comm->sensor_model[ACC_SENSOR] == FXOS8700_ACC)
    {
        //Set up the communication struct needed to use the FXOS8700 driver using my own communication struct
        fxos_com.pComm = (void*)(&comm->acc_comm);
        sensor_driver.pComHandle = &fxos_com; 
        fxos8700_driver_t* tester = &sensor_driver;
        
        //Apply default settings for the acc
        update_sensor_setting(p_sensor_settings + ACC_START, FS_RANGE, FXOS8700_XYZ_DATA_CFG_FS_4G_0P488); //accelerometer full scale range (+/- 4 g)

        if (imu_comm->sensor_model[MAG_SENSOR] == FXOS8700_MAG) update_sensor_setting(p_sensor_settings + ACC_START, ODR, FXOS8700_ODR_HYBRID_50_HZ); //accelerometer + magnetomter ODR (50 Hz)
        else update_sensor_setting(p_sensor_settings + ACC_START, ODR, FXOS8700_ODR_SINGLE_50_HZ); //accelerometer ODR (50 Hz)
        
        update_sensor_setting(p_sensor_settings + ACC_START, POWER, FXOS8700_ACCEL_NORMAL); //accelerometer Power (normal power mode)

        update_sensor_setting(p_sensor_settings + ACC_START, FILTER_SELECTION, FXOS8700_XYZ_DATA_CFG_HPF_OUT_DISABLE); //accelerometer filter selection (high pass filter disabled)
        update_sensor_setting(p_sensor_settings + ACC_START, HIGH_PASS_FILTER, FXOS8700_HP_FILTER_CUTOFF_SEL_DISABLE); //accelerometer high pass filter setting (frequency selection disabled)

        //After setting default settings, attempt to read the whoAmI register
        uint8_t whoamI;
        uint8_t ret = sensor_comm_read(sensor_driver.pComHandle, FXOS8700_WHO_AM_I, 1, &whoamI);
        if (whoamI == FXOS8700_WHO_AM_I_PROD_VALUE) SEGGER_RTT_WriteString(0, "FXOS8700 Acc initialized.\n");
        else SEGGER_RTT_WriteString(0, "Error: Couldn't initialize FXOS8700 Acc.\n");
    }

    //initialize read/write methods, address, and default settings for mag
    if (imu_comm->sensor_model[MAG_SENSOR] == FXOS8700_MAG)
    {
        //Set up the communication struct needed to use the FXOS8700 driver using my own communication struct.
        //This will overwrite the acc communication if it's in place, but this is ok because both mag and acc
        //have the same address and communication methods.
        fxos_com.pComm = (void*)(&comm->mag_comm);
        sensor_driver.pComHandle = &fxos_com; 
        fxos8700_driver_t* tester = &sensor_driver;

        //Apply default settings for the mag. Unlike the accelerometer, the magnetometer
        //has a fixed full-scale range of +/- 1200 uT. The power mode is share. Furthermore,
        //low power modes are only applied to the acc, not the mag, so really the only thing
        //we set here is the mag ODR.
        if (imu_comm->sensor_model[ACC_SENSOR] == FXOS8700_ACC) update_sensor_setting(p_sensor_settings + MAG_START, ODR, FXOS8700_ODR_HYBRID_50_HZ); //magnetometer + accelerometer ODR (100 Hz)
        else update_sensor_setting(p_sensor_settings + MAG_START, ODR, FXOS8700_ODR_SINGLE_50_HZ); //magnetometer ODR (50 Hz)
        
        update_sensor_setting(p_sensor_settings + MAG_START, POWER, 0); //power (indicates on)

        //After setting default settings, attempt to read the whoAmI register
        uint8_t whoamI;
        uint8_t ret = sensor_comm_read(sensor_driver.pComHandle, FXOS8700_WHO_AM_I, 1, &whoamI);
        if (whoamI == FXOS8700_WHO_AM_I_PROD_VALUE) SEGGER_RTT_WriteString(0, "FXOS8700 Mag initialized.\n");
        else SEGGER_RTT_WriteString(0, "Error: Couldn't initialize FXOS8700 Mag.\n");
    }
}

int32_t fxos8700_connected_mode_enable()
{
    //In connected mode the fxos8700 is placed into its "Standby" power mode. In this power
    //mode the expected current consumption is 2 uA (0.000002 A). In this mode we can read
    //any register but none of the analogue circuitry needed for the IMU readings are
    //enabled. The sensor enters this mode by default when it first gets powered on so 
    //there's no need to do anything the first time connected mode is entered.
    if (imu_comm->sensor_model[ACC_SENSOR] != FXOS8700_ACC && imu_comm->sensor_model[MAG_SENSOR] != FXOS8700_MAG) return 0; //only carry out this method if an FXOS sensor is active

    uint8_t ret = fxos8700_set_mode(&sensor_driver, FXOS8700_STANDBY_MODE); //First put the chip in standby mode
    if (ret != 0) SEGGER_RTT_WriteString(0, "Error: Couldn't place FXOS8700 into connected mode.\n");

    return ret;
}

int32_t fxos8700_idle_mode_enable(int current_mode)
{
    //Unlike other sensors, the FXOS8700 doesn't have an intermediate power mode. If we would
    //normally be going into sensor idle mode from connected mode, we instead just call the 
    //active mode enable method. If we'd noramlly be entering sensor idle mode from sensor
    //active mode we don't do anything here.
    if (imu_comm->sensor_model[ACC_SENSOR] != FXOS8700_ACC && imu_comm->sensor_model[MAG_SENSOR] != FXOS8700_MAG) return 0; //only carry out this method if an FXOS sensor is active

    if (current_mode == CONNECTED_MODE) return fxos8700_active_mode_enable();
    else return 0;
}

int32_t fxos8700_active_mode_enable()
{
    //First we apply the acc full-scale range and filter settings (if the acc is active).
    //We then handle the ODR and Power settings as these values are updated using the same
    //driver method (which also sets the sensor mode to active).
    if (imu_comm->sensor_model[ACC_SENSOR] != FXOS8700_ACC && imu_comm->sensor_model[MAG_SENSOR] != FXOS8700_MAG) return 0; //only carry out this method if an FXOS sensor is active

    int32_t ret = 0;

    uint8_t reg_val;

    if (imu_comm->sensor_model[ACC_SENSOR] == FXOS8700_ACC)
    {
        ret |= fxos8700_acc_apply_setting(POWER + ACC_START);
        ret |= fxos8700_acc_apply_setting(FS_RANGE + ACC_START);
        ret |= fxos8700_acc_apply_setting(FILTER_SELECTION + ACC_START);
        ret |= fxos8700_acc_apply_setting(HIGH_PASS_FILTER + ACC_START);
    }

    bool acc_on = true, mag_on = true;
    if (imu_comm->sensor_model[ACC_SENSOR] != FXOS8700_ACC || p_sensor_settings[ACC_START + POWER] == FXOS8700_OFF) acc_on = false;
    if (imu_comm->sensor_model[MAG_SENSOR] != FXOS8700_MAG || p_sensor_settings[MAG_START + POWER] == FXOS8700_OFF) mag_on = false;

    if (acc_on && mag_on)
    {
        //Both sensors are present and on so we need to start the chip in hybrid mode. The ODR's for the sensors
        //should be set at the same value, but in the case they aren't take the lower of the two ODRs.
        //Ironically, the larger the setting number is the lower the ODR is.
        uint8_t hybrid_odr = *(p_sensor_settings + MAG_START + ODR) > *(p_sensor_settings + ACC_START + ODR) ?
            *(p_sensor_settings + MAG_START + ODR) : *(p_sensor_settings + ACC_START + ODR);

        ret |= fxos8700_configure_hybrid(&sensor_driver, hybrid_odr, FXOS8700_HYBRID_READ_POLL_MODE);
    }
    else if (acc_on) ret |= fxos8700_configure_accel(&sensor_driver, *(p_sensor_settings + ACC_START + ODR), FXOS8700_ACCEL_NORMAL, FXOS8700_ACCEL_14BIT_READ_POLL_MODE);
    else if (mag_on) ret |= fxos8700_configure_mag(&sensor_driver, *(p_sensor_settings + MAG_START + ODR), FXOS8700_MAG_READ_POLLING_MODE);

    if (ret != 0) SEGGER_RTT_WriteString(0, "Error: FXOS8700 enabled with incorrect settings.\n");

    //After the sensor is placed into active mode, wait for the appropriate amount of time
    //before attempting to take any readings. Normally this this time is dependend on the ODR
    //of the sensor, however, in the worst case scenario the wait time is only 4.56 milliseconds
    //so we just wait for 5 milliseconds regardless.
    sensor_communication_t* comm = (sensor_communication_t*)fxos_com.pComm;
    comm->delay(5000); //delay for 5000 microseconds

    return ret;
}

int32_t fxos8700_acc_apply_setting(uint8_t setting)
{
    //the acc odr, power and full scale range are all updated with the same method.
    //If we want to update only one of these settings then we need to read the other 
    //two from the sensor to see what they are (this is because the sensor settings array
    //can be updated via Bluetooth from the PC app and we have no way to know if all of
    //these settings have been changed, or just one of them). It would be possible to just
    //manually write a single register, however, I'd rather use the built in method from 
    //the driver.

    //Regardless of the setting being updated, we need to put the chip into standby mode
    uint8_t status = fxos8700_set_mode(&sensor_driver, FXOS8700_STANDBY_MODE);

    switch (setting)
    {
        case (FS_RANGE + ACC_START):
        {
            //Read the data configuration register to get the other settings
            uint8_t register_value;
            sensor_comm_read(sensor_driver.pComHandle, FXOS8700_XYZ_DATA_CFG, 1, &register_value);

            //remove the current full-scale range setting and then apply the new one
            register_value &= (0xFF ^ (FXOS8700_XYZ_DATA_CFG_FS_MASK << FXOS8700_XYZ_DATA_CFG_FS_SHIFT)); //remove the original setting
            register_value |= (p_sensor_settings[setting] << FXOS8700_XYZ_DATA_CFG_FS_SHIFT); //then add the new one

            //finally update the register with the new value
            if (status == SENSOR_SUCCESS) status |= sensor_comm_write(sensor_driver.pComHandle, FXOS8700_XYZ_DATA_CFG, 1, &register_value);
            if (status != SENSOR_SUCCESS) SEGGER_RTT_WriteString(0, "Error: Couldn't update FXOS8700 Acc Full-Scale Range.\n");

            break;
        }
        case (ODR + ACC_START):
        {
            //Need to get the current power mode from the chip
            uint8_t power;
            sensor_comm_read(sensor_driver.pComHandle, FXOS8700_CTRL_REG2, 1, &power);

            //We need to right shift and use bitwise AND to extract the appropriate setting
            power &= FXOS8700_CTRL_REG2_SMODS_MASK;

            //We can now update the odr while keeping the same power setting. If the 
            //magnetometer is also on then we need to call the hybrid configuration method.
            status |= fxos8700_configure_accel(&sensor_driver, p_sensor_settings[setting], power, FXOS8700_ACCEL_14BIT_READ_POLL_MODE);

            break;
        }
        case (POWER + ACC_START):
        {
            //Need to get the current odr from the chip
            uint8_t odr;
            sensor_comm_read(sensor_driver.pComHandle, FXOS8700_CTRL_REG1, 1, &odr);

            //We need to right shift and use bitwise and to extract the appropriate setting
            odr &= FXOS8700_CTRL_REG1_DR_MASK;

            //we can now update the power while keeping the same odr setting
            status |= fxos8700_configure_accel(&sensor_driver, odr, p_sensor_settings[setting], FXOS8700_ACCEL_14BIT_READ_POLL_MODE);

            break;
        }
        case (FILTER_SELECTION + ACC_START):
        {
            //Read the data configuration register to get the other settings
            uint8_t register_value;
            sensor_comm_read(sensor_driver.pComHandle, FXOS8700_XYZ_DATA_CFG, 1, &register_value);

            //remove the current filter setting and then apply the new one
            register_value &= (0xFF ^ (FXOS8700_XYZ_DATA_CFG_HPF_OUT_MASK << FXOS8700_XYZ_DATA_CFG_HPF_OUT_SHIFT)); //remove the original setting
            register_value |= (p_sensor_settings[setting] << FXOS8700_XYZ_DATA_CFG_HPF_OUT_SHIFT); //then add the new one

            //finally update the register with the new value
            if (status == SENSOR_SUCCESS) status |= sensor_comm_write(sensor_driver.pComHandle, FXOS8700_XYZ_DATA_CFG, 1, &register_value);
            if (status != SENSOR_SUCCESS) SEGGER_RTT_WriteString(0, "Error: Couldn't update FXOS8700 Acc Filter Selection.\n");

            break;
        }
        case (HIGH_PASS_FILTER + ACC_START):
        {
            //Read the filter configuration register to get the other settings
            uint8_t register_value;
            sensor_comm_read(sensor_driver.pComHandle, FXOS8700_HP_FILTER_CUTOFF, 1, &register_value);

            //remove the current filter setting and then apply the new one
            register_value &= (0xFF ^ (FXOS8700_HP_FILTER_CUTOFF_SEL_MASK << FXOS8700_HP_FILTER_CUTOFF_SEL_SHIFT)); //remove the original setting
            register_value |= (p_sensor_settings[setting] << FXOS8700_HP_FILTER_CUTOFF_SEL_SHIFT); //then add the new one

            //finally update the register with the new value
            if (status == SENSOR_SUCCESS) status |= sensor_comm_write(sensor_driver.pComHandle, FXOS8700_HP_FILTER_CUTOFF, 1, &register_value);
            if (status != SENSOR_SUCCESS) SEGGER_RTT_WriteString(0, "Error: Couldn't update FXOS8700 Acc High Pass Filter Cut-off.\n");

            break;
        }
        default:
            SEGGER_RTT_WriteString(0, "Error: Incorrect setting entered for FXOS8700 Acc.\n");
    }

    //Some of the built-in driver methods automatically put the sensor into active mode, so make
    //sure the sensor is in standby mode when leaving this method
    status |= fxos8700_set_mode(&sensor_driver, FXOS8700_STANDBY_MODE);
    return status;
}

int32_t fxos8700_mag_apply_setting(uint8_t setting)
{
    //The magnetometer doesn't have as many settings as the accelerometer does, in fact,
    //the only thing we can actually change is the ODR.

    uint8_t status = 0;
    switch (setting)
    {
        case (ODR + MAG_START):
        {
            status |= fxos8700_configure_mag(&sensor_driver, p_sensor_settings[setting], FXOS8700_MAG_READ_POLLING_MODE);
            break;
        }
        default:
            SEGGER_RTT_WriteString(0, "Error: Incorrect setting entered for FXOS8700 Mag.\n");
    }

    //Put the sensor back into standby mode when leaving this method
    status |= fxos8700_set_mode(&sensor_driver, FXOS8700_STANDBY_MODE);
    return status;
}

void fxos8700_get_actual_settings()
{
    //For debugging purposes it's nice to see that the settings we have stored in the sensor array
    //physically make their way onto the chip. This method prints out the current register values
    //of some of the more important registers
    uint8_t reg_val;

    if (imu_comm->sensor_model[ACC_SENSOR] == FXOS8700_ACC)
    {
        SEGGER_RTT_WriteString(0, "FXOS8700 Acc. Register Values:\n");
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, FXOS8700_SYSMOD, &reg_val, 1);
        SEGGER_RTT_printf(0, "SYSMOD Register: 0x%x\n", reg_val);
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, FXOS8700_CTRL_REG1, &reg_val, 1);
        SEGGER_RTT_printf(0, "CTRL_REG1 Register: 0x%x\n", reg_val);
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, FXOS8700_CTRL_REG2, &reg_val, 1);
        SEGGER_RTT_printf(0, "CTRL_REG2 Register: 0x%x\n", reg_val);
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, FXOS8700_CTRL_REG3, &reg_val, 1);
        SEGGER_RTT_printf(0, "CTRL_REG3 Register: 0x%x\n", reg_val);
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, FXOS8700_CTRL_REG4, &reg_val, 1);
        SEGGER_RTT_printf(0, "CTRL_REG4 Register: 0x%x\n", reg_val);
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, FXOS8700_CTRL_REG5, &reg_val, 1);
        SEGGER_RTT_printf(0, "CTRL_REG5 Register: 0x%x\n", reg_val);
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, FXOS8700_XYZ_DATA_CFG, &reg_val, 1);
        SEGGER_RTT_printf(0, "XYZ_DATA_CONFIG Register: 0x%x\n", reg_val);
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, FXOS8700_HP_FILTER_CUTOFF, &reg_val, 1);
        SEGGER_RTT_printf(0, "HP_FILTER_CUTOFF Register: 0x%x\n\n", reg_val);
    }

    //initialize read/write methods, address, and default settings for mag
    if (imu_comm->sensor_model[MAG_SENSOR] == FXOS8700_MAG)
    {
        SEGGER_RTT_WriteString(0, "FXOS8700 Mag. Register Values:\n");
        imu_comm->mag_comm.read_register((void*)imu_comm->mag_comm.twi_bus,  imu_comm->mag_comm.address, FXOS8700_M_CTRL_REG1, &reg_val, 1);
        SEGGER_RTT_printf(0, "M_CTRL_REG1 Register: 0x%x\n", reg_val);
        imu_comm->mag_comm.read_register((void*)imu_comm->mag_comm.twi_bus,  imu_comm->mag_comm.address, FXOS8700_M_CTRL_REG2, &reg_val, 1);
        SEGGER_RTT_printf(0, "M_CTRL_REG2 Register: 0x%x\n", reg_val);
        imu_comm->mag_comm.read_register((void*)imu_comm->mag_comm.twi_bus,  imu_comm->mag_comm.address, FXOS8700_M_CTRL_REG3, &reg_val, 1);
        SEGGER_RTT_printf(0, "M_CTRL_REG3 Register: 0x%x\n\n", reg_val);
    }
}

int32_t fxos8700_get_acc_data(uint8_t* pBuff, uint8_t offset)
{
    //create a fxos8700 dataType struct and zero all values
    fxos8700_data_t dataBuffer;
    //for (int i = 0; i < NUM_AXES; i++) dataBuffer.mag[i] = 0;
    //for (int i = 0; i < NUM_AXES * FIFO_SIZE; i++) dataBuffer.accel[i] = 0;

    //Then read the chip, storing the data in the above struct
    uint8_t ret;
    ret = fxos8700_read_data(&sensor_driver, FXOS8700_ACCEL_14BIT_DATAREAD, &dataBuffer);

    //TEST: Print out the bytes as they're read
    //SEGGER_RTT_WriteString(0, "Raw FXOS Acc reading.\n");
    //SEGGER_RTT_printf(0, "%x %x %x\n", dataBuffer.accel[0], dataBuffer.accel[1], dataBuffer.accel[2]);
    //SEGGER_RTT_WriteString(0, "\n");

    //Acc data is read into the dataBuffer object, we need to extract it and put it into our
    //own data buffer, one byte at a time, need to be careful of endianness here
    if (__BYTE_ORDER__ == 1234)
    {
        //This byte order represents little endian mode so we need to put the less significant
        //byte before the most significant byte. The chip already stores data in little endian
        //so really we just copy the data as it is
        for (uint8_t i = 0; i < 3; i++)
        {
            pBuff[2 * i + offset] = dataBuffer.accel[i] & 0xFF;
            pBuff[2 * i + 1 + offset] = (dataBuffer.accel[i] & 0xFF00) >> 8;
        }
    }
    else
    {
        //This byte order represents big endian mode so we need to put the less significant
        //byte after the most significant byte. Since the chip stores data in little endian
        //we need to swap bytes as they come in.
        for (uint8_t i = 0; i < 3; i++)
        {
            pBuff[2 * i + offset] = (dataBuffer.accel[i] & 0xFF00) >> 8;;
            pBuff[2 * i + 1 + offset] = dataBuffer.accel[i] & 0xFF;

            //Erase below when done debugging
            //SEGGER_RTT_printf(0, "Buffer %d = 0x%#01x\n", 2 * i + offset, (dataBuffer.accel[i] & 0xFF00) >> 8);
            //SEGGER_RTT_printf(0, "Buffer %d = 0x%#01x\n", 2 * i + 1 + offset, dataBuffer.accel[i] & 0xFF);
        }
        //SEGGER_RTT_WriteString(0, "\n");
    }

    return ret;
}

int32_t fxos8700_get_mag_data(uint8_t* pBuff, uint8_t offset)
{
    fxos8700_data_t dataBuffer;
    uint8_t ret = fxos8700_read_data(&sensor_driver, FXOS8700_MAG_DATAREAD, &dataBuffer);

    //Mag data is read into the dataBuffer object, we need to extract it and put it into our
    //own data buffer, one byte at a time, need to be careful of endianness here
    if (__BYTE_ORDER__ == 1234)
    {
        //This byte order represents little endian mode so we need to put the less significant
        //byte before the most significant byte. The chip already stores data in little endian
        //so really we just copy the data as it is
        for (uint8_t i = 0; i < 3; i++)
        {
            pBuff[2 * i + offset] = dataBuffer.mag[i] & 0xFF;
            pBuff[2 * i + 1 + offset] = (dataBuffer.mag[i] & 0xFF00) >> 8;
        }
    }
    else
    {
        //This byte order represents big endian mode so we need to put the less significant
        //byte after the most significant byte. Since the chip stores data in little endian
        //we need to swap bytes as they come in.
        for (uint8_t i = 0; i < 3; i++)
        {
            pBuff[2 * i + offset] = (dataBuffer.mag[i] & 0xFF00) >> 8;;
            pBuff[2 * i + 1 + offset] = dataBuffer.mag[i] & 0xFF;
        }
    }

    return ret;
}