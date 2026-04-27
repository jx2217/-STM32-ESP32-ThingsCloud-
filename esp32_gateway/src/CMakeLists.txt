idf_component_register(SRCS "app_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES common
                                uart_service
                                proto_bridge
                                status_cache
                                app_service
                                stm32_ctrl
                                console_service
                                wifi_service
                                mqtt_service
                                param_cache)