/*
 * =====================================================================================
 *
 *       Filename:  user_time.c
 *
 *    Description:  user time 
 *
 *        Version:  1.0
 *        Created:  06/04/2013 11:47:47 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  echo "Q2hhbmdxaW5nLFpoYW8K" | base64 -d  (tain)
 *			Email:	echo "Y2hhbmdxaW5nLjEyMzBAMTYzLmNvbQo=" | base64 -d 
 *        Company:  FreedomIsNotFree.com
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <time.h>

//#include <netinet/in.h>/* define uint8_t & uint32_t */

int main(void)
{
	time_t curtime;
	struct tm tm = {
		.tm_year = 2013 - 1900,
		.tm_mon = 6 - 1,
		.tm_mday = 5,
		.tm_hour = 9,
		.tm_min = 28,
		.tm_sec = 0,
		.tm_isdst = 1,
	};

	time(&curtime);
	printf("curtime = %ld\n", curtime);

	printf("mktim = %ld\n", mktime(&tm));
	return 0;
}

