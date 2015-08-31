#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#define fileSave "fileSaveGPS.txt"

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio, oldtio;
	if (tcgetattr(fd, &oldtio) != 0) {
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
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E':
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':
		newtio.c_cflag &= ~PARENB;
		break;
	}

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
	if (nStop == 1)
		newtio.c_cflag &= ~CSTOPB;
	else if (nStop == 2)
		newtio.c_cflag |= CSTOPB;
	newtio.c_cc[VTIME] = 0;//重要
	newtio.c_cc[VMIN] = 100;//返回的最小值  重要
	tcflush(fd, TCIFLUSH);
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
	{
		perror("com set error");
		return -1;
	}
	//	printf("set done!\n\r");
	return 0;
}

int main(void)
{
	int fd1, nset1, nread;
	char buf[1024];
	int i;

	int fdFileSave = 0;

	if ((fdFileSave = open(fileSave, O_WRONLY | O_CREAT | O_APPEND)) == -1)
	{
		printf("Open %s Error\n", fileSave);
		exit(1);
	}

	//fd1 = open("/dev/ttyACM0", O_RDWR|O_NONBLOCK);//打开串口
	fd1 = open("/dev/ttyS1", O_RDWR | O_NONBLOCK);//打开串口
	if (fd1 == -1)
		exit(1);

	nset1 = set_opt(fd1, 9600, 8, 'N', 1);//设置串口属性
	if (nset1 == -1)
		exit(1);

	while (1)

	{
		memset(buf, 0, 1024);
		nread = read(fd1, buf, 1024);//读串口
		write(fdFileSave, buf, nread);//将数据保存在文件fdFileSave 即"fileSave.txt"
		if (nread > 0){
			printf("\nGPS DATALen=%d\n", nread);
			buf[nread] = '\0';
			printf("GPS\n");
			/*for (i = 0; i < nread; i++){
				printf("%c", buf[i]);
			}*/
			printf("%s\n", buf); //输出所读数据
		}
		sleep(5);//睡眠，等待数据多一点

	}
	close(fd1);
	return 0;
}