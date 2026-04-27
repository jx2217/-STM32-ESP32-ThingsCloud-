#include "bsp_flash.h"
#include "common_def.h"
#include "main.h"
#include <string.h>

/*
 * 这里必须按你的 MCU 型号来改：
 * - F1/F3 常按 page 擦除
 * - F4/F7/H7 常按 sector 擦除
 */

#define STORAGE_PARAM_FLASH_ADDR   0x0807F800UL

int bsp_flash_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    if ((buf == 0U) || (len == 0U))
    {
        return RET_INVALID_PARAM;
    }

    memcpy(buf, (const void *)addr, len);
    return RET_OK;
}

int bsp_flash_erase_param_area(void)
{
    HAL_StatusTypeDef hal_ret;
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0U;

    HAL_FLASH_Unlock();

    erase_init.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = STORAGE_PARAM_FLASH_ADDR;
    erase_init.NbPages     = 1;

    hal_ret = HAL_FLASHEx_Erase(&erase_init, &page_error);

    HAL_FLASH_Lock();

    return (hal_ret == HAL_OK) ? RET_OK : RET_ERR;
}

int bsp_flash_write(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    HAL_StatusTypeDef hal_ret;
    uint32_t i;
    uint16_t halfword;

    if ((buf == 0U) || (len == 0U))
    {
        return RET_INVALID_PARAM;
    }

    HAL_FLASH_Unlock();

    /*
     * STM32F103 按半字（16位）写入
     * 每次写 2 字节
     */
    for (i = 0U; i < len; i += 2U)
    {
        if ((i + 1U) < len)
        {
            /* 正常 2 字节拼成一个 halfword，小端 */
            halfword = (uint16_t)(buf[i] | (buf[i + 1U] << 8));
        }
        else
        {
            /*
             * 如果长度是奇数，最后一个字节补 0xFF
             * 不过你当前参数结构体一般都会是偶数字节，对齐后通常不会进这里
             */
            halfword = (uint16_t)(buf[i] | (0xFFU << 8));
        }

        hal_ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                                    addr + i,
                                    halfword);
        if (hal_ret != HAL_OK)
        {
            HAL_FLASH_Lock();
            return RET_ERR;
        }
    }

    HAL_FLASH_Lock();

    return RET_OK;
}

