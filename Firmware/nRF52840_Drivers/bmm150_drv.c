#include "bmm150_drv.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "sensor_settings.h"
#include "personal_caddie_operating_modes.h"

//set up settings variables
static imu_communication_t* imu_comm;
static uint8_t*             p_sensor_settings;

static struct bmm150_dev    bmm150; //driver defined struct for holding functional pointers and other info

//custom enums
enum bmi270_power_mode {
    BMI270_SUSPEND_POWER_MODE = 0,
    BMI270_CONFIGURATION_POWER_MODE = 1,
    BMI270_LOW_POWER_MODE = 2,
    BMI270_NORMAL_POWER_MODE = 3,
    BMI270_PERFORMANCE_POWER_MODE = 4
};

void bmm150init(imu_communication_t* comm, uint8_t* settings)
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
    bmm150.intf = BMM150_I2C_INTF; //set the I2C interface
    bmm150.read = bmm150_read_register; //set the I2C read functional pointer
    bmm150.write = bmm150_write_register; //set the I2C write functional pointer
    bmm150.delay_us = bmm150_delay; //set the microsecond delay functional pointer
    
    //First we set up the communication interface pointer and then call the 
    //driver's built-in init method. This method performs a soft-reset of the 
    //device which puts all registers back to their default values so it must
    //be called before we put our own values into the registers.
    if (imu_comm->sensor_model[MAG_SENSOR] == BMM150_MAG)  bmm150.intf_ptr = (void*) &imu_comm->mag_comm;
    else return; //if we're not looking at a BMM150 mag then there's nothing more to do here

    //Apply default settings for the mag
    update_sensor_setting(p_sensor_settings + MAG_START, ODR, BMM150_DATA_RATE_30HZ); //accelerometer ODR (50 Hz)
    update_sensor_setting(p_sensor_settings + MAG_START, POWER, BMM150_POWERMODE_NORMAL); //magnetometer power mode (normal power mode)

    update_sensor_setting(p_sensor_settings + MAG_START, EXTRA_1, BMM150_REPXY_LOWPOWER); //xy axis repititions (low power)
    update_sensor_setting(p_sensor_settings + MAG_START, EXTRA_2, BMM150_REPZ_LOWPOWER); //z axis repititions (low power)

    SEGGER_RTT_WriteString(0, "BMM150 Mag initialized.\n");

}

int32_t bmm150_connected_mode_enable(bool init)
{
    //In connected mode the BMM150 gets placed into its "Suspend" power mode. The expected 
    //current draw in this mode is 1 uA (0.000001 A). In this power mode the contents of
    //all registers lose their values, and only a handful of registers can actuall be accessed.
    if (imu_comm->sensor_model[MAG_SENSOR] != BMM150_MAG) return 0; //only proceed if a BMM150 sensor is active
    
    int8_t rslt = 0;
    if (init)
    {
        //like the BMI270, the BMM150 has an initialization function. It doesn't appear that this 
        //method is crucial to the proper functioning of the BMM150 (unlike with the BMI270), however,
        //calling this method initializes the bmm150_dev struct with important values so we still
        //call this init method. The device will be in sleep mode after the init() method returns
        //so we manually put it back into suspend mode afterwards.
        rslt = bmm150_init(&bmm150);
        if (rslt != BMM150_OK) SEGGER_RTT_WriteString(0, "Error: Couldn't initialize BMM150 Sensor.\n");
    }

    struct bmm150_settings settings;
    settings.pwr_mode = BMM150_POWERMODE_SUSPEND;
    rslt = bmm150_set_op_mode(&settings, &bmm150);
    if (rslt != BMM150_OK) SEGGER_RTT_WriteString(0, "Error: BMM150 couldn't be put into connected mode.\n");

    return rslt;
}

int32_t bmm150_idle_mode_enable()
{
    //In sensor idle mode the BMM150 is placed into its "SLeep" power mode. The expected
    //current draw in this mode is ??? (not listed in data sheet, may want to get my own
    //reading with the power profiler). In this power mode all registers can be read, but
    //the IMU isn't actively taking measurements.
    if (imu_comm->sensor_model[MAG_SENSOR] != BMM150_MAG) return 0; //only proceed if a BMM150 sensor is active

    int8_t rslt = 0;
    //When calling this method from sensor active mode we simply put the sensor
    //into sleep mode to save power.
    struct bmm150_settings settings;
    settings.pwr_mode = BMM150_POWERMODE_SLEEP;
    rslt = bmm150_set_op_mode(&settings, &bmm150);
    if (rslt != BMM150_OK) SEGGER_RTT_WriteString(0, "Error: BMM150 couldn't be put into sensor idle mode.\n");

    return rslt;
}

int32_t bmm150_active_mode_enable(float highest_odr, int current_mode)
{
    //In sensor active mode the BMM150 will be placed in either its "Normal" or "Forced" power 
    //mode depending on the contents of the sensor settings array. In normal mode there are 
    //three different performance states we can have: Low power which has an expected current
    //draw of 170 uA (0.00017 A), standard power which has an expected current draw of 0.5 mA
    //(0.0005 A) and performance power which has an expected current draw of 0.8 mA (0.0008 A).
    //Regardless of the perforamnce state, there will be current spikes in the range of 15-18 mA.
    //In forced power mode a single reading gets taken and then the sensor is placed back into
    //sleep mode. Every time the sensor is taken out of sleep mode and placed back into forced 
    //mode it will always immediately take a reading and go straight back into sleep mode. Using
    //this mode it's possible to achieve custom ODRs which will all have unique current draws.
    //Make sure that the bmm150 sensor is actually in use. To ensure data is good, we set a delay
    //of 3.2 milliseconds (3200 microseconds) when coming from connected mode and 0.2 milliseconds
    //(200 microseconds) when coming from sensor idle mode.
    if (imu_comm->sensor_model[MAG_SENSOR] != BMM150_MAG) return 0;

    //Put the pertinent settings from the sensor setting array into the
    //bmm150_settings struct from the driver
    struct bmm150_settings settings;

    settings.pwr_mode = p_sensor_settings[MAG_START + POWER];
    settings.data_rate  = p_sensor_settings[MAG_START + ODR];

    //I've created a custom setting that matches the BMM150 ODR to that of
    //the accelerometer and gyroscope. This is achieved by putting the
    //sensor in forced mode and selecting the custom odr option.
    if (settings.pwr_mode == BMM150_POWERMODE_FORCED && settings.data_rate == 0x8)
    {
        //The custom ODR is achieved by manipulating the s, y and z axis 
        //repetitions per the following equations
        bmm150_set_axes_repetitions((void*)&settings, highest_odr);
    }
    else
    {
        //Just use one of the pre-built ODRs
        settings.xy_rep  = p_sensor_settings[MAG_START + EXTRA_1];
        settings.z_rep  = p_sensor_settings[MAG_START + EXTRA_2];
    }
    
    int8_t rtrn = bmm150_set_op_mode(&settings, &bmm150); //set the power mode first

    if (rtrn == BMM150_OK)
    {
        rtrn = bmm150_set_sensor_settings(0x000F, &settings, &bmm150); //the 0x000F lets the driver know we want to change the sensor mode
    }
    else SEGGER_RTT_WriteString(0, "Error: BMM150 couldn't be put into sensor active mode.\n");

    //After all settings have been applied and the sensor is turned on, wait for the 
    //necessary amount of time to ensure proper reaedings
    if (current_mode = CONNECTED_MODE) bmm150_delay(BMM150_SUSPEND_TO_ACTIVE_DELAY_US, bmm150.intf_ptr);
    else if (current_mode = SENSOR_IDLE_MODE) bmm150_delay(BMM150_SLEEP_TO_ACTIVE_DELAY_US, bmm150.intf_ptr);
    
    return rtrn;
}

void bmm150_set_axes_repetitions(void* settings, float odr)
{
    //Custom ODR's are calculated with the following formulas:

    //ODR ~= 1 / (0.000145 * nXY + 0.0005 * nZ + 0.00098)
    //nXY  = 1 + 2 * (register_0x51)
    //nZ   = 1 + register_0x52

    //The goal is to get as close to the goal ODR while keeping
    //nXY and nZ as close to each other as we can. The maximum
    //value for nZ is only 256 while for nXY it is 511, however,
    //this many samples averaged together would create a large
    //current draw any way so this shouldn't be an issue. Since
    //we want the values to be equal the register values become:

    //register_0x51 = (((1 / ODR - 0.00098) / 0.000645) - 1) / 2
    //register_0x52 = register_0x51 * 2

    //Calculate the nXY value and write the register values into the settings struct
    struct bmm150_settings* bmi_settings = (struct bmm150_settings*)settings;
    float nXY = (1.0f / odr - 0.00098f) / 0.000645f;
    bmi_settings->xy_rep = (nXY - 1) / 2;
    bmi_settings->z_rep = bmi_settings->xy_rep * 2;
}

void bmm150_get_actual_settings()
{
    //For debugging purposes it's nice to see that the settings we have stored in the sensor array
    //physically make their way onto the chip. This method prints out the current register values
    //of some of the more important registers
    uint8_t reg_val;

    //initialize read/write methods, address, and default settings for mag
    if (imu_comm->sensor_model[ACC_SENSOR] == BMI270_ACC)
    {
        SEGGER_RTT_WriteString(0, "BMM150 Mag. Register Values:\n");
        imu_comm->mag_comm.read_register((void*)imu_comm->mag_comm.twi_bus,  imu_comm->mag_comm.address, BMM150_REG_POWER_CONTROL, &reg_val, 1);
        SEGGER_RTT_printf(0, "POWER_CONTROL Register: 0x%x\n", reg_val);
        imu_comm->mag_comm.read_register((void*)imu_comm->mag_comm.twi_bus,  imu_comm->mag_comm.address, BMM150_REG_OP_MODE, &reg_val, 1);
        SEGGER_RTT_printf(0, "OP_MODE Register: 0x%x\n", reg_val);
        imu_comm->mag_comm.read_register((void*)imu_comm->mag_comm.twi_bus,  imu_comm->mag_comm.address, BMM150_REG_REP_XY, &reg_val, 1);
        SEGGER_RTT_printf(0, "REP_XY Register: 0x%x\n", reg_val);
        imu_comm->mag_comm.read_register((void*)imu_comm->mag_comm.twi_bus,  imu_comm->mag_comm.address, BMM150_REG_REP_Z, &reg_val, 1);
        SEGGER_RTT_printf(0, "REP_Z Register: 0x%x\n\n", reg_val);
    }
}

int8_t bmm150_read_register(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    //This method converts my function pointer for a I2C read into the form required
    //by Bosch's drivers
    sensor_communication_t* comm = (sensor_communication_t*)intf_ptr;
    return (int8_t) imu_comm->acc_comm.read_register((void*)comm->twi_bus, comm->address, reg_addr, reg_data, (uint16_t)len);
}

int8_t bmm150_write_register(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    //This method converts my function pointer for a I2C write into the form required
    //by Bosch's drivers.
    sensor_communication_t* comm = (sensor_communication_t*)intf_ptr;
    return (int8_t) imu_comm->acc_comm.write_register((void*)comm->twi_bus, comm->address, reg_addr, reg_data, (uint16_t)len);
}

void bmm150_delay(uint32_t period, void *intf_ptr)
{
    //The Bosch drivers sometimes force delays so they need a method that can implement
    //this. Since the nRF chip has the BLE stack initialized I can't take advantage of
    //the simple nrf_delay() method and instead need to use the nrf_drv_timer library
    //which is a little more complex (although has a resolution of 62.5 nanoseconds).
    sensor_communication_t* comm = (sensor_communication_t*)intf_ptr;
    comm->delay(period);
}

int32_t bmm150_get_data(uint8_t* pBuff, uint8_t offset)
{
    //If the device is currently being run in Forced mode then we need to 
    //wake it up every time we want a new data reading

    if (p_sensor_settings[MAG_START + POWER] == BMM150_POWERMODE_FORCED)
    {
        struct bmm150_settings settings;
        settings.pwr_mode = BMM150_POWERMODE_FORCED;
        int8_t rslt = bmm150_set_op_mode(&settings, &bmm150);
        if (rslt != BMM150_OK)
        {
            SEGGER_RTT_WriteString(0, "Couldn't read BMM150 data in Forced Mode.\n");
            return 0;
        }
    }

    struct bmm150_mag_data sensor_data;
    int8_t rslt = bmm150_read_mag_data(&sensor_data, &bmm150);

    //transfer data from driver built-in struct to pBuff array
    //x-data
    pBuff[offset] = sensor_data.x & 0xFF;
    pBuff[offset + 1] = ((sensor_data.x & 0xFF00) >> 8);

    //y-data
    pBuff[offset + 2] = sensor_data.y & 0xFF;
    pBuff[offset + 3] = ((sensor_data.y & 0xFF00) >> 8);
    
    //z-data
    pBuff[offset + 4] = sensor_data.z & 0xFF;
    pBuff[offset + 5] = ((sensor_data.z & 0xFF00) >> 8);

    return rslt;
}