#ifndef PROTO_CMD_H
#define PROTO_CMD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTO_HEADER1           0x55U
#define PROTO_HEADER2           0xAAU
#define PROTO_MAX_PAYLOAD       64U
#define PROTO_FRAME_OVERHEAD    6U   /* H1 H2 LEN CMD SEQ CHKSUM */
#define PROTO_MAX_FRAME_LEN     (PROTO_MAX_PAYLOAD + PROTO_FRAME_OVERHEAD)

/* 命令字 */
typedef enum
{
    CMD_REPORT    = 0x01,   /* 状态上报 */
    CMD_HEARTBEAT = 0x02,   /* 心跳 */
    CMD_LED_SET   = 0x03,   /* 先拿 LED 做控制示例 */
    CMD_PARAM_SET = 0x04,   /* 参数下发 */
    CMD_ACK       = 0x05    /* 应答 */
} proto_cmd_t;

/* ACK 结果码
0x00  成功
0x01  参数错误
0x02  不支持的命令
0x03  执行失败 */
typedef enum
{
    ACK_RESULT_OK          = 0x00,
    ACK_RESULT_PARAM_ERR   = 0x01,
    ACK_RESULT_UNSUPPORTED = 0x02,
    ACK_RESULT_EXEC_ERR    = 0x03
} ack_result_t;

/* 参数 ID 定义 */
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
