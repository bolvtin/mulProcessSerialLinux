#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "gps.h"
static int GetComma(int num,char* str);       //获得str字符指针对应字符串中第num个逗号的位置
static void UTC2BTC(date_time *GPS);          //将GPS获得的UTC时间转换成北京时间
static double get_double_number(char *s);     //获得两个逗号中间的浮点数，并转成double值返回
void show_gps(GPS_INFO *GPS)//输出GPS分离原始数据后的可用数据（包含时间及经纬度）
{
   printf("DATE     : %d-%02d-%02d \n",GPS->D.year,GPS->D.month,GPS->D.day);
   printf("TIME     :  %02d:%02d:%02d \n",GPS->D.hour,GPS->D.minute,GPS->D.second);
   printf("Latitude : %10.4f %c\n",GPS->latitude,GPS->NS); 
   printf("Longitude: %10.4f %c\n",GPS->longitude,GPS->EW); 
}
////////////////////////////////////////////////////////////////////////////////
//解释gps发出的数据
//0      7  0   4 6   0     6 8 0        90         0  3      0        9   
//$GPRMC,091400,A,3958.9870,N,11620.3278,E,000.0,000.0,120302,005.6,W*62  
void gps_parse(char *line,GPS_INFO *GPS)
{
   int tmp;
   char c;
   char* buf=line;
   c=buf[5];
   if(c=='C'){//"GPRMC"
      GPS->D.hour   =(buf[ 7]-'0')*10+(buf[ 8]-'0');    //原始数据串中第7位（从0计起）为时钟的十位，而8为个位
      GPS->D.minute =(buf[ 9]-'0')*10+(buf[10]-'0');    //              9               分钟          10
      GPS->D.second =(buf[11]-'0')*10+(buf[12]-'0');    //              11              秒钟          12
      tmp = GetComma(9,buf);              //获得buf中第9个逗号的起始位置，即在数据buf中的下标
      GPS->D.day    =(buf[tmp+0]-'0')*10+(buf[tmp+1]-'0');        
      GPS->D.month  =(buf[tmp+2]-'0')*10+(buf[tmp+3]-'0');
      GPS->D.year   =(buf[tmp+4]-'0')*10+(buf[tmp+5]-'0')+2000;
      GPS->latitude =get_double_number(&buf[GetComma(3,buf)]);      //纬度
      GPS->NS       =buf[GetComma(4,buf)];                          //南s北n
      GPS->longitude=get_double_number(&buf[GetComma(5,buf)]);      //经度
      GPS->EW       =buf[GetComma(6,buf)];                          //东e西w
      UTC2BTC(&GPS->D);
   }
}
//获得在字符串s中第一个逗号前的数值，并转换为浮点数返回
static double get_double_number(char *s)
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
//返回str中第num个逗号的下一位置(从0起)
static int GetComma(int num,char *str)
{
  int i,j=0;
  int len=strlen(str);
  for(i=0;i<len;i++)
  {
     if(str[i]==',')j++;
     if(j==num)return i+1; 
  }
  return 0; 
}
//将世界时转换为北京时
static void UTC2BTC(date_time *GPS)
{
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
  GPS->hour+=8;   //北京时间与世界时间相差8个时区，即相差8个钟
  if(GPS->hour>23)
  {
     GPS->hour-=24;
     GPS->day+=1;
     if(GPS->month==2 || GPS->month==4 || GPS->month==6 || GPS->month==9 || GPS->month==11 ){
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
   if((GPS->year % 4 ==0) && (GPS->year % 400 == 0 || GPS->year % 100 != 0)){  //判断闰年
       if(GPS->day > 29 && GPS->month ==2){   //闰年二月比平年二月多一天
          GPS->day=1;
          GPS->month++;
       }
   }
   else{
       if(GPS->day>28 &&GPS->month ==2){
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
