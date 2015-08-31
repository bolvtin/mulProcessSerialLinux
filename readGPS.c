#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <termios.h> //set baud rate

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include<sys/stat.h>

//#define rec_buf_wait_2s 2
//#define buffLen 1024
#define buffLen 1024
#define rcvTimeOut 4
#define fileSave "fileSave.txt"
#define fileSaveGPS "fileSaveGPS.txt"

/*************************     gps  ******************************/
typedef struct{
	int year;    //年
	int month;   //月
	int day;     //日
	int hour;    //时
	int minute;  //分
	int second;  //秒
}date_time;  //时间结构体
typedef struct{
	date_time D;//时间
	double latitude;   //纬度
	double longitude;  //经度
	char NS;           //南北极
	char EW;           //东西
}GPS_INFO;
//void gps_parse(char *line, GPS_INFO *GPS);
//void show_gps(GPS_INFO *GPS);

//static int GetComma(int num, char* str);       //获得str字符指针对应字符串中第num个逗号的位置
//static void UTC2BTC(date_time *GPS);          //将GPS获得的UTC时间转换成北京时间
//static double get_double_number(char *s);     //获得两个逗号中间的浮点数，并转成double值返回

void gps_parse(char *line, GPS_INFO *GPS);
void show_gps(GPS_INFO *GPS);

int GetComma(int num, char* str);       //获得str字符指针对应字符串中第num个逗号的位置
void UTC2BTC(date_time *GPS);          //将GPS获得的UTC时间转换成北京时间
double get_double_number(char *s);     //获得两个逗号中间的浮点数，并转成double值返回

GPS_INFO gps_info;                 //存放分离后的gps数据，是一个结构体，在gps.h中定义
char GPS_BUF[1024];

void show_gps(GPS_INFO *GPS)//输出GPS分离原始数据后的可用数据（包含时间及经纬度）
{
	printf("DATE     : %d-%02d-%02d \n", GPS->D.year, GPS->D.month, GPS->D.day);
	printf("TIME     :  %02d:%02d:%02d \n", GPS->D.hour, GPS->D.minute, GPS->D.second);
	printf("Latitude : %10.4f %c\n", GPS->latitude, GPS->NS);
	printf("Longitude: %10.4f %c\n", GPS->longitude, GPS->EW);
}
////////////////////////////////////////////////////////////////////////////////
//解释gps发出的数据
//0      7  0   4 6   0     6 8 0        90         0  3      0        9   
//$GPRMC,091400,A,3958.9870,N,11620.3278,E,000.0,000.0,120302,005.6,W*62  
void gps_parse(char *line, GPS_INFO *GPS)
{
	int tmp;
	char c;
	char* buf = line;
	c = buf[5];
	if (c == 'C'){//"GPRMC"
		GPS->D.hour = (buf[7] - '0') * 10 + (buf[8] - '0');    //原始数据串中第7位（从0计起）为时钟的十位，而8为个位
		GPS->D.minute = (buf[9] - '0') * 10 + (buf[10] - '0');    //              9               分钟          10
		GPS->D.second = (buf[11] - '0') * 10 + (buf[12] - '0');    //              11              秒钟          12
		tmp = GetComma(9, buf);              //获得buf中第9个逗号的起始位置，即在数据buf中的下标
		GPS->D.day = (buf[tmp + 0] - '0') * 10 + (buf[tmp + 1] - '0');
		GPS->D.month = (buf[tmp + 2] - '0') * 10 + (buf[tmp + 3] - '0');
		GPS->D.year = (buf[tmp + 4] - '0') * 10 + (buf[tmp + 5] - '0') + 2000;
		GPS->latitude = get_double_number(&buf[GetComma(3, buf)]);      //纬度
		GPS->NS = buf[GetComma(4, buf)];                          //南s北n
		GPS->longitude = get_double_number(&buf[GetComma(5, buf)]);      //经度
		GPS->EW = buf[GetComma(6, buf)];                          //东e西w
		UTC2BTC(&GPS->D);
	}
}
//获得在字符串s中第一个逗号前的数值，并转换为浮点数返回
//static double get_double_number(char *s)
double get_double_number(char *s)
{
	char buf[128];
	int i;
	double rev;
	i = GetComma(1, s);
	strncpy(buf, s, i);
	buf[i] = 0;
	rev = atof(buf);
	return rev;
}
//返回str中第num个逗号的下一位置(从0起)
//static int GetComma(int num, char *str)
int GetComma(int num, char *str)
{
	int i, j = 0;
	int len = strlen(str);
	for (i = 0; i<len; i++)
	{
		if (str[i] == ',')j++;
		if (j == num)return i + 1;
	}
	return 0;
}
//将世界时转换为北京时
//static void UTC2BTC(date_time *GPS)
void UTC2BTC(date_time *GPS)
{
	//如果秒号先出,再出时间数据,则将时间数据+1秒
	GPS->second++; //加一秒
	if (GPS->second>59){
		GPS->second = 0;
		GPS->minute++;
		if (GPS->minute>59){
			GPS->minute = 0;
			GPS->hour++;
		}
	}
	GPS->hour += 8;   //北京时间与世界时间相差8个时区，即相差8个钟
	if (GPS->hour>23)
	{
		GPS->hour -= 24;
		GPS->day += 1;
		if (GPS->month == 2 || GPS->month == 4 || GPS->month == 6 || GPS->month == 9 || GPS->month == 11){
			if (GPS->day>30){
				GPS->day = 1;
				GPS->month++;
			}
		}
		else{
			if (GPS->day>31){
				GPS->day = 1;
				GPS->month++;
			}
		}
		if ((GPS->year % 4 == 0) && (GPS->year % 400 == 0 || GPS->year % 100 != 0)){  //判断闰年
			if (GPS->day > 29 && GPS->month == 2){   //闰年二月比平年二月多一天
				GPS->day = 1;
				GPS->month++;
			}
		}
		else{
			if (GPS->day>28 && GPS->month == 2){
				GPS->day = 1;
				GPS->month++;
			}
		}
		if (GPS->month>12){
			GPS->month -= 12;
			GPS->year++;
		}
	}
}



/*********************************************************/

/*************Linux and Serial Port *********************/
/*************Linux and Serial Port *********************/
int openPort(int fd, int comport)
{

	if (comport == 1)
	{
		fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyS0 .....\n");
		}
	}
	else if (comport == 2)
	{
		//fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NDELAY);
		//fd = open("/dev/ttyS1", O_RDWR | O_NONBLOCK);
		fd = open("/dev/ttyS1", O_RDWR);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyS1 .....\n");
		}
	}
	else if (comport == 3)
	{
		fd = open("/dev/ttyS2", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyS2 .....\n");
		}
	}
	/*************************************************/
	else if (comport == 4)
	{
		fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyUSB0 .....\n");
		}
	}

	/*************************************************/
	else if (comport == 5)
	{
		//fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);
		fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
		//fd = open("/dev/ttyACM1", O_RDWR | O_NOCTTY);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyACM0 .....\n");
		}
	}

	if (fcntl(fd, F_SETFL, 0)<0)
	{
		printf("fcntl failed!\n");
	}
	else
	{
		printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
	}
	if (isatty(STDIN_FILENO) == 0)
	{
		printf("standard input is not a terminal device\n");
	}
	else
	{
		printf("is a tty success!\n");
	}
	printf("fd-open=%d\n", fd);
	return fd;
}

int setOpt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio, oldtio;
	if (tcgetattr(fd, &oldtio) != 0)
	{
		perror("SetupSerial 1");
		return -1;
	}
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch (nBits)
	{
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}

	switch (nEvent)
	{
	case 'O':                     //奇校验
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E':                     //偶校验
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':                    //无校验
		newtio.c_cflag &= ~PARENB;
		break;
	}

	switch (nSpeed)
	{
	case 1200:
		cfsetispeed(&newtio, B1200);
		cfsetospeed(&newtio, B1200);
		break;
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
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if (nStop == 1)
	{
		newtio.c_cflag &= ~CSTOPB;
	}
	else if (nStop == 2)
	{
		newtio.c_cflag |= CSTOPB;
	}
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 100;//果然很重要，没有这个GPS的数据就读不回来呀
	//newtio.c_cc[VTIME] = 0;//重要
	//newtio.c_cc[VMIN] = 100;//返回的最小值  重要
	tcflush(fd, TCIFLUSH);
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
	{
		perror("com set error");
		return -1;
	}
	printf("set done!\n");
	return 0;
}

int readDataTty(int fd, char *rcv_buf, int TimeOut, int Len)
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
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		retval = select(fd + 1, &rfds, NULL, NULL, &tv);//正值：某些文件可读写或出错 
		//retval = select(fd + 1, &rfds, NULL, NULL, NULL);
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

int sendDataTty(int fd, char *send_buf, int Len)
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

int serialSubProcess(int num,int max)
{
	pid_t pid;
	int iSetOpt = 0;
	int fdSerial = 0;

	int iSetOpt1 = 0;
	int fdSerial1 = 0;

	unsigned int readDataNum = 0;
	char buffRcvData[buffLen] = { 0 };
	
	int printNum = 0;
	//打开保存文件
	int fdFileSave = 0;

	if ((fdFileSave = open(fileSave, O_WRONLY | O_CREAT | O_APPEND)) == -1)
	{
		printf("Open %s Error\n", fileSave);
		exit(1);
	}

	if (num >= max)return;
	pid = fork();
	if (pid<0)
	{
		perror("fork error!\n");
		exit(1);
	}
	//子进程   
	else if (pid == 0)
	{
		//sleep(3);
		printf("Son Process id=%d,Parent Process id=%d\n", getpid(), getppid());
		switch (num){
		case 0:
			//openPort
			if ((fdSerial = openPort(fdSerial, 5))<0)//1--"/dev/ttyS0",2--"/dev/ttyS1",3--"/dev/ttyS2",4--"/dev/ttyUSB0" 小电脑上是2--"/dev/ttyS1"
			{
				perror("open_port error");
				return -1;
			}
			//setOpt(fdSerial, 9600, 8, 'N', 1)
			if ((iSetOpt = setOpt(fdSerial, 9600, 8, 'N', 1))<0)
			{
				perror("set_opt error");
				//return -1;
			}
			printf("Serial fdSerial=%d\n", fdSerial);

			tcflush(fdSerial, TCIOFLUSH);//清掉串口缓存
			fcntl(fdSerial, F_SETFL, 0);//这个是设置为默认的阻塞模式的

			/*readDataNum = readDataTty(fdSerial, buffRcvData, rcvTimeOut, buffLen);
			while (1){
				printNum++;
				printf("%d\n", printNum);
			}*/
			
			
			while (1){
				readDataNum = readDataTty(fdSerial, buffRcvData, rcvTimeOut, buffLen);
				write(fdFileSave, buffRcvData, readDataNum);//将数据保存在文件fdFileSave 即"fileSave.txt"
			}

			break;
			
		case 1:
			printf("**********************************************\n");
			printf("this is the 1 serial subProcess\n");
			printf("**********************************************\n");
			////openPort
			//if ((fdSerial1 = openPort(fdSerial1, 0))<0)//1--"/dev/ttyS0",2--"/dev/ttyS1",3--"/dev/ttyS2",4--"/dev/ttyUSB0" 小电脑上是2--"/dev/ttyS1"
			//{
			//	perror("open_port1 error");
			//	return -1;
			//}
			////setOpt(fdSerial, 9600, 8, 'N', 1)
			//if ((iSetOpt1 = setOpt(fdSerial1, 9600, 8, 'N', 1))<0)
			//{
			//	perror("set_opt1 error");
			//	return -1;
			//}
			//printf("Serial fdSerial1=%d\n", fdSerial1);

			//tcflush(fdSerial1, TCIOFLUSH);//清掉串口缓存
			//fcntl(fdSerial1, F_SETFL, 0);//这个是设置为默认的阻塞模式的
			break;
			
		default:
			break;			
		}
	}
	//父进程   
	else
	{
		num++;
		if (num == 1)printf("Parent Process id=%d\n", getpid());
		if (num<max)serialSubProcess(num,max);
		//此处加sleep是为了防止父进程先退出,从而产生异常   
		//sleep(5);
		sleep(5);
	}
}

void parseData(char *buf)
{
	int ret, nQ, nN, nB, nC;

	char cX, cY, cM1, cM2;

	float fTime, fX, fY, fP, fH, fB, fD;



	if (buf == NULL)

		return;

	ret = sscanf(buf,

		"$GPGGA,%f,%f,%c,%f,%c,%d,%02d,%f,%f,%c,%f,%c,%f,%04d%02x",

		&fTime, &fX, &cX, &fY, &cY, &nQ, &nN, &fP, &fH, &cM1, &fB,

		&cM2, &fD, &nB, &nC);

	printf("x: %c %f, y: %c %f, h %f, satellite: %d\n",

		cX, fX, cY, fY, fH, nN);

}

int main(int argc, char** argv)
{
	char bufGPS[1024] ="$GPGGA,064746.000,4925.4895,N,00103.99255,E,1,05,2.1,-68.0,M,47.1,M,,0000*4F\r\n"; // 此处赋值用于测试

	pid_t pid;
	int fdGPS, i, ret;
	int iSetOpt;

	//gps
	int j, startI, endI;
	char buf[1024];
	char c;

	//打开保存文件
	int fdFileSaveGPS = 0;

	if ((fdFileSaveGPS = open(fileSaveGPS, O_WRONLY | O_CREAT | O_APPEND)) == -1)
	{
		printf("Open %s Error\n", fileSaveGPS);
		exit(1);
	}

	//openPort
	if ((fdGPS = openPort(fdGPS, 2))<0)//1--"/dev/ttyS0",2--"/dev/ttyS1",3--"/dev/ttyS2",4--"/dev/ttyUSB0" 小电脑上是2--"/dev/ttyS1"
	{
		perror("open_port error");
		return -1;
	}
	//setOpt(fdSerial, 9600, 8, 'N', 1)
	if ((iSetOpt = setOpt(fdGPS, 9600, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 4800, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 2400, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 115200, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 1200, 8, 'N', 1))<0)
	{
		perror("set_opt error");
		return -1;
	}

	printf("Serial fdSerial=%d\n", fdGPS);

	tcflush(fdGPS, TCIOFLUSH);//清掉串口缓存
	fcntl(fdGPS, F_SETFL, 0);//这个是设置为默认的阻塞模式的	

	//for (i = 0; i < 100; i++)
	//{
	//	ret = read(fdGPS, bufGPS, 1024);
	//	write(fdFileSaveGPS, bufGPS, ret);
	//	/*if (ret > 1)
	//	{
	//		if (strstr(bufGPS, "GPGGA") != NULL)
	//		parseData(bufGPS);
	//	}*/
	//}
	while (1){
		read(fdGPS, &c, 1);    //从打开的串口COM1（因为GPS模块接到了COM1）读取1个字符
		buf[i++] = c;
		if (c == '\n'){
			strncpy(GPS_BUF, buf, i); //从buf中复制i个字符到GPS_BUF中
			i = 0;                    //重置数据的起始位置
			printf("%s", GPS_BUF);   //原样输出
			if (buf[5] == 'C'){
				gps_parse(GPS_BUF, &gps_info);  //分离原始数据
				show_gps(&gps_info);           //显示分离后的数据
			}
		}
		/*********************************************************************************/
		//ret = read(fdGPS, bufGPS, 1024);
		//if (ret > 1)
		//{
		//	/*printf("\n GPS DATALen=%d\n", ret);
		//	bufGPS[ret] = '\0';*/

		//	/*for (i = 0; i < ret; i++){
		//		printf("%c", bufGPS[i]);
		//	}*/

		//	//wangbo自己写的
		//	for (i = 0; i < ret; i++){
		//		if ('$'==bufGPS[i] ){
		//			startI = i;
		//		}
		//		else if ('\n' == bufGPS[i]){
		//			endI = i;
		//			break;
		//		}
		//	}
		//	printf("%d    %d\n", startI, endI);
		//	for (j = 0, i = startI; i < endI; i++,j++){
		//		buf[j] = bufGPS[i];
		//	}
		//	strncpy(GPS_BUF, buf, endI-startI);
		//	if (buf[5] == 'C'){
		//		          gps_parse(GPS_BUF,&gps_info);  //分离原始数据
		//		          show_gps(&gps_info);           //显示分离后的数据
		//	}




		//	//下面的2行需要
		//	//printf("GPS %s\n", bufGPS); //输出所读数据
		//	//if (strstr(bufGPS, "GPGGA") != NULL)
		//	//parseData(bufGPS);

		//}
		/*********************************************************************************/

		//write(fdFileSaveGPS, bufGPS, ret);//将数据保存在文件fdFileSave 即"fileSave.txt"
		sleep(2);
	}
	close(fdGPS);
	return 0;
}
