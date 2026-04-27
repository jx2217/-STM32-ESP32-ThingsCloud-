#ifndef PROTO_CMD_H
#define PROTO_CMD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTO_HEADER1           0x55U
#define PROTO_HEADER2           0xAAU
#define PROTO_MAX_PAYLOAD       64U
#define PROTO_FRAME_OVERHEAD    6U
#define PROTO_MAX_FRAME_LEN     (PROTO_MAX_PAYLOAD + PROTO_FRAME_OVERHEAD)

typedef enum
{
    CMD_REPORT    = 0x01,
    CMD_HEARTBEAT = 0x02,
    CMD_LED_SET   = 0x03,
    CMD_PARAM_SET = 0x04,
    CMD_ACK       = 0x05
} proto_cmd_t;

typedef enum
{
    PARAM_ID_UPLOAD_PERIOD_MS = 0x01,
    PARAM_ID_TEMP_HIGH_TH     = 0x02,
    PARAM_ID_LIGHT_LOW_TH     = 0x03,
    PARAM_ID_WORK_MODE        = 0x04
} param_id_t;

#ifdef __cplusplus
}
#endif

#endif /* PROTO_CMD_H */