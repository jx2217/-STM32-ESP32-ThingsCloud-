idf_component_register(SRCS "console_service.c"
                       INCLUDE_DIRS "include"
                       REQUIRES common stm32_ctrl status_cache param_cache)