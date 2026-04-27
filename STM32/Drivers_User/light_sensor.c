#include "light_sensor.h"
#include "main.h"
#include "common_def.h"

extern ADC_HandleTypeDef hadc3;

/*
 * 如果你换了引脚，只改这里
 */
#define LIGHT_DO_GPIO_PORT      GPIOB
#define LIGHT_DO_GPIO_PIN       GPIO_PIN_13

int light_sensor_init(void)
{
    return RET_OK;
}

int light_sensor_read_ao(uint16_t *value)
{
    if (value == 0U)
    {
        return RET_NULL_PTR;
    }

    if (HAL_ADC_Start(&hadc3) != HAL_OK)
    {
        return RET_ERR;
    }

    if (HAL_ADC_PollForConversion(&hadc3, 10) != HAL_OK)
    {
        HAL_ADC_Stop(&hadc3);
        return RET_ERR;
    }

    *value = (uint16_t)HAL_ADC_GetValue(&hadc3);

    HAL_ADC_Stop(&hadc3);
    return RET_OK;
}

//int light_sensor_read_do(uint8_t *level)
//{
//    if (level == 0U)
//    {
//        return RET_NULL_PTR;
//    }

//    *level = (HAL_GPIO_ReadPin(LIGHT_DO_GPIO_PORT, LIGHT_DO_GPIO_PIN) == GPIO_PIN_SET) ? 1U : 0U;
//    return RET_OK;
//}

int light_sensor_read(light_sensor_data_t *data)
{
    int ret;

    if (data == 0U)
    {
        return RET_NULL_PTR;
    }

    ret = light_sensor_read_ao(&data->ao_raw);
    if (ret != RET_OK)
    {
        return ret;
    }

//    ret = light_sensor_read_do(&data->do_level);
//    if (ret != RET_OK)
//    {
//        return ret;
//    }

    return RET_OK;
}
