#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <string.h>
#include "gps.h"
#include <termios.h>

class gps{
public:
	gps();
	~gps();
	void gps_parse();
	double gps::get_double_number(char *s);
	double gps::get_locate(double temp);
	int gps::GetComma(int num, char *str);
	void gps::UTC2BTC(date_time *GPS);


};