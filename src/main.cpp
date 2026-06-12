#include <stdio.h>
#include <string.h>

extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "driver/i2c_master.h"
    #include "esp_log.h"
    #include "bme680_driver.h"
    #include "u8g2.h"
}

#define I2C_SDA GPIO_NUM_4
#define I2C_SCL GPIO_NUM_5
#define I2C_PORT I2C_NUM_0
#define BME680_ADDR 0x77
#define OLED_X_OFFSET 0
#define OLED_Y_OFFSET (-30)

static const char *TAG = "MAIN";
static u8g2_t u8g2;
static i2c_master_dev_handle_t oled_dev = NULL;

extern "C" uint8_t u8x8_byte_espidf_i2c(
    u8x8_t *u8x8,
    uint8_t msg,
    uint8_t arg_int,
    void *arg_ptr
) {
    static uint8_t buffer[256];
    static uint8_t buffer_len;

    switch (msg) {
        case U8X8_MSG_BYTE_INIT:
            return 1;

        case U8X8_MSG_BYTE_START_TRANSFER:
            buffer_len = 0;
            return 1;

        case U8X8_MSG_BYTE_SEND:
            if (buffer_len + arg_int > sizeof(buffer)) {
                return 0;
            }
            memcpy(buffer + buffer_len, arg_ptr, arg_int);
            buffer_len += arg_int;
            return 1;

        case U8X8_MSG_BYTE_END_TRANSFER:
            if (buffer_len == 0) {
                return 1;
            }

            return i2c_master_transmit(
                oled_dev,
                buffer,
                buffer_len,
                pdMS_TO_TICKS(1000)
            ) == ESP_OK;

        default:
            return 0;
    }
}

extern "C" void app_main(void) {
    //Setup

    i2c_master_bus_handle_t bus_handle;

    i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = I2C_PORT;
    bus_config.sda_io_num = I2C_SDA;
    bus_config.scl_io_num = I2C_SCL;
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t oled_config = {};
    oled_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    oled_config.device_address = 0x3C;
    oled_config.scl_speed_hz = 100000;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &oled_config, &oled_dev));

    u8g2_Setup_sh1107_i2c_128x128_f(
        &u8g2,
        U8G2_R1,
        u8x8_byte_espidf_i2c,
        u8x8_dummy_cb
    );

    u8x8_SetI2CAddress(&u8g2.u8x8, 0x3C << 1);

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_helvB12_tr);
    
    int cx = 64 + OLED_X_OFFSET;
    int cy = 64 + OLED_Y_OFFSET;

    // Sonnenkörper
    u8g2_DrawDisc(&u8g2, cx, cy, 18, U8G2_DRAW_ALL);

    // Strahlen oben/unten/links/rechts
    u8g2_DrawLine(&u8g2, cx, cy - 28, cx, cy - 34);
    u8g2_DrawLine(&u8g2, cx, cy + 28, cx, cy + 42);
    u8g2_DrawLine(&u8g2, cx - 28, cy, cx - 42, cy);
    u8g2_DrawLine(&u8g2, cx + 28, cy, cx + 42, cy);

    // diagonale Strahlen
    u8g2_DrawLine(&u8g2, cx - 16, cy - 18, cx - 24, cy - 24);
    u8g2_DrawLine(&u8g2, cx + 16, cy - 18, cx + 24, cy - 24);
    u8g2_DrawLine(&u8g2, cx - 20, cy + 20, cx - 32, cy + 32);
    u8g2_DrawLine(&u8g2, cx + 20, cy + 20, cx + 32, cy + 32);

    u8g2_SendBuffer(&u8g2);

    bme680_t bme;
    ESP_ERROR_CHECK(bme680_init(&bme, bus_handle, BME680_ADDR));

    ESP_ERROR_CHECK(bme680_soft_reset(&bme));
    vTaskDelay(pdMS_TO_TICKS(100));

    uint8_t chip_id = 0;
    ESP_ERROR_CHECK(bme680_read_chip_id(&bme, &chip_id));
    ESP_LOGI(TAG, "BME680 address: 0x%02X", bme.address);

    ESP_ERROR_CHECK(bme680_configure(&bme));

    ESP_ERROR_CHECK(bme680_load_calibration(&bme));

    vTaskDelay(pdMS_TO_TICKS(5000));

    
    //Loop
    while(true){
        bme680_start_measurement(&bme);
        vTaskDelay(pdMS_TO_TICKS(100));
        bme680_raw_data_t raw_data;
        ESP_ERROR_CHECK(bme680_read_raw_data(&bme, &raw_data));

        ESP_LOGI(TAG, "Raw Pressure: %u", raw_data.raw_press);
        ESP_LOGI(TAG, "Raw Temperature: %u", raw_data.raw_temp);
        ESP_LOGI(TAG, "Raw Humidity: %u", raw_data.raw_hum);

        float temperature = bme680_compensate_temperature(&bme, raw_data.raw_temp) - 5;
        ESP_LOGI(TAG, "Compensated Temperature: %.2f °C", temperature);

        float pressure = bme680_compensate_pressure(&bme, raw_data.raw_press);
        ESP_LOGI(TAG, "Compensated Pressure: %.2f hPa", pressure);

        uint8_t ctrl_hum = 0;
        bme680_read_register(&bme, BME680_REG_CRTL_HUM, &ctrl_hum, 1);
        ESP_LOGI(TAG, "CTRL_HUM = 0x%02X", ctrl_hum);

        float humidity = bme680_compensate_humidity(&bme, raw_data.raw_hum, temperature);
        ESP_LOGI(TAG, "Compensated Humidity: %.2f %%", humidity);

        u8g2_ClearBuffer(&u8g2);

        char line[32];

        snprintf(line, sizeof(line), "Temp: %.1f C", temperature);
        u8g2_DrawStr(&u8g2, 0, 20, line);

        snprintf(line, sizeof(line), "Feuchte: %.1f %%", humidity);
        u8g2_DrawStr(&u8g2, 0, 40, line);

        snprintf(line, sizeof(line), "Druck: %.0f hPa", pressure);
        u8g2_DrawStr(&u8g2, 0, 60, line);

        u8g2_SendBuffer(&u8g2);




        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}