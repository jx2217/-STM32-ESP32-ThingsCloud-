#include "acquire_service.h"
#include "device_data.h"
#include "common_def.h"
#include "buzzer.h"
#include "dht11.h"
#include "main.h"
#include "usart.h"
#include "light_sensor.h"
#include "control_service.h"

#include <stdio.h>
#include <string.h>


/*
 * 内部函数声明
 */
static void acquire_service_update_alarm(void);
static void acquire_service_update_alarm_action(void);
static void acquire_service_update_auto_action(void);

int acquire_service_init(void)
{
		dht11_init();
		light_sensor_init();
    return RET_OK;
}

int acquire_service_update(void)
{
    device_status_t *st = device_data_get_status();
		device_param_t *pa = device_data_get_param();
		dht11_data_t dht11_data;
		light_sensor_data_t light_data;
    int ret_dht, ret_light;
		uint8_t temp[128];
	
    if (st == 0U)
    {
        return RET_NULL_PTR;
    }
		
		if (pa == 0U)
    {
        return RET_NULL_PTR;
    }

    /*
     * =========================
     * 这里是示例数据更新逻辑
     * 如果你已经接了真实传感器，请替换掉这里
     * =========================
     */

    /* 示例：温度在一定范围内变化 */
    ret_dht = dht11_read(&dht11_data);
    if (ret_dht == RET_OK)
    {
        st->temperature = (float)dht11_data.temperature_int;
        st->humidity    = (float)dht11_data.humidity_int;
			
				sprintf((char *)temp, "temp:%.2f, hum:%.2f, ", st->temperature, st->humidity);
				HAL_UART_Transmit(&huart1, temp, strlen((char *)temp), HAL_MAX_DELAY);
    }
    else
    {
        /*
         * 第一版处理策略：
         * 读取失败时保留上次有效值
         * 同时打上传感器异常告警
         */
        st->alarm_code = ALARM_SENSOR_ERR;
				sprintf((char *)temp, "temp:erro, hum:erro\r\n");
				HAL_UART_Transmit(&huart1, temp, strlen((char *)temp), HAL_MAX_DELAY);
    }

		
    ret_light = light_sensor_read(&light_data);
    if (ret_light == RET_OK)
    {
        st->light = light_data.ao_raw;
				sprintf((char *)temp, "ADC value:%d, ", st->light);
				HAL_UART_Transmit(&huart1, temp, strlen((char *)temp), HAL_MAX_DELAY);
    }
    else
    {
        st->alarm_code = ALARM_SENSOR_ERR;
				sprintf((char *)temp, "ADC value:erro\r\n");
				HAL_UART_Transmit(&huart1, temp, strlen((char *)temp), HAL_MAX_DELAY);
    }

		

    /* 示例：电压固定 */
    st->voltage = 3.30f;

    /* 系统运行时间 */
    st->tick_ms = HAL_GetTick();
		sprintf((char *)temp, "Voltage value:%.1fV, sys run time:%d, alarm_code:%d, link_state:%d\r\n"
														, st->voltage, st->tick_ms, st->alarm_code, st->link_state);
		HAL_UART_Transmit(&huart1, temp, strlen((char *)temp), HAL_MAX_DELAY);
    
		sprintf((char *)temp, "sample_period:%dms, upload_period:%dms, temp_high:%.1f, light_low:%.0f, work_mode:%d, relay_default:%d\r\n"
														, pa->sample_period_ms, pa->upload_period_ms, pa->temp_high_th, pa->light_low_th, pa->work_mode, pa->relay_default);
		HAL_UART_Transmit(&huart1, temp, strlen((char *)temp), HAL_MAX_DELAY);
		
		
		/*
     * 状态更新完成后，重新判断告警
     */
    if ((ret_dht == RET_OK) && (ret_light == RET_OK))
    {
        acquire_service_update_alarm();
    }

    /*
     * 根据告警结果更新联动动作
     * 第一版先只联动蜂鸣器状态
     */
    acquire_service_update_alarm_action();
		acquire_service_update_auto_action();
		
    return RET_OK;
}

/*
 * @brief 告警判断
 *
 * 第一版优先级：
 * 1. 链路异常
 * 2. 电压过低
 * 3. 温度过高
 * 4. 光照过低
 * 5. 无告警
 *
 * 同一时刻只保留一个 alarm_code
 */
static void acquire_service_update_alarm(void)
{
    device_status_t *st = device_data_get_status();
    device_param_t *param = device_data_get_param();

    if ((st == 0U) || (param == 0U))
    {
        return;
    }

    if (st->link_state == LINK_OFFLINE)
    {
        st->alarm_code = ALARM_LINK_LOST;
    }
    else if (st->voltage < 3.00f)
    {
        st->alarm_code = ALARM_VOLT_LOW;
    }
    else if (st->temperature > param->temp_high_th)
    {
        st->alarm_code = ALARM_TEMP_HIGH;
    }
    else if ((float)st->light > param->light_low_th)
    {
        st->alarm_code = ALARM_LIGHT_LOW;
    }
    else
    {
        st->alarm_code = ALARM_NONE;
    }
}

/*
 * @brief 告警联动动作
 *
 * 第一版策略：
 * - 无告警：蜂鸣器关
 * - 有告警：蜂鸣器开
 *
 * 如果你后面接了真实蜂鸣器驱动，
 * 可以在这里调用 buzzer_set(1/0)
 */
static void acquire_service_update_alarm_action(void)
{
    device_status_t *st = device_data_get_status();

    if (st == 0U)
    {
        return;
    }

    if (st->alarm_code == ALARM_NONE)
    {
				st->buzzer_state = 0U;
				buzzer_off();
        
    }
    else
    {
        st->buzzer_state = 1U;
				buzzer_on();
    }
}

static void acquire_service_update_auto_action(void)
{
    device_param_t *param = device_data_get_param();
    device_status_t *st = device_data_get_status();

    if ((param == 0U) || (st == 0U))
    {
        return;
    }

    if (param->work_mode != MODE_AUTO)
    {
        return;
    }

    /*
     * 第一版自动模式策略：
     * 光照过低时自动点亮 LED
     * 其他情况自动关闭 LED
     */
    if (st->alarm_code == ALARM_LIGHT_LOW)
    {
        (void)control_service_set_led(1U);
    }
    else
    {
        (void)control_service_set_led(0U);
    }
}
