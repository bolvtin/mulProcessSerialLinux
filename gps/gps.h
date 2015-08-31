#ifndef _GPS_H
#define _GPS_H
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
void gps_parse(char *line,GPS_INFO *GPS);
void show_gps(GPS_INFO *GPS);
#endif
