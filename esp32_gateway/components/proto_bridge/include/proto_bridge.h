#ifndef PROTO_BRIDGE_H
#define PROTO_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 向协议桥喂入收到的原始字节流
 */
int proto_bridge_feed_bytes(const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* PROTO_BRIDGE_H */