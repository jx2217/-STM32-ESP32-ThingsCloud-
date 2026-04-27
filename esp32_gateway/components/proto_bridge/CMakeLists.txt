idf_component_register(SRCS "proto_frame.c" "proto_bridge.c"
                       INCLUDE_DIRS "include"
                       REQUIRES common status_cache app_service)