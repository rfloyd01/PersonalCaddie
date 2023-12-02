#include "pc_twi.h"
#include "app_error.h"
#include "SEGGER_RTT.h"
#include "nrf_gpio.h"
#include "sensor_settings.h"
#include "stdlib.h"

//Global TWI variables
#if TWI0_ENABLED
#define INTERNAL_TWI_INSTANCE_ID     0
#endif

#if TWI1_ENABLED
#define EXTERNAL_TWI_INSTANCE_ID     1
#endif

const nrf_drv_twi_t m_twi_internal = NRF_DRV_TWI_INSTANCE(INTERNAL_TWI_INSTANCE_ID);
const nrf_drv_twi_t m_twi_external = NRF_DRV_TWI_INSTANCE(EXTERNAL_TWI_INSTANCE_ID);

//Pin definitions
#define EXTERNAL_SENSOR_POWER_PIN    NRF_GPIO_PIN_MAP(1, 10)                         /**< Pin used to power external sensors (mapped to RX on BLE 33)*/
#define EXTERNAL_SCL_PIN             NRF_GPIO_PIN_MAP(0, 21)                         /**< Pin used for external TWI clock (mapped to D8 on BLE 33) */
#define EXTERNAL_SDA_PIN             NRF_GPIO_PIN_MAP(0, 23)                         /**< Pin used for external TWI data (mapped to D7 on BLE 33) */
#define EXTERNAL_PULLUP              0                                               /**< Pin used for external TWI data (currently unused) */
#define INTERNAL_SENSOR_POWER_PIN    NRF_GPIO_PIN_MAP(0, 22)                         /**< Pin used to power BLE33 onboard sensors*/
#define INTERNAL_SCL_PIN             NRF_GPIO_PIN_MAP(0, 15)                         /**< Pin used for internal TWI clock for BLE33*/
#define INTERNAL_SDA_PIN             NRF_GPIO_PIN_MAP(0, 14)                         /**< Pin used for internal TWI data for BLE33*/
#define INTERNAL_PULLUP              NRF_GPIO_PIN_MAP(1, 0)                          /**< Pullup resistors on BLE 33 sense have separate power source*/

volatile bool m_xfer_internal_done = false; //Indicates if operation on the internal TWI bus has ended.
volatile bool m_xfer_external_done = false; //Indicates if operation on the external TWI bus has ended.
bool m_display_twi_events = true;  //There are times were we don't bother displaying twi event messages (such as getting address NACKs during device scan)

static int m_twi_internal_bus_status, m_twi_external_bus_status;  // lets us know the status of the internal and external TWI bus after each communication attempt

void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context, uint8_t twi_bus)
{
    //A common handler for both TWI buses. Each bus has its own handler method, which in turn
    //will call this method. This was done to allow for processing of simultaneous events in 
    //different threads. For now this method is used more for alerting/debugging purposes than
    //anything. Also, for each TWI bus to be run in non-blocking mode a handler method is 
    //required.
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (!m_display_twi_events) SEGGER_RTT_printf(0, "TWI Success: Bus %d: Successfully reached slave address 0x%x.\n", twi_bus, p_event->xfer_desc.address);
            break;
        case NRF_DRV_TWI_EVT_ADDRESS_NACK:
            if (m_display_twi_events) SEGGER_RTT_printf(0, "TWI Error: Bus %d: Address NACK received while trying to reach slave address 0x%x.\n", twi_bus, p_event->xfer_desc.address);
            break;
        case NRF_DRV_TWI_EVT_DATA_NACK:
            if (m_display_twi_events) SEGGER_RTT_printf(0, "TWI Error: Bus %d: Data NACK received while trying to reach slave address 0x%x.\n", twi_bus, p_event->xfer_desc.address);
            break;
        case NRFX_TWI_EVT_OVERRUN:
            if (m_display_twi_events) SEGGER_RTT_printf(0, "TWI Error: Bus %d: Overrun event while trying to reach slave address 0x%x.\n", twi_bus, p_event->xfer_desc.address);
            break;
        case NRFX_TWI_EVT_BUS_ERROR:
            if (m_display_twi_events) SEGGER_RTT_printf(0, "TWI Error: Bus %d: Bus error received while trying to reach slave address 0x%x.\n", twi_bus, p_event->xfer_desc.address);
            break;
        default:
            if (m_display_twi_events) SEGGER_RTT_WriteString(0, "TWI Error: Bus %d: Unknown error occured.\n");
            break;
    }
}

void internal_twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    m_twi_internal_bus_status = p_event->type;
    twi_handler(p_event, p_context, INTERNAL_TWI_INSTANCE_ID); //forward the event to the main twi handler
    m_xfer_internal_done = true; //unblock TWI communications
}

void external_twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    m_twi_external_bus_status = p_event->type;
    twi_handler(p_event, p_context, EXTERNAL_TWI_INSTANCE_ID);  //forward the event to the main twi handler
    m_xfer_external_done = true; //unblock TWI communications
}

void twi_init()
{
    //The Personal Caddie makes use of two separate TWI buses. One is for communicating with sensors
    //that are onboard the current chip, while the other is for communicating to external sensors on 
    //a bread board.
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_internal_config = {
       .scl                = INTERNAL_SCL_PIN,
       .sda                = INTERNAL_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = true
        };

    const nrf_drv_twi_config_t twi_external_config = {
       .scl                = EXTERNAL_SCL_PIN,
       .sda                = EXTERNAL_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = true
        };

    //A handler method is necessary to enable non-blocking mode. TXRX operations can only be carried 
    //out when non-blocking mode is enabled so a handler is needed here.
    err_code = nrf_drv_twi_init(&m_twi_internal, &twi_internal_config, internal_twi_handler, NULL);
    err_code = nrf_drv_twi_init(&m_twi_external, &twi_external_config, external_twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    //After initializing the buses, configure the pins needed to power up IMU sensors
    nrf_gpio_cfg(INTERNAL_SENSOR_POWER_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0H1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(EXTERNAL_SENSOR_POWER_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0H1, NRF_GPIO_PIN_NOSENSE);
    //nrf_gpio_cfg(INTERNAL_SENSOR_POWER_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    //nrf_gpio_cfg(EXTERNAL_SENSOR_POWER_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);

    //Configure pullup resistor pins if necessary
    if (INTERNAL_PULLUP != 0) nrf_gpio_cfg_output(INTERNAL_PULLUP);
    if (EXTERNAL_PULLUP != 0) nrf_gpio_cfg_output(EXTERNAL_PULLUP);
}

void enable_twi_bus(int instance_id)
{
    //This method enables the given twi bus instance, as well as turns on any 
    //necessary power and pullup resistor pins. Before enabling, make sure the
    //current instance isn't already enabled as this will cause an error.
    if (instance_id == INTERNAL_TWI_INSTANCE_ID)
    {
        if (!m_twi_internal.u.twim.p_twim->ENABLE)
        {
            //Send power to the internal sensors and any pullup resistors,
            //also make sure that power for the external sensors is off
            nrf_gpio_pin_set(INTERNAL_SENSOR_POWER_PIN);
            if (INTERNAL_PULLUP != 0)
            {
                nrf_gpio_pin_set(INTERNAL_PULLUP);
                SEGGER_RTT_WriteString(0, "Internal Pullup Enabled.\n");
            }

            nrf_drv_twi_enable(&m_twi_internal); //enable the bus
            SEGGER_RTT_WriteString(0, "Internal TWI Bus Enabled.\n");
        }
    }
    else
    {
        if (!m_twi_external.u.twim.p_twim->ENABLE)
        {
            //Send power to the external sensors. also make sure that power,
            //afor the internal sensors and the pullup resistor is off
            nrf_gpio_pin_set(EXTERNAL_SENSOR_POWER_PIN);
            if (EXTERNAL_PULLUP != 0)
            {
                nrf_gpio_pin_set(EXTERNAL_PULLUP);
                SEGGER_RTT_WriteString(0, "External Pullup Enabled.\n");
            }

            nrf_drv_twi_enable(&m_twi_external); //enable the bus
            SEGGER_RTT_WriteString(0, "External TWI Bus Enabled.\n");
        }
    }
}

void enable_twi_bus_test(int instance_id)
{
    //This method enables the given twi bus instance, as well as turns on any 
    //necessary power and pullup resistor pins. Before enabling, make sure the
    //current instance isn't already enabled as this will cause an error.
    if (instance_id == INTERNAL_TWI_INSTANCE_ID)
    {
        if (!m_twi_internal.u.twim.p_twim->ENABLE)
        {
            //Send power to the internal sensors and any pullup resistors,
            //also make sure that power for the external sensors is off
            nrf_gpio_pin_set(INTERNAL_SENSOR_POWER_PIN);
            //if (INTERNAL_PULLUP != 0)
            //{
            //    nrf_gpio_pin_set(INTERNAL_PULLUP);
            //    SEGGER_RTT_WriteString(0, "Internal Pullup Enabled.\n");
            //}

            //nrf_drv_twi_enable(&m_twi_internal); //enable the bus
            //SEGGER_RTT_WriteString(0, "Internal TWI Bus Enabled.\n");
        }
    }
    else
    {
        if (!m_twi_external.u.twim.p_twim->ENABLE)
        {
            //Send power to the external sensors. also make sure that power,
            //afor the internal sensors and the pullup resistor is off
            nrf_gpio_pin_set(EXTERNAL_SENSOR_POWER_PIN);
            //if (EXTERNAL_PULLUP != 0)
            //{
            //    nrf_gpio_pin_set(EXTERNAL_PULLUP);
            //    SEGGER_RTT_WriteString(0, "External Pullup Enabled.\n");
            //}

            //nrf_drv_twi_enable(&m_twi_external); //enable the bus
            //SEGGER_RTT_WriteString(0, "External TWI Bus Enabled.\n");
        }
    }
}

void disable_twi_bus(int instance_id)
{
    //this method disables a currently enabled twi bus, as well as shuts
    //off power to any gpio pins that it requires.
    if (instance_id == INTERNAL_TWI_INSTANCE_ID)
    {
        nrf_gpio_pin_clear(INTERNAL_SENSOR_POWER_PIN);
        nrf_gpio_pin_clear(INTERNAL_PULLUP);

        nrf_drv_twi_disable(&m_twi_internal); //disable the current bus
    }
    else
    {
        nrf_gpio_pin_clear(EXTERNAL_SENSOR_POWER_PIN);
        nrf_gpio_pin_clear(EXTERNAL_PULLUP);

        nrf_drv_twi_disable(&m_twi_external); //disable the current bus
    }
}

const nrf_drv_twi_t* get_internal_twi_bus()
{
    return &m_twi_internal;
}

const nrf_drv_twi_t* get_external_twi_bus()
{
    return &m_twi_external;
}

int get_internal_twi_bus_id()
{
    return INTERNAL_TWI_INSTANCE_ID;
}

int get_external_twi_bus_id()
{
    return EXTERNAL_TWI_INSTANCE_ID;
}

void twi_address_scan(uint8_t* addresses, uint8_t* device_count, nrf_drv_twi_t const * bus)
{
    //This method scans for all possible TWI addresses on the given bus. If an
    //address is found it gets added to the given array of addresses and the count
    //of devices gets incremented.
    uint8_t sample_data;

    //We expect to get failures on most of our address search attempts. Because of this
    //we surpress logging of address NACKs (of which there will be a lot), but enable
    //successful TWI events so we can see what addresses lead to a hit.
    m_display_twi_events = false;
    SEGGER_RTT_WriteString(0, "Initiating Sensor Scan on TWI buses.\n");

    for (uint8_t add = 0; add <= 127; add++)
    {
        ret_code_t err_code;
        
        //set the m_xfer_done bool to false given the current TWI bus
        uint8_t instance = ((nrf_drv_twi_t const*)bus)->inst_idx;
        volatile bool * m_xfer_done;
        if (instance == INTERNAL_TWI_INSTANCE_ID) m_xfer_done = &m_xfer_internal_done; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)
        else m_xfer_done = &m_xfer_external_done;

        *m_xfer_done = false;

        do
        {
            err_code = nrf_drv_twi_rx(bus, add, &sample_data, sizeof(sample_data));
        } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
        while (*m_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

        //If a sensor was found so we add it to the list, get the bus status from the 
        //current twi bus and add the current address if we get an ACK
        int* m_twi_bus_status;
        if (instance == INTERNAL_TWI_INSTANCE_ID) m_twi_bus_status = &m_twi_internal_bus_status;
        else m_twi_bus_status = &m_twi_external_bus_status;

        if (*m_twi_bus_status == NRF_DRV_TWI_EVT_DONE)
        {
            //We've found a TWI address, before adding it to the array though make sure that
            //it's an IMU sensor. The BLE 33 Sense comes with some other sensors on board
            //which we (for now at least) don't care about.
            bool add_to_list = false;
            for (int i = ACC_SENSOR; i <= MAG_SENSOR; i++)
            {
                int stop;
                if (i == ACC_SENSOR) stop = ACC_MODEL_END;
                else if (i == GYR_SENSOR) stop = GYR_MODEL_END;
                else stop = MAG_MODEL_END;

                //iterate through all sensor models of type i
                for (int j = 0; j < stop; j++)
                {
                    uint8_t valid_sensor_address_l = get_sensor_low_address(i, j);
                    uint8_t valid_sensor_address_h = get_sensor_high_address(i, j);

                    if (add == valid_sensor_address_l || add == valid_sensor_address_h)
                    {
                        addresses[(*device_count)++] = add;
                        add_to_list = true;
                        break;
                    }
                }

                if (add_to_list) break;
            }
        }
    }

    m_display_twi_events = true; //flip this boolean back to true so we can see future error messages
    SEGGER_RTT_WriteString(0, "\n");
}

int32_t sensor_read_register(void *bus, uint8_t add, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    //This method allows for the reading of a sensor register(s). In most cases only a single register 
    //will be read, however, some sensors have a register auto-increment method so if the input read 
    //length is greater than 1 multiple registers can be read with a single command.
    ret_code_t err_code = 0;

    const nrf_drv_twi_xfer_desc_t sensor_read = {
        .address = add,
        .primary_length = 1,
        .secondary_length = (uint8_t)len,
        .p_primary_buf = &reg,
        .p_secondary_buf = bufp,
        .type =  NRF_DRV_TWI_XFER_TXRX};

    //set the m_xfer_done bool to false given the current TWI bus
    uint8_t instance = ((nrf_drv_twi_t const*)bus)->inst_idx;
    volatile bool * m_xfer_done;
    if (instance == INTERNAL_TWI_INSTANCE_ID) m_xfer_done = &m_xfer_internal_done; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)
    else m_xfer_done = &m_xfer_external_done;

    *m_xfer_done = false;

    do
    {
        err_code = nrf_drv_twi_xfer((nrf_drv_twi_t const*)bus, &sensor_read, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (*m_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}
int32_t sensor_write_register(void *bus, uint8_t add, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    //This method is for writing data to a single register for a sensor
    ret_code_t err_code = 0;

    uint8_t* register_and_data = (uint8_t*)malloc(len * sizeof(uint8_t));
    register_and_data[0] = reg;
    for (int i = 0; i < len; i++) register_and_data[i + 1] = bufp[i];

    //the primary buffer will hold the register address, as well as the value to write into it.
    //for a single write operation, the secondary buffer has no use in a single write command
    const nrf_drv_twi_xfer_desc_t sensor_write = {
        .address = add,
        .primary_length = 1 + len,
        .secondary_length = 0,
        .p_primary_buf = register_and_data,
        .p_secondary_buf = NULL,
        .type =  NRF_DRV_TWI_XFER_TX};

    //set the m_xfer_done bool to false given the current TWI bus
    uint8_t instance = ((nrf_drv_twi_t const*)bus)->inst_idx;
    volatile bool * m_xfer_done;
    if (instance == INTERNAL_TWI_INSTANCE_ID) m_xfer_done = &m_xfer_internal_done; //set to false before any read or write operations (this gets set to true in twi_handler when the transfer is complete)
    else m_xfer_done = &m_xfer_external_done;

    *m_xfer_done = false;

    do
    {
        err_code = nrf_drv_twi_xfer((nrf_drv_twi_t const*)bus, &sensor_write, 0); //no flags needed here
    } while (err_code == 0x11); //if the nrf is currently busy doing something else this line will wait until its done before executing
    while (*m_xfer_done == false); //this line forces the program to wait for the TWI transfer to complete before moving on

    free(register_and_data); //release memory after write operation is complete

    APP_ERROR_CHECK(err_code);
    return (int32_t)err_code;
}