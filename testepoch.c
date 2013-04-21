#include<time.h>
#include<stdio.h>
#include<stdint.h>

#define NOTPEBBLE 1

typedef struct tm PblTm;

#include "src/unix.c"

/*
gcc -o testepoch testepoch.c
TZ= ./testepoch
*/

int main() {
	struct tm *tm;
	time_t t;
	int i;

	time(&t);

	t = 544835;

	printf("%d\n", t);

	for(i = 0; i < 60; i++) {
		printf("%d %s", current_phase(t), ctime(&t));
		t += 24*3600;
	}

	return 0;
}

int current_phase(time_t t) {
	struct tm *tm;
	int current;
	int past;
	int lp = 2551443;
	int phase;

	// using 1970   Jan  7  20:35 as a known New Moon and speculating from
	// there using lp, the length of a lunar month in seconds
	tm = localtime(&t);
	tm->tm_sec = 35;
	tm->tm_min = 20;
	tm->tm_hour = 7;
	current = get_epoch_seconds(tm);
	tm->tm_yday = 6;
	tm->tm_year = 70;
	past = get_epoch_seconds(tm);
	phase = (current - past) % lp;
	return (phase / (24*3600)) + 1;
}


int get_epoch_seconds2(struct tm *current_time) {
	int unix_time;

	// shamelessly stolen from WhyIsThisOpen's Unix Time source: http://forums.getpebble.com/discussion/4324/watch-face-unix-time
	unix_time = ((0)*3600) + /* time zone offset */
		+ current_time->tm_sec /* start with seconds */
		+ current_time->tm_min*60 /* add minutes */
		+ current_time->tm_hour*3600 /* add hours */
		+ current_time->tm_yday*86400 /* add days */
		+ (current_time->tm_year-70)*31536000 /* add years since 1970 */
		+ ((current_time->tm_year-69)/4)*86400 /* add a day after leap years, starting in 1973 */
		- ((current_time->tm_year-1)/100)*86400 /* remove a leap day every 100 years, starting in 2001 */
		+ ((current_time->tm_year+299)/400)*86400; /* add a leap day back every 400 years, starting in 2001*/


	return unix_time;
}
