/*
*     Copyright (c) 2026 SmartHome LLC.
*     All rights reserved.
*
*     Licensed under the Apache License, Version 2.0 (the "License");
*     you may not use this file except in compliance with the License.
*     You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0
*
*     Unless required by applicable law or agreed to in writing, software
*     distributed under the License is distributed on an "AS IS" BASIS,
*     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*     See the License for the specific language governing permissions and
*     limitations under the License.
*/
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define SECOND_MAJOR 232

static int second_major = SECOND_MAJOR;
module_param(second_major, int, S_IRUGO);

struct second_dev {
	struct cdev cdev;
	atomic_t counter;
	struct timer_list s_timer;
};

static struct second_dev *second_devp;

static void second_timer_handler(struct timer_list *t)
{
	struct second_dev *dev = from_timer(dev, t, s_timer);
	atomic_inc(&dev->counter);
	mod_timer(&dev->s_timer, jiffies + HZ);

	printk(KERN_INFO "current jiffies is %ld\n", jiffies);
}

static int second_open(struct inode *inode, struct file *filp)
{
	timer_setup(&second_devp->s_timer, second_timer_handler, 0);
	second_devp->s_timer.expires = jiffies + HZ;

	add_timer(&second_devp->s_timer);
	atomic_set(&second_devp->counter, 0);

	return 0;
}

static int second_release(struct inode *inode, struct file *filp)
{
	del_timer(&second_devp->s_timer);
	return 0;
}

static ssize_t second_read(struct file *filp, char __user *buf, size_t count,
			   loff_t *f_pos)
{
	int counter = atomic_read(&second_devp->counter);
	if (copy_to_user(buf, &counter, sizeof(counter)))
		return -EFAULT;

	return sizeof(counter);
}

static const struct file_operations second_fops = {
	.owner = THIS_MODULE,
	.open = second_open,
	.release = second_release,
	.read = second_read,
};

static void second_setup_cdev(struct second_dev *dev, int index)
{
	int err, devno = MKDEV(second_major, index);

	cdev_init(&dev->cdev, &second_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding second%d", err, index);
}

static int __init second_init(void)
{
	int ret;
	dev_t devno = MKDEV(second_major, 0);

	if (second_major)
		ret = register_chrdev_region(devno, 1, "second");
	else {
		ret = alloc_chrdev_region(&devno, 0, 1, "second");
		second_major = MAJOR(devno);
	}

	if (ret < 0) {
		printk(KERN_WARNING "second: can't get major %d\n",
		       second_major);
		return ret;
	}

	second_devp = kzalloc(sizeof(struct second_dev), GFP_KERNEL);
	if (!second_devp) {
		unregister_chrdev_region(devno, 1);
		ret = -ENOMEM;
		goto faild_malloc;
	}

	second_setup_cdev(second_devp, 0);

	return 0;

faild_malloc:
	unregister_chrdev_region(devno, 1);
	return ret;
}
module_init(second_init);

static void __exit second_exit(void)
{
	del_timer(&second_devp->s_timer);
	cdev_del(&second_devp->cdev);
	kfree(second_devp);
	unregister_chrdev_region(MKDEV(second_major, 0), 1);
}
module_exit(second_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("SmartHome LLC");
