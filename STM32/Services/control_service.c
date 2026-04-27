#include "control_service.h"
#include "common_def.h"
#include "device_data.h"
#include "led.h"

int control_service_init(void)
{
    led_init();

    /* 初始化时同步一次状态 */
    {
        device_status_t *st = device_data_get_status();
        if (st != 0U)
        {
            st->led_state = led_get();
        }
    }

    return RET_OK;
}

int control_service_remote_action_allowed(void)
{
    device_param_t *param = device_data_get_param();

    if (param == 0U)
    {
        return 0;
    }

    /*
     * 只有 REMOTE 模式允许远程执行类命令
     * MANUAL / AUTO 下不允许远程直接改执行器状态
     */
    if (param->work_mode == MODE_REMOTE)
    {
        return 1;
    }

    return 0;
}

int control_service_set_led(uint8_t onoff)
{
    device_status_t *st = device_data_get_status();

    if (st == 0U)
    {
        return RET_NULL_PTR;
    }

    /* 控制实际硬件 */
    led_set(onoff ? 1U : 0U);

    /* 同步更新系统状态 */
    st->led_state = led_get();

    return RET_OK;
}

int control_service_toggle_led(void)
{
    device_status_t *st = device_data_get_status();

    if (st == 0U)
    {
        return RET_NULL_PTR;
    }

    /* 翻转实际硬件 */
    led_toggle();

    /* 同步更新系统状态 */
    st->led_state = led_get();

    return RET_OK;
}
