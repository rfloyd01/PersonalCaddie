#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "nrf_delay.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "fds.h"
#include "bsp_btn_ble.h" //includes the nrf_gpio_pin_map() macro
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "SEGGER_RTT.h"

#include "lsm9ds1.h"
#include "fxos8700.h"
#include "fxas21002.h"
#include "bmi270_drv.h"
#include "bmm150_drv.h"
#include "ble_sensor_service.h"
#include "ble_pc_service.h"
#include "personal_caddie_operating_modes.h"
#include "nRF_Implementations/pc_twi.h"
#include "nRF_Implementations/pc_ble.h"
#include "nRF_Implementations/pc_timer.h"

//Soft Device Parameters
#define DEAD_BEEF                       0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
BLE_SENSOR_SERVICE_DEF(m_ss);                                                      /**< IMU Sensor Service instance. */
BLE_PC_SERVICE_DEF(m_pc);                                                          /**< Personal Caddie Service instance. */
uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                                  /**< Handle of the current connection. */

//IMU Sensor Parameters
static uint8_t default_sensors[3] = {BMI270_ACC, BMI270_GYR, BMM150_MAG};          /**< Default sensors that are attempted to be initialized first. */
//static uint8_t default_sensors[3] = {FXOS8700_ACC, FXAS21002_GYR, FXOS8700_MAG};
static bool sensors_initialized[3] = {false, false, false};                        /**< Keep track of which sensors are currently initialized */
static uint8_t internal_sensors[10];                                               /**< An array for holding the addresses of sensors on the internal TWI line */
static uint8_t external_sensors[10];                                               /**< An array for holding the addresses of sensors on the external TWI line */
static uint8_t internal_sensors_found = 0;
static uint8_t external_sensors_found = 0;

//IMU Sensor Data Parameters
static uint8_t small_characteristic_data[SMALL_DATA_CHARACTERISTIC_SIZE];           /**< A small array for holding current sensor readings */
static uint8_t medium_characteristic_data[MEDIUM_DATA_CHARACTERISTIC_SIZE];         /**< A medium array for holding current sensor readings */
static uint8_t large_characteristic_data[LARGE_DATA_CHARACTERISTIC_SIZE];           /**< A large array for holding current sensor readings */
static uint8_t* composite_characteristic_data;                                      /**< A pointer to the current data characteristic (changes with number of sensor samples */
uint8_t sensor_settings[SENSOR_SETTINGS_LENGTH];                                    /**< An array represnting the IMU sensor settings */
uint8_t m_current_sensor_samples = 10;                                              /**< The number of sensor samples currently being put into the acc,gy and mag characteristics (must be less than MAX_SENSOR_SAMPLES */
uint32_t m_time_stamp;                                                              /**< Keeps track of the time that each data set is read at (this is measured in ticks of a 16MHz clock, i.e. 1 LSB = 1/16000000s = 62.5ns) */
volatile bool m_data_ready = false;                                                 /**< Indicates when all characteristics have been filled with new data and we're ready to send it to the client  */
bool m_use_composite_data  = true;                                                  /**< If this boolean is true, then all three sensors will have their data put into a single shared characteristic */

//IMU Sensor Communication Parameters
imu_communication_t imu_comm;                                                       /**< Structure that Holds information on how to communicate with each sensor */
uint16_t desired_minimum_connection_interval, desired_maximum_connection_interval;  /**< The desired min and max connection interval */
uint16_t m_connection_interval = 0;                                                 /**< The actuall connection interval of the current connection */
static float current_sensor_odr = 59.5;                                             /**< Keeps track of the current sensor ODR, connection interval is set based on this variable */
volatile bool m_notification_done = true;                                           /**< Indicates the number of notifications sent out in a single connection interval  */
volatile int m_notifications_in_queue = 0;                                          /**< Let's us know the current number of notifications in the queue  */
volatile int m_notification_queue_limit = 1;                                        /**< Limits the number of notifications allowed in the notification queue based on the current sensor ODR and connection interval*/

//LED Pin Parameters
#define RED_LED            NRF_GPIO_PIN_MAP(0, 24)                                  /**< Red LED Indicator on BLE 33 sense (part of triple RGB LED)*/
#define BLUE_LED           NRF_GPIO_PIN_MAP(0, 6)                                   /**< Blue LED Indicator on BLE 33 sense (part of triple RGB LED)*/
#define GREEN_LED          NRF_GPIO_PIN_MAP(0, 16)                                  /**< Green LED Indicator on BLE 33 sense (part of triple RGB LED)*/
#define DARK_GREEN_LED     NRF_GPIO_PIN_MAP(1, 9)                                   /**< Individual Green LED Indicator on BLE 33 sense*/
volatile uint8_t active_led = BLUE_LED;                                             /**< Variable used to keep track of which color LED to turn on/off*/

//Personal Caddie Parameters
static personal_caddie_operating_mode_t current_operating_mode = ADVERTISING_MODE;  /**< The chip starts in advertising mode*/

//Debugging Parameters
int notification_failures = 0;

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    //TODO: Need some kind of custom error handling here, any time this
    //method gets called it results in a crash
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static void sensor_communication_init(sensor_type_t type, uint8_t model, uint8_t address, nrf_drv_twi_t const * bus)
{
    //This method is used for setting up function pointers for communicating with sensors. Each sensor
    //has slightly different methods for reading and writing (defined in their drivers) so we use the
    //function pointers stored in the imu_comm structure for communication. The imu_comm strucutre also
    //holds other info for communication, such as the address of the sensor and the TWI bus that it's located
    //on.
    switch (type)
    {
        case ACC_SENSOR:
            imu_comm.acc_comm.address = address;
            imu_comm.acc_comm.twi_bus = bus;
            imu_comm.acc_comm.read_register = sensor_read_register;
            imu_comm.acc_comm.write_register = sensor_write_register;
            imu_comm.acc_comm.delay = delay_microseconds;
            if (model == LSM9DS1_ACC)
            {
                imu_comm.acc_comm.update_settings = lsm9ds1_acc_apply_setting; //deprecated
                imu_comm.acc_comm.get_data = lsm9ds1_get_acc_data;
            }
            else if (model == BMI270_ACC)
            {
                if (m_use_composite_data) imu_comm.acc_comm.get_data = bmi270_get_data;
                else imu_comm.acc_comm.get_data = bmi270_get_acc_data;
            }
            else if (model == FXOS8700_ACC)
            {
                imu_comm.acc_comm.update_settings = fxos8700_acc_apply_setting; //deprecated
                imu_comm.acc_comm.get_data = fxos8700_get_acc_data;
            }
            break;
        case GYR_SENSOR:
            imu_comm.gyr_comm.address = address;
            imu_comm.gyr_comm.twi_bus = bus;
            imu_comm.gyr_comm.read_register = sensor_read_register;
            imu_comm.gyr_comm.write_register = sensor_write_register;
            imu_comm.gyr_comm.delay = delay_microseconds;
            if (model == LSM9DS1_GYR)
            {
                imu_comm.gyr_comm.update_settings = lsm9ds1_gyr_apply_setting; //deprecated
                imu_comm.gyr_comm.get_data = lsm9ds1_get_gyr_data;
            }
            else if (model == BMI270_GYR)
            {
                if (m_use_composite_data) imu_comm.gyr_comm.get_data = bmi270_get_dummy_data;
                else imu_comm.gyr_comm.get_data = bmi270_get_gyr_data;
            }
            else if (model == FXAS21002_GYR)
            {
                imu_comm.gyr_comm.update_settings = NULL;  //deprecated
                imu_comm.gyr_comm.get_data = fxas21002_get_gyr_data;
            }
            break;
        case MAG_SENSOR:
            imu_comm.mag_comm.address = address;
            imu_comm.mag_comm.twi_bus = bus;
            imu_comm.mag_comm.read_register = sensor_read_register;
            imu_comm.mag_comm.write_register = sensor_write_register;
            imu_comm.mag_comm.delay = delay_microseconds;
            if (model == LSM9DS1_MAG)
            {
                imu_comm.mag_comm.update_settings = lsm9ds1_mag_apply_setting;  //deprecated
                imu_comm.mag_comm.get_data = lsm9ds1_get_mag_data;
            }
            else if (model == BMM150_MAG)
            {
                imu_comm.mag_comm.get_data = bmm150_get_data;
            }
            else if (model == FXOS8700_MAG)
            {
                imu_comm.mag_comm.update_settings = fxos8700_mag_apply_setting;  //deprecated
                imu_comm.mag_comm.get_data = fxos8700_get_mag_data;
            }
            break;
    }
}

static float sensor_odr_calculate()
{
    //Calculates the highest ODR of the three active sensors. The highest ODR
    //value will dictate the appropriate connection interval for the BLE connection.
    float current_highest_odr = 0.0;

    for (int i = 0; i < 3; i++)
    {
        if (imu_comm.sensor_model[i] == LSM9DS1_ACC)
        {
            float new_odr = lsm9ds1_odr_calculate(sensor_settings, imu_comm.sensor_model[0], imu_comm.sensor_model[1], imu_comm.sensor_model[2], i);
            if (new_odr > current_highest_odr) current_highest_odr = new_odr;
        }
        else if (imu_comm.sensor_model[i] == FXOS8700_ACC)
        {
            float new_odr;

            if (i == GYR_SENSOR) new_odr = fxas21002_odr_calculate(imu_comm.sensor_model[1], sensor_settings[GYR_START + ODR]);
            else new_odr = fxos8700_odr_calculate(imu_comm.sensor_model[0], imu_comm.sensor_model[2], sensor_settings[ACC_START + ODR], sensor_settings[MAG_START + ODR]);
            
            if (new_odr > current_highest_odr) current_highest_odr = new_odr;
        }
        else if (imu_comm.sensor_model[i] == BMI270_ACC)
        {
            float new_odr = bmi_bmm_odr_calculate(imu_comm.sensor_model[0], imu_comm.sensor_model[1], imu_comm.sensor_model[2], sensor_settings);
            if (new_odr > current_highest_odr) current_highest_odr = new_odr;
        }
    }

    return current_highest_odr;
}

static void default_sensor_select()
{
    //Check that all sensors have been initialized after scanning the internal and external bus.
    //If not, then we're going to need to choose a non-default sensor. The first priority is to 
    //pick a sensor on the same chip (i.e. same model and TWI bus). If this can't work for 
    //whatever reason then try to select a sensor of the same model but on the other bus. If this
    //doesn't work, then select the first option available.
    if (!sensors_initialized[0] || !sensors_initialized[1] || !sensors_initialized[2])
    {
        for (int i = ACC_SENSOR; i <= MAG_SENSOR; i++)
        {
            if (sensors_initialized[i]) continue;

            //We'll need to put the new model number into the sensor settings array
            //so figure out the right place to do so.
            int setting_location;
            if (i == ACC_SENSOR) setting_location = ACC_START + SENSOR_MODEL;
            else if (i == GYR_SENSOR) setting_location = GYR_START + SENSOR_MODEL;
            else setting_location = MAG_START + SENSOR_MODEL;

            for (int j = 0; j < 3; j++)
            {
                if (!sensors_initialized[j]) continue;

                //Get the addresses for sensor i from sensor j's manufacturer
                uint8_t new_sensor_address_l = get_sensor_low_address(i, default_sensors[j]);
                uint8_t new_sensor_address_h = get_sensor_high_address(i, default_sensors[j]);

                //figure out which TWI bus sensor j is on
                nrf_drv_twi_t const * known_sensor_bus;
                if (j == 0) known_sensor_bus = imu_comm.acc_comm.twi_bus;
                else if (j == 1) known_sensor_bus = imu_comm.gyr_comm.twi_bus;
                else if (j == 2) known_sensor_bus = imu_comm.mag_comm.twi_bus;

                uint8_t* primary_search = internal_sensors, *secondary_search = external_sensors;
                int primary_stop = internal_sensors_found, secondary_stop = external_sensors_found;
                if (known_sensor_bus->inst_idx == 1)
                {
                    primary_search = external_sensors;
                    secondary_search = internal_sensors;
                    primary_stop = external_sensors_found;
                    secondary_stop = internal_sensors_found;
                }

                //In the below loops, we can only initialize sensor i using sensor j's model number
                //because currently all sensor manufacturers I use have an acc, gyr and
                //mag so the model numbers line up (i.e. lsm9ds1_acc = 0, lsm9ds1_gyr = 0).
                //If I start using chips that don't have all three sensors then the 
                //model numbers will no longer line up and the below loops won't be safe anymore.

                //Search sensor j's TWI bus for sensor i.
                for (int k = 0; k < primary_stop; k++)
                {
                    if (primary_search[k] == new_sensor_address_l || primary_search[k] == new_sensor_address_h)
                    {
                        sensor_communication_init(i, default_sensors[j], primary_search[k], known_sensor_bus);
                        sensors_initialized[i] = true;
                        sensor_settings[setting_location] = default_sensors[j];
                        default_sensors[i] = default_sensors[j];
                        break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                    }
                }

                if (sensors_initialized[i]) break; //move onto the next sensor

                //If we were external on sensor j's TWI bus, search the other TWI bus
                for (int k = 0; k < secondary_stop; k++)
                {
                    if (secondary_search[k] == new_sensor_address_l || secondary_search[k] == new_sensor_address_h)
                    {
                        sensor_communication_init(i, default_sensors[j], secondary_search[k], known_sensor_bus);
                        sensors_initialized[i] = true;
                        sensor_settings[setting_location] = default_sensors[j];
                        default_sensors[i] = default_sensors[j];
                        break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                    }
                }

                if (sensors_initialized[i]) break; //move onto the next sensor
            }

            if (sensors_initialized[i]) continue; //if this sensor has been initialized then move on to the next one.

            //If we haven't initialized the sensor yet then it means we can't find one of
            //the appropriate model on either bus, so we need to just initialize the first
            //one possible.
            int stop;
            if (i == ACC_SENSOR) stop = ACC_MODEL_END;
            else if (i == GYR_SENSOR) stop = GYR_MODEL_END;
            else stop = MAG_MODEL_END;

            //search the external bus first
            for (int k = 0; k < external_sensors_found; k++)
            {
                //loopt through all models of the current sensor type
                for (int l = 0; l < stop; l++)
                {
                    uint8_t valid_sensor_address_l = get_sensor_low_address(i, l);
                    uint8_t valid_sensor_address_h = get_sensor_high_address(i, l);

                    if (external_sensors[k] == valid_sensor_address_l || external_sensors[k] == valid_sensor_address_h)
                    {
                        sensor_communication_init(i, l, external_sensors[k], get_external_twi_bus());
                        sensors_initialized[i] = true;
                        sensor_settings[setting_location] = l;
                        default_sensors[i] = l;
                        break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                    }
                }
            }

            if (sensors_initialized[i]) continue; //move onto the next sensor

            //and finally search the internal bus
            for (int k = 0; k < internal_sensors_found; k++)
            {
                //loop through all models of the current sensor type
                for (int l = 0; l < stop; l++)
                {
                    uint8_t valid_sensor_address_l = get_sensor_low_address(i, l);
                    uint8_t valid_sensor_address_h = get_sensor_high_address(i, l);

                    if (internal_sensors[k] == valid_sensor_address_l || internal_sensors[k] == valid_sensor_address_h)
                    {
                        sensor_communication_init(i, l, internal_sensors[k], get_internal_twi_bus());
                        sensors_initialized[i] = true;
                        sensor_settings[setting_location] = l;
                        default_sensors[i] = l;
                        break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                    }
                }
            }

            //If we haven't found a suitable sensor yet then there's something wrong. Print
            //an error statement
            if (!sensors_initialized[i]) SEGGER_RTT_WriteString(0, "Couldn't find any sensors, check board connections.\n");
        }
    }
}

static void sensors_init(bool discovery)
{
    //When first turning on the Personal Caddie we need send power to all sensors on
    //both the internal and external lines. We then scan both the internal
    //and external TWI bus to see what sensors are available so we can populate some
    //arrays with this information. If there are no sensors on the external line then 
    //power will be shut off (there will always be sensors on the internal line).
    //Subsequent calls to this method don't require this procedure.
    //Regardless of whether or not we enter this method in discovery mode, both TWI buses
    //need to be enabled.

    //Enable the power lines
    power_line_init();
    enable_internal_power_line();
    enable_external_power_line();

    //Then enable the TWI lines
    enable_twi_bus(get_internal_twi_bus_id());
    enable_twi_bus(get_external_twi_bus_id());
    delay_microseconds(50000); //slight delay so sensors have time to power on
    //nrf_delay_ms(50);
    
    if (discovery)
    {
        //First reset the sensor settings array, everything goes to zero except
        //the sensor models which are set to the default sensors.
        for (int i = 0; i < SENSOR_SETTINGS_LENGTH; i++) sensor_settings[i] = 0;
        sensor_settings[ACC_START + SENSOR_MODEL] = default_sensors[0];
        sensor_settings[GYR_START + SENSOR_MODEL] = default_sensors[1];
        sensor_settings[MAG_START + SENSOR_MODEL] = default_sensors[2];

        //Then initialize the internal and external sensors arrays so that each element
        //has a value of 0xFF. This value alerts the front end that no sensor is present.
        for (int i = 0; i < 10; i++)
        {
            internal_sensors[i] = 0xff;
            external_sensors[i] = 0xff;
        }

        //Initiate the TWI bus scan, looking for sensors
        twi_address_scan(internal_sensors, &internal_sensors_found, get_internal_twi_bus()); //Scan the internal TWI bus
        twi_address_scan(external_sensors, &external_sensors_found, get_external_twi_bus()); //Then scan the external bus

        //We also need to populate the available sensors characteristic with all the sensors
        //that were found on the internal and external TWI buses. This will allow the front
        //end application to dynamically choose which sensors to use.
        ble_gatts_value_t internal_available_sensors, external_available_sensors;

        internal_available_sensors.len = 10;
        internal_available_sensors.p_value = internal_sensors;
        internal_available_sensors.offset = 0;

        external_available_sensors.len = 10;
        external_available_sensors.p_value = external_sensors;
        external_available_sensors.offset = 10;

        uint32_t err_code = sd_ble_gatts_value_set(m_conn_handle, m_ss.available_handle.value_handle, &internal_available_sensors);
        APP_ERROR_CHECK(err_code);

        err_code = sd_ble_gatts_value_set(m_conn_handle, m_ss.available_handle.value_handle, &external_available_sensors);
        APP_ERROR_CHECK(err_code);

        //If no sensors were found on the external line it means that nothing is actually
        //hooked up which will leach current so there's no need to keep the external
        //power line enabled
        if (external_sensors_found == 0) disable_external_power_line();
    }

    //Scan the external sensors to see if any of them match the default sensors that were
    //set in the settings array. If not, then scan the internal sensors to see if any of
    //them are there. If one of the default sensors isn't found on either bus then a
    //different one will need to be selected.
    if (external_sensors_found > 0)
    {
        for (int i = 0; i < 3; i++)
        {
            uint8_t default_sensor_address_l = get_sensor_low_address(i, default_sensors[i]);
            uint8_t default_sensor_address_h = get_sensor_high_address(i, default_sensors[i]);

            for (int j = 0; j < external_sensors_found; j++)
            {
                if (external_sensors[j] == default_sensor_address_l || external_sensors[j] == default_sensor_address_h)
                {
                    sensor_communication_init(i, default_sensors[i], external_sensors[j], get_external_twi_bus());
                    sensors_initialized[i] = true;
                    break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                }
            }
        }
    }

    //See if all sensors were initialized on the external bus, if not, then initialize sensors
    //from internal bus
    if (!sensors_initialized[0] || !sensors_initialized[1] || !sensors_initialized[2])
    {
        for (int i = 0; i < 3; i++)
        {
            if (sensors_initialized[i]) continue; //this sensor was initialized on the external bus so skip it
            uint8_t default_sensor_address_l = get_sensor_low_address(i, default_sensors[i]);
            uint8_t default_sensor_address_h = get_sensor_high_address(i, default_sensors[i]);

            for (int j = 0; j < internal_sensors_found; j++)
            {
                if (internal_sensors[j] == default_sensor_address_l || internal_sensors[j] == default_sensor_address_h)
                {
                    sensor_communication_init(i, default_sensors[i], internal_sensors[j], get_internal_twi_bus());
                    sensors_initialized[i] = true;
                    break; //if for whatever reason there are two of the same sensor on the line we only want to add the first one
                }
            }
        }
    }

    //If any of the three sensors haven't been initialized yet, use the below
    //function to pick a different default sensor(s).
    default_sensor_select();

    //Set the sensor models in the imu_comm struct
    for (int i = 0; i < ACC_MODEL_END; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (default_sensors[j] == i)
            {
                imu_comm.sensor_model[j] = i;
            }
        }
    }

    //Call the initialization method for eacch type of sensor. Even if a sensor of 
    //a specific type isn't actually in use, we call the init methods to set some
    //important pointers which will be needed in the case that we swap to one of 
    //these sensors later
    lsm9ds1_init(&imu_comm, sensor_settings);
    bmi270init(&imu_comm, sensor_settings);
    bmm150init(&imu_comm, sensor_settings);
    fxos8700init(&imu_comm, sensor_settings);
    fxas21002init(&imu_comm, sensor_settings);

    //regardless of whether or not any sensors are found, disable the power pins and TWI bus
    disable_twi_bus(get_internal_twi_bus_id());
    disable_twi_bus(get_external_twi_bus_id());

    //Update the global sensor ODR. This variable is used for setting the connection interval
    //of the BLE connection to be equal to the fastest ODR between the three sensors.
    current_sensor_odr = sensor_odr_calculate();
    update_data_read_timer(1000.0 / current_sensor_odr); //set the data reading timer based on the sensor odr

    //After all sensors have been initialized, update the sensor settings characteristic to 
    //reflect the sensor settings array. Furthermore, we also set the connection interval 
    //to be equal to the highest sensor ODR (to the nearest 15th millisecond)
    ble_gatts_value_t settings;
    settings.len = SENSOR_SETTINGS_LENGTH;
    settings.p_value = sensor_settings;
    settings.offset = 0;
    uint32_t err_code = sd_ble_gatts_value_set(m_conn_handle, m_ss.settings_handles.value_handle, &settings);
    APP_ERROR_CHECK(err_code);
}

static void error_notification(int err_code)
{
    //This method is used to alert the front end application of nRF errors that have occured.
    //This happens by putting the error code into a characteristic and sending it via a
    //notification.
    ble_gatts_hvx_params_t err_notify_params;
    memset(&err_notify_params, 0, sizeof(err_notify_params));

    uint16_t err_characteristic_size = 4; //currently only error codes less than or equal to 4 bytes can be sent

    //Convert the error code into a byte array
    uint8_t err_array[4];
    uint8_t* err_start = (uint8_t*)&err_code; //cast the float to a 32-bit integer to take up 4 array slots
    for (int i = 0; i < 4; i++) err_array[i] = *(err_start + i);

    //Setup accelerometer notification first
    err_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    err_notify_params.handle = m_pc.error_handle.value_handle;
    err_notify_params.p_data = err_array;
    err_notify_params.p_len  = &err_characteristic_size;
    err_notify_params.offset = 0;

    uint32_t ret = sd_ble_gatts_hvx(m_conn_handle, &err_notify_params); //acc data notification
    APP_ERROR_CHECK(ret);
}

static uint32_t data_notification_error_handler(uint32_t* ret)
{
    //This method is for handling common errors that happen during notification
    //can be handled without crashing the application.
    if (*ret == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
    {
        //This error will pop up during a secure connection when we aren't sure who
        //the sender is (this happens when we try to send notifications without first
        //setting the CCCD to notify from the app). Calling the below method should fix
        //things.
        *ret = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
    }
    else if (*ret == NRF_ERROR_RESOURCES)
    {
        //This error occurs when we try to add a notification to the queue when the queue
        //is full. The m_notifications_in_queue variable of this file should prevent
        //this error code from occuring, so if it does occur it's indicative of some bigger
        //failure elsewhere. We don't want this to crash our program so we return a custom
        //error code.
        SEGGER_RTT_WriteString(0, "Error: Notification queue is full, waiting for it to empty.\n");
        *ret = 0x69;
    }

    return 0;
}

static void characteristic_update_and_notify_composite_characteristic()
{
    //Add data to the appropriate characteristic based on the number of 
    //samples we're collecting (which is a factor of the current sensor
    //ODR and BLE connection interval).
    uint16_t data_characteristic_size, characteristic_handle;
    if (composite_characteristic_data == small_characteristic_data)
    {
        data_characteristic_size = SMALL_DATA_CHARACTERISTIC_SIZE; //5 bytes for time and samples + 4 samples max * 6 bytes/sample * 3 sensors
        characteristic_handle = m_ss.data_handles[0].value_handle;
    }
    else if (composite_characteristic_data == medium_characteristic_data)
    {
        data_characteristic_size = MEDIUM_DATA_CHARACTERISTIC_SIZE; //5 bytes for time and samples + 8 samples max * 6 bytes/sample * 3 sensors
        characteristic_handle = m_ss.data_handles[1].value_handle;
    }
    else
    {
        data_characteristic_size = LARGE_DATA_CHARACTERISTIC_SIZE; //5 bytes for time and samples + 13 samples max * 6 bytes/sample * 3 sensors
        characteristic_handle = m_ss.data_handles[2].value_handle;
    }

    //Add the time stamp for the current data set and current number of 
    //samples to the beginning of the characteristic.
    uint8_t* time_start = (uint8_t*)&m_time_stamp; //cast the float to a 32-bit integer to take up 4 array slots
    for (int i = 0; i < 4; i++) composite_characteristic_data[i] = *(time_start + i);
    composite_characteristic_data[4] = m_current_sensor_samples;

    //Setup composite data notification
    ble_gatts_hvx_params_t data_notify_params;
    memset(&data_notify_params, 0, sizeof(data_notify_params));
    
    data_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    data_notify_params.handle = characteristic_handle;
    data_notify_params.p_data = composite_characteristic_data;
    data_notify_params.p_len  = &data_characteristic_size;
    data_notify_params.offset = 0;

    //Attempt to add the notification to the notification queue. During times
    //of heavy BLE traffic, this queue may fill up at higher sensor ODRs so if
    //the queue is full don't add the current data set. Newer data will get
    //added as the queue empties out.
    uint32_t ret = 0;
    ret = sd_ble_gatts_hvx(m_conn_handle, &data_notify_params);
    data_notification_error_handler(&ret);

    if (ret == 0x69) return;
    m_notifications_in_queue++;

    //SEGGER_RTT_printf(0, "Queue has %d notifications in it.\n", m_notifications_in_queue);
}

static void characteristic_update_and_notify_individual_characteristics()
{
//    //Add the time stamp for the current data set and current number of 
//    //samples to the end of each of the data characteristics.
//    for (int i = 0; i < 4; i++)
//    {
//        uint8_t* time_start = (uint8_t*)&m_time_stamp; //cast the float to a 32-bit integer to take up 4 array slots

//        acc_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + i] = *(time_start + i);
//        gyr_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + i] = *(time_start + i);
//        mag_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + i] = *(time_start + i);
//    }

//    acc_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + 4] = m_current_sensor_samples;
//    gyr_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + 4] = m_current_sensor_samples;
//    mag_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + 4] = m_current_sensor_samples;

//    ble_gatts_hvx_params_t acc_notify_params, gyr_notify_params, mag_notify_params;
//    memset(&acc_notify_params, 0, sizeof(acc_notify_params));
//    memset(&gyr_notify_params, 0, sizeof(gyr_notify_params));
//    memset(&mag_notify_params, 0, sizeof(mag_notify_params));

//    //uint16_t acc_data_characteristic_size = 5 + MAX_SENSOR_SAMPLES * SAMPLE_SIZE;
//    uint16_t acc_data_characteristic_size = 5;
//    uint16_t gyr_data_characteristic_size = 5 + MAX_SENSOR_SAMPLES * SAMPLE_SIZE;
//    uint16_t mag_data_characteristic_size = 5 + MAX_SENSOR_SAMPLES * SAMPLE_SIZE;

//    //Setup accelerometer notification first
//    acc_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
//    acc_notify_params.handle = m_ss.data_handles[0].value_handle;
//    acc_notify_params.p_data = acc_characteristic_data;
//    acc_notify_params.p_len  = &acc_data_characteristic_size;
//    acc_notify_params.offset = 0;

//    //Setup gyroscope notification second
//    gyr_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
//    gyr_notify_params.handle = m_ss.data_handles[1].value_handle;
//    gyr_notify_params.p_data = gyr_characteristic_data;
//    gyr_notify_params.p_len  = &gyr_data_characteristic_size;
//    gyr_notify_params.offset = 0;

//    //Setup magnetometer notification third
//    mag_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
//    mag_notify_params.handle = m_ss.data_handles[2].value_handle;
//    mag_notify_params.p_data = mag_characteristic_data;
//    mag_notify_params.p_len  = &mag_data_characteristic_size;
//    mag_notify_params.offset = 0;

//    //Notifications are never just sent out, they get added to an internal
//    //queue on the BLE stack first. When using individual characteristics
//    //for each sensor we need to wait for this queue to completely empty out
//    //before adding more data. This is because during times of higher BLE
//    //traffic we might not be able to transmit all of the notifications in the
//    //queue during the same connection interval. This will cause the queue to 
//    //fill up, which in turn means we will only every try to add data
//    //from the some characteristic into the queue until the queue empties out.
//    //The result is major lag of the image in the front end of the application.
//    while (m_notifications_in_queue > 0) {} //allow all notifications in the queue to transmit before adding more

//    uint32_t ret = 0; //start with some arbitrary value that isn't NRF_SUCCESS
//    //while (true)
//    //{
//    //    ret = sd_ble_gatts_hvx(m_conn_handle, &acc_notify_params);
//    //    data_notification_error_handler(&ret);

//    //    if (ret == 0x69) break;
//    //    SEGGER_RTT_printf(0, "queue has %d notifications in it.\n", ++m_notifications_in_queue);
//    //}

//    //The above while loop should make it so we're guaranteed to queue the below
//    //three notifications without encountering the NRF_RESOURCES error, however,
//    //we still need to do an error check after each call to to the hvx() method.
//    while (ret != NRF_SUCCESS)
//    {
//        ret = sd_ble_gatts_hvx(m_conn_handle, &acc_notify_params); //acc data notification
//        data_notification_error_handler(&ret);

//        ret = sd_ble_gatts_hvx(m_conn_handle, &gyr_notify_params); //gyr data notification
//        data_notification_error_handler(&ret);

//        ret = sd_ble_gatts_hvx(m_conn_handle, &mag_notify_params); //mag data notification
//        data_notification_error_handler(&ret);
//    }
}


static void characteristic_update_and_notify()
{
    if (m_use_composite_data)
    {
        characteristic_update_and_notify_composite_characteristic();
    }
    else
    {
        characteristic_update_and_notify_individual_characteristics(); //deprecated
    }
}

uint64_t findGCD(uint64_t a, uint64_t b)
{
    if (b == 0) return a;
    return findGCD(b, a % b);
}

uint64_t findLCM(uint64_t a, uint64_t b)
{
    return (a * b) / findGCD(a, b);
}

float findDecimalLCM(float a, float b)
{
    return findLCM(a * 1000000, b * 1000000) / 1000000.0;
}

void calculate_samples_and_connection_interval()
{
   //This method is used to select the number of samples to store in the 
   //data characteristics as a function of the current sensor ODR and desired data collection time.
   //It get's called when the application first starts, and any time the
   //ODR of the sensors is updated from the front end application.
   //We want to minimize the amount of lag that occurs with the image on screen,
   //which is accomplished by minimizing the data collection time. 
   //After some testing, 80 milliseconds seems to be a good target time for the data collection period.
   //There is a small lag seen on the screen in this case, but not really that noticeable,
   //and it allows us to collect at least a few samples at lower ODR values, which is 
   //more efficient than sending out samples 1 at a time.

   float desired_lag_time = 0.08; //in seconds.
   m_current_sensor_samples = current_sensor_odr * desired_lag_time;

   if ((float)m_current_sensor_samples == (float)(current_sensor_odr * desired_lag_time))
   {
       //We need the data collection period to be less than the connection
       //interval to ensure we get data in every single connection 
       //interval. If the data collection time evenly divides the connection
       //interval reduce the sample count by 1.
       m_current_sensor_samples--;
   }

   //The desired lag time variable above represents what we "hope" the connection 
   //interval will be. Since the connection interval is negotiated between the 
   //BLE peripheral and Central we won't actually know what the connection interval
   //is until it's set. In this method we assume that the desired lag time becomes the 
   //connection interval and set the number of samples to collect for eac notification
   //accordingly. Essentially, we want the data collection time to be as close to the 
   //lag time while still being less than it. This will ensure that new data makes it to 
   //the front end during each connection interval
   if (m_use_composite_data)
   {
       //In the case where a single characteristic is being used for each sensor, we can hold
       //MAX_SENSOR_SAMPLES worth of data in the entire characteristic, which means each 
       //individual sensor can only contribute MAX_SENSOR_SAMPLES/3 before the characteristic
       //is full.
       if (m_current_sensor_samples > (MAX_SENSOR_SAMPLES / 3))
       {
          
           //If we exceed the number of samples that will fit in the characteristic
           //we can simply split up the data between a few notifications. The fewer notifications
           //the better, and there's a cap on five notifcations. As an example, at a sensor
           //ODR of 100Hz it will take 16 samples to reach the desired lag time of 0.16 seconds.
           //Since the data collection time equals the connection interval we lower the number
           //of samples to 15 (which will instead take 0.15 seconds to collect). The maximum
           //number of samples that fit in the characteristic is 13 so what we do is split the 
           //15 data points into 3 groups of 5 data points, each of which is sent out in a 
           //different notification.
           bool found = false;
           for (int i = 2; i <= 5; i++)
           {
               if (m_current_sensor_samples % i == 0)
               {
                   m_current_sensor_samples = m_current_sensor_samples / i;
                   found = true;
                   break;
               }
           }

           //If the number of samples can't be split up into 5 notifications or
           //less then just use the full characteristic
           if (!found) m_current_sensor_samples = (MAX_SENSOR_SAMPLES / 3);
       }
   }
   else
   {
       //Unlike when using a single 
       //composite charcteristic, when using three separate characteristics the odds are lower 
       //that data from each sensor will make it to the front end in a single connection interval
       //(although it's possible to achieve multiple characteristic notifications in a single 
       //connection interval, only one is guaranteed during times of high BLE traffic). Since this
       //is the case we want to minimize the total notifications going out, so just fill each
       //characteristic to max capacity.
       if (m_current_sensor_samples > MAX_SENSOR_SAMPLES) m_current_sensor_samples = MAX_SENSOR_SAMPLES;
   }
   
   SEGGER_RTT_printf(0, "%d sensor samples have been selected.\n", m_current_sensor_samples);

   //Set the desired maximum and minimum connection interval values (which are used to negotiate
   //the interval with the central device) based on the desired lag time above. These values
   //must be multiples of 15 milliseconds with at least 15 millisecond spacing between them.
   desired_maximum_connection_interval = (int)(desired_lag_time * 1000) - ((int)(desired_lag_time * 1000) % 15); //round down to the nearst 15th millisecond
   desired_minimum_connection_interval = desired_maximum_connection_interval - 15;
}

void set_sensor_samples(int actual_connection_interval)
{
    //This method gets called after the connection interval has been changed. It's used to 
    //confirm that the number of data samples we collect in a single period is maximized.
    //It pretty much has the same logic as the calculate_samples_and_connection_interval()
    //method above, however, it uses the actual negotiated connection interval instead of 
    //the desired one.
    if (actual_connection_interval != 0) m_connection_interval = actual_connection_interval;
    int maximum_samples = current_sensor_odr * (float)m_connection_interval / 1000.0;

    if ((float)maximum_samples == (float)(current_sensor_odr * (float)m_connection_interval / 1000.0))
    {
        maximum_samples--;
    }

    if (m_use_composite_data)
    {
        if (maximum_samples > (MAX_SENSOR_SAMPLES / 3))
        {
            bool found = false;
            for (int i = 2; i <= 5; i++)
            {
                if (maximum_samples % i == 0)
                {
                    maximum_samples = maximum_samples / i;
                    found = true;
                    break;
                }
            }

            if (!found) maximum_samples = (MAX_SENSOR_SAMPLES / 3);
        }
    }
    else
    {
        if (maximum_samples > MAX_SENSOR_SAMPLES) maximum_samples = MAX_SENSOR_SAMPLES;
    }

    //If the new maximum value doesn't match the value for m_current_sensor_samples
    //calculated in the calculate_samples_and_connection_interval() method, update
    //that value now.
    if (maximum_samples != m_current_sensor_samples)
    {
        m_current_sensor_samples = maximum_samples;
        SEGGER_RTT_printf(0, "Based on connection interval and ODR, sensor samples have been adjusted to %d.\n", m_current_sensor_samples);
    }

    //Make sure we collect at least 1 sample
    if (m_current_sensor_samples < 1) m_current_sensor_samples = 1;

    //Set the data characteristic pointer to the appropriately sized characteristic
    //based on the number of samples
    if (m_current_sensor_samples < 5)
    {
        composite_characteristic_data = small_characteristic_data;
        SEGGER_RTT_WriteString(0, "Using small data characteristic.\n");
    }
    else if (m_current_sensor_samples < 9)
    {
        composite_characteristic_data = medium_characteristic_data;
        SEGGER_RTT_WriteString(0, "Using medium data characteristic.\n");
    }
    else
    {
        composite_characteristic_data = large_characteristic_data;
        SEGGER_RTT_WriteString(0, "Using large data characteristic.\n");
    }
    SEGGER_RTT_WriteString(0, "\n");
}

void data_read_handler(int measurements_taken)
{
    //Everytime the data reading timer goes off we take sensor readings and then 
    //update the appropriate characteristic values. The timer should go off SENSOR_SAMPLES times
    //every connection interval
    int offset = SAMPLE_SIZE * measurements_taken;

    if (m_use_composite_data)
    {
        //If the m_use_composite_data boolean is true then data from all three sensors is 
        //stored in a single characteristic.
        offset *= 3; //we can only fit 1/3 of the data when sharing a single characteristic
        offset += 5; //the composite data characteristic has the timestamp and number of data points variables at the front of the characteristic
        imu_comm.acc_comm.get_data(composite_characteristic_data, offset);
        imu_comm.gyr_comm.get_data(composite_characteristic_data, offset + SAMPLE_SIZE);
        imu_comm.mag_comm.get_data(composite_characteristic_data, offset + SAMPLE_SIZE + SAMPLE_SIZE);
    }
    else
    {
        //Deprecated block, keeping around for the next few updates just in case
        //If the m_use_composite_data boolean is false then data from all each sensor is
        //stored into their own characteristics
        //imu_comm.acc_comm.get_data(acc_characteristic_data, offset);
        //imu_comm.gyr_comm.get_data(gyr_characteristic_data, offset);
        //imu_comm.mag_comm.get_data(mag_characteristic_data, offset);
    }
}


//Functions for updating sensor power modes and settings
static void sensor_idle_mode_start()
{ 
    //In this mode, the appropriate TWI bus(es) is active and power is going to the sensors but the 
    //sensors are put into a lower power mode and are not actively taking measurements. A red LED
    //on the board blinks during this mode
    if (current_operating_mode == SENSOR_IDLE_MODE) return; //no need to change anything if already in idle mode

    //swap to the red LED, also make sure that the blue and green LEDs are off
    active_led = RED_LED;
    nrf_gpio_pin_set(GREEN_LED); //LEDs must be set high to turn off
    nrf_gpio_pin_set(BLUE_LED); //LEDs must be set high to turn off

    ret_code_t err_code;

    if (current_operating_mode == SENSOR_ACTIVE_MODE)
    {
        //If we're transitioning from active to idle mode we need to stop the data collection timer
        //and start the led timer back up.
        data_timers_stop();

        //the LED is deactivated during data collection so turn it back on
        led_timers_start();
    }
    else 
    {
        //If we're transitioning from connected mode to sensor idle mode then we're going to need 
        //to turn on any necessary TWI busses to allow for communication with the sensors.
        enable_twi_bus(imu_comm.acc_comm.twi_bus->inst_idx);
        enable_twi_bus(imu_comm.gyr_comm.twi_bus->inst_idx);
        enable_twi_bus(imu_comm.mag_comm.twi_bus->inst_idx);
    }

    //Put all sensors into idle mode, any of the sensors that are active
    //will be properly de-initialized
    lsm9ds1_idle_mode_enable(current_operating_mode);
    fxos8700_idle_mode_enable(current_operating_mode);
    fxas21002_idle_mode_enable();
    bmi270_idle_mode_enable(current_operating_mode);
    bmm150_idle_mode_enable();

    current_operating_mode = SENSOR_IDLE_MODE; //set the current operating mode to idle
    SEGGER_RTT_WriteString(0, "Sensor Idle Mode engaged.\n");
}

static void sensor_active_mode_start()
{
    //All of the settings for the sensor should have been set already, we just need to activate them
    //and then start the data collection timer. We also disable the LED to save on power
    if (current_operating_mode == SENSOR_ACTIVE_MODE) return; //no need to change anything if already in active mode
    led_timers_stop();

    //TODO: Comment out the twi bus enable commands below and put a break point in the assert_nrf_callback()
    //method around line 95. See what error it kicks up. In the future, handle this error in that method by
    //attempting to turn on the twi buses. Use the error characteristic to alert the front end of this action.
    if (current_operating_mode == CONNECTED_MODE)
    {
        //If we're transitioning from connected mode to sensor active mode then we're going to need 
        //to turn on any necessary TWI busses to allow for communication with the sensors.
        enable_twi_bus(imu_comm.acc_comm.twi_bus->inst_idx);
        enable_twi_bus(imu_comm.gyr_comm.twi_bus->inst_idx);
        enable_twi_bus(imu_comm.mag_comm.twi_bus->inst_idx);
    }

    //Put all initialized sensors into active mode.
    lsm9ds1_active_mode_enable();
    fxos8700_active_mode_enable();
    fxas21002_active_mode_enable(current_operating_mode);
    bmi270_active_mode_enable(current_operating_mode);
    bmm150_active_mode_enable(sensor_odr_calculate(), current_operating_mode);

    //uncomment the below lines to read active sensor registers and confirm settings
    bmm150_get_actual_settings();

    //start data acquisition by turning on the data timers
    data_timers_start();
    
    current_operating_mode = SENSOR_ACTIVE_MODE; //set the current operating mode to active
    m_data_ready = false; //Want to make sure we start with fresh data
    SEGGER_RTT_WriteString(0, "Sensor Active Mode engaged.\n");
}

static void connected_mode_start()
{
    //There are two ways to enter connected mode. We either enter this mode when a connection is
    //first established, or when we navigate to one of the menu screens in the front end. In the 
    //case where the connection is first established, we use this opportunity to initialize any
    //sensors that need an initialization (as of this writing only the BMI/BMM sensors require
    //this). After this initialization is complete, or if connected mode is entered via menu
    //navigation in the front end, we make sure that any active sensors are placed into sleep mode
    //and that any TWI busses are turned off. We also change the on-board blinking LED to green.
    if (current_operating_mode == CONNECTED_MODE) return; //no need to change anything if already in connected mode

    bool bmi270_init = false, bmm150_init = false;
    if (current_operating_mode == ADVERTISING_MODE)
    {
        //all of the connected_mode_enabled() methods require the TWI bus(es) to be
        //on. These buses shouldn't be on when going from advertising mode to 
        //connected mode so turn them on now to avoid any TWI errors being thrown.
        enable_twi_bus(imu_comm.acc_comm.twi_bus->inst_idx);
        enable_twi_bus(imu_comm.gyr_comm.twi_bus->inst_idx);
        enable_twi_bus(imu_comm.mag_comm.twi_bus->inst_idx);

        //If any BMI/BMM sensors are selected they will need to be configured before
        //they can be used. Setting the necessary booleans to true will allow this
        //configuration to occur.
        if (default_sensors[0] == BMI270_ACC || default_sensors[1] == BMI270_GYR) bmi270_init = true; 
        if (default_sensors[2] == BMM150_MAG) bmm150_init = true;
    }

    //Call the connected_mode_enable() method for all sensors. Only sensors that are in active
    //use will actually do anything with these methods
    bmi270_connected_mode_enable(bmi270_init);
    bmm150_connected_mode_enable(bmm150_init);
    fxos8700_connected_mode_enable();
    fxas21002_connected_mode_enable();
    lsm9ds1_connected_mode_enable();

    //Disable all active TWI busses
    disable_twi_bus(imu_comm.acc_comm.twi_bus->inst_idx);
    disable_twi_bus(imu_comm.gyr_comm.twi_bus->inst_idx);
    disable_twi_bus(imu_comm.mag_comm.twi_bus->inst_idx);

    //change the color of the blinking LED to green, also make sure that the red
    //and blue LEDs are off. If we're coming here from sensor active mode then 
    //we're going to need to turn on the LED first and also deactivate the data timer
    if (current_operating_mode == SENSOR_ACTIVE_MODE)
    {
        data_timers_stop();
        led_timers_start();
    }

    active_led = GREEN_LED;
    nrf_gpio_pin_set(RED_LED); //LEDs must be set high to turn off
    nrf_gpio_pin_set(BLUE_LED); //LEDs must be set high to turn off

    current_operating_mode = CONNECTED_MODE; //set the current operating mode to idle
    SEGGER_RTT_WriteString(0, "Connected Mode engaged.\n");
}

static void advertising_mode_start()
{
    //There are two ways to get into advertising mode, either by turning the device on,
    //or by losing a connection to a central device (whether this is intentional or 
    //accidental). In the latter case, we need to disable any sensors or peripherals
    //to save on power.

    switch (current_operating_mode)
    {
        case ADVERTISING_MODE:
            return; //no changes need to be made so leave the method
        case SENSOR_ACTIVE_MODE:
            sensor_idle_mode_start();
            //break statement purposely omitted here
        case SENSOR_IDLE_MODE:
            connected_mode_start();
            //break statement purposely omitted here
        default:
            active_led = BLUE_LED;
            nrf_gpio_pin_set(RED_LED); //LEDs must be set high to turn off
            nrf_gpio_pin_set(GREEN_LED); //LEDs must be set high to turn off
            break;
    }

    current_operating_mode = ADVERTISING_MODE; //set the current operating mode to idle
    
    //finally, we need to terminate the existing connection
    //TODO: Implement this feature
}

static void update_sensor_settings_array()
{
    //This method gets called when the sensor settings array is updated in the front end.
    //For the most part all we do here is apply the new settings to the physical array
    //stored in this file, however, depending on the new settings coming in there may be
    //a few other things to change. For example, if a new ODR for a sensor is selected
    //then we'll need to adjust the data reading timer.

    //See if a new sensor was selected for use. If so we need to uninitialize the current
    //sensor and initialize the new one.
    uint8_t new_sensors = 0;

    if (imu_comm.sensor_model[ACC_SENSOR] != sensor_settings[ACC_START + SENSOR_MODEL])
    {
        new_sensors |= 0b001;
        default_sensors[ACC_SENSOR] = sensor_settings[ACC_START + SENSOR_MODEL];
        sensors_initialized[ACC_SENSOR] = false;
    }

    if (imu_comm.sensor_model[GYR_SENSOR] != sensor_settings[GYR_START + SENSOR_MODEL])
    {
        new_sensors |= 0b010;
        default_sensors[GYR_SENSOR] = sensor_settings[GYR_START + SENSOR_MODEL];
        sensors_initialized[GYR_SENSOR] = false;
    }
    
    if (imu_comm.sensor_model[MAG_SENSOR] != sensor_settings[MAG_START + SENSOR_MODEL])
    {
        new_sensors |= 0b100;
        default_sensors[MAG_SENSOR] = sensor_settings[MAG_START + SENSOR_MODEL];
        sensors_initialized[MAG_SENSOR] = false;
    }

    if (new_sensors) sensors_init(false);

    //Check to see if the ODR was updated. If so we update the data reading timer
    //and update the connection interval accordingly.
    float new_sensor_odr = sensor_odr_calculate();
    
    if (new_sensor_odr != current_sensor_odr)
    {
        //NOTE: I no longer change the connection interval to match the current sensor
        //ODR. I'm leaving this code like this though in the event I ever want to 
        //go back.
        //float temp = current_sensor_odr; //save the original odr in case something goes wrong
        //
        //uint32_t err_code = update_connection_interval();
        //if (err_code != NRF_SUCCESS)
        //{
        //    current_sensor_odr = temp; //reset the sensor odr as it wasn't actually updated
        //    SEGGER_RTT_WriteString(0, "Couldn't update connection interval.\n");
        //    error_notification(err_code);
        //}
        //else
        //{
           
        //}
        current_sensor_odr = new_sensor_odr; //update the ODR;

        //If the sensor ODR has changed then we need to check if it would be 
        //more efficient to store more samples in the data characteristic. We
        //also need to update the data acquisition timer to math the ODR.
        update_data_read_timer(1000.0 / current_sensor_odr);
        set_sensor_samples(0); //using 0 here forces the method to use the current connection interval
    }
}

/**@brief Function for handling write events to the Sensor Settings Characteristic.
 *
 * @param[in] p_lbs     Instance of Sensor Settings Service to which the write applies.
 * @param[in] led_state Written/desired state for the sensor settings.
 */
static void sensor_settings_write_handler(uint16_t conn_handle, ble_sensor_service_t * p_ss, const uint8_t* settings_state)
{
    //Different things can happen depending on what gets written to the settings characteristic
    switch (*settings_state)
    {
        case 0:
            advertising_mode_start();
            break;
        case 1:
            connected_mode_start();
            break;
        case 2:
            sensor_idle_mode_start();
            break;
        case 3:
            for (int i = 1; i < SENSOR_SETTINGS_LENGTH; i++) sensor_settings[i] = *(settings_state + i);
            update_sensor_settings_array();
            break;
        case 4:
            sensor_settings[*(settings_state + 1)] = *(settings_state + 2);
            sensor_idle_mode_start();
            break;
        case 5:
            sensor_active_mode_start();
            break;
    }    
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    ret_code_t                err_code;
    ble_sensor_service_init_t init  = {0};

    // Initialize sensor service
    init.setting_write_handler = sensor_settings_write_handler;

    err_code = ble_sensor_service_init(&m_ss, &init, SENSOR_SETTINGS_LENGTH);
    APP_ERROR_CHECK(err_code);

    err_code = ble_pc_service_init(&m_pc);
    APP_ERROR_CHECK(err_code);
}


void on_gap_connection_handler()
{
    //This handler method gets called every time a new connection is initiated
    connected_mode_start();
    float new_sensor_odr = sensor_odr_calculate();
    if (new_sensor_odr != current_sensor_odr)
    {
        float temp = current_sensor_odr; //save the original odr in case something goes wrong
        current_sensor_odr = new_sensor_odr;
        uint32_t err_code = update_connection_interval();
        if (err_code != NRF_SUCCESS)
        {
            current_sensor_odr = temp; //reset the sensor odr as it wasn't actually updated
            SEGGER_RTT_WriteString(0, "Couldn't update connection interval.\n");
            error_notification(err_code);
        }
        else
        {
            //If the change was successful we also need to update the data
            //aquisition timer to reflect this new odr
            update_data_read_timer(1000.0 / current_sensor_odr);
        }
    }
}

void on_gap_disconnection_handler()
{
    //This handler method gets called when a connection is lost
    advertising_mode_start();
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


static void leds_init()
{
    //configure the pins for the BLE 33 Sense on-board red, green and blue tri-colored LED.
    nrf_gpio_cfg_output(RED_LED);
    nrf_gpio_cfg_output(BLUE_LED);
    nrf_gpio_cfg_output(GREEN_LED);
    nrf_gpio_cfg_output(DARK_GREEN_LED);

    //these LEDs are situated backwards from what you would expect so the pin needs to be 
    //pulled high for the LEDs to turn off
    nrf_gpio_pin_set(RED_LED);
    nrf_gpio_pin_set(BLUE_LED);
    nrf_gpio_pin_set(GREEN_LED);

    //The single green LED doesn't get used due to higher current draw than the 3 RGB LEDs.
    //Make sure its GPIO pin isn't floating to reduced current leak
    nrf_gpio_pin_clear(DARK_GREEN_LED);
}

static void power_saving_init()
{
    //This method implements certain power saving techniques that I've discovered
    //with my time using the SDK

    //enable the DCDC regulators
    uint32_t err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    APP_ERROR_CHECK(err_code);

    //Voltage Regulator 0 can only be enabled if the nRF52840 is being run
    //in High Voltage mode. This requires a voltage > 3.6V to be supplied to
    //the VDDH pin (which won't be the case when running from a coin cell battery).
    err_code = sd_power_dcdc0_mode_set(NRF_POWER_DCDC_ENABLE);
    APP_ERROR_CHECK(err_code);

    SEGGER_RTT_WriteString(0, "DCDC Engaged.\n");

    turn_off_mic(); //used for lowering overall power consumption (prevents GPIO pin from floating)
}

/**@brief Function for application main entry.
 */
int main(void)
 {
    bool erase_bonds;

    //Create structures for handler methods
    ble_event_handler_t ble_handlers;
    ble_handlers.gap_connected_handler = on_gap_connection_handler;
    ble_handlers.gap_disconnected_handler = on_gap_disconnection_handler;
    ble_handlers.gap_connection_interval_handler = calculate_samples_and_connection_interval;
    ble_handlers.gap_update_sensor_samples = set_sensor_samples;

    timer_handlers_t timer_handlers;
    timer_handlers.data_read_handler = data_read_handler;
    timer_handlers.error_handler = error_notification;

    //Initialize.
    log_init();
    timers_init(&active_led, &m_data_ready, &m_current_sensor_samples, &m_time_stamp, &timer_handlers);
    power_management_init();
    ble_stack_init(&ble_handlers, &m_conn_handle, &m_notification_done, &m_notifications_in_queue, &m_current_sensor_samples, &desired_minimum_connection_interval, &desired_maximum_connection_interval);
    enable_connection_event_extension();
    gatt_init();
    services_init();
    twi_init();
    sensors_init(true);
    gap_params_init(current_sensor_odr);
    advertising_init();
    conn_params_init();
    peer_manager_init();
    leds_init();
    power_saving_init();

    //Start execution.
    led_timers_start();
    advertising_start(erase_bonds);

    //Enter main loop.
    for (;;)
    {
        idle_state_handle(); //puts CPU into sleep and waits for an event signal to wake it up

        //after the CPU get's woken up (at a minimum this should happen once every
        //connection interval), see if the data ready flag has been set to true and
        //if so, send out data notifications).
        if (m_data_ready)
        {
            characteristic_update_and_notify();
            m_data_ready = false;
        }
    }
}