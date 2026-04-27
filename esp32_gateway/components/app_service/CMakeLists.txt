idf_component_register(SRCS "app_service.c"
                       INCLUDE_DIRS "include"
                       REQUIRES common proto_bridge status_cache esp_timer uart_service)