idf_component_register(
    SRCS "bme680_driver.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c
)