#include <linux/module.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);
static int init_i2c_proxy(void);
static void exit_i2c_proxy(void);
static int alloc_device(void);
static int init_device(void);
static void dealloc_device(void);
static void exit_device(void);

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
};

///////////////////////////////////////

static struct i2c_proxy_device *device;

static struct file_operations device_fops = 
{
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
};

///////////////////////////////////////

static int device_open(struct inode *inode, struct file *file) {
	printk(KERN_INFO "i2c_proxy: Device file opened\n");
    return -ENOMEM;
}

static int device_release(struct inode *inode, struct file *file) {
	printk(KERN_INFO "i2c_proxy: Device file released\n");
    return -ENOMEM;
}

static ssize_t device_read(struct file *file, char __user *buf, size_t len, loff_t *pos) {
	printk(KERN_INFO "i2c_proxy: Device file read\n");
    return -ENOMEM;
}

static ssize_t device_write(struct file *file, const char __user *buf, size_t len, loff_t *pos) {
	printk(KERN_INFO "i2c_proxy: Device file write\n");
    return -ENOMEM;
}

static int init_i2c_proxy(void) {
    int rc;

	printk(KERN_INFO "i2c_proxy: Initializing\n");

    if ((rc = alloc_device()) < 0) {
        return rc;
    }
	printk(KERN_INFO "i2c_proxy: Allocated device\n");

    if ((rc = init_device()) < 0) {
        return rc;
    }
	printk(KERN_INFO "i2c_proxy: Initialized device\n");
    
	return 0;
}

static void exit_i2c_proxy(void) {
	printk(KERN_INFO "i2c_proxy: Exiting\n");

    exit_device();
	printk(KERN_INFO "i2c_proxy: Exited device\n");

    dealloc_device();
	printk(KERN_INFO "i2c_proxy: Deallocated device\n");
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

static void dealloc_device(void) {
    kfree(device);
	printk(KERN_INFO "i2c_proxy: Free'd device\n");
}

static void exit_device(void) {
    cdev_del(&device->cdev);
	printk(KERN_INFO "i2c_proxy: Deleted cdev\n");

	unregister_chrdev_region(device->chrdev_region, 1);
	printk(KERN_INFO "i2c_proxy: Unregistered chrdev region\n");
}
