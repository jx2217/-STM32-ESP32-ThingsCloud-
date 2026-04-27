idf_component_register(SRCS "mqtt_service.c"
                       INCLUDE_DIRS "include"
                       REQUIRES mqtt common status_cache stm32_ctrl wifi_service param_cache)