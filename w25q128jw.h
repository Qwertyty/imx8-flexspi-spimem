#ifndef W25Q128JW_H
#define W25Q128JW_H

#include <linux/device.h>
#include <linux/spi/spi.h>
#include "flexspidev.h"

/* FLASH相关特性 */
#define FLASH_SIZE          (16*1024)  /* 单位为KByte,256Mb = 32768KByte */
#define FLASH_PAGE_SIZE     256     /* 页大小 */
#define SECTOR_SIZE         (4*1024)  /* 扇区大小4K */

/* 使用的FLASH地址宽度，单位：bit */
#define FLASH_ADDR_LENGTH    24    

struct flash_ops_t
{
    void *buf;
    u32 addr;
    size_t len;
};

int w25q128jw_read_ID(struct device *dev, u8 *buf);
int w25q128jw_read_data(struct device *dev, u32 addr, void *buf, size_t len);
int w25q128jw_write_enable(struct device *dev);
int w25q128jw_erase_sector(struct device *dev, u32 addr);
int w25q128jw_write_data(struct device *dev, u32 addr, const void *buf, size_t len);
unsigned char w25q128jw_get_busy_status(struct device *dev);

#endif