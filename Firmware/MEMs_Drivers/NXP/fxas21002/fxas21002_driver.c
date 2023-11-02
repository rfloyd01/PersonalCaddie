#include "fxas21002_driver.h"
#include "SEGGER_RTT.h"

int32_t fxas21002_data_rate_set(sensor_communication_t* gyr_comm, fxas21002_odr_t val)
{
    uint8_t p_reg1; //read register directly into uint8_t type
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG1,
        &p_reg1, 1);

    //convert the integers to register structs
    fxas21002_ctrl_reg1_t ctrl_reg1 = *((fxas21002_ctrl_reg1_t*)&p_reg1);

    //Before changing the ODR we need to make sure that the sensor is in
    //standby or ready mode
    if (ret == 0 && ctrl_reg1.active == 1 && ctrl_reg1.ready == 1)
    {
        ctrl_reg1.active = 0;
        ret = gyr_comm->write_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG1,
            (uint8_t*)&ctrl_reg1);
    }

    if (ret == 0)
    {
        ctrl_reg1.dr = ((uint8_t)val & 0x07);
        ret = gyr_comm->write_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG1,
            (uint8_t*)&ctrl_reg1);
    }

    return ret;
}

int32_t fxas21002_data_rate_get(sensor_communication_t* gyr_comm, fxas21002_odr_t* val)
{
    uint8_t p_reg1; //read register directly into uint8_t type
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG1,
        &p_reg1, 1);

    //convert the integers to register structs
    fxas21002_ctrl_reg1_t ctrl_reg1 = *((fxas21002_ctrl_reg1_t*)&p_reg1);

    switch (ctrl_reg1.dr)
    {
    case FXAS21002_ODR_800_HZ:
        *val = FXAS21002_ODR_800_HZ;
        break;
    case FXAS21002_ODR_400_HZ:
        *val = FXAS21002_ODR_400_HZ;
        break;
    case FXAS21002_ODR_200_HZ:
        *val = FXAS21002_ODR_200_HZ;
        break;
    case FXAS21002_ODR_100_HZ:
        *val = FXAS21002_ODR_100_HZ;
        break;
    case FXAS21002_ODR_50_HZ:
        *val = FXAS21002_ODR_50_HZ;
        break;
    case FXAS21002_ODR_25_HZ:
        *val = FXAS21002_ODR_25_HZ;
        break;
    case FXAS21002_ODR_12_5_HZ:
        *val = FXAS21002_ODR_12_5_HZ;
        break;
    default:
        *val = FXAS21002_ODR_50_HZ;
        break;
    }

    return ret;
}

int32_t fxas21002_power_mode_set(sensor_communication_t* gyr_comm, fxas21002_power_t val)
{
    uint8_t p_reg1; //read register directly into uint8_t type
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG1,
        &p_reg1, 1);

    //convert the integers to register structs
    fxas21002_ctrl_reg1_t ctrl_reg1 = *((fxas21002_ctrl_reg1_t*)&p_reg1);

    if (ret == 0)
    {
        ctrl_reg1.ready = (uint8_t)val & 0b01;
        ctrl_reg1.active = (((uint8_t)val & 0b10) >> 1);

        ret = gyr_comm->write_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG1,
            (uint8_t*)&ctrl_reg1);
    }

    return ret;
}

int32_t fxas21002_power_mode_get(sensor_communication_t* gyr_comm, fxas21002_power_t* val)
{
    uint8_t p_reg1; //read register directly into uint8_t type
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG1,
        &p_reg1, 1);

    //convert the integers to register structs
    fxas21002_ctrl_reg1_t ctrl_reg1 = *((fxas21002_ctrl_reg1_t*)&p_reg1);

    uint8_t pow = (0b11 & (((uint8_t)ctrl_reg1.active << 1) | (uint8_t)ctrl_reg1.ready));
    switch (pow)
    {
        case 0b00:
        default:
            *val = FXAS21002_POWER_STANDBY;
            break;
        case 0b01:
            *val = FXAS21002_POWER_READY;
            break;
        case 0b10:
        case 0b11:
            *val = FXAS21002_POWER_ACTIVE;
            break;
    }

    return ret;
}

int32_t fxas21002_full_scale_range_set(sensor_communication_t* gyr_comm, fxas21002_fs_range_t val)
{
    uint8_t p_reg0, p_reg3; //read registers directly into uint8_t types
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
        &p_reg0, 1);

    if (ret = 0)
    {
        ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG3,
        &p_reg3, 1);
    }

    //convert the integers to register structs
    fxas21002_ctrl_reg0_t ctrl_reg0 = *((fxas21002_ctrl_reg0_t*)&p_reg0);
    fxas21002_ctrl_reg3_t ctrl_reg3 = *((fxas21002_ctrl_reg3_t*)&p_reg3);

    if (ret == 0)
    {
        ctrl_reg0.fs = (uint8_t)val & 0b11;
        ret = gyr_comm->write_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
            (uint8_t*)&ctrl_reg0);
    }

    if (ret == 0)
    {
        ctrl_reg3.fs_double = (((uint8_t)val & 0x10) >> 1);
        ret = gyr_comm->write_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG3,
            (uint8_t*)&ctrl_reg3);
    }

    return ret;
}

int32_t fxas21002_full_scale_range_get(sensor_communication_t* gyr_comm, fxas21002_fs_range_t* val)
{
    uint8_t p_reg0, p_reg3; //read registers directly into uint8_t types
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
        &p_reg0, 1);

    if (ret = 0)
    {
        ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG3,
        &p_reg3, 1);
    }

    //convert the integers to register structs
    fxas21002_ctrl_reg0_t ctrl_reg0 = *((fxas21002_ctrl_reg0_t*)&p_reg0);
    fxas21002_ctrl_reg3_t ctrl_reg3 = *((fxas21002_ctrl_reg3_t*)&p_reg3);

    uint8_t fsr = (0x1F & ((uint8_t)ctrl_reg0.fs | ((uint8_t)ctrl_reg3.fs_double << 4)));
    switch (fsr)
    {
        case 0b00000:
            *val = FXAS21002_RANGE_2000DPS;
            break;
        case 0b00001:
            *val = FXAS21002_RANGE_1000DPS;
            break;
        case 0b00010:
            *val = FXAS21002_RANGE_500DPS;
            break;
        case 0b00011:
            *val = FXAS21002_RANGE_250DPS;
            break;
        case 0b10000:
            *val = FXAS21002_RANGE_4000DPS;
            break;
        case 0b10001:
            *val = FXAS21002_RANGE_2000DPS;
            break;
        case 0b10010:
            *val = FXAS21002_RANGE_1000DPS;
            break;
        case 0b10011:
            *val = FXAS21002_RANGE_500DPS;
            break;
    }

    return ret;
}

int32_t fxas21002_filter_out_set(sensor_communication_t* gyr_comm, fxas21002_filter_select_t val)
{
    uint8_t p_reg0; //read register directly into uint8_t type
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
        &p_reg0, 1);

    //convert the integers to register structs
    fxas21002_ctrl_reg0_t ctrl_reg0 = *((fxas21002_ctrl_reg0_t*)&p_reg0);

    if (ret == 0)
    {
        ctrl_reg0.hpf_en = (uint8_t)val & 0b01;
        ret = gyr_comm->write_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
            (uint8_t*)&ctrl_reg0);
    }

    return ret;
}

int32_t fxas21002_filter_out_get(sensor_communication_t* gyr_comm, fxas21002_filter_select_t* val)
{
    uint8_t p_reg0; //read register directly into uint8_t type
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
        &p_reg0, 1);

    //convert the integers to register structs
    fxas21002_ctrl_reg0_t ctrl_reg0 = *((fxas21002_ctrl_reg0_t*)&p_reg0);

    switch (ctrl_reg0.hpf_en)
    {
    case 0b00:
    default:
        *val = FXAS21002_FILTER_LPF;
        break;
    case 0b01:
        *val = FXAS21002_FILTER_LPF_HPF;
        break;
    }

    return ret;
}

int32_t fxas21002_lp_filter_bw_set(sensor_communication_t* gyr_comm, fxas21002_lpf_bandwidth_t val)
{
    uint8_t p_reg0; //read register directly into uint8_t type
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
        &p_reg0, 1);

    //convert the integers to register structs
    fxas21002_ctrl_reg0_t ctrl_reg0 = *((fxas21002_ctrl_reg0_t*)&p_reg0);

    if (ret == 0)
    {
        ctrl_reg0.bw = (uint8_t)val & 0b11;
        ret = gyr_comm->write_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
            (uint8_t*)&ctrl_reg0);
    }

    return ret;
}

int32_t fxas21002_lp_filter_bw_get(sensor_communication_t* gyr_comm, fxas21002_lpf_bandwidth_t* val)
{
    uint8_t p_reg0; //read register directly into uint8_t type
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
        &p_reg0, 1);

    //convert the integers to register structs
    fxas21002_ctrl_reg0_t ctrl_reg0 = *((fxas21002_ctrl_reg0_t*)&p_reg0);

    switch (ctrl_reg0.bw)
    {
    case 0b00:
    default:
        *val = FXAS21002_LPF_STRONG;
        break;
    case 0b01:
        *val = FXAS21002_LPF_MEDIUM;
        break;
    case 0b10:
        *val = FXAS21002_LPF_LIGHT;
        break;
    }

    return ret;
}

int32_t fxas21002_hp_filter_bw_set(sensor_communication_t* gyr_comm, fxas21002_hpf_bandwidth_t val)
{
    uint8_t p_reg0; //read register directly into uint8_t type
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
        &p_reg0, 1);

    //convert the integers to register structs
    fxas21002_ctrl_reg0_t ctrl_reg0 = *((fxas21002_ctrl_reg0_t*)&p_reg0);

    if (ret == 0)
    {
        ctrl_reg0.sel = (uint8_t)val & 0b11;
        ret = gyr_comm->write_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
            (uint8_t*)&ctrl_reg0);
    }

    return ret;
}

int32_t fxas21002_hp_filter_bw_get(sensor_communication_t* gyr_comm, fxas21002_hpf_bandwidth_t* val)
{
    uint8_t p_reg0; //read register directly into uint8_t type
    int32_t ret;

    ret = gyr_comm->read_register((void*)gyr_comm->twi_bus, gyr_comm->address, FXAS21002_REG_CTRL_REG0,
        &p_reg0, 1);

    //convert the integers to register structs
    fxas21002_ctrl_reg0_t ctrl_reg0 = *((fxas21002_ctrl_reg0_t*)&p_reg0);

    switch (ctrl_reg0.sel)
    {
    case 0b00:
    default:
        *val = FXAS21002_HPF_EXTREME;
        break;
    case 0b01:
        *val = FXAS21002_HPF_MEDIUM;
        break;
    case 0b10:
        *val = FXAS21002_HPF_LIGHT;
        break;
    case 0b11:
        *val = FXAS21002_HPF_LIGHT;
        break;
    }

    return ret;
}