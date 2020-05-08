#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include<linux/ioctl.h>
#include<linux/proc_fs.h>


#define mem_size 1024
char chr_array[40] = "hello world\n";
static int len = 1;
dev_t dev = 0;



static irqreturn_t irq_handler(int irq, void *dev_id){
	printk(KERN_INFO"keyboard interrupt occured %d\n",i);
	i++;
	return IRQ_HANDLED;
}


//define the ioctl cod 
#define WR_DATA _IOW('a','a',int32_t*)
#define RD_DATA _IOR('a','b',int32_t*)

int32_t val = 0;

static struct class *dev_class;
static struct cdev my_cdev;

static int __init chr_driver_init(void);
static void __exit chr_driver_exit(void);
static int my_open(struct inode *inode,struct file *file);
static int my_release(struct inode *inode,struct file *file);
static ssize_t my_read(struct file *filp,char __user *buf,size_t len,loff_t *off);
static ssize_t my_write(struct file *filp,const char *buf, size_t len,loff_t *off);
static long chr_ioctl(struct file *file,unsigned int cmd,unsigned long arg);


uint8_t *kernel_buffer;

static int open_proc(struct inode *inode, struct file *file);
static int release_proc(struct inode *inode,struct file *file);
static ssize_t read_proc(struct file *file,char __user *buffer,size_t length,loff_t *offset);
static ssize_t write_proc(struct file *filp,const char *buff,size_t len,loff_t *off);


static struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.read = my_read,
	.write = my_write,
	.open = my_open,
	.unlocked_ioctl = chr_ioctl,
	.release = my_release,
};


static struct file_operations proc_fops = {
	.open = open_proc,
	.read = read_proc,
	.write = write_proc,
	.release = release_proc,
};


static int open_proc(struct inode *inode,struct file *file)
{
	printk(KERN_INFO"procfd file is opened\n");
	return 0;
}

static ssize_t read_proc(struct file *file,char __user *buffer,size_t length,loff_t *offset){
	printk(KERN_INFO"procfd file is read\n");
	if(len)
		len = 0;
	else{
		len = 1;
		return 0;
        }

	copy_to_user(buffer,chr_array,40);
	return length;

}


static ssize_t write_proc(struct file *filp,const char *buff,size_t len,loff_t *off){
	printk(KERN_INFO"proc file is writing\n");
	copy_from_user(chr_array,buff,len);
        return len;
}

static int release_proc(struct inode *inode,struct file *file)
{
	printk(KERN_INFO"proc file is released\n");
	return 0;
}





static int my_open(struct inode *inode, struct file *file)
{
	/*creating physical memory */
	if((kernel_buffer = kmalloc(mem_size,GFP_KERNEL))== 0){
		printk(KERN_INFO"CANNOT ALLOCATE THE MEMORY to the kernel\n");
		return -1;
	}
	printk(KERN_INFO"device file opened\n");
	return 0;
}


static int my_release(struct inode *inode, struct file *file)
{
	kfree(kernel_buffer);
	printk(KERN_INFO" DEVICE FILE CLOSED\n");
	return 0;
}

static ssize_t my_read(struct file *filp,char __user *buf,size_t len,loff_t *off)
{
	copy_to_user(buf,kernel_buffer,mem_size);
	printk(KERN_INFO"data read :done\n");
	return mem_size;
}


static ssize_t my_write(struct file  *filp,const char __user *buf,size_t len,loff_t *off)
{
	copy_from_user(kernel_buffer,buf,len);
	printk(KERN_INFO"data is written successfully\n");
	return len;
}


static long chr_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	switch(cmd){
		case WR_DATA:
			copy_from_user(&val,(int32_t *)arg,sizeof(val));
			printk(KERN_INFO"VALUE = %d\n",val);
			break;
		
		case RD_DATA: copy_to_user((int32_t*)arg,&val,sizeof(val));
			      break;
		}
	return 0;
}



static int __init chr_driver_init(void)
{
	/* allocating major no *
	*/

	if((alloc_chrdev_region(&dev,0,1,"my_Dev")) < 0 ){
		printk(KERN_INFO"cannot allocate major no\n");
		return -1;
	}

	printk(KERN_INFO"major = %d minor = %d \n",MAJOR(dev),MINOR(dev));


	/* creating cdev structure */

	cdev_init(&my_cdev,&fops);


	/* adding character device to the system */

	if((cdev_add(&my_cdev,dev,1)) < 0){
		printk(KERN_INFO"cannot add the device to the system");
		goto r_class;

	}

	/*create struct class */

	if((dev_class = class_create(THIS_MODULE,"my_class")) == NULL){

		printk(KERN_INFO"we are unable to allocate struct class\n");
		goto r_class;
	}

	/*creating device */

	if(device_create(dev_class,NULL,dev,NULL,"my_device") == NULL){
		printk(KERN_INFO"cannot create the device\n");
		goto r_device;
	}


	if(request_irq(IRQ_NO,irq_handler,IRQF_SHARED,"chr_device",(void *)(irq_handler))){
		printk(KERN_INFO"chr_device:cannot register IRQ");
		goto irq;
	}
	


	/*creating a proc entry*/
	proc_create("chr_proc",0666,NULL,&proc_fops);

	printk(KERN_INFO"device driver insert... done properly\n");
	return 0;

irq: 	
		free_irq(IRQ_NO,(void *)(irq_handler));

				
r_device:
		class_destroy(dev_class);
		return -1;


r_class:	
		unregister_chrdev_region(dev,1);
		return -1;

}


void __exit chr_driver_exit(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev,1);
	printk(KERN_INFO"device driver removed successfully\n");
}


module_init(chr_driver_init);
module_exit(chr_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("codeSwitch");
MODULE_DESCRIPTION("Character device driver");








