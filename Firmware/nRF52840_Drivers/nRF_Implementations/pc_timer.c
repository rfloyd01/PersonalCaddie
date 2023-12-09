#include "pc_timer.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "nrf_gpio.h"

//Global Timer Variables
const nrf_drv_timer_t m_data_read_timer = NRF_DRV_TIMER_INSTANCE(1);         /**< This timer goes off every time we need to read data from the sensors */
const nrf_drv_timer_t m_data_start_timer = NRF_DRV_TIMER_INSTANCE(2);        /**< This timer starts as soon as we take our first data reading, it gives us the absolute time for data reads */
const nrf_drv_timer_t m_delay_timer = NRF_DRV_TIMER_INSTANCE(3);             /**< This timer is used for creating delays */
APP_TIMER_DEF(m_led_on_timer);                                               /**< A timer used for turning on LEDs */
APP_TIMER_DEF(m_led_off_timer);                                              /**< A timer used for turning off LEDs */

//Pointers
static uint8_t* p_total_sensor_samples;  //As the application runs the number of samples to collect from the IMU sensors is subject to change
volatile uint8_t* p_active_led; //Keeps track of which led needs to be turned on and off when the timers go off
volatile bool* p_data_ready; //Once this bool is set to true all data in the data characteristics is sent out via notification
uint32_t* p_current_time_stamp; //Holds the time stamp for the first data reading of each set
timer_handlers_t m_timer_handlers; //Structure to hold function pointers needed by the timers

int measurements_taken = 0;                                              /**< keeps track of how many IMU measurements have taken in the given connection interval. */

void timers_init(volatile uint8_t* led, volatile bool* data_ready, uint8_t* sensor_samples, uint32_t* time_stamp, timer_handlers_t* handlers)
{
    //There are four separate timers that get initialized here. Two
    //timers for tunring LEDs on and off, and two timers for reading data
    //from IMU sensors. The timers for reading data need to be very precise
    //so they uses the nrf_drv_timer module's clock (this clock has a resolution of 
    //16 MHz = 62.5 nanoseconds). The LED timers don't need to be nearly 
    //as precise to they use the app_timer library's clock (this clock has a 
    //resolution of 32 kHz = 21.25 microseconds).

    // Initialize the app timer module for the led timers.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    //Set up pointers to methods and variables else where in the program
    p_total_sensor_samples = sensor_samples;
    p_active_led = led;
    p_data_ready = data_ready;
    p_current_time_stamp = time_stamp;
    m_timer_handlers.data_read_handler = handlers->data_read_handler;
    m_timer_handlers.error_handler = handlers->error_handler; //currently don't have any error handling here, but include this method anyway

    //Create the data reading and delay timers using the nrf_drv_timer library
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.frequency = NRF_TIMER_FREQ_16MHz; //each timer tick equals 62.5 nanoseconds
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32; //2^32 maximum ticks * 62.5 nanoseconds = ~4.5 minutes max for timer

    err_code = nrf_drv_timer_init(&m_data_read_timer, &timer_cfg, data_read_timer_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_timer_init(&m_data_start_timer, &timer_cfg, data_start_timer_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_timer_init(&m_delay_timer, &timer_cfg, delay_timer_handler);
    APP_ERROR_CHECK(err_code);

    //Create app timers for turning the LEDs on and off.
    app_timer_create(&m_led_on_timer, APP_TIMER_MODE_REPEATED, led_on_timer_handler);
    app_timer_create(&m_led_off_timer, APP_TIMER_MODE_REPEATED, led_off_timer_handler);
}

void led_timers_start(void)
{
    //Start the LED on timer. The LED off timer is controlled from within the ON timer
    uint32_t err_code = app_timer_start(m_led_on_timer, APP_TIMER_TICKS(2000), NULL); //the LEDs blink once every 2 seconds
    
}

void led_timers_stop(void)
{
    //Stop the LED on timers. This call can come at any time so turn off both timers.
    app_timer_stop(m_led_on_timer);
    app_timer_stop(m_led_off_timer);
}

void data_timers_start(void)
{
    //Start the data timers
    nrf_drv_timer_enable(&m_data_read_timer);
    nrf_drv_timer_enable(&m_data_start_timer);
}

void data_timers_stop(void)
{
    //Stop and reset the data timers
    nrf_drv_timer_disable(&m_data_read_timer);
    nrf_drv_timer_clear(&m_data_read_timer);

    nrf_drv_timer_disable(&m_data_start_timer);
    nrf_drv_timer_clear(&m_data_start_timer);

    //Set the data ready bool to false, this will prevent
    //notifying old characteristic data the next time the 
    //data timer starts
    *p_data_ready = false;
    measurements_taken = 0; //also reset the measurements taken variable
}

void delay_microseconds(uint32_t microseconds)
{
    //This method turns on the delay timer and enters a while loop (doing nothing)
    //for the given amount of time. It then turns the delay timer off.
    nrf_drv_timer_enable(&m_delay_timer);

    uint32_t start_ticks = nrf_drv_timer_capture(&m_delay_timer, NRF_TIMER_CC_CHANNEL2);
    uint32_t goal_ticks = 16 * microseconds; //each tick represents 1/16,000,000 seconds

    while ((nrf_drv_timer_capture(&m_delay_timer, NRF_TIMER_CC_CHANNEL2) - start_ticks) < goal_ticks)
    {
        //Intentionally left blank
    }

    nrf_drv_timer_disable(&m_delay_timer);
    nrf_drv_timer_clear(&m_delay_timer);
}

uint32_t get_current_data_time()
{
    //If the data timers are currently on, this method returns
    //the current value (in ticks) of the data start timer.
    return nrf_drv_timer_capture(&m_data_start_timer, NRF_TIMER_CC_CHANNEL2);
}

void update_data_read_timer(float milliseconds)
{
    //Used to change how often the data read timer goes off.

    //We need to convert from milliseconds into clock ticks to set the timer.
    //The built-in methods for converting to ticks can't take a number with a decimal
    //point so to be as accurate as possible we use the microsecond method. As an example
    //let's say the sensor ODR is 400 Hz, this corresponds to a sample being taken every 
    //2.5 milliseconds. Using the built-in ms_to_ticks method would round this down to 
    //2 milliseconds flat. This would cause us to take readings 1.25x quicker than need
    //to, resulting in a percieved ODR of 500 instead of 400. Using the microsecond to ticks
    //method though and we'd get the correct value here of 2500 microseconds and take the
    //correct readings.
    uint32_t microseconds = 1000 * milliseconds;
    uint32_t new_data_read_time = nrf_drv_timer_us_to_ticks(&m_data_read_timer, microseconds); //convert from milliseconds to clock ticks
    nrf_drv_timer_extended_compare(&m_data_read_timer, NRF_TIMER_CC_CHANNEL0, new_data_read_time, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
}

static void data_read_timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    //Read data from each of the three sensors using the data read function pointer
    m_timer_handlers.data_read_handler(measurements_taken); //measurements taken is the offset in bytes for the data to be read
    //SEGGER_RTT_printf(0, "Reading taken at %d.\n", nrf_drv_timer_capture(&m_data_start_timer, NRF_TIMER_CC_CHANNEL2));
    
    measurements_taken++;
    if (measurements_taken == 1)
    {
        //We record the time stamp for the first measurement of each data set. This helps
        //keep everything in order in the front end application
        uint32_t time_stamp = nrf_drv_timer_capture(&m_data_start_timer, NRF_TIMER_CC_CHANNEL0);
        *p_current_time_stamp = time_stamp;
        //SEGGER_RTT_printf(0, "Data set begins at time (in ticks) %d\n", time_stamp);
    }
    else if ( measurements_taken == *p_total_sensor_samples)
    {
        //after all the samples are read, update the characteristics and notify
        *p_data_ready = true; //flags the main loop to broadcast data notifications
        measurements_taken = 0; //reset the data counter
        //SEGGER_RTT_printf(0, "Full reading taken at %d.\n", nrf_drv_timer_capture(&m_data_start_timer, NRF_TIMER_CC_CHANNEL2));
    }
}

static void data_start_timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    //Currently have no need for this handler, however, eacch instance of the nrf_drv_timer
    //requires its own handler function.
}

static void delay_timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    //Currently have no need for this handler, however, eacch instance of the nrf_drv_timer
    //requires its own handler function.
}

static void led_on_timer_handler(void * p_context)
{
    //This timer causes the currently active LED to light up. We also turn on the 
    //LED off timer, which will turn the LED back off after 5 milliseconds
    nrf_gpio_pin_clear(*p_active_led); //The LEDs must be pulled low to turn on

    app_timer_start(m_led_off_timer, APP_TIMER_TICKS(5), NULL);
}

void led_off_timer_handler(void * p_context)
{
    //This timer gets called when the LED needs to be turned off. We also turn the timer itself
    //of (it will get turned on again the next time the LED turns on)
    nrf_gpio_pin_set(*p_active_led); //The LEDs must be set high to turn off
    app_timer_stop(m_led_off_timer);
}