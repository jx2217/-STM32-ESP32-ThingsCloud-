#include "proto_frame.h"
#include "common_crc.h"
#include <string.h>

int proto_frame_pack(uint8_t cmd,
                     uint8_t seq,
                     const uint8_t *payload,
                     uint8_t payload_len,
                     uint8_t *out_buf,
                     uint16_t *out_len)
{
    uint16_t frame_len = 0U;
    uint8_t checksum = 0U;

    if ((out_buf == 0U) || (out_len == 0U))
    {
        return RET_NULL_PTR;
    }

    if (payload_len > PROTO_MAX_PAYLOAD)
    {
        return RET_INVALID_PARAM;
    }

    frame_len = (uint16_t)(payload_len + PROTO_FRAME_OVERHEAD);

    out_buf[0] = PROTO_HEADER1;
    out_buf[1] = PROTO_HEADER2;
    out_buf[2] = payload_len;
    out_buf[3] = cmd;
    out_buf[4] = seq;

    if ((payload != 0U) && (payload_len > 0U))
    {
        memcpy(&out_buf[5], payload, payload_len);
    }

    checksum = common_checksum8(&out_buf[2], (uint16_t)(payload_len + 3U));
    out_buf[5U + payload_len] = checksum;

    *out_len = frame_len;
    return RET_OK;
}

int proto_frame_unpack(const uint8_t *buf,
                       uint16_t len,
                       proto_msg_t *msg)
{
    uint8_t payload_len = 0U;
    uint8_t checksum = 0U;
    uint16_t expected_len = 0U;

    if ((buf == 0U) || (msg == 0U))
    {
        return RET_NULL_PTR;
    }

    if (len < PROTO_FRAME_OVERHEAD)
    {
        return RET_PROTO_ERR;
    }

    if ((buf[0] != PROTO_HEADER1) || (buf[1] != PROTO_HEADER2))
    {
        return RET_PROTO_ERR;
    }

    payload_len = buf[2];
    expected_len = (uint16_t)(payload_len + PROTO_FRAME_OVERHEAD);

    if (len != expected_len)
    {
        return RET_PROTO_ERR;
    }

    checksum = common_checksum8(&buf[2], (uint16_t)(payload_len + 3U));
    if (checksum != buf[5U + payload_len])
    {
        return RET_CHECKSUM_ERR;
    }

    msg->len = payload_len;
    msg->cmd = buf[3];
    msg->seq = buf[4];

    if (payload_len > 0U)
    {
        memcpy(msg->data, &buf[5], payload_len);
    }

    return RET_OK;
}
