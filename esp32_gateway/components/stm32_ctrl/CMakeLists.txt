idf_component_register(SRCS "stm32_ctrl.c"
                       INCLUDE_DIRS "include"
                       REQUIRES common proto_bridge uart_service)