// SPDX-License-Identifier: GPL-2.0-only
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/string.h>

static dev_t lkcamp_dev; // Holds the major and minor number for our driver
static struct cdev lkcamp_cdev; // Char device. Holds fops and device number

// Possible states for the driver
enum driver_state {
    STATUS_OFF = 0,
    STATUS_ON  = 1,
};

static enum driver_state status = STATUS_OFF;
static const char *status_strings[] = {"OFF\n", "ON\n"};

static ssize_t lkcamp_read(struct file *file, char __user *buf, size_t size,
               loff_t *ppos)
{
    // Return the string corresponding to the current driver state
    return simple_read_from_buffer(buf, size, ppos, status_strings[status],
                       strlen(status_strings[status]));
}

static ssize_t lkcamp_write(struct file *file, const char __user *buf,
                size_t size, loff_t *ppos)
{
    char value;

    // Copy the first character written to this device to 'value'
    if (copy_from_user(&value, buf, 1))
        return -EFAULT; // Something went very wrong

    if (value == '0')
        status = STATUS_OFF;
    else if (value == '1')
        status = STATUS_ON;
    else
        return -EINVAL;

    return 1; // We only read one character from the written string
}

// Define the functions that implement our file operations
static struct file_operations lkcamp_fops =
{
    .read = lkcamp_read,
    .write = lkcamp_write,
};

static int __init lkcamp_init(void)
{
    int ret;

    // Allocate a major and a minor number
    ret = alloc_chrdev_region(&lkcamp_dev, 0, 1, "lkcamp");
    if (ret)
        pr_err("Failed to allocate device number\n");

    // Initialize our character device structure
    cdev_init(&lkcamp_cdev, &lkcamp_fops);

    // Register our character device to our device number
    ret = cdev_add(&lkcamp_cdev, lkcamp_dev, 1);
    if (ret)
        pr_err("Char device registration failed\n");

    pr_info("LKCAMP driver initialized!\n");

    return 0;
}

static void __exit lkcamp_exit(void)
{
    // Clean up our mess
    cdev_del(&lkcamp_cdev);
    unregister_chrdev_region(lkcamp_dev, 1);

    pr_info("LKCAMP driver exiting!\n");
}

module_init(lkcamp_init); // Register our functions so they get called when our
module_exit(lkcamp_exit); // module is loaded and unloaded

MODULE_AUTHOR("LKCAMP");
MODULE_DESCRIPTION("LKCAMP's incredibly useful char driver");
MODULE_LICENSE("GPL");
