/*
 *  Copyright 2008 James Peach
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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <poll.h>
#include <errno.h>
#include <time.h>

#include <fam.h>

#define arraysz(x) (sizeof(x)/sizeof(x[0])

static const char PROGNAME[] = "fammonitor";

void fam_print_event(const FAMEvent * ev)
{

    static const char * evstr[] = 
    {
	NULL,
	"FAMChanged",
	"FAMDeleted",
	"FAMStartExecuting",
	"FAMStopExecuting",
	"FAMCreated",
	"FAMMoved",
	"FAMAcknowledge",
	"FAMExists",
	"FAMEndExist"
    };

    time_t  now;
    char    tbuf[64];

    now = time(NULL);
    strftime(tbuf, sizeof(tbuf), "%T", localtime(&now));

    printf("%s: %s: %s event on %s\n",
	    PROGNAME, tbuf, evstr[ev->code], ev->filename);
}

FAMConnection * fam_init(void)
{
    FAMConnection * fam;

    if ((fam = calloc(sizeof(FAMConnection), 1)) == NULL)
	return(NULL);


    if (FAMOpen2(fam, "jpeach") < 0)
    {
	free(fam);
	return(NULL);
    }

    printf("%s: connected to fam\n", PROGNAME);
    return(fam);
}

int fam_add_path(FAMConnection * fam, const char * path)
{
    struct stat statbuf;
    char * fullpath;

    if (*path != '/')
    {
	char * cwd;

#ifdef _GNU_SOURCE
	cwd = get_current_dir_name();
#else
	cwd = getcwd(NULL, -1);
#endif

	fullpath = malloc(strlen(cwd) + strlen(path) + 5);
	sprintf(fullpath, "%s/%s", cwd, path);
	free(cwd);
    }
    else
    {
	fullpath = strdup(path);
    }

    if (stat(fullpath, &statbuf) < 0)
    {
	fprintf(stderr, "%s: stat %s: %s\n", PROGNAME,
		fullpath, strerror(errno));
	free(fullpath);
	return(-1);
    }

    printf("%s: monitoring %s\n", PROGNAME, fullpath);
    if (S_ISDIR(statbuf.st_mode))
    {
	FAMRequest rq;
	if (FAMMonitorDirectory(fam, fullpath, &rq, NULL) < 0)
	{
	    fprintf(stderr, "%s: FAMMonitorDirectory %s failed\n",
		    PROGNAME, fullpath);
	}
    }
    else
    {
	FAMRequest rq;
	if (FAMMonitorFile(fam, fullpath, &rq, NULL) < 0)
	{
	    fprintf(stderr, "%s: FAMMonitorFile %s failed\n",
		    PROGNAME, fullpath);
	}
    }

    free(fullpath);
    return(1);
}

int fam_monitor(FAMConnection * fam)
{
    FAMEvent ev;

#if USE_POLL
    for( ;; )
    {
	struct pollfd fds = {0};

	fds.fd = FAMCONNECTION_GETFD(fam);
	fds.events = POLLRDNORM | POLLWRNORM;

	if (poll(&fds, 1, -1) < 0)
	{
	    fprintf(stderr, "%s: poll: %s\n",
		    PROGNAME, strerror(errno));
	    exit(1);
	}

	if (FAMNextEvent(fam, &ev) >= 0)
	{
	    fam_print_event(&ev);
	}
    }
#else
    for ( ;; )
    {
	while (FAMPending(fam))
	{
	    if (FAMNextEvent(fam, &ev) >= 0)
	    {
		fam_print_event(&ev);
	    }
	}

	sleep(1);
    }

#endif

}

int main(int argc, const char ** argv)
{
    if (argc > 1)
    {
	int i;
	FAMConnection * fam;


	if ((fam = fam_init()) == NULL)
	    return(1);

	for (i = 1; i < argc; ++i)
	    fam_add_path(fam, argv[i]);

	fam_monitor(fam);
    }
    else
    {
	fprintf(stderr, "Usage: %s file [file ...]\n", PROGNAME);
	return(1);
    }

    return(0);
}

