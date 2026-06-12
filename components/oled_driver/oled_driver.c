#include "oled_driver.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

static i2c_master_dev_handle_t oled_handle = NULL;

static esp_err_t oled_cmd(uint8_t cmd){
    uint8_t data[] = {0x00, cmd};
    return i2c_master_transmit(oled_handle, data, sizeof(data), pdMS_TO_TICKS(1000));
}

static esp_err_t oled_data(const uint8_t *data, size_t len){
    uint8_t buffer[129];
    buffer[0] = 0x40;

    while (len > 0){
        size_t chunk = len > 128 ? 128 : len;
        memcpy(&buffer[1], data, chunk);

        esp_err_t ret = i2c_master_transmit(oled_handle, buffer, chunk + 1, pdMS_TO_TICKS(1000));
        if (ret != ESP_OK) return ret;

        data += chunk;
        len -= chunk;
    }

    return ESP_OK;
}

esp_err_t oled_init(i2c_master_bus_handle_t bus_handle){
    #define OLED_CHECK(x) do{esp_err_t ret = (x); if (ret != ESP_OK)return ret; } while(0)

    i2c_device_config_t oled_config = {0};
    oled_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    oled_config.device_address = OLED_ADDR;
    oled_config.scl_speed_hz = 100000;

    //ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &oled_config, &oled_handle));
    ESP_LOGI("OLED", "Before add device");
    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &oled_config, &oled_handle);
    if (ret != ESP_OK) return ret;

    OLED_CHECK(oled_cmd(0xAE));
    OLED_CHECK(oled_cmd(0xA8));
    OLED_CHECK(oled_cmd(0x7F));
    OLED_CHECK(oled_cmd(0xD3));
    OLED_CHECK(oled_cmd(0x00));
    OLED_CHECK(oled_cmd(0x40));
    OLED_CHECK(oled_cmd(0xA0));
    OLED_CHECK(oled_cmd(0xC0));
    OLED_CHECK(oled_cmd(0xDA));
    OLED_CHECK(oled_cmd(0x12));
    OLED_CHECK(oled_cmd(0x81));
    OLED_CHECK(oled_cmd(0x7F));
    OLED_CHECK(oled_cmd(0xA4));
    OLED_CHECK(oled_cmd(0xA6));
    OLED_CHECK(oled_cmd(0xAF));

    return ESP_OK;
}

esp_err_t oled_clear(void){
    uint8_t empty[128] = {0};

    for(uint8_t page = 0; page < 16; page++){
        esp_err_t ret = oled_cmd(0xB0 | page);
        if (ret != ESP_OK) return ret;

        ret = oled_cmd(0x00);
        if (ret != ESP_OK) return ret;

        ret = oled_cmd(0x12);
        if (ret != ESP_OK) return ret;

        ret = oled_data(empty, 128);
        if (ret != ESP_OK) {
            ESP_LOGE("OLED", "oled_data failed: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    return ESP_OK;
}

esp_err_t oled_fill(void){
    uint8_t full[128];
    memset(full, 0xFF, sizeof(full));

    for (uint8_t page = 0; page < 16; page++){
        oled_cmd(0xB0 | page);
        oled_cmd(0x00);
        oled_cmd(0x12);
        ESP_ERROR_CHECK(oled_data(full, 128));
    }

    return ESP_OK;
}