
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#define DRIVER_NAME "bmp180_driver"
#define BMP180_ADDR 0x77
#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_RESULT 0xF6
#define BMP180_CMD_READ_TEMP 0x2E

static struct i2c_client *bmp180_client;

static int bmp180_read_temperature(struct i2c_client *client)
{
    int ret;
    u8 msb, lsb;
    s16 temp_raw;

    ret = i2c_smbus_write_byte_data(client, BMP180_REG_CONTROL, BMP180_CMD_READ_TEMP);
    if (ret < 0) {
        printk(KERN_ERR "Failed to write temperature command\n");
        return ret;
    }

    msleep(5); // Wait for conversion

    msb = i2c_smbus_read_byte_data(client, BMP180_REG_RESULT);
    lsb = i2c_smbus_read_byte_data(client, BMP180_REG_RESULT + 1);
    temp_raw = (msb << 8) | lsb;

    printk(KERN_INFO "BMP180 raw temperature: %d\n", temp_raw);
    return 0;
}

static int bmp180_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    bmp180_client = client;
    printk(KERN_INFO "BMP180 device detected\n");
    return bmp180_read_temperature(client);
}

static void bmp180_remove(struct i2c_client *client)
{
    printk(KERN_INFO "BMP180 driver removed\n");
}

static const struct i2c_device_id bmp180_id[] = {
    { "bmp180", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, bmp180_id);

static struct i2c_driver bmp180_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
    },
    .probe = bmp180_probe,
    .remove = bmp180_remove,
    .id_table = bmp180_id,
};

static int __init bmp180_init(void)
{
    return i2c_add_driver(&bmp180_driver);
}

static void __exit bmp180_exit(void)
{
    i2c_del_driver(&bmp180_driver);
}

module_init(bmp180_init);
module_exit(bmp180_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PMD22146098");
MODULE_DESCRIPTION("BMP180 I2C Driver");
