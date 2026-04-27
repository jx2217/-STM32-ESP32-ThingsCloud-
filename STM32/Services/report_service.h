#ifndef REPORT_SERVICE_H
#define REPORT_SERVICE_H

#include "common_def.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int report_service_init(void);
int report_service_build_payload(uint8_t *buf, uint8_t *len);
int report_service_send_once(void);

/*
 * @brief  发送 ACK 应答
 * @param  ack_cmd: 被应答的命令字
 * @param  ack_seq: 被应答命令的序号
 * @param  result : ACK 结果码
 * @return 0=成功, 负数=失败
 */
int report_service_send_ack(uint8_t ack_cmd, uint8_t ack_seq, uint8_t result);


#ifdef __cplusplus
}
#endif

#endif /* REPORT_SERVICE_H */
