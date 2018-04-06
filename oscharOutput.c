#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#define DEVICE_NAME "oscharOutput"
#define CLASS_NAME  "oscharOutput_class"

MODULE_LICENSE("GPL");

#define BUFFER_SIZE 1024

extern char *deviceString;
extern struct mutex oscharMutex;

static short size = 0 ;
static int numberOpens = 0;

static int majorNumber;
static struct class* oscharClass  = NULL;
static struct device* oscharDevice = NULL;

int start = 0;
int end = 0;

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);

static struct file_operations fops = {
	.open = dev_open,
	.read = dev_read,
	.release = dev_release,
};

static int __init oschar_output_init(void){
   printk(KERN_INFO "OSCharOutput INIT: Initializing the OSCharOutput LKM\n");

   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

   if(majorNumber<0){
      printk(KERN_ALERT "OSCharOutput failed to register a major number\n");
      return majorNumber;
   }

   printk(KERN_INFO "OSCharOutput INIT: registered correctly with major number %d\n", majorNumber);
 
   oscharClass = class_create(THIS_MODULE, CLASS_NAME);

   if(IS_ERR(oscharClass)){
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(oscharClass);
   }

   printk(KERN_INFO "OSCharOutput INIT: device class registered correctly\n");
 
   oscharDevice = device_create(oscharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   
   if(IS_ERR(oscharDevice)){
      class_destroy(oscharClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(oscharDevice);
   }

   mutex_init(&oscharMutex);

   printk(KERN_INFO "OSCharOutput INIT: device class created correctly\n"); 
   return 0;
}

static void __exit oschar_output_exit(void){
   device_destroy(oscharClass, MKDEV(majorNumber, 0));
   class_unregister(oscharClass);
   class_destroy(oscharClass);
   unregister_chrdev(majorNumber, DEVICE_NAME);
   printk(KERN_INFO "OSCharOutput EXIT: Goodbye from the LKM!\n\n");
}


static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   printk(KERN_INFO "OSCharOutput OPEN: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   int i;
   char retString[len];

   mutex_lock(&oscharMutex);

   for (i = 0; i < len; i++)
      retString[i] = 0;

   printk(KERN_INFO "OSCharOutput READ: Device String [%s]\n", deviceString);

   for (i = 0; i < len; i++){
      if(size <= 0) {
         printk(KERN_INFO "OSCharOutput READ: No characters left in device\n");
         break;
      }
      
      retString[i] = deviceString[start % BUFFER_SIZE];
      deviceString[start % BUFFER_SIZE] = ' ';
      start = (start + 1) % BUFFER_SIZE;
      
      size--;
   }

   error_count = copy_to_user(buffer, retString, len);
 
   if (error_count != 0){
      printk(KERN_INFO "OSCharOutput READ: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;
   }

   mutex_unlock(&oscharMutex);

   printk(KERN_INFO "OSCharOutput READ: Sent characters to the user [%s]\n", retString); 

   return len;
}

static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "OSCharOutput RELEASE: Device successfully closed\n");
   return 0;
}

module_init(oschar_output_init);
module_exit(oschar_output_exit);