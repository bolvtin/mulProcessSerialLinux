#include "SerDrive.h"  
#include <fcntl.h>      /*文件控制定义*/  
#include <stdio.h>  
#include <string.h>
//======================================  
int SerFd = -1;  
double a[3],w[3],Angle[3],T;
/******************************************************************/

void ProcessInit(void)  
{  
    SerFd = open("/dev/ttyAMA0", O_RDWR|O_NOCTTY|O_NDELAY);  
    if (1)  
    {  
        set_opt(SerFd, BAUD_115200, DATA_BIT_8, PARITY_NONE, STOP_BIT_1);//设置串口参数  
    }  
    else  
    {  
        printf("open_port ERROR !\n");  
    }  
}  
  

void DecodeIMUData(unsigned char chrTemp[])
{
	switch(chrTemp[1])
	{
	case 0x51:
		a[0] = ((short)(chrTemp[3]<<8|chrTemp[2]))/32768.0*16;
		a[1] = ((short)(chrTemp[5]<<8|chrTemp[4]))/32768.0*16;
		a[2] = ((short)(chrTemp[7]<<8|chrTemp[6]))/32768.0*16;
		T = ((short)(chrTemp[9]<<8|chrTemp[8]))/340.0+36.25;
		printf("a = %4.3f\t%4.3f\t%4.3f\t\r\n",a[0],a[1],a[2]);
		break;
	case 0x52:
		w[0] = ((short)(chrTemp[3]<<8|chrTemp[2]))/32768.0*2000;
		w[1] = ((short)(chrTemp[5]<<8|chrTemp[4]))/32768.0*2000;
		w[2] = ((short)(chrTemp[7]<<8|chrTemp[6]))/32768.0*2000;
		T = ((short)(chrTemp[9]<<8|chrTemp[8]))/340.0+36.25;
		printf("w = %4.3f\t%4.3f\t%4.3f\t\r\n",w[0],w[1],w[2]);
		break;
	case 0x53:
		Angle[0] = ((short)(chrTemp[3]<<8|chrTemp[2]))/32768.0*180;
		Angle[1] = ((short)(chrTemp[5]<<8|chrTemp[4]))/32768.0*180;
		Angle[2] = ((short)(chrTemp[7]<<8|chrTemp[6]))/32768.0*180;
		T = ((short)(chrTemp[9]<<8|chrTemp[8]))/340.0+36.25;
		printf("Angle = %4.2f\t%4.2f\t%4.2f\tT=%4.2f\r\n",Angle[0],Angle[1],Angle[2],T);
		break;
	}
}

int main(int argc, char *argv[])  
{  
    int nTmp = 0, usRxLength=0, i=0, j=0;  
    char chrBuffer[1024];
    unsigned char chrTemp[1024];
    ProcessInit();  
    send_data_tty(SerFd, "hello series\n",sizeof("hello series\n"));  
	//中断，20ms
    while (1)  
    {  
        nTmp = read_datas_tty(SerFd, chrBuffer, 100, 1024);  
        //if(nTmp)printf("%s",Buf);  
        if (0 < nTmp)  
        {  
		if (nTmp>0)
		{
			usRxLength += nTmp;
                while (usRxLength >= 11)
                {
                    memcpy(chrTemp,chrBuffer,usRxLength);
                    if (!((chrTemp[0] == 0x55) & ((chrTemp[1] == 0x51) | (chrTemp[1] == 0x52) | (chrTemp[1] == 0x53))))
                    {
                        for (i = 1; i < usRxLength; i++) chrBuffer[i - 1] = chrBuffer[i];
                        usRxLength--;
                        continue;
                    }
		    DecodeIMUData(chrTemp);
			/***********************************************************/
			//v[0] = v[0] + a[0] * dt;
			//x = x + v[0] * dt;
			/*************************************************************/
                    for (i = 11; i < usRxLength; i++) chrBuffer[i - 11] = chrBuffer[i];
                    usRxLength -= 11;
                    //printf("rcv len=%d,data:%s\n",nTmp,Buf);  
	            //printf("Data:%f\n",Buf);  
                    //send_data_tty(SerFd, Buf, nTmp);  
                }
		}
		for(i=0;i<1000;i++)
		{
		   for(j=0;j<2000;j++);
		}

        }  
    }  
}  