#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>

#include "data.h"

static dev_t hbmod_dev;

struct cdev hbmod_cdev;

struct class *myclass = NULL;

static char buffer[64];
static int i = 0;
static int N = sizeof(ppg)/sizeof(ppg[0]);

ssize_t hbmod_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){

 	int value = ppg[i];
	i = i % (N);
	i++;
  	copy_to_user(buf, &value, 2);

  	printk(KERN_INFO "[hbmod] read (value = %d , buf_size = %d)\n", value, buf);
  return count;
}

struct file_operations hbmod_fops = {
  .owner = THIS_MODULE,
  .read = hbmod_read,
};

static int __init hbmod_module_init(void){
  printk(KERN_INFO  "loading hbmod_module\n");

  alloc_chrdev_region(&hbmod_dev, 0, 1, "hbmod_cdev");
  printk(KERN_INFO "%s\n", format_dev_t(buffer, hbmod_dev));

  myclass = class_create(THIS_MODULE, "hbmod_sys");
  device_create(myclass, NULL, hbmod_dev, NULL, "hbmod_dev");

  cdev_init(&hbmod_cdev, &hbmod_fops);
  hbmod_cdev.owner = THIS_MODULE;
  cdev_add(&hbmod_cdev, hbmod_dev, 1);
  return 0;
}

static void __exit hbmod_module_cleanup(void){
  printk(KERN_INFO "Cleaning-up hbmod_dev\n");

  device_destroy(myclass, hbmod_dev);
  cdev_del(&hbmod_cdev);
  class_destroy(myclass);
  unregister_chrdev_region(hbmod_dev, 1);
}

module_init(hbmod_module_init);
module_exit(hbmod_module_cleanup);

MODULE_AUTHOR("Nicola Battaglino");
MODULE_LICENSE("GPL");
