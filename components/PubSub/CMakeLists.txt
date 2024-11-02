idf_component_register(SRCS "PubSub.c"
                        INCLUDE_DIRS "."
                        REQUIRES esp_http_client cJSON mbedtls freertos jwt_manager)
