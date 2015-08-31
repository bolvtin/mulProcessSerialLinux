#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>

//int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
//{
//	struct termios newtio,oldtio;
//	if  ( tcgetattr( fd,&oldtio)  !=  0) { 
//		perror("SetupSerial 1");
//		return -1;
//	}
//	bzero( &newtio, sizeof( newtio ) );
//	newtio.c_cflag  |=  CLOCAL | CREAD;
//	newtio.c_cflag &= ~CSIZE;
//
//	switch( nBits )
//	{
//	case 7:
//		newtio.c_cflag |= CS7;
//		break;
//	case 8:
//		newtio.c_cflag |= CS8;
//		break;
//	}
//
//	switch( nEvent )
//	{
//	case 'O':
//		newtio.c_cflag |= PARENB;
//		newtio.c_cflag |= PARODD;
//		newtio.c_iflag |= (INPCK | ISTRIP);
//		break;
//	case 'E': 
//		newtio.c_iflag |= (INPCK | ISTRIP);
//		newtio.c_cflag |= PARENB;
//		newtio.c_cflag &= ~PARODD;
//		break;
//	case 'N':  
//		newtio.c_cflag &= ~PARENB;
//		break;
//	}
//
//	switch( nSpeed )
//	{
//	case 2400:
//		cfsetispeed(&newtio, B2400);
//		cfsetospeed(&newtio, B2400);
//		break;
//	case 4800:
//		cfsetispeed(&newtio, B4800);
//		cfsetospeed(&newtio, B4800);
//		break;
//	case 9600:
//		cfsetispeed(&newtio, B9600);
//		cfsetospeed(&newtio, B9600);
//		break;
//	case 115200:
//		cfsetispeed(&newtio, B115200);
//		cfsetospeed(&newtio, B115200);
//		break;
//	case 460800:
//		cfsetispeed(&newtio, B460800);
//		cfsetospeed(&newtio, B460800);
//		break;
//	default:
//		cfsetispeed(&newtio, B9600);
//		cfsetospeed(&newtio, B9600);
//		break;
//	}
//	if( nStop == 1 )
//		newtio.c_cflag &=  ~CSTOPB;
//	else if ( nStop == 2 )
//	newtio.c_cflag |=  CSTOPB;
//	newtio.c_cc[VTIME]  = 0;//重要
//	newtio.c_cc[VMIN] = 100;//返回的最小值  重要
//	tcflush(fd,TCIFLUSH);
//	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
//	{
//		perror("com set error");
//		return -1;
//	}
////	printf("set done!\n\r");
//	return 0;
//}
//
//int main(void)
//{
//	int fd1,nset1,nread;
//	char buf[1024];
//
//	fd1 = open("/dev/ttyS1", O_RDWR | O_NONBLOCK);//打开串口
//	if (fd1 == -1)
//		exit(1);
//
//	nset1 = set_opt(fd1,4800, 8, 'N', 1);//设置串口属性
//	if (nset1 == -1)
//		exit(1);
//
//	while	(1)
//
//	{
//		memset(buf,0,1024);	
//		nread = read(fd1, buf, 1024);//读串口
//		if (nread > 0){
//			printf("\n GPS DATALen=%d\n",nread); 
//			buf[nread] = '\0';
//			printf( "GPS %s\n", buf); //输出所读数据
//		}
//		sleep(2);//睡眠，等待数据多一点
// 
//	}
//	close(fd1);
//	return 0;
//}








void set_speed(int, int);
int main(int argc, char* args[])
{
	int fd, result, wr_num = 0, rd_num = 0, z = 1, buff_num;
	struct termios term_param;//串口通信参数结构体
	speed_t baud_rate_i, baud_rate_o;        //无符号整数类型
	char buff[100], read_buf[1];
	//打开串口 （根据实际情况修改串口设备名）
	fd = open("/dev/ttyS1", O_RDWR | O_NONBLOCK);
	if (!fd){//打开串口失败
		perror("syd168:Can't open the COM1!\n");
		exit(-1);
	}
	else
		printf("syd168:Open COM1 ok!\n");
	//获取打开串口的参数
	result = tcgetattr(fd, &term_param); //取得原配置参数
	if (result == -1){
		perror("syd168:Get param error!\n");
		exit(-1);
	}
	baud_rate_i = cfgetispeed(&term_param);//读取输入速度
	baud_rate_o = cfgetospeed(&term_param);//读取发送速度
	printf("初始：输入波特率是%d，输出波特率是%d，文件描述符是%d\n", baud_rate_i, baud_rate_o, fd);
	set_speed(fd, 4800);
	result = tcgetattr(fd, &term_param);
	if (result == -1){
		perror("syd168:Set param error!\n");
		exit(-1);
	}
	baud_rate_i = cfgetispeed(&term_param);
	baud_rate_o = cfgetospeed(&term_param);
	printf("设置后：输入波特率是%d，输出波特率是%d，文件描述符是%d\n", baud_rate_i, baud_rate_o, fd);
	buff_num = 0;//缓存中字符数为0
	printf("syd168:Receive....\n");
	while (1)
	{
		//接收一个字符
		rd_num = read(fd, read_buf, sizeof(read_buf));
		if (rd_num>0){   //接收到字符
			/*收到字符直接输出
			printf("%c",read_buf[0]);
			continue;
			*/
			if (read_buf[0] == '$'){//如果该字符是$
				if (buff[0] == '$'&&buff_num>0){//一个GPS信号接收已经完成，处理接受的GPS信息（打印）
					int j = 0;
					while (j<buff_num){
						printf("%c", buff[j]);
						j++;
					}
						//清除缓存和缓存计数，继续接受GPS信号
						buff_num = 0;
					memset(buff, 0, sizeof(buff));
				}
				buff[buff_num++] = read_buf[0];//记录GPS信息开始符
				goto delay;     //延时1秒钟
			}
			else{ //接收到非$字符
				if (buff[0] == '$')//已经存在首字符，则添加后续字符
					buff[buff_num++] = read_buf[0];
				continue;//如果开头的$不存在，放弃该字符
			}
		}
		else{
			//printf("Receive....\n");
			continue;
		}
	delay:
		sleep(1);
	}
}

int speed_arr[] = { B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = { 115200, 38400, 19200, 9600, 4800, 2400, 1200, 300, 38400,
19200, 9600, 4800, 2400, 1200, 300, };
void set_speed(int fd, int speed){
	int   i;
	int   status;
	struct termios   Opt;
	tcgetattr(fd, &Opt);
	for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0) {
				perror("tcsetattr fd1");
				return;
			}
			tcflush(fd, TCIOFLUSH);
		}
	}
}