#include "bmi270_drv.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "sensor_settings.h"
#include "personal_caddie_operating_modes.h"

//set up settings variables
static imu_communication_t* imu_comm;
static uint8_t*             p_sensor_settings;

static struct bmi2_dev    bmi270; //driver defined struct for holding functional pointers and other info

//custom enums
enum bmi270_power_mode {
    BMI270_SUSPEND_POWER_MODE = 0,
    BMI270_CONFIGURATION_POWER_MODE = 1,
    BMI270_LOW_POWER_MODE = 2,
    BMI270_NORMAL_POWER_MODE = 3,
    BMI270_PERFORMANCE_POWER_MODE = 4
};

void bmi270init(imu_communication_t* comm, uint8_t* settings)
{
    //create a pointer to an array which holds settings for the sensor
    p_sensor_settings = settings;

    //set up communication with the chip. The FXOS driver comes with their
    //own communication struct which we must utilize to communicate with the
    //chip. If both the FXOS acc and mag ar in use then the built-in communication
    //struct will default to the magnetometer.
    imu_comm = comm;

    //initialize functional pointers in the driver defined struct for things like
    //reading and writing
    bmi270.intf = BMI2_I2C_INTF; //set the I2C interface
    bmi270.read = bmi270_read_register; //set the I2C read functional pointer
    bmi270.write = bmi270_write_register; //set the I2C write functional pointer
    bmi270.delay_us = bmi270_delay; //set the microsecond delay functional pointer
    
    //First we set up the communication interface pointer and then call the 
    //driver's built-in init method. This method performs a soft-reset of the 
    //device which puts all registers back to their default values so it must
    //be called before we put our own values into the registers.
    if (imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC)  bmi270.intf_ptr = (void*) &imu_comm->acc_comm;
    else bmi270.intf_ptr = (void*) &imu_comm->gyr_comm;

    if (imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC)
    {
        //Apply default settings for the acc
        update_sensor_setting(p_sensor_settings + ACC_START, FS_RANGE, BMI2_ACC_RANGE_4G); //accelerometer full scale range (+/- 2000 dps)

        update_sensor_setting(p_sensor_settings + ACC_START, ODR, BMI2_ACC_ODR_50HZ); //accelerometer ODR (50 Hz)
        update_sensor_setting(p_sensor_settings + ACC_START, POWER, BMI270_NORMAL_POWER_MODE); //accelerometer power mode (normal power mode)

        update_sensor_setting(p_sensor_settings + ACC_START, LOW_PASS_FILTER, BMI2_ACC_NORMAL_AVG4); //accelerometer low pass filter bandwidth (22Hz at 50Hz odr and normal mode)
        update_sensor_setting(p_sensor_settings + ACC_START, EXTRA_FILTER, BMI2_PERF_OPT_MODE); //accelerometer low pass filter performance (performance optimized)

        SEGGER_RTT_WriteString(0, "BMI270 Acc initialized.\n");
    }

    //initialize read/write methods, address, and default settings for gyro
    if (imu_comm->sensor_model[GYR_SENSOR] == BMI270_GYR)
    {
        //Apply default settings for the gyro
        update_sensor_setting(p_sensor_settings + GYR_START, FS_RANGE, BMI2_GYR_RANGE_2000); //gyroscope full scale range (+/- 2000 dps)

        update_sensor_setting(p_sensor_settings + GYR_START, ODR, BMI2_GYR_ODR_50HZ); //gyroscope ODR (50 Hz)
        update_sensor_setting(p_sensor_settings + GYR_START, POWER, BMI270_NORMAL_POWER_MODE); //gyroscope power mode (normal power mode)

        update_sensor_setting(p_sensor_settings + GYR_START, LOW_PASS_FILTER, BMI2_GYR_NORMAL_MODE); //gyroscope low pass filter bandwitdh (16Hz at 50Hz odr)
        update_sensor_setting(p_sensor_settings + GYR_START, EXTRA_FILTER, BMI2_PERF_OPT_MODE); //gyroscope low pass filter performance (performance optimized)
        update_sensor_setting(p_sensor_settings + GYR_START, EXTRA_1, BMI2_POWER_OPT_MODE); //gyroscope low pass filter noise performance (power optimized)

        SEGGER_RTT_WriteString(0, "BMI270 Gyr initialized.\n");
    }
}

int32_t bmi270_connected_mode_enable(bool init)
{
    //In connected mode the BMI270 sensor is placed into its "Suspend" power mode. In this mode
    //the average current draw is only 3.5 uA (0.0000035 A). While in suspend mode, the IMU
    //functions of the chip are inaccessible, however, the information in all registers is
    //retained.

    //Any time the chip is first powered on (or initiates a soft reset) a configuration file needs to be 
    //loaded to ensure the sensor works properly. The only times a soft reset are initiated automatically
    //within the BMI270 API are when the bmi270_init(), bmi2_nvm_prog() or bmi2_perform_accel_self_test()
    //methods are called. Because of this, when connected mode is first entered we call the init
    //method here. Any other time connected mode is entered (i.e. when we go back to one of the 
    //menus in the front end app) there's no need to call the init method again as the 
    //configuration document has already been loaded up.
    if (imu_comm->sensor_model[ACC_SENSOR] != BMI270_ACC && imu_comm->sensor_model[GYR_SENSOR] != BMI270_GYR) return 0; //no need to do anything if neither sensor is in use
    
    int8_t rslt = 0;
    if (init)
    {
        rslt = bmi270_init(&bmi270); //The sensor will be in suspend mode when this method returns
        if (rslt != BMI2_OK) SEGGER_RTT_WriteString(0, "Error: Couldn't initialize BMI270 Sensor.\n");
    }
    else
    {
        //When coming to connected mode from sensor idle mode or sensor active mode, we need
        //to ensure that the acc. and gyr. power bits of the POWER_CTRL register are 0 and the
        //advanced power save bit of the POWER_CONF register is set to 1 to put the chip back into
        //the Suspend power mode.
        uint8_t sensor_list[2] = { BMI2_ACCEL, BMI2_GYRO };
        rslt = bmi2_sensor_disable(sensor_list, 2, &bmi270);
        if (rslt != BMI2_OK) SEGGER_RTT_WriteString(0, "Error: Couldn't put BMI270 Sensor into Connected Mode.\n");

        rslt = bmi2_set_adv_power_save(1, &bmi270);
        if (rslt != BMI2_OK) SEGGER_RTT_WriteString(0, "Error: Couldn't put BMI270 Sensor into Connected Mode.\n");
    }

    return rslt;
}

int32_t bmi270_idle_mode_enable(int current_mode)
{
    //In sensor idle mode the BMI270 is placed into its "Configuration" power mode. In this mode
    //the average current draw is 120 uA (0.000120 A). While in configuration mode, the IMU functions
    //are accessible although the chip isn't actively measuring anything. To enter configuration mode
    //from the Personal Caddie connected mode all we need to do is write the advanced power saving bit
    //of the POWER_CONF register to 0. To enter configuration mode from the Personal Caddie sensor active
    //mode we also need to write the acc and gyr power bits of the POWER_CTRL register to 0.
    if (imu_comm->sensor_model[ACC_SENSOR] != BMI270_ACC && imu_comm->sensor_model[GYR_SENSOR] != BMI270_GYR) return 0; //no need to do anything if neither sensor is in use
    
    int8_t rslt = 0;
    if (current_mode == SENSOR_ACTIVE_MODE)
    {
        //We've come from sensor active mode so we need to write the acc and gyr power bits.
        uint8_t sensor_list[2] = { BMI2_ACCEL, BMI2_GYRO };
        rslt = bmi2_sensor_disable(sensor_list, 2, &bmi270);

        if (rslt != BMI2_OK) SEGGER_RTT_WriteString(0, "Error: Couldn't take BMI270 Sensor out of Sensor Active Mode.\n");
    }
    
    //Make sure that the advanced power save bit is set to 0
    rslt = bmi2_set_adv_power_save(0, &bmi270);
    if (rslt != BMI2_OK) SEGGER_RTT_WriteString(0, "Error: Couldn't put BMI270 Sensor into Connected Mode.\n");

    return rslt;
}

int32_t bmi270_active_mode_enable(int current_mode)
{
    //In sensor active mode there are three different power levels for the BMI270. There is "Low" power mode
    //which has a current draw of ~0.42 mA, "Normal" power mode which has a current draw of ~0.685 mA and 
    //"Performance" power mode which has a current draw of ~0.97 mA. Depending on the settings currently in
    //the sensor settings array the chip will enter one of these power states. To ensure we get proper readings
    //a delay of 45 milliseconds (45,000 microseconds) is applied when coming from connected mode and a delay of 
    //2 milliseconds (2,000 microseconds) is applied
    if (imu_comm->sensor_model[ACC_SENSOR] != BMI270_ACC && imu_comm->sensor_model[GYR_SENSOR] != BMI270_GYR) return 0; //Make sure that one of the bmi270 sensors is actually in use

    int8_t rslt = 0;

    //Add both sensor types to the config structure, even if only 1 is actuall in use
    struct bmi2_sens_config config[2];

    config[ACC_SENSOR].type = BMI2_ACCEL;
    config[GYR_SENSOR].type = BMI2_GYRO;

    //Get the existing configuration for the sensors
    rslt = bmi2_get_sensor_config(config, 2, &bmi270);

    //Set ODR. I use a custom ODR value to turn individual sensors
    //off. Since this won't be a valid value for the driver I need 
    //to handle this case specially
    if (p_sensor_settings[ACC_START + ODR] != 0) config[ACC_SENSOR].cfg.acc.odr = p_sensor_settings[ACC_START + ODR];
    else config[ACC_SENSOR].cfg.acc.odr = BMI2_ACC_ODR_50HZ;

    if (p_sensor_settings[GYR_START + ODR] != 0) config[GYR_SENSOR].cfg.gyr.odr = p_sensor_settings[GYR_START + ODR];
    else config[ACC_SENSOR].cfg.acc.odr = BMI2_GYR_ODR_50HZ;

    //Set Fullscale Range
    config[ACC_SENSOR].cfg.acc.range = p_sensor_settings[ACC_START + FS_RANGE];
    config[GYR_SENSOR].cfg.gyr.range = p_sensor_settings[GYR_START + FS_RANGE];

    //The Filter and Power modes are tied together. The front end should ensure that
    //the ACC and GYR power modes are the same buit if for whaterver reason they
    //aren't we default to the accelerometer power setting
    int power_save = 0;
    bool sensor_enable = true;
    switch (p_sensor_settings[ACC_START + POWER])
    {
        case BMI270_SUSPEND_POWER_MODE:
            power_save = 1;
            sensor_enable = false;
            break;
        case BMI270_CONFIGURATION_POWER_MODE:
            sensor_enable = false;
            break;
        case BMI270_LOW_POWER_MODE:
            power_save = 1;
            config[ACC_SENSOR].cfg.acc.filter_perf = BMI2_POWER_OPT_MODE;
            config[GYR_SENSOR].cfg.gyr.filter_perf = BMI2_POWER_OPT_MODE;
            config[GYR_SENSOR].cfg.gyr.noise_perf  = BMI2_POWER_OPT_MODE;
            break;
        case BMI270_NORMAL_POWER_MODE:
        default:
            config[ACC_SENSOR].cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;
            config[GYR_SENSOR].cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE;
            config[GYR_SENSOR].cfg.gyr.noise_perf  = BMI2_POWER_OPT_MODE;
            break;
        case BMI270_PERFORMANCE_POWER_MODE:
            config[ACC_SENSOR].cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;
            config[GYR_SENSOR].cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE;
            config[GYR_SENSOR].cfg.gyr.noise_perf  = BMI2_PERF_OPT_MODE;
            break;
    }

    //Low pass filter bandwidth
    config[ACC_SENSOR].cfg.acc.bwp = p_sensor_settings[ACC_START + LOW_PASS_FILTER];
    config[GYR_SENSOR].cfg.gyr.bwp = p_sensor_settings[GYR_START + LOW_PASS_FILTER];

    //Apply the above settings to the config file, only apply the settings
    //if the appropriate BMI sensor is active
    if (imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC && imu_comm->sensor_model[GYR_SENSOR] == BMI270_GYR)
    {
        rslt = bmi2_set_sensor_config(config, 2, &bmi270);
    }
    else if (imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC)
    {
        struct bmi2_sens_config actual_config[1];
        actual_config[0].type = BMI2_ACCEL;
        rslt = bmi2_set_sensor_config(actual_config, 1, &bmi270);
    }
    else if (imu_comm->sensor_model[GYR_SENSOR] == BMI270_GYR)
    {
        struct bmi2_sens_config actual_config[1];
        actual_config[0].type = BMI2_GYRO;
        rslt = bmi2_set_sensor_config(actual_config, 1, &bmi270);
    }

    //Apply the above settings to the appropriate sensors
    if (rslt == BMI2_OK)
    {
        //Activate the appropriate sensors if the sensor_enable variable is true and the sensor
        //odr is greater than 0.
        if ((imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC && imu_comm->sensor_model[GYR_SENSOR] == BMI270_GYR) && 
        (p_sensor_settings[ACC_START + ODR] != 0 && p_sensor_settings[GYR_START + ODR] != 0) && sensor_enable)
        {
            uint8_t sensor_list[2] = { BMI2_ACCEL, BMI2_GYRO };
            rslt = bmi2_sensor_enable(sensor_list, 2, &bmi270);
        }
        else if ((imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC && p_sensor_settings[ACC_START + ODR] != 0) && sensor_enable)
        {
            uint8_t sensor_list[1] = { BMI2_ACCEL };
            rslt = bmi2_sensor_enable(sensor_list, 1, &bmi270);
        }
        else if ((imu_comm->sensor_model[GYR_SENSOR] == BMI270_GYR && p_sensor_settings[GYR_START + ODR] != 0) && sensor_enable)
        {
            uint8_t sensor_list[1] = { BMI2_GYRO };
            rslt = bmi2_sensor_enable(sensor_list, 1, &bmi270);
        }
        else
        {
            return 0; //no sensors are actually being turned on here so there's nothing more to do
        }

        //Set the advanced power saving bit based on the current power setting
        rslt = bmi2_set_adv_power_save(power_save, &bmi270);
    }
    else SEGGER_RTT_WriteString(0, "Error: BMI270 active mode enabled with incorrect settings.\n");

    //DEBUG: Confirm the sensor config was updated
    rslt = bmi2_get_sensor_config(config, 2, &bmi270);

    //After all settings have been applied and the sensor is turned on, wait for the 
    //necessary amount of time to ensure proper reaedings
    if (current_mode = CONNECTED_MODE) bmi270_delay(BMI270_SUSPEND_TO_ACTIVE_DELAY_US, bmi270.intf_ptr);
    else if (current_mode = SENSOR_IDLE_MODE) bmi270_delay(BMI270_CONFIG_TO_ACTIVE_DELAY_US, bmi270.intf_ptr);
    
    return rslt;
}

void bmi270_get_actual_settings()
{
    //For debugging purposes it's nice to see that the settings we have stored in the sensor array
    //physically make their way onto the chip. This method prints out the current register values
    //of some of the more important registers
    uint8_t reg_val;

    //initialize read/write methods, address, and default settings for mag
    if (imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC)
    {
        SEGGER_RTT_WriteString(0, "BMI270 Acc. Register Values:\n");
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, 0x41, &reg_val, 1);
        SEGGER_RTT_printf(0, "ACC_RANGE Register: 0x%x\n", reg_val);
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, BMI2_ACC_CONF_ADDR, &reg_val, 1);
        SEGGER_RTT_printf(0, "ACC_CONF Register: 0x%x\n", reg_val);
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, BMI2_PWR_CONF_ADDR, &reg_val, 1);
        SEGGER_RTT_printf(0, "PWR_CONF Register: 0x%x\n", reg_val);
        imu_comm->acc_comm.read_register((void*)imu_comm->acc_comm.twi_bus,  imu_comm->acc_comm.address, BMI2_PWR_CTRL_ADDR, &reg_val, 1);
        SEGGER_RTT_printf(0, "PWR_CTRL Register: 0x%x\n\n", reg_val);
    }

    if (imu_comm->sensor_model[GYR_SENSOR] == BMI270_GYR)
    {
        SEGGER_RTT_WriteString(0, "BMI270 Gyr. Register Values:\n");
        imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, 0x43, &reg_val, 1);
        SEGGER_RTT_printf(0, "GYR_RANGE Register: 0x%x\n", reg_val);
        imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, BMI2_GYR_CONF_ADDR, &reg_val, 1);
        SEGGER_RTT_printf(0, "GYR_CONF Register: 0x%x\n", reg_val);
        imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, BMI2_PWR_CONF_ADDR, &reg_val, 1);
        SEGGER_RTT_printf(0, "PWR_CONF Register: 0x%x\n", reg_val);
        imu_comm->gyr_comm.read_register((void*)imu_comm->gyr_comm.twi_bus,  imu_comm->gyr_comm.address, BMI2_PWR_CTRL_ADDR, &reg_val, 1);
        SEGGER_RTT_printf(0, "PWR_CTRL Register: 0x%x\n\n", reg_val);
    }
}

int8_t bmi270_read_register(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    //This method converts my function pointer for a I2C read into the form required
    //by Bosch's drivers
    sensor_communication_t* comm = (sensor_communication_t*)intf_ptr;
    return (int8_t) imu_comm->acc_comm.read_register((void*)comm->twi_bus, comm->address, reg_addr, reg_data, (uint16_t)len);
}

int8_t bmi270_write_register(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    //This method converts my function pointer for a I2C write into the form required
    //by Bosch's drivers.
    sensor_communication_t* comm = (sensor_communication_t*)intf_ptr;
    return (int8_t) imu_comm->acc_comm.write_register((void*)comm->twi_bus, comm->address, reg_addr, reg_data, (uint16_t)len);
}

void bmi270_delay(uint32_t period, void *intf_ptr)
{
    //The Bosch drivers sometimes force delays so they need a method that can implement
    //this. Since the nRF chip has the BLE stack initialized I can't take advantage of
    //the simple nrf_delay() method and instead need to use the nrf_drv_timer library
    //which is a little more complex (although has a resolution of 62.5 nanoseconds).
    sensor_communication_t* comm = (sensor_communication_t*)intf_ptr;
    comm->delay(period);
}

int32_t bmi270_get_data(uint8_t* pBuff, uint8_t offset)
{
    //This method assumes that pBuff is an array that holds data for multiple
    //different sensors. If the array is just for accelerometer or just for 
    //gyroscope data then the bmi270_get_acc_data() and bmi270_get_gyr_data()
    //methods below should be used
    uint8_t data_ready = 0;

    struct bmi2_sens_data sensor_data = { { 0 } };
    int8_t rslt = bmi2_get_sensor_data(&sensor_data, &bmi270);

    if (imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC)
    {
        //x-data
        pBuff[offset] = sensor_data.acc.x & 0xFF;
        pBuff[offset + 1] = ((sensor_data.acc.x & 0xFF00) >> 8);

        //y-data
        pBuff[offset + 2] = sensor_data.acc.y & 0xFF;
        pBuff[offset + 3] = ((sensor_data.acc.y & 0xFF00) >> 8);
        
        //z-data
        pBuff[offset + 4] = sensor_data.acc.z & 0xFF;
        pBuff[offset + 5] = ((sensor_data.acc.z & 0xFF00) >> 8);
    }

    if (imu_comm->sensor_model[GYR_SENSOR] == BMI270_GYR)
    {
        //x-data
        pBuff[offset + 6] = sensor_data.gyr.x & 0xFF;
        pBuff[offset + 7] = ((sensor_data.gyr.x & 0xFF00) >> 8);

        //y-data
        pBuff[offset + 8] = sensor_data.gyr.y & 0xFF;
        pBuff[offset + 9] = ((sensor_data.gyr.y & 0xFF00) >> 8);
        
        //z-data
        pBuff[offset + 10] = sensor_data.gyr.z & 0xFF;
        pBuff[offset + 11] = ((sensor_data.gyr.z & 0xFF00) >> 8);
    }

    return rslt;
}

int32_t bmi270_get_dummy_data(uint8_t* pBuff, uint8_t offset)
{
    //If both the acc and gyro are in use then the above method gets data for both sensors
    //in a single read when the accelerometer's data retrieval method gets called. The code
    //in main still needs to call the data retrieval method for the gyroscope though, so we
    //don't want to read repeat data. This method get's called instead which doesn't actually
    //read anything.
    return 0;
}

int32_t bmi270_get_acc_data(uint8_t* pBuff, uint8_t offset)
{
    //Method for reading accelerometer data
    uint8_t data_ready = 0;

    struct bmi2_sens_data sensor_data = { { 0 } };
    int8_t rslt = bmi2_get_sensor_data(&sensor_data, &bmi270);

    if (imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC)
    {
        //x-data
        pBuff[offset] = sensor_data.acc.x & 0xFF;
        pBuff[offset + 1] = ((sensor_data.acc.x & 0xFF00) >> 8);

        //y-data
        pBuff[offset + 2] = sensor_data.acc.y & 0xFF;
        pBuff[offset + 3] = ((sensor_data.acc.y & 0xFF00) >> 8);
        
        //z-data
        pBuff[offset + 4] = sensor_data.acc.z & 0xFF;
        pBuff[offset + 5] = ((sensor_data.acc.z & 0xFF00) >> 8);
    }

    return rslt;
}

int32_t bmi270_get_gyr_data(uint8_t* pBuff, uint8_t offset)
{
    //Method for reading accelerometer data
    uint8_t data_ready = 0;

    struct bmi2_sens_data sensor_data = { { 0 } };
    int8_t rslt = bmi2_get_sensor_data(&sensor_data, &bmi270);

    if (imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC)
    {
        //x-data
        pBuff[offset] = sensor_data.gyr.x & 0xFF;
        pBuff[offset + 1] = ((sensor_data.gyr.x & 0xFF00) >> 8);

        //y-data
        pBuff[offset + 2] = sensor_data.gyr.y & 0xFF;
        pBuff[offset + 3] = ((sensor_data.gyr.y & 0xFF00) >> 8);
        
        //z-data
        pBuff[offset + 4] = sensor_data.gyr.z & 0xFF;
        pBuff[offset + 5] = ((sensor_data.gyr.z & 0xFF00) >> 8);
    }

    return rslt;
}