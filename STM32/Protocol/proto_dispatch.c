#include "proto_dispatch.h"
#include "proto_cmd.h"
#include "control_service.h"
#include "report_service.h"
#include "device_data.h"
#include "common_def.h"
#include "storage_service.h"

static uint16_t get_u16_le(const uint8_t *buf)
{
    return (uint16_t)(buf[0] | (buf[1] << 8));
}

int proto_dispatch_handle(const proto_msg_t *msg)
{
    int ret = RET_OK;
    uint8_t ack_result = ACK_RESULT_OK;

    if (msg == 0U)
    {
        return RET_NULL_PTR;
    }

    switch (msg->cmd)
    {
        case CMD_LED_SET:
				{
						if (!control_service_remote_action_allowed())
						{
								ack_result = ACK_RESULT_EXEC_ERR;
								(void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
								return RET_ERR;
						}

						if (msg->len < 1U)
						{
								ack_result = ACK_RESULT_PARAM_ERR;
								(void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
								return RET_INVALID_PARAM;
						}

						if (msg->data[0] == 0U)
						{
								ret = control_service_set_led(0U);
						}
						else if (msg->data[0] == 1U)
						{
								ret = control_service_set_led(1U);
						}
						else if (msg->data[0] == 2U)
						{
								ret = control_service_toggle_led();
						}
						else
						{
								ack_result = ACK_RESULT_PARAM_ERR;
								(void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
								return RET_INVALID_PARAM;
						}

						ack_result = (ret == RET_OK) ? ACK_RESULT_OK : ACK_RESULT_EXEC_ERR;
						(void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
						return ret;
				}

        case CMD_PARAM_SET:
        {
            uint8_t param_id;
            uint16_t value_u16;

            /*
             * 当前协议:
             * Byte0: param_id
             * Byte1: value_low
             * Byte2: value_high
             */
            if (msg->len < 3U)
            {
                ack_result = ACK_RESULT_PARAM_ERR;
                (void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
                return RET_INVALID_PARAM;
            }

            param_id = msg->data[0];
            value_u16 = get_u16_le(&msg->data[1]);

            switch (param_id)
            {
                case PARAM_ID_UPLOAD_PERIOD_MS:
								{
										ret = device_data_set_upload_period(value_u16);
										if (ret == RET_OK)
										{
												(void)storage_service_save_param();
												ack_result = ACK_RESULT_OK;
										}
										else
										{
												ack_result = ACK_RESULT_PARAM_ERR;
										}

										(void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
										return ret;
								}
								
								case PARAM_ID_TEMP_HIGH_TH:
                {
                    ret = device_data_set_temp_high_th(value_u16);
                    if (ret == RET_OK)
                    {
                        (void)storage_service_save_param();
                        ack_result = ACK_RESULT_OK;
                    }
                    else
                    {
                        ack_result = ACK_RESULT_PARAM_ERR;
                    }

                    (void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
                    return ret;
                }

                case PARAM_ID_LIGHT_LOW_TH:
                {
                    ret = device_data_set_light_low_th(value_u16);
                    if (ret == RET_OK)
                    {
                        (void)storage_service_save_param();
                        ack_result = ACK_RESULT_OK;
                    }
                    else
                    {
                        ack_result = ACK_RESULT_PARAM_ERR;
                    }

                    (void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
                    return ret;
                }
								
								case PARAM_ID_WORK_MODE:
								{
										ret = device_data_set_work_mode(value_u16);
										if (ret == RET_OK)
										{
												(void)storage_service_save_param();
												ack_result = ACK_RESULT_OK;
										}
										else
										{
												ack_result = ACK_RESULT_PARAM_ERR;
										}

										(void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
										return ret;
								}
								
                default:
                {
                    ack_result = ACK_RESULT_UNSUPPORTED;
                    (void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
                    return RET_INVALID_PARAM;
                }
            }
        }

        default:
        {
            ack_result = ACK_RESULT_UNSUPPORTED;
            (void)report_service_send_ack(msg->cmd, msg->seq, ack_result);
            return RET_INVALID_PARAM;
        }
    }
}
