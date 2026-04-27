#ifndef PROTO_DISPATCH_H
#define PROTO_DISPATCH_H

#include "proto_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief  协议分发处理
 * @param  msg: 已经解包后的协议消息
 * @return 0=成功, 负数=失败
 */
int proto_dispatch_handle(const proto_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* PROTO_DISPATCH_H */
