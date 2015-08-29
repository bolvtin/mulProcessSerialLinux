#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
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

int main(int argc, char **argv)

{

	int fd, i, ret;

	char buf[1024] =

	"$GPGGA,064746.000,4925.4895,N,00103.99255,E,1,05,2.1,-68.0,M,47.1,M,,0000*4F\r\n"; // 此处赋值用于测试

	if ((fd = open("/dev/ttyS1", O_RDWR)) == -1)

	return -1;

	// set fd: tcsetattr... 直接连接串口的设备需要在此设置波特率

	for (i = 0; i < 100; i++)

		{

		ret = read(fd, buf, 1024);

		if (ret > 1)

		{

		if (strstr(buf, "GPGGA") != NULL)

		parseData(buf);

		}

	}

	// restore fd: tcsetattr... 直接连接串口的设备需要在此恢复波特率

	close(fd);

} 