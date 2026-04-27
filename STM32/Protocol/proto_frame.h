#ifndef PROTO_FRAME_H
#define PROTO_FRAME_H

#include "common_def.h"
#include "proto_msg.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int proto_frame_pack(uint8_t cmd,
                     uint8_t seq,
                     const uint8_t *payload,
                     uint8_t payload_len,
                     uint8_t *out_buf,
                     uint16_t *out_len);

int proto_frame_unpack(const uint8_t *buf,
                       uint16_t len,
                       proto_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* PROTO_FRAME_H */
