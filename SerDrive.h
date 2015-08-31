#ifndef SERDRIVE_H  
#define SERDRIVE_H  
//=======================  
//串口名字  
#define     TTY0_NAME   "/dev/ttySAC0"  
#define     TTY1_NAME   "/dev/ttySAC1"  
#define     TTY2_NAME   "/dev/ttySAC2"  
  
  
//串口  
#define     TTYS0   1  
#define     TTYS1   2  
#define     TTYS2   3  
//波特率  
#define     BAUD_2400       2400  
#define     BAUD_4800       4800  
#define     BAUD_9600       9600  
#define     BAUD_115200     115200  
#define     BAUD_460800     460800  
//奇偶校验位  
#define     PARITY_ODD    'O' //奇数  
#define     PARITY_EVEN   'E' //偶数  
#define     PARITY_NONE   'N' //无奇偶校验位  
//停止位  
#define     STOP_BIT_1     1  
#define     STOP_BIT_2     2  
//数据位  
#define     DATA_BIT_7     7  
#define     DATA_BIT_8     8  
//========================================  
//串口API  
  
/*打开串口函数*/  
int open_port(int fd,int comport);  
//串口配置的函数  
int set_opt(int fd,int nSpeed, int nBits,  
            char nEvent, int nStop);  
//从串口中读取数据  
int read_datas_tty(int fd,char *rcv_buf,int TimeOut,int Len);  
//向串口传数据  
int send_data_tty(int fd, char *send_buf,int Len);  
  
  
//==============  
#endif // SERDRIVE_H  