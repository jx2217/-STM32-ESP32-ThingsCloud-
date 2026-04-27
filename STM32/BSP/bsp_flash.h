#ifndef BSP_FLASH_H
#define BSP_FLASH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief  从 Flash 指定地址读取数据
 * @param  addr: Flash 地址
 * @param  buf : 目标缓冲区
 * @param  len : 读取长度
 * @return 0=成功, 负数=失败
 */
int bsp_flash_read(uint32_t addr, uint8_t *buf, uint32_t len);

/*
 * @brief  擦除包含指定地址的参数存储区
 * @return 0=成功, 负数=失败
 */
int bsp_flash_erase_param_area(void);

/*
 * @brief  向 Flash 指定地址写入数据
 * @param  addr: Flash 地址
 * @param  buf : 源数据
 * @param  len : 写入长度
 * @return 0=成功, 负数=失败
 */
int bsp_flash_write(uint32_t addr, const uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* BSP_FLASH_H */
