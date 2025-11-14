/*
 * @Author: Qwerty 263690179@qq.com
 * @Date: 2025-11-12 13:52:42
 * @LastEditors: Qwerty 263690179@qq.com
 * @LastEditTime: 2025-11-13 10:27:49
 * @FilePath: \flexdriver\flexspi_chrdev.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <linux/list.h>      // Linked list definitions and functions
#include <linux/sched.h>     // `Current` global variable for current task
#include <linux/device.h>    // Device and class creation functions
#include <linux/cdev.h>      // Character device functions
#include <linux/ioctl.h>     // IOCTL macros and definitions
#include <linux/fs.h>        // File operations and file types
#include <linux/mm.h>        // Memory types and remapping functions
#include <linux/uaccess.h>   // Userspace memory access functions
#include <linux/slab.h>      // Kernel allocation functions
#include <linux/errno.h>     // Linux error codes
#include <linux/of_device.h> // Device tree device related functions
#include <linux/dma-mapping.h>
#include <linux/iommu.h>
#include <linux/pagemap.h>
#include <linux/dma-direct.h>
#include <linux/memremap.h>
#include <linux/version.h>

#include <linux/dma-buf.h>     // DMA shared buffers interface
#include <linux/scatterlist.h> //

#include "flexspidev.h"
#include "w25q128jw.h"

static struct flexspidev *flexspi_dev;

static int flexspi_dev_open(struct inode *inode, struct file *file)
{
    file->private_data = flexspi_dev;
    dev_info(flexspi_dev->dev, "open operation invoked \n");
    return 0;
}

static int flexspi_dev_release(struct inode *inode, struct file *file)
{
    struct flexspidev *dev = file->private_data;
    dev_info(dev->dev, "close operation invoked\n");
    file->private_data = NULL;
    return 0;
}

static long flexspi_dev_ioctl(struct file *file, u32 cmd, unsigned long arg)
{
    int ret = 0;
    struct flexspidev *dev = file->private_data;

    switch (cmd)
    {
    case FLEX_FLASH_READ_ID:
    {
        u8 buf[3] = {0};
        struct flash_ops_t *ops = kzalloc(sizeof(struct flash_ops_t), GFP_KERNEL);
        if (copy_from_user((struct flash_ops_t *)ops, (struct flash_ops_t *)arg, sizeof(struct flash_ops_t)))
        {
            kfree(ops);
            dev_info(dev->dev, "copy_from_user 1 failed\n");
            return -EACCES;
        }

        ret = w25q128jw_read_ID(dev->dev, buf);
        if (ret)
        {
            dev_err(dev->dev, "Failed to w25q128jw_read_ID : %d \n", ret);
        }

        if (copy_to_user((void *)ops->buf, (void *)buf, 3))
        {
            kfree(ops);
            printk("copy_to_user 2 failed\n");
            return -EACCES;
        }
        kfree(ops);
        break;
    }
    case FLEX_FLASH_READ_DATA:
    {
        uint8_t *readAddr, *origin_addr;
        int ahb_buf_size = dev->fspi->devtype_data->ahb_buf_size;
        int read_size;
        int NumByteToReadRest = 0;
        int Addr = 0;
        struct flash_ops_t *ops = kzalloc(sizeof(struct flash_ops_t), GFP_KERNEL);

        if (copy_from_user((struct flash_ops_t *)ops, (struct flash_ops_t *)arg, sizeof(struct flash_ops_t)))
        {
            kfree(ops);
            dev_info(dev->dev, "copy_from_user 1 failed\n");
            return -EACCES;
        }

        NumByteToReadRest = ops->len;
        Addr = ops->addr;
        origin_addr = (uint8_t *)kzalloc(ops->len, GFP_KERNEL);
        readAddr = origin_addr;

        while (NumByteToReadRest > 0)
        {
            read_size = NumByteToReadRest > ahb_buf_size ? ahb_buf_size : NumByteToReadRest;
            printk("Addr = %d, read_size = %d, NumByteToReadRest = %d\n", Addr, read_size, NumByteToReadRest);
            ret = w25q128jw_read_data(dev->dev, Addr, (void *)readAddr, read_size);
            if (ret)
            {
                dev_err(dev->dev, "Failed to flexspidev_read_data : %d \n", ret);
            }
            NumByteToReadRest -= read_size;
            Addr += read_size;
            readAddr += read_size;
        };

        if (copy_to_user((void *)ops->buf, (void *)origin_addr, ops->len))
        {
            ret = -EACCES;
        }

        kfree(origin_addr);
        kfree(ops);
        break;
    }
    case FLEX_FLASH_WRITE_DATA:
    {
        uint8_t *writebuf;
        struct flash_ops_t *ops = kzalloc(sizeof(struct flash_ops_t), GFP_KERNEL);
        if (copy_from_user((struct flash_ops_t *)ops, (struct flash_ops_t *)arg, sizeof(struct flash_ops_t)))
        {
            kfree(ops);
            dev_info(dev->dev, "copy_from_user 1 failed\n");
            return -EACCES;
        }

        if (ops->len > 0)
            writebuf = kzalloc(ops->len, GFP_KERNEL);

        if (copy_from_user((void *)writebuf, (void *)ops->buf, ops->len))
        {
            kfree(ops);
            dev_info(dev->dev, "copy_from_user 2 failed\n");
            return -EACCES;
        }

        ret = w25q128jw_write_data(dev->dev, ops->addr, (void *)writebuf, ops->len);
        if (ret)
        {
            dev_err(dev->dev, "Failed to w25q128jw_write_data : %d \n", ret);
        }

        kfree(writebuf);
        break;
    }
    case FLEX_FLASH_ERASE_SECTOR:
    {
        struct flash_ops_t *ops = kzalloc(sizeof(struct flash_ops_t), GFP_KERNEL);
        if (copy_from_user((struct flash_ops_t *)ops, (struct flash_ops_t *)arg, sizeof(struct flash_ops_t)))
        {
            kfree(ops);
            dev_info(dev->dev, "copy_from_user 1 failed\n");
            return -EACCES;
        }

        ret = w25q128jw_erase_sector(dev->dev, ops->addr);
        if (ret)
        {
            dev_err(dev->dev, "Failed to w25q128jw_write_data : %d \n", ret);
        }

        break;
    }
    case FLEX_SET_CLK:
    {
        int flexspidev_set_speed(struct flexspidev *flexdev, u32 speed_hz);
        uint32_t clk = 0;
        if (copy_from_user((void *)&clk, (void *)arg, 4))
        {
            dev_info(dev->dev, "copy_from_user 1 failed\n");
            return -EACCES;
        }

        if(clk == 0 && clk > 133000000)
            return -EACCES;
        
        ret = flexspidev_set_speed(dev, clk);
        break;
    }
    default:
    {
        return -EINVAL;
    }
    }

    return ret;
}

static ssize_t flexspi_dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    return 0;
}

static ssize_t flexspi_dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    return 0;
}

static struct file_operations flexspi_dev_fops = {
    .owner = THIS_MODULE,
    .open = flexspi_dev_open,
    .release = flexspi_dev_release,
    .unlocked_ioctl = flexspi_dev_ioctl,
    .read = flexspi_dev_read,
    .write = flexspi_dev_write,
};

int flexspi_chrdev_init(struct flexspidev *dev)
{
    int count = 1, ret = 0;
    flexspi_dev = dev;
    dev->chrdev.chrdev_name = FLEXSPI_DEV_NAME;
    /* Request dynamic allocation of a device major number */
    if (alloc_chrdev_region(&dev->chrdev.dev_num, 0, count, FLEXSPI_DEV_NAME) < 0)
    {
        dev_info(dev->dev, "failed to reserve major/minor range\n");
        return -1;
    }

    dev_info(dev->dev, "%s: Got Major %d\n", FLEXSPI_DEV_NAME, MAJOR(dev->chrdev.dev_num));

    if (!(dev->chrdev.cdev = cdev_alloc()))
    {
        dev_info(dev->dev, "cdev_alloc() failed\n");
        unregister_chrdev_region(dev->chrdev.dev_num, count);
        return -1;
    }

    /* Connect the file operations with cdev*/
    cdev_init(dev->chrdev.cdev, &flexspi_dev_fops);

    /* Connect the majot/mionr number to the cdev */
    ret = cdev_add(dev->chrdev.cdev, dev->chrdev.dev_num, count);
    if (ret < 0)
    {
        dev_info(dev->dev, "Error registering device driver\n");
        cdev_del(dev->chrdev.cdev);
        unregister_chrdev_region(dev->chrdev.dev_num, count);
        return -1;
    }
    /* Populate sysfs entry */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
    dev->chrdev.dev_class = class_create("VIRTUAL");
#else
    dev->chrdev.dev_class = class_create(THIS_MODULE, "VIRTUAL");
#endif

    /* Send uevents to udev, So it will create /dev nodes */
    device_create(dev->chrdev.dev_class, NULL, dev->chrdev.dev_num, "%s", dev->chrdev.chrdev_name);

    dev_info(dev->dev, "Successfully created /dev/%s\n", FLEXSPI_DEV_NAME);
    dev_info(dev->dev, "Device Registered: %s\n", dev->chrdev.chrdev_name);
    dev_info(dev->dev, "Major number = %d, Minor number = %d\n", MAJOR(dev->chrdev.dev_num), MINOR(dev->chrdev.dev_num));

    return 0;
}

void flexspi_chrdev_cleanup(struct flexspidev *dev)
{
    cdev_del(dev->chrdev.cdev);
    unregister_chrdev_region(dev->chrdev.dev_num, 1);
    device_destroy(dev->chrdev.dev_class, dev->chrdev.dev_num);
    class_destroy(dev->chrdev.dev_class);
}

