/*
 * @Author: Qwerty 263690179@qq.com
 * @Date: 2025-11-12 16:37:28
 * @LastEditors: Qwerty 263690179@qq.com
 * @LastEditTime: 2025-11-13 10:55:45
 * @FilePath: \flexdriver\testApp.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>    // Memset and memcpy functions
#include <fcntl.h>     // Flags for open()
#include <sys/stat.h>  // Open() system call
#include <sys/types.h> // Types for open()
#include <sys/mman.h>  // Mmap system call
#include <sys/ioctl.h> // IOCTL system call
#include <unistd.h>    // Close() system call
#include <errno.h>     // Error codes
#include <signal.h>    //
#include <time.h>

#include "testApp.h"

#define TEST_ADDR 4096 * 4
#define TEST_SIZE 4096

int flexspi_init(void *arg)
{
    struct flexspi_dev *dev = (struct flexspi_dev *)arg;

    dev->fd = open(FLEXSPI_DEV_NAME, O_RDWR);
    if (dev->fd < 0)
    {
        printf("Error: Unable to open FLEXSPI device %s.\n",FLEXSPI_DEV_NAME);
        printf("fd = %d\n", dev->fd);
        return -1;
    }
    else
    {
        printf("Initialization successful: FLEXSPI device %s is ready for use.\n",FLEXSPI_DEV_NAME);
    }
    
    dev->initialized = true;

    return 0;
}

int flexspi_close(void *arg)
{
    struct flexspi_dev *dev = (struct flexspi_dev *)arg;
    close(dev->fd);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int i = 0;
    int clk = 5000000;
    struct flexspi_dev *dev = malloc(sizeof(struct flexspi_dev));
    dev->txbuf = malloc(TEST_SIZE);
    dev->rxbuf = malloc(TEST_SIZE);

    ret = flexspi_init(dev);
    if(ret)
    {
        printf("Error: flexspi_init failed\n");
        return -1;
    }

    printf("flexspi_init successful\n");

    struct flash_ops_t *op = malloc(sizeof(struct flash_ops_t));
    op->addr = TEST_ADDR;
    op->buf = dev->rxbuf;
    op->len = TEST_SIZE;

    // SET CLK
    if (ioctl(dev->fd, FLEX_SET_CLK, &clk) == -1)
    {
        printf("FSPI_SET_CLK failed.\n");
        return -2;
    }

    //  READ ID
    if (ioctl(dev->fd, FLEX_FLASH_READ_ID, op) == -1)
    {
        printf("FLEX_FLASH_READ_ID failed.\n");
        return -2;
    }

    uint8_t *id = (uint8_t *)op->buf;
    printf("ID: %x %x %x\n", id[0], id[1], id[2]);
    

    // ERASE SECTOR
    if (ioctl(dev->fd, FLEX_FLASH_ERASE_SECTOR, op) == -1)
    {
        printf("FLEX_FLASH_READ_ID failed.\n");
        return -2;
    }

    // WRITE TEST DATA.
    uint32_t *writeData = (uint32_t *)dev->txbuf;
    for(i = 0;i < TEST_SIZE / 4;i++)
    {
        writeData[i] = i + 1;
    }
    op->buf = dev->txbuf;
    op->len = TEST_SIZE;
    if (ioctl(dev->fd, FLEX_FLASH_WRITE_DATA, op) == -1)
    {
        printf("FLEX_FLASH_READ_ID failed.\n");
        return -2;
    }

    // READ TEST DATA
    uint32_t *readData = (uint32_t *)dev->rxbuf;
    op->buf = dev->rxbuf;
    op->len = TEST_SIZE;
    if (ioctl(dev->fd, FLEX_FLASH_READ_DATA, op) == -1)
    {
        printf("FLEX_FLASH_READ_ID failed.\n");
        return -2;
    }
    printf("Read data:\n");
    for (i = 0; i < 8; i++)
    {
        printf("%08x ", readData[i]);
    }

    printf("\n");
    
    for (i = TEST_SIZE / 4 - 8; i < TEST_SIZE / 4; i++)
    {
        printf("%08x ", readData[i]);
    }
    
    printf("\n");

    for(i = 0;i < TEST_SIZE / 4;i++)
    {
        if(writeData[i] != readData[i])
        {
            printf("Error: writeData[%d] = %x, readData[%d] = %x\n", i, writeData[i], i, readData[i]);
            break;
        }
    }

    if(i == TEST_SIZE / 4)
        printf("Test successful\n");

    free(dev->rxbuf);
    free(dev->txbuf);
    flexspi_close(dev);
    free(dev);
}
