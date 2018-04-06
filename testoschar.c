#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
 
#define BUFFER_LENGTH 1024

 
void writeToDeviceInteractive(int fd) {
   int ret;
   char stringToSend[BUFFER_LENGTH];

   printf("Type in a short string to send to the kernel module:\n");
   scanf("%[^\n]%*c", stringToSend);

   ret = write(fd, stringToSend, strlen(stringToSend));

   if (ret < 0){
      printf("Device String Full\n");
      return;
   }

   printf("Wrote message to the device [%s].\n", stringToSend);
}

void readDeviceString(int fd, int amount) {
   int ret;
   char receive[BUFFER_LENGTH] = {0};

   ret = read(fd, receive, amount);

   if (ret < 0){
      perror("Failed to read the message from the device.");
      return;
   }

   printf("Reading message from the device [%s].\n", receive);
}

int main(){
   int ret, ifd, ofd;

   printf("Starting device test code example...\n");

   ifd = open("/dev/oscharInput", O_RDWR);
   ofd = open("/dev/oscharOutput", O_RDWR);

   if (ifd < 0){
      perror("Failed to open the input device...");
      return errno;
   }

   if (ofd < 0){
      perror("Failed to open the output device...");
      return errno;
   }

   writeToDeviceInteractive(ifd);

   readDeviceString(ofd, 5);

   return 0;
}