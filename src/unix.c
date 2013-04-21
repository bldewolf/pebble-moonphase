/*
Copyright (c) 2013 WhyIsThisOpen

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software 
without restriction, including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or 
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.
*/

#ifndef NOTPEBBLE
#include "pebble_os.h"
#endif

int TIME_ZONE_OFFSET = 0;

uint32_t get_epoch_seconds(PblTm *current_time) {
	uint32_t unix_time;

	// from: http://forums.getpebble.com/discussion/4324/watch-face-unix-time
	unix_time = ((0-TIME_ZONE_OFFSET)*3600) + /* time zone offset */
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

