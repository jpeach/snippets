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

#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

/* from smbd/notify_kernel.c */
#ifndef RT_SIGNAL_NOTIFY
#define RT_SIGNAL_NOTIFY (SIGRTMIN+2)
#endif

/* from smbd/aio.c */
#ifndef RT_SIGNAL_AIO
#define RT_SIGNAL_AIO (SIGRTMIN+3)
#endif

/* from smbd/oplock_linux.c */
#ifndef RT_SIGNAL_LEASE
#define RT_SIGNAL_LEASE (SIGRTMIN+1)
#endif

static void signal_handler(int sig, siginfo_t *info, void *unused)
{
    int do_nothing = 0;
}

int main(void)
{
    int result = 0;
    struct sigaction act = {0};

    act.sa_sigaction = signal_handler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset( &act.sa_mask );

    if (sigaction(RT_SIGNAL_LEASE, &act, 0) != 0) {
	    /* Failed to setup RT_SIGNAL_LEASE handler */
	    result += 1;
    }

    if (sigaction(RT_SIGNAL_NOTIFY, &act, 0) != 0) {
	    /* Failed to setup RT_SIGNAL_NOTIFY handler */
	    result += 10;
    }

    if (sigaction(RT_SIGNAL_AIO, &act, 0) != 0) {
	    /* Failed to setup RT_SIGNAL_AIO handler */
	    result += 100;
    }

    /* zero on success */
    return result;
}
