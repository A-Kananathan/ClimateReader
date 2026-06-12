#include "bme680_driver.h"
#include "esp_log.h"

static const char *TAG = "BME680";

esp_err_t bme680_init(
    bme680_t *sensor,
    i2c_master_bus_handle_t bus_handle,
    uint8_t address
){
    sensor->address = address;

    i2c_device_config_t dev_config = {0};

    dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_config.device_address = address;
    dev_config.scl_speed_hz = 100000;

    return i2c_master_bus_add_device(
        bus_handle,
        &dev_config,
        &sensor->dev_handle
    );
}

esp_err_t bme680_configure(
    bme680_t *sensor
){
    ESP_ERROR_CHECK(bme680_write_register(sensor, BME680_REG_CRTL_HUM, 0x01));
    ESP_ERROR_CHECK(bme680_write_register(sensor, BME680_REG_CONFIG, 0x00));

    return ESP_OK;
}

esp_err_t bme680_read_register(
    bme680_t *sensor,
    uint8_t reg,
    uint8_t *buffer,
    size_t len
){
    return i2c_master_transmit_receive(
        sensor->dev_handle,
        &reg,
        1,
        buffer,
        len,
        -1
    );
}

esp_err_t bme680_write_register(
    bme680_t *sensor,
    uint8_t reg,
    uint8_t value
){
    uint8_t data[2] = {reg, value};
    
    return i2c_master_transmit(
        sensor->dev_handle,
        data,
        2,
        -1
    );
}

esp_err_t bme680_read_chip_id(
    bme680_t *sensor,
    uint8_t *chip_id
){
    esp_err_t err = bme680_read_register(sensor, BME680_CHIP_ID_REG, chip_id, 1);

    if(err != ESP_OK){
        return err;
    }

    ESP_LOGI(TAG, "BME680 Chip ID: 0x%02X", *chip_id);

    if(*chip_id == BME680_EXPECTED_CHIP_ID){
        ESP_LOGI(TAG, "BME680 erkannt!");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Unerwartete CHIP ID!");

    return ESP_FAIL;
}

esp_err_t bme680_soft_reset(
    bme680_t *sensor
){
    return bme680_write_register(
        sensor,
        BME680_RESET_REG,
        BME680_RESET_CMD
    );
}

esp_err_t bme680_load_calibration(
    bme680_t *sensor
){
    uint8_t calib1[25];
    uint8_t calib2[16];

    ESP_ERROR_CHECK(bme680_read_register(sensor, 0x89, calib1, sizeof(calib1)));
    ESP_ERROR_CHECK(bme680_read_register(sensor, 0xE1, calib2, sizeof(calib2)));

    sensor->par_t1 = (uint16_t)(((uint16_t)calib2[9] << 8) | calib2[8]);
    sensor->par_t2 = (int16_t)(((uint16_t)calib1[2] << 8) | calib1[1]);
    sensor->par_t3 = (int8_t)calib1[3];

    sensor->par_p1  = (uint16_t)(((uint16_t)calib1[6] << 8) | calib1[5]);
    sensor->par_p2  = (int16_t)(((uint16_t)calib1[8] << 8) | calib1[7]);
    sensor->par_p3  = (int8_t)calib1[9];
    sensor->par_p4  = (int16_t)(((uint16_t)calib1[12] << 8) | calib1[11]);
    sensor->par_p5  = (int16_t)(((uint16_t)calib1[14] << 8) | calib1[13]);
    sensor->par_p7  = (int8_t)calib1[15];
    sensor->par_p6  = (int8_t)calib1[16];
    sensor->par_p8  = (int16_t)(((uint16_t)calib1[20] << 8) | calib1[19]);
    sensor->par_p9  = (int16_t)(((uint16_t)calib1[22] << 8) | calib1[21]);
    sensor->par_p10 = calib1[23];

    sensor->par_h1 = (uint16_t)(((uint16_t)calib2[2] << 4) | (calib2[1] & 0x0F));
    sensor->par_h2 = (uint16_t)(((uint16_t)calib2[0] << 4) | (calib2[1] >> 4));
    sensor->par_h3 = (int8_t)calib2[3];
    sensor->par_h4 = (int8_t)calib2[4];
    sensor->par_h5 = (int8_t)calib2[5];
    sensor->par_h6 = calib2[6];
    sensor->par_h7 = (int8_t)calib2[7];

    sensor->par_g1 = (int8_t)calib2[12];
    sensor->par_g2 = (int16_t)(((uint16_t)calib2[11] << 8) | calib2[10]);
    sensor->par_g3 = (int8_t)calib2[13];


    /*ESP_LOGI(TAG, "par_t1  = %u", sensor->par_t1);
    ESP_LOGI(TAG, "par_t2  = %d", sensor->par_t2);
    ESP_LOGI(TAG, "par_t3  = %d", sensor->par_t3);

    ESP_LOGI(TAG, "par_p1  = %u", sensor->par_p1);
    ESP_LOGI(TAG, "par_p2  = %d", sensor->par_p2);
    ESP_LOGI(TAG, "par_p3  = %d", sensor->par_p3);
    ESP_LOGI(TAG, "par_p4  = %d", sensor->par_p4);
    ESP_LOGI(TAG, "par_p5  = %d", sensor->par_p5);
    ESP_LOGI(TAG, "par_p6  = %d", sensor->par_p6);
    ESP_LOGI(TAG, "par_p7  = %d", sensor->par_p7);
    ESP_LOGI(TAG, "par_p8  = %d", sensor->par_p8);
    ESP_LOGI(TAG, "par_p9  = %d", sensor->par_p9);
    ESP_LOGI(TAG, "par_p10 = %u", sensor->par_p10);

    ESP_LOGI(TAG, "par_h1  = %u", sensor->par_h1);
    ESP_LOGI(TAG, "par_h2  = %u", sensor->par_h2);
    ESP_LOGI(TAG, "par_h3  = %d", sensor->par_h3);
    ESP_LOGI(TAG, "par_h4  = %d", sensor->par_h4);
    ESP_LOGI(TAG, "par_h5  = %d", sensor->par_h5);
    ESP_LOGI(TAG, "par_h6  = %u", sensor->par_h6);
    ESP_LOGI(TAG, "par_h7  = %d", sensor->par_h7);

    ESP_LOGI(TAG, "par_g1  = %d", sensor->par_g1);
    ESP_LOGI(TAG, "par_g2  = %d", sensor->par_g2);
    ESP_LOGI(TAG, "par_g3  = %d", sensor->par_g3);*/


    return ESP_OK;
}

esp_err_t bme680_start_measurement(
    bme680_t *sensor
){
    return bme680_write_register(sensor, BME680_REG_CRTL_MEAS, 0x25);
}

esp_err_t bme680_read_raw_data(
    bme680_t *sensor,
    bme680_raw_data_t *raw_data
){
    uint8_t data[8];
    ESP_ERROR_CHECK(bme680_read_register(sensor, BME680_REG_FIELD0, data, sizeof(data)));

    raw_data->raw_press = (uint32_t)(((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | (data[2] >> 4));
    raw_data->raw_temp  = (uint32_t)(((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | (data[5] >> 4));
    raw_data->raw_hum   = (uint16_t)(((uint16_t)data[6] << 8) | data[7]);
    
    return ESP_OK;
}

int32_t bme680_calculate_t_fine(
    bme680_t *sensor,
    uint32_t raw_temp
){
    int32_t var1, var2, var3;
    var1 = ((int32_t)raw_temp >> 3) - ((int32_t)sensor->par_t1 << 1);
    var2 = (var1 * (int32_t)sensor->par_t2) >> 11;
    var3 = (((var1 >> 1) * (var1 >> 1)) >> 12) * ((int32_t)sensor->par_t3 << 4) >> 14;

    return var2 + var3;
}

float bme680_compensate_temperature(
    bme680_t *sensor,
    uint32_t raw_temp
){
    sensor->t_fine = bme680_calculate_t_fine(sensor, raw_temp);
    int32_t temp = (sensor->t_fine * 5 + 128) >> 8;
    return temp / 100.0f;
}

float bme680_compensate_pressure(
    bme680_t *sensor,
    uint32_t raw_press
){
    int32_t var1, var2, var3, press_comp;
    var1 = ((int32_t)sensor->t_fine >> 1) - 64000;
    var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)sensor->par_p6) >> 2;
    var2 += ((var1 * (int32_t)sensor->par_p5) << 1);
    var2 = (var2 >> 2) + ((int32_t)sensor->par_p4 << 16);
    var1 = ((((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t)sensor->par_p3 << 5)) >> 3) + (((int32_t)sensor->par_p2 * var1) >> 1)) >> 18;
    var1 = ((32768 + var1) * (int32_t)sensor->par_p1) >> 15;
    press_comp = 1048576 - raw_press;
    press_comp = (uint32_t)((press_comp - (var2 >> 12)) * ((uint32_t)3125));
    if(press_comp >= (1 << 30)){
        press_comp = ((press_comp / (uint32_t)var1) << 1);
    } else {
        press_comp = ((press_comp << 1) / (uint32_t)var1);
    }
    var1 = ((int32_t)sensor->par_p9 * (int32_t)(((press_comp >> 3) * (press_comp >> 3)) >> 13)) >> 12;
    var2 = ((int32_t)press_comp >> 2) * (int32_t)sensor->par_p8 >> 13;
    var3 = ((int32_t)(press_comp >> 8) * (int32_t)(press_comp >> 8) * (int32_t)press_comp >> 8) * (int32_t)sensor->par_p10 >>17;
    press_comp = (int32_t)(press_comp) + ((var1 + var2 + var3 + ((int32_t)sensor->par_p7 << 7)) >> 4);
    return press_comp / 100.0f;
}

float bme680_compensate_humidity(
    bme680_t *sensor,
    uint16_t raw_hum,
    float temp_comp
){
    int32_t var1, var2, var3, var4, var5, var6, temp_scaled;

    temp_scaled = (int32_t)(temp_comp * 100);
    var1 = (int32_t)raw_hum - (int32_t)((int32_t)sensor->par_h1 << 4) - (((temp_scaled * (int32_t)sensor->par_h3) / ((int32_t)100)) >> 1);
    var2 = ((int32_t)sensor->par_h2 * (((temp_scaled*(int32_t)sensor->par_h4) / ((int32_t)100)) + (((temp_scaled * ((temp_scaled * (int32_t)sensor->par_h5) / ((int32_t)100))) >> 6) / ((int32_t)100)) + ((int32_t)(1 << 14)))) >> 10;
    var3 = var1 * var2;
    var4 = (((int32_t)sensor->par_h6 << 7) + ((temp_scaled * (int32_t)sensor->par_h7) / ((int32_t)100))) >> 4;
    var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;
    var6 = (var4 * var5) >> 1;

    return ((((var3 + var6) >> 10) * ((int32_t)1000)) >> 12) / 1000.0f;
}