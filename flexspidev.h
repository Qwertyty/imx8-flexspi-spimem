/*
 * @Author: Qwerty 263690179@qq.com
 * @Date: 2025-11-11 11:10:59
 * @LastEditors: Qwerty 263690179@qq.com
 * @LastEditTime: 2025-11-12 11:19:18
 * @FilePath: \flexdriver\flexspidev.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
// flexspidev.h
#ifndef FLEXSPIDEV_H
#define FLEXSPIDEV_H

#include <linux/device.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_qos.h>
#include <linux/sizes.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi-mem.h>

#include <linux/pm_runtime.h>

struct nxp_fspi
{
    void __iomem *iobase;
    void __iomem *ahb_addr;
    u32 memmap_phy;
    u32 memmap_phy_size;
    u32 memmap_start;
    u32 memmap_len;
    u32 dll_slvdly;
    bool individual_mode;
    struct clk *clk, *clk_en;
    struct device *dev;
    struct completion c;
    struct nxp_fspi_devtype_data *devtype_data;
    struct mutex lock;
    struct pm_qos_request pm_qos_req;
    int selected;
#define FSPI_INITILIZED (1 << 0)
#define FSPI_RXCLKSRC_3 (1 << 1)
#define FSPI_DTR_ODD_ADDR (1 << 2)
    int flags;
};

struct flexspidev
{
    struct device *dev;
    struct spi_device *spi;
    struct nxp_fspi *fspi; // FlexSPI 控制器私有数据
};

int flexspidev_exec_op(struct device *dev, const struct spi_mem_op *op);
bool flexspidev_supports_op(struct device *dev, const struct spi_mem_op *op);

int flexspidev_send_command(struct device *dev, u8 opcode, u32 addr);
int flexspidev_read_data(struct device *dev, u8 opcode, u32 addr,
                         void *buf, size_t len, u8 dummy_cycles);
int flexspidev_write_data(struct device *dev, u8 opcode, u32 addr,
                          const void *buf, size_t len);

#endif