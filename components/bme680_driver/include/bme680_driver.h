#ifndef BME680_DRIVER_H
#define BME680_DRIVER_H

#include <stdint.h>
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BME680_CHIP_ID_REG 0xD0
#define BME680_EXPECTED_CHIP_ID 0x61
#define BME680_RESET_REG 0xE0
#define BME680_RESET_CMD 0xB6

#define BME680_REG_CRTL_HUM 0x72
#define BME680_REG_CRTL_MEAS 0x74
#define BME680_REG_CONFIG 0x75
#define BME680_REG_FIELD0 0x1F

typedef struct{
    i2c_master_dev_handle_t dev_handle;
    uint8_t address;

    uint16_t par_t1;
    int16_t par_t2;
    int8_t par_t3;

    uint16_t par_p1;
    int16_t par_p2;
    int8_t par_p3;
    int16_t par_p4;
    int16_t par_p5;
    int8_t par_p6;
    int8_t par_p7;
    int16_t par_p8;
    int16_t par_p9;
    uint8_t par_p10;

    uint16_t par_h1;
    uint16_t par_h2;
    int8_t par_h3;
    int8_t par_h4;
    int8_t par_h5;
    uint8_t par_h6;
    int8_t par_h7;

    int8_t par_g1;
    int16_t par_g2;
    int8_t par_g3;

    int32_t t_fine;
} bme680_t;

typedef struct{
    uint32_t raw_press;
    uint32_t raw_temp;
    uint16_t raw_hum;
} bme680_raw_data_t;

esp_err_t bme680_init(
    bme680_t *sensor,
    i2c_master_bus_handle_t bus_handle,
    uint8_t address
);

esp_err_t bme680_configure(
    bme680_t *sensor
);

esp_err_t bme680_read_register(
    bme680_t *sensor,
    uint8_t reg,
    uint8_t *buffer,
    size_t len
);

esp_err_t bme680_write_register(
    bme680_t *sensor,
    uint8_t reg,
    uint8_t value
);

esp_err_t bme680_read_chip_id(
    bme680_t *sensor,
    uint8_t *chip_id
);

esp_err_t bme680_soft_reset(
    bme680_t *sensor
);

esp_err_t bme680_load_calibration(
    bme680_t *sensor
);

esp_err_t bme680_start_measurement(
    bme680_t *sensor
);

esp_err_t bme680_read_raw_data(
    bme680_t *sensor,
    bme680_raw_data_t *raw_data
);

int32_t bme680_calculate_t_fine(
    bme680_t *sensor,
    uint32_t raw_temp
);

float bme680_compensate_temperature(
    bme680_t *sensor,
    uint32_t raw_temp
);

float bme680_compensate_pressure(
    bme680_t *sensor,
    uint32_t raw_press
);

float bme680_compensate_humidity(
    bme680_t *sensor,
    uint16_t raw_hum,
    float temp_comp
);

#ifdef __cplusplus
}
#endif

#endif