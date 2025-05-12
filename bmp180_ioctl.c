
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "bmp180"
#define CLASS_NAME "bmp_class"

#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_RESULT 0xF6
#define BMP180_CMD_READ_TEMP 0x2E

#define BMP180_IOCTL_MAGIC 'b'
#define BMP180_IOCTL_READ_TEMP _IOR(BMP180_IOCTL_MAGIC, 1, int)

static struct i2c_client *bmp180_client;
static struct class* bmp180_class = NULL;
static struct device* bmp180_device = NULL;
static int major_number;

static int bmp180_read_temp(struct i2c_client *client)
{
    int ret;
    u8 msb, lsb;
    s16 temp_raw;

    ret = i2c_smbus_write_byte_data(client, BMP180_REG_CONTROL, BMP180_CMD_READ_TEMP);
    if (ret < 0)
        return ret;

    msleep(5);

    msb = i2c_smbus_read_byte_data(client, BMP180_REG_RESULT);
    lsb = i2c_smbus_read_byte_data(client, BMP180_REG_RESULT + 1);
    temp_raw = (msb << 8) | lsb;

    return temp_raw;
}

static long bmp180_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int data;

    switch (cmd) {
        case BMP180_IOCTL_READ_TEMP:
            data = bmp180_read_temp(bmp180_client);
            break;
        default:
            return -EINVAL;
    }

    if (copy_to_user((int __user *)arg, &data, sizeof(data)))
        return -EFAULT;

    return 0;
}

static int bmp180_open(struct inode *inodep, struct file *filep)
{
    return 0;
}

static int bmp180_release(struct inode *inodep, struct file *filep)
{
    return 0;
}

static struct file_operations fops = {
    .open = bmp180_open,
    .unlocked_ioctl = bmp180_ioctl,
    .release = bmp180_release,
};

static int bmp180_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    bmp180_client = client;

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    bmp180_class = class_create(THIS_MODULE, CLASS_NAME);
    bmp180_device = device_create(bmp180_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);

    return 0;
}

static void bmp180_remove(struct i2c_client *client)
{
    device_destroy(bmp180_class, MKDEV(major_number, 0));
    class_unregister(bmp180_class);
    class_destroy(bmp180_class);
    unregister_chrdev(major_number, DEVICE_NAME);
}

static const struct of_device_id bmp180_of_match[] = {
    { .compatible = "bosch,bmp180", },
    { }
};
MODULE_DEVICE_TABLE(of, bmp180_of_match);

static struct i2c_driver bmp180_ioctl_driver = {
    .driver = {
        .name = "bmp180_ioctl_driver",
        .of_match_table = of_match_ptr(bmp180_of_match),
    },
    .probe = bmp180_probe,
    .remove = bmp180_remove,
};

static int __init bmp180_ioctl_init(void)
{
    return i2c_add_driver(&bmp180_ioctl_driver);
}

static void __exit bmp180_ioctl_exit(void)
{
    i2c_del_driver(&bmp180_ioctl_driver);
}

module_init(bmp180_ioctl_init);
module_exit(bmp180_ioctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PMD22146098");
MODULE_DESCRIPTION("BMP180 IOCTL Driver");
