#ifndef BMI270_DRV_H__
#define BMI270_DRV_H__

#include <stdint.h>
#include <stdbool.h>
#include "Bosch/bmi270.h"
#include "sensor_settings.h"
#include "sensor_communication.h"

#define BMI270_SUSPEND_TO_ACTIVE_DELAY_US 45000
#define BMI270_CONFIG_TO_ACTIVE_DELAY_US 2000

#ifdef __cplusplus
extern "C" {
#else

void bmi270init(imu_communication_t* comm, uint8_t sensors, uint8_t* settings);
#endif

int32_t bmi270_connected_mode_enable(bool init);
int32_t bmi270_idle_mode_enable(bool active);
int32_t bmi270_active_mode_enable(int current_mode);

void bmi270_get_actual_settings();

//Implementations of Driver Functional Pointers
int8_t bmi270_read_register(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t bmi270_write_register(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
void bmi270_delay(uint32_t period, void *intf_ptr);

int32_t bmi270_get_data(uint8_t* pBuff, uint8_t offset);
int32_t bmi270_get_dummy_data(uint8_t* pBuff, uint8_t offset);
int32_t bmi270_get_acc_data(uint8_t* pBuff, uint8_t offset);
int32_t bmi270_get_gyr_data(uint8_t* pBuff, uint8_t offset);

#ifdef __cplusplus
}
#endif

#endif // BMI270_DRV_H__

/** @} */