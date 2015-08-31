#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <string.h>
#include "gps.h"
#include <termios.h>


gps::gps()
{
    baud=4800;
    GpsName=NULL;
    Gpsfd=0;
    for(int i=0;i<1024;i++)
    {
        GPS_BUF[i]=0;
    }

    GPS = new GPS_INFO;
    memset(GPS,0,sizeof(GPS_INFO));
}

gps::~gps()
{
    delete GPS;
    closeGpsDev();
    tcsetattr(Gpsfd,TCSANOW,&oldtio); /* restore old modem setings */
    tcsetattr(0,TCSANOW,&oldstdtio); /* restore old tty setings */
}


////////////////////////////////////////////////////////////////////////////////
//解释gps发出的数据
//0      7  0   4 6   0     6 8 0        90         0  3      0        9
//$GPRMC,091400,A,3958.9870,N,11620.3278,E,000.0,000.0,120302,005.6,W*62
//$GPGGA,091400,3958.9870,N,11620.3278,E,1,03,1.9,114.2,M,-8.3,M,,*5E
void gps::gps_parse()
////////////////////////////////////////////////////////////////////////////////
{
    int tmp;
    char c;


    c = GPS_BUF[5];
    if(c=='C')
    {
        //"GPRMC"
        GPS->D.hour   =(GPS_BUF[ 7]-'0')*10+(GPS_BUF[ 8]-'0');
        GPS->D.minute =(GPS_BUF[ 9]-'0')*10+(GPS_BUF[10]-'0');
        GPS->D.second =(GPS_BUF[11]-'0')*10+(GPS_BUF[12]-'0');
        tmp = GetComma(9,GPS_BUF);
        GPS->D.day    =(GPS_BUF[tmp+0]-'0')*10+(GPS_BUF[tmp+1]-'0');
        GPS->D.month  =(GPS_BUF[tmp+2]-'0')*10+(GPS_BUF[tmp+3]-'0');
        GPS->D.year   =(GPS_BUF[tmp+4]-'0')*10+(GPS_BUF[tmp+5]-'0')+2000;

        GPS->status      = GPS_BUF[GetComma(2,GPS_BUF)];
        GPS->latitude = get_locate(get_double_number(&GPS_BUF[GetComma(3,GPS_BUF)]));
        GPS->NS       = GPS_BUF[GetComma(4,GPS_BUF)];
        GPS->longitude= get_locate(get_double_number(&GPS_BUF[GetComma(5,GPS_BUF)]));
        GPS->EW       = GPS_BUF[GetComma(6,GPS_BUF)];
        GPS->speed    = get_double_number(&GPS_BUF[GetComma(7,GPS_BUF)]);
        UTC2BTC(&GPS->D);

    }

    if(c=='A')
    {
        //"$GPGGA"
        GPS->high     = get_double_number(&GPS_BUF[GetComma(9,GPS_BUF)]);
    }



 }


//将获取文本信息转换为double型

double gps::get_double_number(char *s)
{
    char buf[128];
    int i;
    double rev;
    i=GetComma(1,s);
    strncpy(buf,s,i);
    buf[i]=0;
    rev=atof(buf);

    return rev;
}

double gps::get_locate(double temp)
{
    int m;
    double  n;
    m=(int)temp/100;
    n=(temp-m*100)/60;
    n=n+m;
    return n;

}

////////////////////////////////////////////////////////////////////////////////
//得到指定序号的逗号位置
int gps::GetComma(int num,char *str)
{
    int i,j=0;
    int len=strlen(str);
    for(i=0;i<len;i++)
    {
        if(str[i]==',')
        {
             j++;
        }

        if(j==num)
            return i+1;
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//将世界时转换为北京时
void gps::UTC2BTC(date_time *GPS)
{

//***************************************************
//如果秒号先出,再出时间数据,则将时间数据+1秒
        GPS->second++; //加一秒
        if(GPS->second>59){
            GPS->second=0;
            GPS->minute++;
            if(GPS->minute>59){
                GPS->minute=0;
                GPS->hour++;
            }
        }

//***************************************************
        GPS->hour+=8;
        if(GPS->hour>23)
        {
            GPS->hour-=24;
            GPS->day+=1;
            if(GPS->month==2 ||
                    GPS->month==4 ||
                    GPS->month==6 ||
                    GPS->month==9 ||
                    GPS->month==11 ){
                if(GPS->day>30){
                    GPS->day=1;
                    GPS->month++;
                }
            }
            else{
                if(GPS->day>31){
                    GPS->day=1;
                    GPS->month++;
                }
            }
            if(GPS->year % 4 == 0 ){//
                if(GPS->day > 29 && GPS->month ==2){
                    GPS->day=1;
                    GPS->month++;
                }
            }
            else{
                if(GPS->day>28 && GPS->month ==2){
                    GPS->day=1;
                    GPS->month++;
                }
            }
            if(GPS->month>12){
                GPS->month-=12;
                GPS->year++;
            }
        }
}


void gps::receive(void)
{
    int i=0;
    char c;
    char buf[1024];
    volatile bool t=true;

    for(int i=0;i<1024;i++)
    {
        GPS_BUF[i]='\0';
    }

    while (t)
    {
        if(read(Gpsfd,&c,1) < 0 )
        {
            qDebug()<<"read Gpsdev error!";
            return ;
        }

        buf[i++] = c;
        if(c == '\n')
        {
            strncpy(GPS_BUF,buf,i);
            i=0;
            t=false;
        }

     }
}

int gps::openGpsDev()
{

    Gpsfd = open("/dev/ttySAC1",O_RDWR);

    if (Gpsfd < 0)
    {
        qDebug()<<"Cannot open gps device"<<endl;
        close(Gpsfd);
        return -1;
    }
    qDebug()<<"open gps device OK!"<<endl;
    return Gpsfd;
}
int gps::closeGpsDev()
{
    close(Gpsfd);
    qDebug()<<"close gps device OK!"<<endl;
    return 0;
}

int gps::initGpsDev()
{

    set_opt(Gpsfd,4800,8,'N',1);
    return 0;

}
void gps::readGpsDev(void)
{
    receive();
    gps_parse();
    int lat=(int)(GPS->latitude*10000);
    int lon=(int)(GPS->longitude*10000);

//    qDebug()<<"gps"<<"lat="<<lat<<endl;
//    qDebug()<<"gps"<<"lon="<<lon<<endl;
}
int gps::set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    if  ( tcgetattr( fd,&oldtio)  !=  0)
    {
        perror("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag  |=  CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch( nEvent )
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

switch( nSpeed )
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
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if( nStop == 1 )
    {
        newtio.c_cflag &=  ~CSTOPB;
    }
    else if ( nStop == 2 )
    {
        newtio.c_cflag |=  CSTOPB;
    }
    newtio.c_cc[VTIME]  = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
		printf("com set error\n");
        return -1;
    }
	printf("set done!\n");
    return 0;
}