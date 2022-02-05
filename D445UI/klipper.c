

#include "klipper.h"
#include<stdio.h> 
#include<fcntl.h> 
#include<errno.h> 

 

 static const char klipperPath[] = "/tmp/printer";
       int fd; 
int init_klipper()
{
    fd = open(klipperPath, O_RDWR ); 
    if (fd==NULL)
    {
        printf("Failed to open %s, klipper likely not running\r\n",klipperPath);
    }
    // fcntl(fd, F_SETFL, O_NONBLOCK);  
    // send status to check that it's operating
    write(fd,"status\n",7);

}