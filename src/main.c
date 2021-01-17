#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>

static int init_i2c_proxy(void);
static void exit_i2c_proxy(void);

MODULE_AUTHOR("Marco Satti");
MODULE_LICENSE("GPL");

module_init(init_i2c_proxy);
module_exit(exit_i2c_proxy);

///////////////////////////////////////

static struct kobject *i2c_proxy_kobject;

static int init_i2c_proxy(void) 
{
	printk(KERN_INFO "Initializing i2c_proxy module...\n");

	i2c_proxy_kobject = kobject_create_and_add("i2c_proxy", kernel_kobj);
	if (!i2c_proxy_kobject)
		return -ENOMEM;

	return 0;
}

static void exit_i2c_proxy(void)
{
	printk(KERN_INFO "Exiting i2c_proxy module...\n");

	if (i2c_proxy_kobject)
		kobject_put(i2c_proxy_kobject);
}

