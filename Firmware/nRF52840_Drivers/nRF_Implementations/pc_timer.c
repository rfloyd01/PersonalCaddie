#include "pc_timer.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "nrf_gpio.h"

//Global Timer Variables
//APP_TIMER_DEF(m_data_read_timer);                                               /**< A timer used for collecting data from the LSM9DS1 (interrupts are disabled so a timer is needed). */
//#define NEXT_MEASUREMENT_DELAY          APP_TIMER_TICKS(1500)                   /**< Defines the delay between Sensor Measurments (1500 milliseconds). */
//#define LED_DELAY                       APP_TIMER_TICKS(2000)                   /**< Defines the delay between LED blinks */
const nrf_drv_timer_t m_data_read_timer = NRF_DRV_TIMER_INSTANCE(1);         /**< This timer goes off every time we need to read data from the sensors */
const nrf_drv_timer_t m_data_start_timer = NRF_DRV_TIMER_INSTANCE(2);         /**< This timer starts as soon as we take our first data reading, it gives us the absolute time for data reads */
APP_TIMER_DEF(m_led_on_timer);                                                     /**< A timer used for turning on LEDs */
APP_TIMER_DEF(m_led_off_timer);                                                        /**< A timer used for turning off LEDs */

//Pointers
uint8_t* p_total_sensor_samples;  //As the application runs the number of samples to collect from the IMU sensors is subject to change
volatile uint8_t* p_active_led; //Keeps track of which led needs to be turned on and off when the timers go off
volatile bool* p_data_ready; //Once this bool is set to true all data in the data characteristics is sent out via notification
timer_handlers_t m_timer_handlers; //Structure to hold function pointers needed by the timers

int measurements_taken = 0;                                              /**< keeps track of how many IMU measurements have taken in the given connection interval. */

void timers_init(volatile uint8_t* led, volatile bool* data_ready, uint8_t* sensor_samples, timer_handlers_t* handlers)
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
    m_timer_handlers.data_read_handler = handlers->data_read_handler;

    //Create the data reading timers using the nrf_drv_timer library
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.frequency = NRF_TIMER_FREQ_16MHz; //each timer tick equals 62.5 nanoseconds
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32; //2^32 maximum ticks * 62.5 nanoseconds = ~4.5 minutes max for timer

    err_code = nrf_drv_timer_init(&m_data_read_timer, &timer_cfg, data_read_timer_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_timer_init(&m_data_start_timer, &timer_cfg, data_start_timer_handler);
    APP_ERROR_CHECK(err_code);

    //Create app timers for turning the LEDs on and off.
    app_timer_create(&m_led_on_timer, APP_TIMER_MODE_REPEATED, led_on_timer_handler);
    app_timer_create(&m_led_off_timer, APP_TIMER_MODE_REPEATED, led_off_timer_handler);
}

void led_timers_start(void)
{
    //Start the LED on timer. The LED off timer is controlled from within the ON timer
    app_timer_start(m_led_on_timer, APP_TIMER_TICKS(2000), NULL); //the LEDs blink once every 2 seconds
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
}

void update_data_read_timer(int milliseconds)
{
    //Used to change how often the data read timer goes off.
    uint32_t new_data_read_time = nrf_drv_timer_ms_to_ticks(&m_data_read_timer, milliseconds); //convert from milliseconds to clock ticks
    nrf_drv_timer_extended_compare(&m_data_read_timer, NRF_TIMER_CC_CHANNEL0, new_data_read_time, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
}

static void data_read_timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    //Read data from each of the three sensors using the data read function pointer
    m_timer_handlers.data_read_handler(measurements_taken); //measurements taken is the offset in bytes for the data to be read
    
    measurements_taken++;
    if (measurements_taken == 1)
    {
        //We record the time stamp for the first measurement of each data set. This helps
        //keep everything in order in the front end application
        uint32_t time_stamp = nrf_drv_timer_capture(&m_data_start_timer, NRF_TIMER_CC_CHANNEL0);
        //SEGGER_RTT_printf(0, "Data set begins at time (in ticks) %d\n", time_stamp);
    }
    else if ( measurements_taken == *p_total_sensor_samples)
    {
        //after all the samples are read, update the characteristics and notify
        *p_data_ready = true; //flags the main loop to broadcast data notifications
        measurements_taken = 0; //reset the data counter
    }
}

static void data_start_timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    //Currently have no need for this handler, however, eacch instance of the nrf_drv_timer
    //requires its own handler function.
}

static void led_on_timer_handler(void * p_context)
{
    //This timer causes the currently active LED to light up. We also turn on the 
    //LED off timer, which will turn the LED back off after 5 milliseconds
    nrf_gpio_pin_clear(*p_active_led); //The LEDs must be pulled low to turn on

    app_timer_start(m_led_off_timer, APP_TIMER_TICKS(5), NULL); //the LEDs blink only lasts for 5 milliseconds
}

void led_off_timer_handler(void * p_context)
{
    //This timer gets called when the LED needs to be turned off. We also turn the timer itself
    //of (it will get turned on again the next time the LED turns on)
    nrf_gpio_pin_set(*p_active_led); //The LEDs must be set high to turn off
    app_timer_stop(m_led_off_timer);
}