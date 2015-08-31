#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>  
#include "gps.h"
#include <string.h>
#define BAUDRATE B9600      //串口波特率，即GPS通过COM1发来的数据传输率
#define COM1 "/dev/ttyS1"   //定义与GPS连接的串口1的实际位置，此位置需根据2410箱上的实际来设置
GPS_INFO gps_info;                 //存放分离后的gps数据，是一个结构体，在gps.h中定义
char GPS_BUF[1024];                //存放从串口读到的GPS原始数据
static int baud=BAUDRATE;
volatile int fd;                   //打开串口后保存的设备描述符
void* receive() //当收到一串数据时（以回车符作为分界来识别一串字符），则原样输出，当字符中第6个是C时，则表示是我们需要的数据串（以$GPRMC开头），则对该串进行分离出时间及经纬度并输出来，否则只是原样输出。
{
   int i=0;
   char c;     
   char buf[1024];
   printf("read gps\n");
   while (1)
   {
     read(fd,&c,1);    //从打开的串口COM1（因为GPS模块接到了COM1）读取1个字符
	 //printf("read a char");
	 if (i < 10){
		 buf[i++] = c;
		 printf("read a char %d\n",c);
		 printf("i de zhi is %d", i);
	 }
	 //buf[11] = '\0';
	 //printf("%s", buf);
  //   if(c == '\n'){
  //      strncpy(GPS_BUF,buf,i); //从buf中复制i个字符到GPS_BUF中
  //      i=0;                    //重置数据的起始位置
  //      printf("%s",GPS_BUF);   //原样输出
  //      if(buf[5]=='C'){
  //          gps_parse(GPS_BUF,&gps_info);  //分离原始数据
  //          show_gps(&gps_info);           //显示分离后的数据
  //     }
  //    }
   }
   printf("exit from reading gps\n");
   //return NULL;
}
int main()
{
    struct termios options;         //串口设置的结构
    fd=open(COM1,O_RDWR | O_NONBLOCK);
    if (fd <0) {
       perror(COM1);
       exit(-1);
   }
   tcgetattr(fd,&options);
  
    options.c_cflag = baud | CRTSCTS | CS8 | CLOCAL | CREAD;    
    options.c_iflag = IGNPAR;    //忽略奇偶校验
    options.c_oflag = 0;     //Raw 模式输出  //不处理，就原始数据输出
    options.c_lflag = 0; //不处理，原始数据输入  
    options.c_cc[VMIN]=100;     // 阻塞，直到读取到一个字符
    options.c_cc[VTIME]=0;   // 不使用字符间的计时器 
    
    //清空数据线，启动新的串口设置
    tcflush(fd, TCIFLUSH);   //刷新输入队列
    tcsetattr(fd,TCSANOW,&options);            //新属性设置生效
	printf("set options\n");
    if(receive()==NULL) 
		exit(-1);//调用接收函数收取GPS获到的数据
    exit(0);
}
