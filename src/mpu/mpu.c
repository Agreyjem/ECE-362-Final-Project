#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd.h"
#include "hardware/spi.h"

// ***************************************

#define MPU6050_ADDR       0x68

#define PWR_MGMT_1         0x6B
#define GYRO_CONFIG        0x1B
#define GYRO_XOUT_H        0x43

#define I2C_PORT           i2c1
#define I2C_SDA            10
#define I2C_SCL            11

#define GYRO_SENSITIVITY   400.0f


void update_orientation(float * angle_x, float * angle_y, float * angle_z, absolute_time_t previous_time)
{
    float gx, gy, gz;

    mpu6050_read_gyro(&gx, &gy, &gz);

    absolute_time_t current_time = get_absolute_time();

    float dt = absolute_time_diff_us(
        previous_time, current_time
    ) / 1000000.0f;

    previous_time = current_time;

    *angle_x += gx * dt;
    *angle_y += gy * dt;
    *angle_z += gz * dt;
}

void mpu6050_init(void)
{

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    sleep_ms(100);

    uint8_t data[2];

    data[0] = PWR_MGMT_1;
    data[1] = 0x00;

    i2c_write_blocking(I2C_PORT, MPU6050_ADDR,
                       data, 2, false);

    sleep_ms(100);

    data[0] = GYRO_CONFIG;
    data[1] = 0x00;

    i2c_write_blocking(I2C_PORT, MPU6050_ADDR,
                       data, 2, false);
}


void mpu6050_read_gyro(float *gx, float *gy, float *gz)
{
    uint8_t reg = GYRO_XOUT_H;
    uint8_t buffer[6];

    // Set register address
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR,
                       &reg, 1, true);

    // Read six consecutive bytes
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR,
                      buffer, 6, false);

    // Combine high and low bytes
    int16_t raw_x = (int16_t)((buffer[0] << 8) | buffer[1]);
    int16_t raw_y = (int16_t)((buffer[2] << 8) | buffer[3]);
    int16_t raw_z = (int16_t)((buffer[4] << 8) | buffer[5]);

    // Convert raw measurements to degrees/second
    *gx = raw_x / GYRO_SENSITIVITY;
    *gy = raw_y / GYRO_SENSITIVITY;
    *gz = raw_z / GYRO_SENSITIVITY;
}


double mpu_getreading(void)
{
    float angle_x = 0.0f;
    float angle_y = 0.0f;
    float angle_z = 0.0f;

    absolute_time_t previous_time;

    stdio_init_all();
    mpu6050_init();
    sleep_ms(2000);
    previous_time = get_absolute_time;
    
    update_orientation(&angle_x, &angle_y, &angle_z, previous_time);
    sleep_ms(1000);

    return angle_y;

}