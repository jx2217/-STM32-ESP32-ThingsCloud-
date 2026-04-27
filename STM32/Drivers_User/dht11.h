#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t temperature_int;
    uint8_t humidity_int;
} dht11_data_t;

/*
 * @brief  DHT11 初始化（可为空实现，主要保留接口）
 * @return 0=成功, 负数=失败
 */
int dht11_init(void);

/*
 * @brief  读取 DHT11 温湿度
 * @param  data: 输出数据
 * @return 0=成功, 负数=失败
 */
int dht11_read(dht11_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* DHT11_H */
