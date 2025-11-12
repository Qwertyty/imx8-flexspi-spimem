#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi-mem.h>
#include <linux/mtd/spi-nor.h>

#include "w25q128jw.h"

int w25q128jw_read_ID(struct device *dev, u8 *buf)
{
    int ret;
    struct spi_mem_op op =
        SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_RDID, 1),
                   SPI_MEM_OP_NO_ADDR,
                   SPI_MEM_OP_NO_DUMMY,
                   SPI_MEM_OP_DATA_IN(3, buf, 1));

    if (!dev || !buf)
    {
        dev_err(dev, "Invalid parameters to flexspidev_read_ID\n");
        return -EINVAL;
    }

    ret = flexspidev_exec_op(dev, &op);
    if (ret)
    {
        dev_err(dev, "Read ID failed: %d\n", ret);
        return ret;
    }

    dev_info(dev, "Device ID: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
    return 0;
}

int w25q128jw_read_data(struct device *dev, u32 addr, void *buf, size_t len)
{
    struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_READ_1_1_4, 1),
                                      SPI_MEM_OP_ADDR(3, addr, 1),
                                      SPI_MEM_OP_DUMMY(1, 1),
                                      SPI_MEM_OP_DATA_IN(len, buf, 4));

    return flexspidev_exec_op(dev, &op);
}

int w25q1258jw_write_enable(struct device *dev)
{
    struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_WREN, 1),
                                      SPI_MEM_OP_NO_ADDR,
                                      SPI_MEM_OP_NO_DUMMY,
                                      SPI_MEM_OP_NO_DATA);

    return flexspidev_exec_op(dev, &op);
}

int w25q1258jw_erase_sector(struct device *dev, u32 addr)
{
    struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_BE_4K, 1),
                                      SPI_MEM_OP_ADDR(3, addr, 1),
                                      SPI_MEM_OP_NO_DUMMY,
                                      SPI_MEM_OP_NO_DATA);

    return flexspidev_exec_op(dev, &op);
}

unsigned char w25q128jw_get_busy_status(struct device *dev)
{
    unsigned char status;
    bool isBusy;
    struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_RDSR, 1),
                                      SPI_MEM_OP_NO_ADDR,
                                      SPI_MEM_OP_NO_DUMMY,
                                      SPI_MEM_OP_DATA_IN(1, &status, 1));

    do
    {
        flexspidev_exec_op(dev, &op);

        if (status & SR_WIP)
        {
            isBusy = true;
        }
        else
        {
            isBusy = false;
        }
    } while (status & SR_WIP);

    return status;
}

int w25q128jw_page_program(struct device *dev, u32 addr, const void *buf, size_t len)
{
    int ret = 0;
    struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_PP_1_1_4, 1),
                                      SPI_MEM_OP_ADDR(3, addr, 1),
                                      SPI_MEM_OP_NO_DUMMY,
                                      SPI_MEM_OP_DATA_OUT(len, buf, 4));
    ret = w25q1258jw_write_enable(dev);
    if (ret)
    {
        dev_err(dev, "Write enable failed: %d\n", ret);
        return ret;
    }

    ret = flexspidev_exec_op(dev, &op);
    if (ret)
    {
        dev_err(dev, "Page program failed: %d\n", ret);
        return ret;
    }

    ret = w25q128jw_get_busy_status(dev);

    return ret;
}

int w25q1258jw_write_data(struct device *dev, u32 addr, const void *buf, size_t len)
{
    int ret = 0;
    u8 *src = (u8 *)buf;
    u16 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;
    u16 NumByteToWriteRest = len;
    
    Addr = addr % FLASH_PAGE_SIZE;
    count = FLASH_PAGE_SIZE - Addr;

    NumByteToWriteRest = (len > count) ? (len - count) : len;
    NumOfPage = NumByteToWriteRest / FLASH_PAGE_SIZE;
    NumOfSingle = NumByteToWriteRest % FLASH_PAGE_SIZE;

    if (count != 0 && len > count)
    {
        ret = w25q128jw_page_program(dev, addr, src, count);
        if (ret)
        {
            dev_err(dev, "1: Page program failed: %d\n", ret);
            return ret;
        }

        addr += count;
        src += count;
    }

    if (NumOfPage == 0)
    {
        ret = w25q128jw_page_program(dev, addr, src, NumOfSingle);
        if (ret)
        {
            dev_err(dev, "2: Page program failed: %d\n", ret);
            return ret;
        }
    }
    else
    {
        while (NumOfPage--)
        {
            ret = w25q128jw_page_program(dev, addr, src, FLASH_PAGE_SIZE);
            if (ret)
            {
                dev_err(dev, "3: Page program failed: %d\n", ret);
                return ret;
            }

            addr += FLASH_PAGE_SIZE;
            src += FLASH_PAGE_SIZE;
        }
        
        if (NumOfSingle != 0)
        {
            ret = w25q128jw_page_program(dev, addr, src, NumOfSingle);
            if (ret)
            {
                dev_err(dev, "4: Page program failed: %d\n", ret);
                return ret;
            }
        }
    }

    return ret;
}
