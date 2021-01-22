#include <linux/module.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

static int i2c_write_handler(struct device *, void *);
static ssize_t i2c_bus_show(struct device *, struct device_attribute *, char *);
static ssize_t i2c_bus_store(struct device *, struct device_attribute *, const char *, size_t);
static ssize_t i2c_address_show(struct device *, struct device_attribute *, char *);
static ssize_t i2c_address_store(struct device *, struct device_attribute *, const char *, size_t);
static ssize_t i2c_data_show(struct device *, struct device_attribute *, char *);
static ssize_t i2c_data_store(struct device *, struct device_attribute *, const char *, size_t);
static int driver_probe(struct platform_device *);
static int driver_remove(struct platform_device *);
static int device_fops_open(struct inode *, struct file *);
static int device_fops_release(struct inode *, struct file *);
static ssize_t device_fops_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_fops_write(struct file *, const char __user *, size_t, loff_t *);
static int init_i2c_proxy(void);
static void exit_i2c_proxy(void);
static int init_driver(void);
static int alloc_device(void);
static int init_cdev(void);
static int init_device(void);
static void exit_driver(void);
static void dealloc_device(void);
static void exit_device(void);
static void exit_cdev(void);

///////////////////////////////////////

MODULE_AUTHOR("Marco Satti");
MODULE_LICENSE("GPL");
module_init(init_i2c_proxy);
module_exit(exit_i2c_proxy);

///////////////////////////////////////

struct i2c_proxy_device 
{
    dev_t chrdev_region;
    struct cdev cdev;
    struct platform_device platform_device;
    int address;
    int bus;
    int data;
};

struct i2c_proxy_driver
{
    struct platform_driver platform_driver;
};

///////////////////////////////////////

static struct i2c_proxy_device *device;

static struct i2c_proxy_driver driver = {
    .platform_driver = {
        .driver	= {
            .name = "i2c_proxy",
            .owner = THIS_MODULE,
        },
        .probe = driver_probe,
        .remove = driver_remove
    },
};

static struct file_operations device_fops = 
{
    .owner = THIS_MODULE,
    .open = device_fops_open,
    .release = device_fops_release,
    .read = device_fops_read,
    .write = device_fops_write,
};

static DEVICE_ATTR(i2c_bus, 0644, i2c_bus_show, i2c_bus_store);
static DEVICE_ATTR(i2c_address, 0644, i2c_address_show, i2c_address_store);
static DEVICE_ATTR(i2c_data, 0644, i2c_data_show, i2c_data_store);

static struct attribute *device_attrs[] = {
      &dev_attr_i2c_address.attr,
      &dev_attr_i2c_bus.attr,
      &dev_attr_i2c_data.attr,
      NULL,
};

static struct attribute_group device_attr_group = {
      .attrs = device_attrs,
};

static const struct attribute_group *device_attr_groups[] = {
      &device_attr_group,
      NULL,
};

///////////////////////////////////////

static ssize_t i2c_bus_show(struct device *dev, struct device_attribute *attr, char *buf) {
    struct platform_device *this_platform_device = container_of(dev, struct platform_device, dev);
    struct i2c_proxy_device *this_device = container_of(this_platform_device, struct i2c_proxy_device, platform_device);
    return sprintf(buf, "%d\n", this_device->bus);
}

static ssize_t i2c_bus_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    struct platform_device *this_platform_device = container_of(dev, struct platform_device, dev);
    struct i2c_proxy_device *this_device = container_of(this_platform_device, struct i2c_proxy_device, platform_device);
    sscanf(buf, "%d", &this_device->bus);
    return count;
}

static ssize_t i2c_address_show(struct device *dev, struct device_attribute *attr, char *buf) {
    struct platform_device *this_platform_device = container_of(dev, struct platform_device, dev);
    struct i2c_proxy_device *this_device = container_of(this_platform_device, struct i2c_proxy_device, platform_device);
    return sprintf(buf, "0x%X\n", this_device->address);
}

static ssize_t i2c_address_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    struct platform_device *this_platform_device = container_of(dev, struct platform_device, dev);
    struct i2c_proxy_device *this_device = container_of(this_platform_device, struct i2c_proxy_device, platform_device);
    sscanf(buf, "0x%X", &this_device->address);
    return count;
}

static ssize_t i2c_data_show(struct device *dev, struct device_attribute *attr, char *buf) {
    struct platform_device *this_platform_device = container_of(dev, struct platform_device, dev);
    struct i2c_proxy_device *this_device = container_of(this_platform_device, struct i2c_proxy_device, platform_device);
    return sprintf(buf, "0x%X\n", this_device->data);
}

static ssize_t i2c_data_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    struct platform_device *this_platform_device = container_of(dev, struct platform_device, dev);
    struct i2c_proxy_device *this_device = container_of(this_platform_device, struct i2c_proxy_device, platform_device);
    sscanf(buf, "0x%X", &this_device->data);
    return count;
}

static int driver_probe(struct platform_device *platform_device) {
    return strcmp(platform_device->name, "i2c_proxy") != 0;
}

static int driver_remove(struct platform_device *platform_device) {
    return 0;
}

static int device_fops_open(struct inode *inode, struct file *file) {
    //struct i2c_proxy_device *this_device = container_of(inode->i_cdev, struct i2c_proxy_device, cdev);
	printk(KERN_INFO "i2c_proxy: Device file opened\n");
    return 0;
}

static int device_fops_release(struct inode *inode, struct file *file) {
    //struct i2c_proxy_device *this_device = container_of(inode->i_cdev, struct i2c_proxy_device, cdev);
	printk(KERN_INFO "i2c_proxy: Device file released\n");
    return 0;
}

static ssize_t device_fops_read(struct file *file, char __user *buf, size_t len, loff_t *pos) {
    //struct i2c_proxy_device *this_device = container_of(file->f_inode->i_cdev, struct i2c_proxy_device, cdev);
	printk(KERN_INFO "i2c_proxy: Device file read\n");
    return -EINVAL;
}

static ssize_t device_fops_write(struct file *file, const char __user *buf, size_t len, loff_t *pos) {
    struct i2c_proxy_device *this_device = container_of(file->f_inode->i_cdev, struct i2c_proxy_device, cdev);
	printk(KERN_INFO "i2c_proxy: Device file write\n");

    int status = i2c_for_each_dev((void *)this_device, i2c_write_handler);
    switch (status) {
    case 0:
        printk(KERN_ERR "i2c_proxy: No adapter found\n");
        return -EIO;
    case 1:
        break;
    default:
        printk(KERN_ERR "i2c_proxy: Error sending data: %d\n", status);
        return -EIO;
    }

    return (ssize_t)len;
}

static int i2c_write_handler(struct device *dev, void *data) {
    struct i2c_proxy_device *this_device = (struct i2c_proxy_device *)data;
    
    struct i2c_adapter *adapter = i2c_verify_adapter(dev);
    if (!adapter) {
        return 0;
    }

    // Verify its the right i2c bus.
    if (i2c_adapter_id(adapter) != this_device->bus) {
        return 0;
    }
	printk(KERN_INFO "i2c_proxy: Found adapter; attempting to send data using i2c bus %d at address 0x%X\n", this_device->bus, this_device->address);

    // Send a packet.
    struct i2c_msg message = {
        .flags = 0,
        .addr = (u16)this_device->address,
        .len = 1,
        .buf = (u8 *)&this_device->data
    };

    int status = i2c_transfer(adapter, &message, 1);
	if (status < 0) {
        return status;
    }
    printk(KERN_INFO "i2c_proxy: Sent data ok!\n");

    // Stop iterating.
    return 1;
}

///////////////////////////////////////

static int init_i2c_proxy(void) {
    int rc;

	printk(KERN_INFO "i2c_proxy: Initializing\n");

    if ((rc = init_driver()) < 0) {
        return rc;
    }
	printk(KERN_INFO "i2c_proxy: Initialized driver\n");

    if ((rc = alloc_device()) < 0) {
        return rc;
    }
	printk(KERN_INFO "i2c_proxy: Allocated device\n");

    if ((rc = init_device()) < 0) {
        return rc;
    }
	printk(KERN_INFO "i2c_proxy: Initialized device\n");

    if ((rc = init_cdev()) < 0) {
        return rc;
    }
	printk(KERN_INFO "i2c_proxy: Initialized cdev\n");
    
	return 0;
}

static void exit_i2c_proxy(void) {
	printk(KERN_INFO "i2c_proxy: Exiting\n");

    exit_cdev();
	printk(KERN_INFO "i2c_proxy: Exited cdev\n");

    exit_device();
	printk(KERN_INFO "i2c_proxy: Exited device\n");

    dealloc_device();
	printk(KERN_INFO "i2c_proxy: Deallocated device\n");

    exit_driver();
	printk(KERN_INFO "i2c_proxy: Exited driver\n");
}

///////////////////////////////////////

static int init_driver(void) {
    int rc;
    
    rc = platform_driver_register(&driver.platform_driver);
    if (rc < 0) {
        printk(KERN_ERR "i2c_proxy: Can't register platform driver\n");
        return rc;
    }

    return 0;
}

static int alloc_device(void) {
    device = kmalloc(sizeof(struct i2c_proxy_device), GFP_KERNEL);
    if (!device) {
        printk(KERN_ERR "i2c_proxy: Can't allocate device memory\n");
        return -ENOMEM;
    }
    memset(device, 0, sizeof(struct i2c_proxy_device));
    return 0;
}

static int init_device(void) {
    int rc;
        
    rc = alloc_chrdev_region(&device->chrdev_region, 0, 1, "i2c_proxy");
    if (rc < 0) {
        printk(KERN_ERR "i2c_proxy: Can't allocate chrdev region\n");
        return rc;
    }
	printk(KERN_INFO "i2c_proxy: Registered chrdev region: major = %d, minor = %d\n", MAJOR(device->chrdev_region), MINOR(device->chrdev_region));

    device->address = 0x20;
    device->bus = 2;
    device->data = 0x42;
    device->platform_device.name = "i2c_proxy";
    device->platform_device.id = PLATFORM_DEVID_NONE;
    device->platform_device.dev.devt = device->chrdev_region;
    device->platform_device.dev.groups = device_attr_groups;

    rc = platform_device_register(&device->platform_device);
    if (rc < 0) {
        printk(KERN_ERR "i2c_proxy: Can't register platform device\n");
        return rc;
    }

    return 0;
}

static int init_cdev(void) {
    int rc;

    cdev_init(&device->cdev, &device_fops);
    device->cdev.owner = THIS_MODULE;

    rc = cdev_add(&device->cdev, device->chrdev_region, 1);
    if (rc) {
        printk(KERN_ERR "i2c_proxy: Error adding cdev");
        return rc;
    }
	printk(KERN_INFO "i2c_proxy: Added cdev\n");

    return 0;
}

///////////////////////////////////////

static void exit_driver(void) {
    platform_driver_unregister(&driver.platform_driver);
	printk(KERN_INFO "i2c_proxy: Unregistered platform driver\n");
}

static void dealloc_device(void) {
    kfree(device);
	printk(KERN_INFO "i2c_proxy: Free'd device\n");
}

static void exit_device(void) {
    platform_device_unregister(&device->platform_device);
	printk(KERN_INFO "i2c_proxy: Unregistered platform device\n");

	unregister_chrdev_region(device->chrdev_region, 1);
	printk(KERN_INFO "i2c_proxy: Unregistered chrdev region\n");
}

static void exit_cdev(void) {
    cdev_del(&device->cdev);
	printk(KERN_INFO "i2c_proxy: Deleted cdev\n");
}
