/*
 * @Author: Qwerty 263690179@qq.com
 * @Date: 2025-11-12 16:37:41
 * @LastEditors: Qwerty 263690179@qq.com
 * @LastEditTime: 2025-11-13 10:59:36
 * @FilePath: \flexdriver\testApp.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <asm/ioctl.h>   
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>

#define FLEXSPI_DEV_NAME "/dev/myflexspi"

/* Use 'k' as magic number */
#define FLEX_IOC_MAGIC 'm'
/* Please use a different 8-bit number in your code */

#define FLEX_FLASH_READ_ID _IOWR(FLEX_IOC_MAGIC, 0, int)
#define FLEX_FLASH_ERASE_SECTOR _IOWR(FLEX_IOC_MAGIC, 1, int)
#define FLEX_FLASH_WRITE_DATA _IOWR(FLEX_IOC_MAGIC, 2, int)
#define FLEX_FLASH_READ_DATA _IOWR(FLEX_IOC_MAGIC, 3, int)

#define FLEX_SET_CLK _IOWR(FLEX_IOC_MAGIC, 4, int)

struct flexspi_dev {
    bool initialized;           
    int fd;                     

    char *rxbuf;
    char *txbuf;
};

struct flash_ops_t
{
    void *buf;
    uint32_t addr;
    size_t len;
};

