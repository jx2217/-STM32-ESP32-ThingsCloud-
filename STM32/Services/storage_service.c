#include "storage_service.h"
#include "bsp_flash.h"
#include "common_def.h"
#include <string.h>
#include <stdint.h>

#define STORAGE_PARAM_MAGIC        0x50415241UL   /* 'PARA' */
#define STORAGE_PARAM_VERSION      0x0001U


#define STORAGE_PARAM_FLASH_ADDR   0x0807F800UL

typedef struct
{
    uint32_t magic;        //* 固定魔数，用于判断这块数据是不是参数区 */
    uint16_t version;       /* 参数版本号，后续结构升级时用 */
    uint16_t length;				/* 参数区有效长度 */

    device_param_t param;   /* 真正的参数内容 */

    uint32_t checksum;
} storage_param_blob_t;

static uint32_t storage_checksum32(const uint8_t *buf, uint32_t len)
{
    uint32_t i;
    uint32_t sum = 0U;

    if (buf == 0U)
    {
        return 0U;
    }

    for (i = 0; i < len; i++)
    {
        sum += buf[i];
    }

    return sum;
}

static int storage_param_blob_is_valid(const storage_param_blob_t *blob)
{
    uint32_t calc_sum;

    if (blob == 0U)
    {
        return RET_NULL_PTR;
    }

    if (blob->magic != STORAGE_PARAM_MAGIC)
    {
        return RET_INVALID_PARAM;
    }

    if (blob->version != STORAGE_PARAM_VERSION)
    {
        return RET_INVALID_PARAM;
    }

    if (blob->length != sizeof(device_param_t))
    {
        return RET_INVALID_PARAM;
    }

    calc_sum = storage_checksum32((const uint8_t *)blob,
                                  sizeof(storage_param_blob_t) - sizeof(uint32_t));

    if (calc_sum != blob->checksum)
    {
        return RET_CHECKSUM_ERR;
    }

    return RET_OK;
}

static void storage_fill_default_param(device_param_t *param)
{
    if (param == 0U)
    {
        return;
    }

    param->sample_period_ms = 1000U;
    param->upload_period_ms = 5000U;
    param->temp_high_th     = 29.0f;
    param->light_low_th     = 200.0f;
    param->work_mode        = MODE_MANUAL;
    param->relay_default    = 0U;
}

int storage_service_init(void)
{
    return RET_OK;
}

int storage_service_load_param(void)
{
    storage_param_blob_t blob;
    device_param_t *param;
    int ret;

    memset(&blob, 0, sizeof(blob));

    ret = bsp_flash_read(STORAGE_PARAM_FLASH_ADDR, (uint8_t *)&blob, sizeof(blob));
    if (ret != RET_OK)
    {
        return ret;
    }

    ret = storage_param_blob_is_valid(&blob);
    if (ret != RET_OK)
    {
        return ret;
    }

    param = device_data_get_param();
    if (param == 0U)
    {
        return RET_NULL_PTR;
    }

    memcpy(param, &blob.param, sizeof(device_param_t));
    return RET_OK;
}

int storage_service_save_param(void)
{
    storage_param_blob_t blob;
    device_param_t *param;
    int ret;

    memset(&blob, 0, sizeof(blob));

    param = device_data_get_param();
    if (param == 0U)
    {
        return RET_NULL_PTR;
    }

    blob.magic   = STORAGE_PARAM_MAGIC;
    blob.version = STORAGE_PARAM_VERSION;
    blob.length  = sizeof(device_param_t);

    memcpy(&blob.param, param, sizeof(device_param_t));

    blob.checksum = storage_checksum32((const uint8_t *)&blob,
                                       sizeof(storage_param_blob_t) - sizeof(uint32_t));

    ret = bsp_flash_erase_param_area();
    if (ret != RET_OK)
    {
        return ret;
    }

    ret = bsp_flash_write(STORAGE_PARAM_FLASH_ADDR,
                          (const uint8_t *)&blob,
                          sizeof(blob));
    if (ret != RET_OK)
    {
        return ret;
    }

    return RET_OK;
}

int storage_service_reset_param_to_default(void)
{
    device_param_t *param;

    param = device_data_get_param();
    if (param == 0U)
    {
        return RET_NULL_PTR;
    }

    storage_fill_default_param(param);

    return storage_service_save_param();
}
