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
#include "ble_sensor_service.h"
#include "personal_caddie_operating_modes.h"
#include "nRF_Implementations/pc_twi.h"
#include "nRF_Implementations/pc_ble.h"
#include "nRF_Implementations/pc_timer.h"

//Soft Device Parameters
#define DEAD_BEEF                       0xDEADBEEF                                /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
BLE_SENSOR_SERVICE_DEF(m_ss);                                                     /**< IMU Sensor Service instance. */
uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                                 /**< Handle of the current connection. */

//IMU Sensor Parameters
static uint8_t default_sensors[3] = {FXOS8700_ACC, FXAS21002_GYR, FXOS8700_MAG};  /**< Default sensors that are attempted to be initialized first. */
static bool sensors_initialized[3] = {false, false, false};                       /**< Keep track of which sensors are currently initialized */
static uint8_t internal_sensors[10];                                              /**< An array for holding the addresses of sensors on the internal TWI line */
static uint8_t external_sensors[10];                                              /**< An array for holding the addresses of sensors on the external TWI line */
static uint8_t internal_sensors_found = 0;
static uint8_t external_sensors_found = 0;

//IMU Sensor Data Parameters
static uint8_t acc_characteristic_data[5 + MAX_SENSOR_SAMPLES * SAMPLE_SIZE];      /**< An array for holding current accelerometer readings */
static uint8_t gyr_characteristic_data[5 + MAX_SENSOR_SAMPLES * SAMPLE_SIZE];      /**< An array for holding current gyroscope readings */
static uint8_t mag_characteristic_data[5 + MAX_SENSOR_SAMPLES * SAMPLE_SIZE];      /**< An array for holding current magnetometer readings */
uint8_t sensor_settings[SENSOR_SETTINGS_LENGTH];                                   /**< An array represnting the IMU sensor settings */
uint8_t m_current_sensor_samples = 10;                                             /**< The number of sensor samples currently being put into the acc,gy and mag characteristics (must be less than MAX_SENSOR_SAMPLES */
uint32_t m_time_stamp;                                                             /**< Keeps track of the time that each data set is read at (this is measured in ticks of a 16MHz clock, i.e. 1 LSB = 1/16000000s = 62.5ns) */
volatile bool m_data_ready  = false;                                               /**< Indicates when all characteristics have been filled with new data and we're ready to send it to the client  */
volatile bool m_notification_done = false;                                         /**< Indicates when the current data notification has complete  */

//IMU Sensor Communication Parameters
imu_communication_t imu_comm;                                                      /**< Structure that Holds information on how to communicate with each sensor */
uint16_t desired_minimum_connection_interval, desired_maximum_connection_interval; /**< The desired min and max connection interval */
static float current_sensor_odr = 59.5;                                            /**< Keeps track of the current sensor ODR, connection interval is set based on this variable */

//LED Pin Parameters
#define RED_LED            NRF_GPIO_PIN_MAP(0, 24)                                 /**< Red LED Indicator on BLE 33 sense*/
#define BLUE_LED           NRF_GPIO_PIN_MAP(0, 6)                                  /**< Blue LED Indicator on BLE 33 sense*/
#define GREEN_LED          NRF_GPIO_PIN_MAP(0, 16)                                 /**< Green LED Indicator on BLE 33 sense*/
volatile uint8_t active_led = BLUE_LED;                                            /**< Variable used to keep track of which color LED to turn on/off*/

//Personal Caddie Parameters
static personal_caddie_operating_mode_t current_operating_mode = ADVERTISING_MODE; /**< The chip starts in advertising mode*/


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
            if (model == LSM9DS1_ACC)
            {
                imu_comm.acc_comm.update_settings = lsm9ds1_acc_apply_setting;
                imu_comm.acc_comm.get_data = lsm9ds1_get_acc_data;
            }
            else if (model == FXOS8700_ACC)
            {
                imu_comm.acc_comm.update_settings = fxos8700_acc_apply_setting;
                imu_comm.acc_comm.get_data = fxos8700_get_acc_data;
            }
            break;
        case GYR_SENSOR:
            imu_comm.gyr_comm.address = address;
            imu_comm.gyr_comm.twi_bus = bus;
            imu_comm.gyr_comm.read_register = sensor_read_register;
            imu_comm.gyr_comm.write_register = sensor_write_register;
            if (model == LSM9DS1_GYR)
            {
                imu_comm.gyr_comm.update_settings = lsm9ds1_gyr_apply_setting;
                imu_comm.gyr_comm.get_data = lsm9ds1_get_gyr_data;
            }
            else if (model == FXAS21002_GYR)
            {
                imu_comm.gyr_comm.update_settings = NULL;
                imu_comm.gyr_comm.get_data = fxas21002_get_gyr_data;
            }
            break;
        case MAG_SENSOR:
            imu_comm.mag_comm.address = address;
            imu_comm.mag_comm.twi_bus = bus;
            imu_comm.mag_comm.read_register = sensor_read_register;
            imu_comm.mag_comm.write_register = sensor_write_register;
            if (model == LSM9DS1_MAG)
            {
                imu_comm.mag_comm.update_settings = lsm9ds1_mag_apply_setting;
                imu_comm.mag_comm.get_data = lsm9ds1_get_mag_data;
            }
            else if (model == FXOS8700_MAG)
            {
                imu_comm.mag_comm.update_settings = fxos8700_mag_apply_setting;
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


/**@brief Function for the LSM9DS1 initialization.
 *
 * @details Initializes the LSM9DS1 accelerometer, gyroscope and magnetometer
 */
static void sensors_init(bool discovery)
{
    //When first turning on the Personal Caddie we need to scan both the internal
    //and external TWI bus to see what sensors are available so we can populate some
    //arrays with this information. Subsequent calls to this method don't require this.
    //Regardless of whether or not we enter this method in discovery mode, both TWI buses
    //need to be enabled.
    enable_twi_bus(get_internal_twi_bus_id());
    enable_twi_bus(get_external_twi_bus_id());
    nrf_delay_ms(50); //slight delay so sensors have time to power on
    
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

    //See if all sensors were initialized on the external bus, if not, then initialized sensors
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

    //Handle the initialization of individual sensors. Currently the model for each sensor type
    //matches the enums for the other sensor models (i.e. lsm9ds1 acc/gyrmag all have enum values of 0x00).
    //We take advantage of this fact to see how many sensors from each model need to be initialized. This
    //may not hold up as more sensors get added, but I'll address that when the time comes.
    for (int i = 0; i < ACC_MODEL_END; i++)
    {
        int sensors = 0;
        for (int j = 0; j < 3; j++)
        {
            if (default_sensors[j] == i)
            {
                imu_comm.sensor_model[j] = i;
                sensors |= (1 << j);
            }
        }

        if (sensors != 0)
        {
            switch(i)
            {
                case LSM9DS1_ACC:
                    lsm9ds1_init(&imu_comm, sensors, sensor_settings);
                    break;
                case FXOS8700_ACC:
                    fxos8700init(&imu_comm, sensors, sensor_settings);
                    fxas21002init(&imu_comm, sensors, sensor_settings);
                    break;
            }
        }
    }

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

static void characteristic_update_and_notify()
{
    ble_gatts_hvx_params_t acc_notify_params, gyr_notify_params, mag_notify_params;
    memset(&acc_notify_params, 0, sizeof(acc_notify_params));
    memset(&gyr_notify_params, 0, sizeof(gyr_notify_params));
    memset(&mag_notify_params, 0, sizeof(mag_notify_params));

    uint16_t acc_data_characteristic_size = 5 + MAX_SENSOR_SAMPLES * SAMPLE_SIZE;
    uint16_t gyr_data_characteristic_size = 5 + MAX_SENSOR_SAMPLES * SAMPLE_SIZE;
    uint16_t mag_data_characteristic_size = 5 + MAX_SENSOR_SAMPLES * SAMPLE_SIZE;

    //Add the time stamp for the current data set and current number of 
    //samples to the end of each of the data characteristics.
    for (int i = 0; i < 4; i++)
    {
        uint8_t* time_start = (uint8_t*)&m_time_stamp; //cast the float to a 32-bit integer to take up 4 array slots

        acc_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + i] = *(time_start + i);
        gyr_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + i] = *(time_start + i);
        mag_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + i] = *(time_start + i);
    }
    acc_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + 4] = m_current_sensor_samples;
    gyr_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + 4] = m_current_sensor_samples;
    mag_characteristic_data[MAX_SENSOR_SAMPLES * SAMPLE_SIZE + 4] = m_current_sensor_samples;

    //Setup accelerometer notification first
    acc_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    acc_notify_params.handle = m_ss.data_handles[0].value_handle;
    acc_notify_params.p_data = acc_characteristic_data;
    acc_notify_params.p_len  = &acc_data_characteristic_size;
    acc_notify_params.offset = 0;

    //Setup gyroscope notification second
    gyr_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    gyr_notify_params.handle = m_ss.data_handles[1].value_handle;
    gyr_notify_params.p_data = gyr_characteristic_data;
    gyr_notify_params.p_len  = &gyr_data_characteristic_size;
    gyr_notify_params.offset = 0;

    //Setup magnetometer notification third
    mag_notify_params.type = BLE_GATT_HVX_NOTIFICATION;
    mag_notify_params.handle = m_ss.data_handles[2].value_handle;
    mag_notify_params.p_data = mag_characteristic_data;
    mag_notify_params.p_len  = &mag_data_characteristic_size;
    mag_notify_params.offset = 0;

    //Set the notifcation_done boolean to false and start notifications.
    //The boolean will be set to true in the BLE handler when the notification
    //is complete, alerting us to send out the next notification.
    m_notification_done = false;
    uint32_t ret = sd_ble_gatts_hvx(m_conn_handle, &acc_notify_params); //acc data notification
    ret = sd_ble_gatts_hvx(m_conn_handle, &gyr_notify_params); //gyr data notification
    ret = sd_ble_gatts_hvx(m_conn_handle, &mag_notify_params); //mag data notification

    while (!m_notification_done) {} //wait for the three notifications to go out before continuing
    //APP_ERROR_CHECK(ret); //Uncommenting this can help debug notification errors
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
    //This method is used to select the appropriate number of sensor samples to collect
    //and the min-max connection interval to select to minimize data loss.

    //This method is necessary as a consequence of not being able to set a specific 
    //connection interval, the interval must be negotiated between two devices and thus
    //is prone to not be exactly what we want. On iOS devices, in particular, the minimumn
    //and maximum requested connection interval must be at least 15 milliseconds apart. 
    //Depending on the current sensor ODR, 15 milliseconds can be anywhere from 0.75 sensor
    //samples at an ODR of 50 Hz to 6 samples at 400 Hz. Basically, depending on what the 
    //negotiated connection interval is, it may make sense to collect more or less samples
    //at a time from the sensors in order to minimize data being overwritten.

    //Since we won't know what the actual connection interval will be until after it's
    //negotiated, we attempt to pick a min and max interval that will maximize our chances
    //of having low data loss.
    int minimum_time = ceil(1000.0 / current_sensor_odr);
    minimum_time += (15 - minimum_time % 15);

    int maximum_time = ceil(MAX_SENSOR_SAMPLES * 1000.0 / current_sensor_odr);
    maximum_time += (15 - maximum_time % 15);

    float current_expected_loss = 1.0, current_best_loss = 1.0; //at 0 samples read we expect to lose all data
    int current_time = minimum_time, best_minimum_interval = 0;

    for (; current_time <= maximum_time; current_time += 15)
    {
        //For each time interval we calcualte the expected data loss for choosing 1 - MAX_SENSOR_SAMPLES
        //number of samples. If the time needed to collect the samples is longer than the current_time
        //limit we stop iterating (we never want our data collection time to be longer than the connection
        //interval or the data will start to lag).
        float best_data_loss_percentage = 1.0; 
        for (int i = 1; i <= MAX_SENSOR_SAMPLES; i++)
        {
            float data_collection_time = i * 1000.0 / current_sensor_odr;

            //we assume that if the data collection time = the connection interval we will slowly fall behind. So 
            //only carry out the calculation when the data collection time is less than the connection interval
            if (data_collection_time >= current_time) break;

            //Calculate the expected data loss with current data_collection_time and connection interval
            float connection_cycle_length = findDecimalLCM(current_time, data_collection_time);

            float total_connection_intervals = connection_cycle_length / current_time;
            float total_data_packets = connection_cycle_length / data_collection_time;
            float data_loss_percentage = (total_data_packets - total_connection_intervals) / total_data_packets;

            if (data_loss_percentage < best_data_loss_percentage) best_data_loss_percentage = data_loss_percentage;
        }

        //We now assume that we choose the lower of the two current connection intervals
        //being compared. We expect to get the higher connection interval 90% of the time
        //and the lower interval only 10% of the time.
        float expected_data_loss = 0.1 * current_expected_loss + 0.9 * best_data_loss_percentage;
        if (expected_data_loss < current_best_loss)
        {
            current_best_loss = expected_data_loss;
            best_minimum_interval = current_time - 15;
        }

        current_expected_loss = best_data_loss_percentage;
    }
    
    //Once all calculations are done, set the desired minimum and maximum connection
    //intervals based on the best_minimum_interval variable
    desired_minimum_connection_interval = best_minimum_interval;
    desired_maximum_connection_interval = desired_minimum_connection_interval + 15;
}

void set_sensor_samples(int actual_connection_interval)
{
    //As mentioned above in the alcualte_samples_and_connection_interval() method,
    //there's no guarantee as to what the connection interval is going to be set at.
    //Once the interval has been established we update the sample amount so that
    //sensor odr * sample amount is as close to the connection interval (without
    //exceeding it) as possible.
    for (int i = MAX_SENSOR_SAMPLES; i >= 0; i--)
    {
        if ((1000.0 * i / current_sensor_odr) < actual_connection_interval)
        {
            SEGGER_RTT_printf(0, "%d sensor samples selected.\n", i);
            m_current_sensor_samples = i;

            //Not 100% necesary here, however, it may be handy for debugging.
            //Print out the expected data loss per connection interval to 
            //confirm our selection was ok. Ideally this number would be a good
            //deal under 1 byte.
            float connection_cycle_length = findDecimalLCM(actual_connection_interval, 1000 * i / current_sensor_odr);

            float connection_intervals_per_cycle = connection_cycle_length / actual_connection_interval;
            float total_data_packets = connection_cycle_length / (1000 * i / current_sensor_odr);
            float total_data_loss_percent = (total_data_packets - connection_intervals_per_cycle) / total_data_packets; //bytes in a complete sensor reading

            SEGGER_RTT_printf(0, "Expected data loss is ~: %d%%\n", (uint32_t)(100 * total_data_loss_percent));
            break;
        }
    }
}

void data_read_handler(int measurements_taken)
{
    //Everytime the data reading timer goes off we take sensor readings and then 
    //update the appropriate characteristic values. The timer should go off SENSOR_SAMPLES times
    //every connection interval
    imu_comm.acc_comm.get_data(acc_characteristic_data, SAMPLE_SIZE * measurements_taken);
    imu_comm.gyr_comm.get_data(gyr_characteristic_data, SAMPLE_SIZE * measurements_taken);
    imu_comm.mag_comm.get_data(mag_characteristic_data, SAMPLE_SIZE * measurements_taken);
}


//Functions for updating sensor power modes and settings
static void sensor_idle_mode_start()
{ 
    //In this mode, the appropriate TWI bus(es) is active and power is going to the sensors but the 
    //sensors are put into sleep mode. A red LED on the board blinks during this mode
    if (current_operating_mode == SENSOR_IDLE_MODE) return; //no need to change anything if already in idle mode

    //swap to the red LED, also make sure that the blue and green LEDs are off
    active_led = RED_LED;
    nrf_gpio_pin_set(GREEN_LED); //LEDs must be set high to turn off
    nrf_gpio_pin_set(BLUE_LED); //LEDs must be set high to turn off

    ret_code_t err_code;

    if (current_operating_mode == SENSOR_ACTIVE_MODE)
    {
        //If we're transitioning from active to idle mode we need to stop the data collection timer
        //and then put the sensor into sleep mode.
        data_timers_stop();
        
        //Put all sensors into idle mode, any of the sensors that are active
        //will be properly initialized
        lsm9ds1_idle_mode_enable();
        fxos8700_idle_mode_enable();
        fxas21002_idle_mode_enable();

        //the LED is deactivated during data collection so turn it back on
        led_timers_start();
    }
    else 
    {
        //If we aren't in sensor active or idle mode, it means that we arrived here from connection mode.
        //This means that both the TWI bus and sensors need to be turned on. There are two reasons why we'd
        //be turning on the sensors, either we're about to take data readings, or, we want to update the
        //sensor settings. In both cases we need to turn on the TWI bus to enable communication, but
        //before doing so we need to see if a call from the front end to use a different sensor has been made.
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

        //Initialize any new sensors that need it
        if (new_sensors) sensors_init(false);

        //turn on any TWI buses that are needed by the current sensors
        enable_twi_bus(imu_comm.acc_comm.twi_bus->inst_idx);
        enable_twi_bus(imu_comm.gyr_comm.twi_bus->inst_idx);
        enable_twi_bus(imu_comm.mag_comm.twi_bus->inst_idx);
        nrf_delay_ms(50); //slight delay so sensors have time to power on   
    }

    current_operating_mode = SENSOR_IDLE_MODE; //set the current operating mode to idle
}

static void sensor_active_mode_start()
{
    if (current_operating_mode != SENSOR_IDLE_MODE) return; //we can only go to active mode from idle mode

    //All of the settings for the sensor should have been set already, we just need to activate them
    //and then start the data collection timer. We also disable the LED to save on power
    led_timers_stop();
    
    //Put all initialized sensors into active mode
    lsm9ds1_active_mode_enable();
    fxos8700_active_mode_enable();
    fxas21002_active_mode_enable();

    //start data acquisition by turning on the data timers
    data_timers_start();
    
    current_operating_mode = SENSOR_ACTIVE_MODE; //set the current operating mode to active
}

static void connected_mode_start()
{
    //We'll other be put into this mode from advertising mode (in which case all we need to do is 
    //change the color of the blinking LED, or from sensor_idle mode, in which case we need to 
    //disable the power going to the sensor and the TWI bus.
    if (current_operating_mode == SENSOR_IDLE_MODE)
    {
        //The chip is currently in sensor idle mode so we need to power off the sensors and disable any active TWI bus
        disable_twi_bus(imu_comm.acc_comm.twi_bus->inst_idx);
        disable_twi_bus(imu_comm.gyr_comm.twi_bus->inst_idx);
        disable_twi_bus(imu_comm.mag_comm.twi_bus->inst_idx);
    }

    //change the color of the blinking LED to green, also make sure that the red
    //and blue LEDs are off
    active_led = GREEN_LED;
    nrf_gpio_pin_set(RED_LED); //LEDs must be set high to turn off
    nrf_gpio_pin_set(BLUE_LED); //LEDs must be set high to turn off

    current_operating_mode = CONNECTED_MODE; //set the current operating mode to idle
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
            sensor_idle_mode_start();
            float new_sensor_odr = sensor_odr_calculate();

            //update connection interval if necessary
            if (new_sensor_odr != current_sensor_odr)
            {
                float temp = current_sensor_odr; //save the original odr in case something goes wrong
                current_sensor_odr = new_sensor_odr;
                if (update_connection_interval(new_sensor_odr) != NRF_SUCCESS)
                {
                    current_sensor_odr = temp; //reset the sensor odr as it wasn't actually updated
                    SEGGER_RTT_WriteString(0, "Couldn't update connection interval.\n");
                }
                else
                {
                    //If the change was successful we also need to update the data
                    //aquisition timer to reflect this new odr
                    update_data_read_timer(1000.0 / current_sensor_odr);
                }
            }
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
        if (update_connection_interval(new_sensor_odr) != NRF_SUCCESS)
        {
            current_sensor_odr = temp; //reset the sensor odr as it wasn't actually updated
            SEGGER_RTT_WriteString(0, "Couldn't update connection interval.\n");
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

    //these LEDs are situated backwards from what you would expect so the pin needs to be 
    //pulled high for the LEDs to turn off
    nrf_gpio_pin_set(RED_LED);
    nrf_gpio_pin_set(BLUE_LED);
    nrf_gpio_pin_set(GREEN_LED);
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

    //Initialize.
    log_init();
    timers_init(&active_led, &m_data_ready, &m_current_sensor_samples, &m_time_stamp, &timer_handlers);
    power_management_init();
    ble_stack_init(&ble_handlers, &m_conn_handle, &m_notification_done, &m_current_sensor_samples, &desired_minimum_connection_interval, &desired_maximum_connection_interval);
    gatt_init();
    services_init();
    twi_init();
    sensors_init(true);
    gap_params_init(current_sensor_odr);
    advertising_init();
    conn_params_init();
    peer_manager_init();
    leds_init();

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