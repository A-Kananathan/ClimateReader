#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLED_ADDR 0x3C

esp_err_t oled_init(
    i2c_master_bus_handle_t bus_handle
);

esp_err_t oled_clear(void);

esp_err_t oled_fill(void);

#ifdef __cplusplus
}
#endif