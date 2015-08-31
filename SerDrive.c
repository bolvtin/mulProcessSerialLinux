#include "SerDrive.h"  
//===========================================  
  
#include     <stdio.h>      /*标准输入输出定义*/  
#include     <stdlib.h>     /*标准函数库定义*/  
#include     <unistd.h>     /*Unix 标准函数定义*/  
#include     <sys/types.h>  
#include     <sys/stat.h>  
#include     <fcntl.h>      /*文件控制定义*/  
#include     <termios.h>    /*POSIX 终端控制定义*/  
#include     <errno.h>      /*错误号定义*/  
  
//=============================================  
  
//串口配置的函数  
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)  
{  
    struct termios newtio, oldtio;  
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/  
    if (tcgetattr(fd, &oldtio) != 0)  
    {  
        perror("SetupSerial 1");  
        return -1;  
    }  
    bzero(&newtio, sizeof(newtio));  
    /*步骤一，设置字符大小*/  
    newtio.c_cflag |= CLOCAL;   //如果设置，modem 的控制线将会被忽略。如果没有设置，则 open()函数会阻塞直到载波检测线宣告 modem 处于摘机状态为止。  
    newtio.c_cflag |= CREAD;    //使端口能读取输入的数据  
    /*设置每个数据的位数*/  
    switch (nBits)  
    {  
    case 7:  
        newtio.c_cflag |= CS7;  
        break;  
    case 8:  
        newtio.c_cflag |= CS8;  
        break;  
    }  
    /*设置奇偶校验位*/  
    switch (nEvent)  
    {  
    case 'O': //奇数  
        newtio.c_iflag |= (INPCK | ISTRIP);  
        newtio.c_cflag |= PARENB;   //使能校验，如果不设PARODD则是偶校验  
        newtio.c_cflag |= PARODD;   //奇校验  
        break;  
    case 'E': //偶数  
        newtio.c_iflag |= (INPCK | ISTRIP);  
        newtio.c_cflag |= PARENB;  
        newtio.c_cflag &= ~PARODD;  
        break;  
    case 'N':  //无奇偶校验位  
        newtio.c_cflag &= ~PARENB;  
        break;  
    }  
    /*设置波特率*/  
    switch (nSpeed)  
    {  
    case 2400:  
        cfsetispeed(&newtio, B2400);  
        cfsetospeed(&newtio, B2400);  
        break;  
    case 4800:  
        cfsetispeed(&newtio, B4800);  
        cfsetospeed(&newtio, B4800);  
        break;  
    case 9600:  
        cfsetispeed(&newtio, B9600);  
        cfsetospeed(&newtio, B9600);  
        break;  
    case 115200:  
        cfsetispeed(&newtio, B115200);  
        cfsetospeed(&newtio, B115200);  
        break;  
    case 460800:  
        cfsetispeed(&newtio, B460800);  
        cfsetospeed(&newtio, B460800);  
        break;  
    default:  
        cfsetispeed(&newtio, B9600);  
        cfsetospeed(&newtio, B9600);  
        break;  
    }  
    /* 
     * 设置停止位 
     * 设置停止位的位数， 如果设置，则会在每帧后产生两个停止位， 如果没有设置，则产生一个 
     * 停止位。一般都是使用一位停止位。需要两位停止位的设备已过时了。 
     * */  
    if (nStop == 1)  
        newtio.c_cflag &= ~CSTOPB;  
    else if (nStop == 2)  
        newtio.c_cflag |= CSTOPB;  
    /*设置等待时间和最小接收字符*/  
    newtio.c_cc[VTIME] = 0;  
    newtio.c_cc[VMIN] = 0;  
    /*处理未接收字符*/  
    tcflush(fd, TCIFLUSH);  
    /*激活新配置*/  
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)  
    {  
        perror("com set error");  
        return -1;  
    }  
    printf("set done!\n");  
    return 0;  
}  
//======================================================  
//从串口中读取数据  
int read_datas_tty(int fd, char *rcv_buf, int TimeOut, int Len)  
{  
    int retval;  
    fd_set rfds;  
    struct timeval tv;  
    int ret, pos;  
    tv.tv_sec = TimeOut / 1000;  //set the rcv wait time  
    tv.tv_usec = TimeOut % 1000 * 1000;  //100000us = 0.1s  
  
    pos = 0;  
    while (1)  
    {  
        FD_ZERO(&rfds); //每次循环都要清空集合，否则不能检测描述符变化  
        FD_SET(fd, &rfds);  //添加描述符  
        retval = select(fd + 1, &rfds, NULL, NULL, &tv);    //fd+1:描述符最大值加1 返回负数：出错  0:timeout  >0:有文件可读写  
        if (retval == -1)  
        {  
            perror("select()");  
            break;  
        }  
        else if (retval)  
        {  
            ret = read(fd, rcv_buf + pos, 1);  
            if (-1 == ret)  
            {  
                break;  
            }  
  
            pos++;  
            if (Len <= pos)  
            {  
                break;  
            }  
        }  
        else  
        {  
            break;  
        }  
    }  
  
    return pos;  
}  
  
//向串口传数据  
int send_data_tty(int fd, char *send_buf, int Len)  
{  
    ssize_t ret;  
  
    ret = write(fd, send_buf, Len);  
    if (ret == -1)  
    {  
        printf("write device error\n");  
        return -1;  
    }  
  
    return 1;  
} 