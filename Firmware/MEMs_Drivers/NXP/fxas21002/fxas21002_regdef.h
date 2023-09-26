#ifndef FXAS21002_REGDEF_H_
#define FXAS21002_REGDEF_H_

#include <stdint.h>

#ifdef __BYTE_ORDER__
#define DRV_LITTLE_ENDIAN 1234
#define DRV_BIG_ENDIAN    4321
#endif
#if __BYTE_ORDER__ == DRV_LITTLE_ENDIAN
#define DRV_BYTE_ORDER    DRV_LITTLE_ENDIAN
#else
#define DRV_BYTE_ORDER    DRV_BIG_ENDIAN
#endif

typedef struct
    {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
        uint8_t bit0 : 1;
        uint8_t bit1 : 1;
        uint8_t bit2 : 1;
        uint8_t bit3 : 1;
        uint8_t bit4 : 1;
        uint8_t bit5 : 1;
        uint8_t bit6 : 1;
        uint8_t bit7 : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
        uint8_t bit7 : 1;
        uint8_t bit6 : 1;
        uint8_t bit5 : 1;
        uint8_t bit4 : 1;
        uint8_t bit3 : 1;
        uint8_t bit2 : 1;
        uint8_t bit1 : 1;
        uint8_t bit0 : 1;
#endif /* DRV_BYTE_ORDER */
    } fxas21002_bitwise_t;

#define FXAS21002_REG_STATUS		0x00
#define FXAS21002_REG_OUTXMSB		0x01
#define FXAS21002_REG_INT_SOURCE	0x0b
#define FXAS21002_REG_WHOAMI		0x0c

#define FXAS21002_REG_CTRL_REG0		0x0d
typedef struct
    {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
        uint8_t fs : 2;
        uint8_t hpf_en : 1;
        uint8_t sel : 2;
        uint8_t spiw : 1;
        uint8_t bw : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
        uint8_t bw : 2;
        uint8_t spiw : 1;
        uint8_t sel : 2;
        uint8_t hpf_en : 1;
        uint8_t fs : 2;
#endif /* DRV_BYTE_ORDER */
    } fxas21002_ctrl_reg0_t;

#define FXAS21002_REG_CTRL_REG1		0x13
typedef struct
    {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
        uint8_t ready : 1;
        uint8_t active : 1;
        uint8_t dr : 3;
        uint8_t st : 1;
        uint8_t rst : 1;
        uint8_t rsvd : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
        uint8_t rsvd : 1;
        uint8_t rst : 1;
        uint8_t st : 1;
        uint8_t dr : 3;
        uint8_t active : 1;
        uint8_t ready : 1;
#endif /* DRV_BYTE_ORDER */
    } fxas21002_ctrl_reg1_t;

#define FXAS21002_REG_CTRL_REG2		0x14
typedef struct
    {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
        uint8_t pp_od : 1;
        uint8_t ipol : 1;
        uint8_t int_en_drdy : 1;
        uint8_t int_cfg_drdy : 1;
        uint8_t int_en_rt : 1;
        uint8_t int_cfg_rt : 1;
        uint8_t int_en_fifo : 1;
        uint8_t int_cfg_fifo : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
        uint8_t int_cfg_fifo : 1;
        uint8_t int_en_fifo : 1;
        uint8_t int_cfg_rt : 1;
        uint8_t int_en_rt : 1;
        uint8_t int_cfg_drdy : 1;
        uint8_t int_en_drdy : 1;
        uint8_t ipol : 1;
        uint8_t pp_od : 1;
#endif /* DRV_BYTE_ORDER */
    } fxas21002_ctrl_reg2_t;

#define FXAS21002_REG_CTRL_REG3		0x15
typedef struct
    {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
        uint8_t fs_double : 1;
        uint8_t rsvd_1 : 1;
        uint8_t extctrlen : 1;
        uint8_t wraptoone : 1;
        uint8_t rsvd_2 : 4;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
        uint8_t rsvd_2 : 4;
        uint8_t wraptoone : 1;
        uint8_t extctrlen : 1;
        uint8_t rsvd_1 : 1;
        uint8_t fs_double : 1;
#endif /* DRV_BYTE_ORDER */
    } fxas21002_ctrl_reg3_t;

#define FXAS21002_INT_SOURCE_DRDY_MASK	(1 << 0)

#define FXAS21002_CTRLREG0_FS_MASK	(3 << 0)

#define FXAS21002_CTRLREG1_DR_SHIFT	2

#define FXAS21002_CTRLREG1_POWER_MASK	(3 << 0)
#define FXAS21002_CTRLREG1_DR_MASK	(7 << FXAS21002_CTRLREG1_DR_SHIFT)
#define FXAS21002_CTRLREG1_RST_MASK	(1 << 6)

#define FXAS21002_CTRLREG2_CFG_EN_MASK		(1 << 2)
#define FXAS21002_CTRLREG2_CFG_DRDY_MASK	(1 << 3)

#define FXAS21002_MAX_NUM_CHANNELS	3

#define FXAS21002_BYTES_PER_CHANNEL	2

#define FXAS21002_MAX_NUM_BYTES		(FXAS21002_BYTES_PER_CHANNEL * \
					 FXAS21002_MAX_NUM_CHANNELS)

#define FXAS21002_DEVICE_ADDR_SA_0           0x20
#define FXAS21002_DEVICE_ADDR_SA_1           0x21

#define FXAS21002_WHO_AM_I                   0xD7

typedef enum fxas21002_odr {
	FXAS21002_ODR_800_HZ		= 0b000,
	FXAS21002_ODR_400_HZ		= 0b001,
        FXAS21002_ODR_200_HZ		= 0b010,
        FXAS21002_ODR_100_HZ		= 0b011,
        FXAS21002_ODR_50_HZ		= 0b100,
        FXAS21002_ODR_25_HZ		= 0b101,
        FXAS21002_ODR_12_5_HZ		= 0b110,
} fxas21002_odr_t;

typedef enum {
	FXAS21002_POWER_STANDBY		= 0b00,
	FXAS21002_POWER_READY           = 0b01,
	FXAS21002_POWER_ACTIVE          = 0b11,
} fxas21002_power_t;

typedef enum {
	FXAS21002_RANGE_2000DPS		= 0b00,
	FXAS21002_RANGE_1000DPS		= 0b01,
	FXAS21002_RANGE_500DPS		= 0b10,
	FXAS21002_RANGE_250DPS		= 0b11,
        FXAS21002_RANGE_4000DPS		= 0b10000,
} fxas21002_fs_range_t;

typedef enum {
	FXAS21002_FILTER_LPF		= 0b00,
	FXAS21002_FILTER_LPF_HPF	= 0b01,
} fxas21002_filter_select_t;

typedef enum {
	FXAS21002_LPF_STRONG		= 0b00,
	FXAS21002_LPF_MEDIUM	        = 0b01,
        FXAS21002_LPF_LIGHT	        = 0b10,
} fxas21002_lpf_bandwidth_t;

typedef enum {
	FXAS21002_HPF_EXTREME		= 0b00,
        FXAS21002_HPF_STRONG		= 0b01,
	FXAS21002_HPF_MEDIUM	        = 0b10,
        FXAS21002_HPF_LIGHT	        = 0b11,
} fxas21002_hpf_bandwidth_t;

#endif /* FXAS21002_REGDEF_H_ */
