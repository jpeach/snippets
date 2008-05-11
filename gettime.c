/* 
 *  Copyright 2006 James Peach
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void format_time(const struct timespec * tv)
{
    fprintf(stdout, "%lu.%09lu\n",
	    tv->tv_sec, tv->tv_nsec);
}

void format_timediff(const struct timespec * now,
		     const struct timespec * then)
{
    struct timespec diff = {0};

    diff.tv_sec = now->tv_sec - then->tv_sec;
    if (now->tv_nsec < then->tv_nsec)
    {
	diff.tv_sec--;
	diff.tv_nsec = 1000000000 + now->tv_nsec - then->tv_nsec;
    }
    else
    {
	diff.tv_nsec = now->tv_nsec - then->tv_nsec;
    }

    format_time(&diff);
}

void xtime(clockid_t c, struct timespec * tv)
{
    if (clock_gettime(c, tv) < 0)
    {
	perror("clock_gettime");
	exit(1);
    }
}

int main(int argc, const char ** argv)
{
    struct timespec res = {0};
    struct timespec now = {0};
    struct timespec then = {0};

    clock_getres(CLOCK_MONOTONIC, &res);
    fprintf(stdout, "monotonic clock res: ");
    format_time(&res);


    xtime(CLOCK_MONOTONIC, &then);
    for (;;)
    {
	xtime(CLOCK_MONOTONIC, &now);
	format_timediff(&now, &then);
	then = now;
    }

    return(0);
}

