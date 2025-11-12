#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi-mem.h>
#include <linux/mtd/spi-nor.h>

#include "flexspidev.h"
#include "w25q128jw.h"

#ifdef CONFIG_SPI_MEM
extern int spi_mem_exec_op(struct spi_mem *mem, const struct spi_mem_op *op);
extern bool spi_mem_supports_op(struct spi_mem *mem, const struct spi_mem_op *op);
#else
static inline int spi_mem_exec_op(struct spi_mem *mem, const struct spi_mem_op *op)
{
    dev_err(dev, "CONFIG_SPI_MEM undefine\n");
    return -ENOTSUPP;
}

static inline bool spi_mem_supports_op(struct spi_mem *mem, const struct spi_mem_op *op)
{
    dev_err(dev, "CONFIG_SPI_MEM undefine\n");
    return false;
}
#endif

int flexspidev_exec_op(struct device *dev, const struct spi_mem_op *op)
{
    struct flexspidev *flexdev = dev_get_drvdata(dev);
    struct spi_mem mem;

    if (!flexdev || !op || !flexdev->spi)
    {
        dev_err(dev, "Invalid parameters\n");
        return -EINVAL;
    }

    /* 构建spi_mem结构 */
    mem.spi = flexdev->spi;
    return spi_mem_exec_op(&mem, op);
}

bool flexspidev_supports_op(struct device *dev, const struct spi_mem_op *op)
{
    struct flexspidev *flexdev = dev_get_drvdata(dev);
    struct spi_mem mem;

    if (!flexdev || !op || !flexdev->spi)
    {
        dev_err(dev, "Invalid parameters\n");
        return false;
    }

    /* 构建spi_mem结构 */
    mem.spi = flexdev->spi;

    return spi_mem_supports_op(&mem, op);
}


/**
 * @description: 设置SPI频率，需在flexspidev_init_controller_info函数调用后使用
 * @param {flexspidev} *flexdev
 * @param {u32} speed_hz
 * @return {*}
 */
int flexspidev_set_speed(struct flexspidev *flexdev, u32 speed_hz)
{
    if (!flexdev || !flexdev->spi) {
        dev_err(flexdev->dev, "Invalid device\n");
        return -EINVAL;
    }
    
    flexdev->spi->max_speed_hz = speed_hz;
    flexdev->fspi->selected = -1;
    
    dev_info(flexdev->dev, "SPI speed set to %u Hz\n", speed_hz);
    return 0;
}

static int flexspidev_init_controller_info(struct flexspidev *flexdev)
{
    struct spi_controller *ctlr = flexdev->spi->controller;

    /* 获取 FlexSPI 控制器私有数据 */
    flexdev->fspi = spi_controller_get_devdata(ctlr);
    if (!flexdev->fspi) {
        dev_err(flexdev->dev, "Failed to get FSPI controller data\n");
        return -ENODEV;
    }

    dev_info(flexdev->dev, "  FlexSPI controller initialized:\n");
    return 0;
}

static int flexspidev_probe(struct spi_device *spi)
{
    int ret = 0;
    u8 id_buf[8] = {0};
    u32 *rdbuf = (u32 *)kzalloc(4096, GFP_KERNEL);
    u32 *wrbuf = (u32 *)kzalloc(4096, GFP_KERNEL);
    struct device *dev = &spi->dev;
    struct flexspidev *flexdev;

    int i = 0;
    for(i = 0; i < 1024; i++)
    {
        wrbuf[i] = i + 10;
    }

    if (!spi->controller->mem_ops)
    {
        dev_err(dev, "SPI controller doesn't support memory operations\n");
        return -ENOTSUPP;
    }

    flexdev = devm_kzalloc(dev, sizeof(*flexdev), GFP_KERNEL);
    if (!flexdev)
        return -ENOMEM;

    flexdev->dev = dev;
    flexdev->spi = spi;

    spi_set_drvdata(spi, flexdev);
    dev_set_drvdata(dev, flexdev);
    flexspidev_init_controller_info(flexdev);

    flexspidev_set_speed(flexdev, 20000000);

    dev_info(dev, "flexspidev probed successfully\n");
    
    // 读ID
    ret = w25q128jw_read_ID(dev,id_buf);
    if (ret) {
        dev_err(dev, "Failed to w25q128jw_read_ID : %d \n", ret);
    }

    // // 擦除
    // ret = w25q1258jw_write_enable(dev); 
    // if (ret) {
    //     dev_err(dev, "Failed to w25q1258jw_write_enable : %d \n", ret);
    // }

    // ret = w25q1258jw_erase_sector(dev, 4096);
    // if (ret) {
    //     dev_err(dev, "Failed to w25q1258jw_erase_sector : %d \n", ret);
    // }

    // ret = w25q128jw_get_busy_status(dev);
    // if (ret) {
    //     dev_err(dev, "Failed to w25q128jw_get_status erase : %d \n", ret);
    // }

    // // 写入
    // ret = w25q1258jw_write_data(dev, 4096, (void *)wrbuf, 1024);
    // if (ret) {
    //     dev_err(dev, "Failed to w25q1258jw_write_enable 1: %d \n", ret);
    // }

    // ret = w25q1258jw_write_data(dev, 5120, (void *)(wrbuf + 256), 1024);
    // if (ret) {
    //     dev_err(dev, "Failed to w25q1258jw_write_enable 2: %d \n", ret);
    // }

    // ret = w25q1258jw_write_data(dev, 6144, (void *)(wrbuf + 512), 1024);
    // if (ret) {
    //     dev_err(dev, "Failed to w25q1258jw_write_enable 3: %d \n", ret);
    // }

    // ret = w25q1258jw_write_data(dev, 7168, (void *)(wrbuf + 768), 1024);
    // if (ret) {
    //     dev_err(dev, "Failed to w25q1258jw_write_enable 4: %d \n", ret);
    // }

    // 读数据
    ret = w25q128jw_read_data(dev, 4096 , rdbuf, 2048);
    if (ret) {
        dev_err(dev, "Failed to flexspidev_read_data : %d \n", ret);
    }

    ret = w25q128jw_read_data(dev, 4096 + 2048 , rdbuf + 512, 2048);
    if (ret) {
        dev_err(dev, "Failed to flexspidev_read_data : %d \n", ret);
    }

    for(i = 0; i < 1024; i++)
    {
        if(wrbuf[i] != rdbuf[i])
        {
            dev_err(dev, "Data mismatch at index %d: %x != %x\n", i, wrbuf[i], rdbuf[i]);
            break;
        }
    }

    if(i == 1024)
    {
        dev_info(dev, "Data match\n");
    }
    
    kfree(wrbuf);
    kfree(rdbuf);

    return 0;
}

static int flexspidev_remove(struct spi_device *spi)
{
    dev_info(&spi->dev, "flexspidev removed\n");
    return 0;
}

static const struct spi_device_id flexspidev_id[] = {
    {"myflexdriver", 0},
    {}};
MODULE_DEVICE_TABLE(spi, flexspidev_id);

static const struct of_device_id flexspidev_dt_ids[] = {
    {
        .compatible = "myflexdriver",
    },
    {/* sentinel */}};
MODULE_DEVICE_TABLE(of, flexspidev_dt_ids);

static struct spi_driver flexspidev_driver = {
    .driver = {
        .name = "flexspidev",
        .of_match_table = flexspidev_dt_ids,
    },
    .id_table = flexspidev_id,
    .probe = flexspidev_probe,
    .remove = flexspidev_remove,
};

module_spi_driver(flexspidev_driver);

MODULE_AUTHOR("Qwerty");
MODULE_DESCRIPTION("Custom FlexSPI Driver");
MODULE_LICENSE("GPL v2");