#ifndef PC_TIMER_H__
#define PC_IMER_H__

#include <stdint.h>
#include <stdbool.h>

#include "app_timer.h"
#include "nrf_drv_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
The PersonalCaddie_timer files have to do with creating, starting and 
stopping timers needed for the application. There are varios timers used,
such as a data collection timer, a timer for blinking LEDs, etc.
*/

//Forward declaration of the data_event_handler_t type.
typedef struct timer_handlers_s timer_handlers_t;

typedef void (*timer_event_handler_t) (int measurements_taken); //Function pointer for methods to be passed to timer handlers

//Struct to hold function pointers for BLE Event handler
struct timer_handlers_s
{
    timer_event_handler_t data_read_handler;   /**< This method is used to read data every time the data read timer goes off */
};

//Init methods
void timers_init(volatile uint8_t* led, volatile bool* data_ready, uint8_t* sensor_samples, timer_handlers_t* handlers);

//Timer Update Methods
void update_data_read_timer(int milliseconds);

//Start/Stop Timer Methods
void led_timers_start(void);
void led_timers_stop(void);
void data_timers_start(void);
void data_timers_stop(void);

//Handlers
static void data_read_timer_handler(nrf_timer_event_t event_type, void* p_context);
static void data_start_timer_handler(nrf_timer_event_t event_type, void* p_context);
static void led_on_timer_handler(void * p_context);
static void led_off_timer_handler(void * p_context);

#ifdef __cplusplus
}
#endif

#endif // PC_TIMER_H