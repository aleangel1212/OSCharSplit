#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#define DEVICE_NAME "oscharInput"
#define CLASS_NAME  "oscharInput_class"

MODULE_LICENSE("GPL");

#define BUFFER_SIZE 1024

struct mutex oscharMutex;
char deviceString[BUFFER_SIZE] = {0};

EXPORT_SYMBOL(deviceString);
EXPORT_SYMBOL(oscharMutex);

static short size = 0 ;
static int numberOpens = 0;

static int majorNumber;
static struct class* oscharClass  = NULL;
static struct device* oscharDevice = NULL;

int start = 0;
int end = 0;

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
	.open = dev_open,
	.write = dev_write,
	.release = dev_release,
};

static int __init oschar_input_init(void){
   printk(KERN_INFO "OSCharInput INIT: Initializing the OSCharInput LKM\n");

   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

   if(majorNumber<0){
      printk(KERN_ALERT "OSCharInput failed to register a major number\n");
      return majorNumber;
   }

   printk(KERN_INFO "OSCharInput INIT: registered correctly with major number %d\n", majorNumber);
 
   oscharClass = class_create(THIS_MODULE, CLASS_NAME);

   if(IS_ERR(oscharClass)){
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(oscharClass);
   }

   printk(KERN_INFO "OSCharInput INIT: device class registered correctly\n");
 
   oscharDevice = device_create(oscharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   
   if(IS_ERR(oscharDevice)){
      class_destroy(oscharClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(oscharDevice);
   }

   mutex_init(&oscharMutex);

   printk(KERN_INFO "OSCharInput INIT: device class created correctly\n"); 
   return 0;
}

static void __exit oschar_input_exit(void){
   device_destroy(oscharClass, MKDEV(majorNumber, 0));
   class_unregister(oscharClass);
   class_destroy(oscharClass);
   unregister_chrdev(majorNumber, DEVICE_NAME);
   printk(KERN_INFO "OSCharInput EXIT: Goodbye from the LKM!\n\n");
}


static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   printk(KERN_INFO "OSCharInput OPEN: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   int i;

   mutex_lock(&oscharMutex);

   for (i = 0; i < len; i++){
      if(size >= BUFFER_SIZE) {
         printk(KERN_INFO "OSCharInput WRITE: Buffer full\n");
         return -1;
      }
      
      deviceString[end % BUFFER_SIZE] = buffer[i];
      size++;
      end = (end + 1) % BUFFER_SIZE;
   }

   mutex_unlock(&oscharMutex);

   printk(KERN_INFO "OSCharInput WRITE: Device String [%s]\n", deviceString);

   return size; 
}

static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "OSCharInput RELEASE: Device successfully closed\n");
   return 0;
}

module_init(oschar_input_init);
module_exit(oschar_input_exit);