#include "mpu6050.h"


/* Defines ---------------------------*/
#define MPU6050_ADDR_AD0_LOW   0x68
#define MPU6050_ADDR_AD0_HIGH  0x69

// Register map
#define MPU6050_REG_PWR_MGMT_1     0x6B
#define MPU6050_REG_PWR_MGMT_2     0x6C
#define MPU6050_REG_SMPLRT_DIV     0x19
#define MPU6050_REG_CONFIG         0x1A
#define MPU6050_REG_GYRO_CONFIG    0x1B
#define MPU6050_REG_ACCEL_CONFIG   0x1C
#define MPU6050_REG_INT_PIN_CFG    0x37
#define MPU6050_REG_INT_ENABLE     0x38
#define MPU6050_REG_FIFO_EN        0x23
#define MPU6050_REG_FIFO_COUNTH    0x72
#define MPU6050_REG_FIFO_R_W       0x74
#define MPU6050_REG_USER_CTRL      0x6A

#define MPU6050_REG_ACCEL_XOUT_H   0x3B
#define MPU6050_REG_TEMP_OUT_H     0x41
#define MPU6050_REG_GYRO_XOUT_H    0x43
#define MPU6050_REG_WHO_AM_I       0x75

extern void delay_ms(uint16_t ms);

/* Private functions */
static bool mpu6050_write(mpu6050_t *dev, uint8_t reg, uint8_t val)
{
    return i2c_mem_write(dev->I2Cx, dev->address, reg, &val, 1);
}

static bool mpu6050_read(mpu6050_t *dev, uint8_t reg, uint8_t *val)
{
    return i2c_mem_read(dev->I2Cx, dev->address, reg, val, 1);
}

static bool mpu6050_read_multi(mpu6050_t *dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    return i2c_mem_read(dev->I2Cx, dev->address, reg, buf, len);
}




bool mpu6050_init(mpu6050_t *dev, I2C_TypeDef *I2Cx, mpu6050_devid_t id)
{
    dev->I2Cx = I2Cx;
    dev->address = (id == MPU6050_DEVICE_0) ? MPU6050_ADDR_AD0_LOW : MPU6050_ADDR_AD0_HIGH;

    // Power on delay
    delay_ms(1000);
    if (!mpu6050_probe(dev)) return false;

    // Reset y sacar de sleep
    mpu6050_reset(dev);
    delay_ms(1000);
    mpu6050_set_sleep(dev, false);


    mpu6050_set_clock_source(dev, MPU6050_CLOCK_PLL_ZGYRO);
    mpu6050_set_accel_range(dev, MPU6050_ACCEL_RANGE_2G);
    mpu6050_set_gyro_range(dev, MPU6050_GYRO_RANGE_250DPS);

    return true;
}

bool mpu6050_probe(mpu6050_t *dev)
{
    uint8_t id;
    if (!mpu6050_read(dev, MPU6050_REG_WHO_AM_I, &id)) return false;
    return (id == 0x68);
}


bool mpu6050_set_sleep(mpu6050_t *dev, bool enable)
{
    uint8_t reg;
    if (!mpu6050_read(dev, MPU6050_REG_PWR_MGMT_1, &reg)) return false;

    if (enable) reg |= (1 << 6);
    else        reg &= ~(1 << 6);

    return mpu6050_write(dev, MPU6050_REG_PWR_MGMT_1, reg);
}

bool mpu6050_reset(mpu6050_t *dev)
{
    return mpu6050_write(dev, MPU6050_REG_PWR_MGMT_1, (1 << 7));
}


bool mpu6050_set_axis_enable(mpu6050_t *dev,
                               mpu6050_axis_t src,
                               bool enabled)
{
    uint8_t reg;

    if (!mpu6050_read(dev, MPU6050_REG_PWR_MGMT_2, &reg)) return false;

    if (enabled)
        reg &= ~(1 << src);   // 0 = enabled
    else
        reg |=  (1 << src);   // 1 = disabled

    return mpu6050_write(dev, MPU6050_REG_PWR_MGMT_2, reg);
}


bool mpu6050_set_accel_range(mpu6050_t *dev, mpu6050_accel_range_t range)
{
    uint8_t reg;
    if (!mpu6050_read(dev, MPU6050_REG_ACCEL_CONFIG, &reg)) return false;

    reg &= ~(0x3 << 3);
    reg |= (range << 3);

    dev->accel_scale = (range == MPU6050_ACCEL_RANGE_2G) ? 16384.0f :
                       (range == MPU6050_ACCEL_RANGE_4G) ? 8192.0f :
                       (range == MPU6050_ACCEL_RANGE_8G) ? 4096.0f : 2048.0f;

    return mpu6050_write(dev, MPU6050_REG_ACCEL_CONFIG, reg);
}

bool mpu6050_set_gyro_range(mpu6050_t *dev, mpu6050_gyro_range_t range)
{
    uint8_t reg;
    if (!mpu6050_read(dev, MPU6050_REG_GYRO_CONFIG, &reg)) return false;

    reg &= ~(0x3 << 3);
    reg |= (range << 3);

    dev->gyro_scale = (range == MPU6050_GYRO_RANGE_250DPS) ? 131.0f :
                      (range == MPU6050_GYRO_RANGE_500DPS) ? 65.5f :
                      (range == MPU6050_GYRO_RANGE_1000DPS) ? 32.8f : 16.4f;

    return mpu6050_write(dev, MPU6050_REG_GYRO_CONFIG, reg);
}

bool mpu6050_set_clock_source(mpu6050_t *dev, mpu6050_clock_t clksrc)
{
    uint8_t reg;
    if (!mpu6050_read(dev, MPU6050_REG_PWR_MGMT_1, &reg)) return false;

    reg &= ~0x07;
    reg |= clksrc;

    return mpu6050_write(dev, MPU6050_REG_PWR_MGMT_1, reg);
}

bool mpu6050_set_odr(mpu6050_t *dev,
                             mpu6050_bandwidth_t bw,
                             uint8_t divider,
                             uint16_t *sr_out)
{
    if (!mpu6050_write(dev, MPU6050_REG_CONFIG, (uint8_t)bw)) return false;
    if (!mpu6050_write(dev, MPU6050_REG_SMPLRT_DIV, divider)) return false;


    if (sr_out)
	{
		uint16_t base = (bw == MPU6050_BANDWIDTH_0) ? 8000 : 1000;
		*sr_out = base / (1 + divider);
	}

    return true;
}


bool mpu6050_set_interrupt_enable(mpu6050_t *dev,
                                  mpu6050_irq_t irq,
                                  bool enable)
{
    uint8_t reg;
    if (!mpu6050_read(dev, MPU6050_REG_INT_ENABLE, &reg)) return false;

    if (enable) reg |= (1 << irq);
    else        reg &= ~(1 << irq);

    return mpu6050_write(dev, MPU6050_REG_INT_ENABLE, reg);
}

bool mpu6050_set_irq_clear_mode(mpu6050_t *dev, mpu6050_irq_clear_t mode)
{
	uint8_t reg;

	if (!i2c_mem_read(dev->I2Cx, dev->address, MPU6050_REG_INT_PIN_CFG, &reg, 1))
		return false;

	if (mode == MPU6050_IRQ_CLEAR_BY_READ)
		reg |= (1 << 4);   // INT_RD_CLEAR = 1
	else
		reg &= ~(1 << 4);  // INT_RD_CLEAR = 0

	return i2c_mem_write(dev->I2Cx, dev->address, MPU6050_REG_INT_PIN_CFG, &reg, 1);
}



bool mpu6050_set_fifo_source(mpu6050_t *dev,
                             mpu6050_fifo_src_t src,
                             bool enable)
{
    uint8_t reg;
    if (!mpu6050_read(dev, MPU6050_REG_FIFO_EN, &reg)) return false;

    if (enable) reg |= (1 << src);
    else        reg &= ~(1 << src);

    return mpu6050_write(dev, MPU6050_REG_FIFO_EN, reg);
}

bool mpu6050_fifo_enable(mpu6050_t *dev, bool enable)
{
    uint8_t reg;
    if (!mpu6050_read(dev, MPU6050_REG_USER_CTRL, &reg)) return false;

    if (enable) reg |= (1 << 6);
    else        reg &= ~(1 << 6);

    return mpu6050_write(dev, MPU6050_REG_USER_CTRL, reg);
}

bool mpu6050_fifo_get_count(mpu6050_t *dev, uint16_t *count)
{
    uint8_t fifo_cnt[2];

    if (!dev || !count)
        return false;

    if (!mpu6050_read_multi(dev, MPU6050_REG_FIFO_COUNTH, fifo_cnt, 2)) return false;


    *count = ((uint16_t)fifo_cnt[0] << 8) | fifo_cnt[1];

    return true;
}

bool mpu6050_fifo_read(mpu6050_t *dev, uint8_t *data, uint16_t len)
{
    if (!dev || !data || len == 0)
        return false;

    // Lectura secuencial desde FIFO_R_W
    return mpu6050_read_multi(dev, MPU6050_REG_FIFO_R_W, data, len);
}

bool mpu6050_fifo_reset(mpu6050_t *dev)
{
    uint8_t reg;
    if (!mpu6050_read(dev, MPU6050_REG_USER_CTRL, &reg)) return false;

    reg |= (1 << 2);
    return mpu6050_write(dev, MPU6050_REG_USER_CTRL, reg);
}


bool mpu6050_read_raw(mpu6050_t *dev)
{
    uint8_t buf[14];

    if (!mpu6050_read_multi(dev, MPU6050_REG_ACCEL_XOUT_H, buf, 14))
        return false;

    dev->data.accel.x = (buf[0] << 8) | buf[1];
    dev->data.accel.y = (buf[2] << 8) | buf[3];
    dev->data.accel.z = (buf[4] << 8) | buf[5];

    dev->data.temperature = (buf[6] << 8) | buf[7];

    dev->data.gyro.x = (buf[8] << 8) | buf[9];
    dev->data.gyro.y = (buf[10] << 8) | buf[11];
    dev->data.gyro.z = (buf[12] << 8) | buf[13];

    return true;
}

bool mpu6050_read_cal(mpu6050_t *dev, mpu6050_data_calib_t *data_cal)
{
	if(!mpu6050_read_raw(dev)) return false;

	// Do calibration on raw data
	data_cal->accel.x = (dev->data.accel.x - dev->calib.accel_bias.x) * dev->accel_scale;
	data_cal->accel.y = (dev->data.accel.y - dev->calib.accel_bias.y) * dev->accel_scale;
	data_cal->accel.z = (dev->data.accel.z - dev->calib.accel_bias.z) * dev->accel_scale;

	data_cal->gyro.x = (dev->data.gyro.x - dev->calib.gyro_bias.x) * dev->gyro_scale;
	data_cal->gyro.y = (dev->data.gyro.y - dev->calib.gyro_bias.y) * dev->gyro_scale;
	data_cal->gyro.z = (dev->data.gyro.z - dev->calib.gyro_bias.z) * dev->gyro_scale;

	return true;
}

bool mpu6050_calibrate(mpu6050_t *dev, uint16_t num_samples)
{
    if (num_samples == 0) return false;

    int32_t acc_sum_x = 0;
    int32_t acc_sum_y = 0;
    int32_t acc_sum_z = 0;

    int32_t gyro_sum_x = 0;
    int32_t gyro_sum_y = 0;
    int32_t gyro_sum_z = 0;

    for (uint16_t i = 0; i < num_samples; i++)
    {
        if (!mpu6050_read_raw(dev))
            return false;

        acc_sum_x += dev->data.accel.x;
        acc_sum_y += dev->data.accel.y;
        acc_sum_z += dev->data.accel.z;

        gyro_sum_x += dev->data.gyro.x;
        gyro_sum_y += dev->data.gyro.y;
        gyro_sum_z += dev->data.gyro.z;

        delay_ms(10);
    }

    // Promedios
    dev->calib.accel_bias.x = acc_sum_x / num_samples;
    dev->calib.accel_bias.y = acc_sum_y / num_samples;
    dev->calib.accel_bias.z = acc_sum_z / num_samples;

    dev->calib.gyro_bias.x = gyro_sum_x / num_samples;
    dev->calib.gyro_bias.y = gyro_sum_y / num_samples;
    dev->calib.gyro_bias.z = gyro_sum_z / num_samples;

    //  Corrección de gravedad
    dev->calib.accel_bias.z -= (int16_t)(dev->accel_scale);

    return true;
}
