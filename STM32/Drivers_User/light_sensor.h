#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t ao_raw;     /* ADC 原始值 0~4095(F1 实际 0~4095/0~4095? 若12位则0~4095) */
    uint8_t  do_level;   /* 数字量输出，0/1 */
} light_sensor_data_t;

/* 初始化 */
int light_sensor_init(void);

/* 读取 AO 原始 ADC 值 */
int light_sensor_read_ao(uint16_t *value);

/* 读取 DO 电平 */
int light_sensor_read_do(uint8_t *level);

/* 一次性读取 AO + DO */
int light_sensor_read(light_sensor_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* LIGHT_SENSOR_H */
