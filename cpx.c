/*
 *  Copyright (C) 2009 James Peach
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sysexits.h>
#include <fcntl.h>
#include <sys/stat.h>

/*
 * Usage: cpx SRC DST
 *
 * Trivial copy program to demonstrate that openx_np can correctly
 * perform ACL inheritance.
 */

#define CHECK(err, msg) do { \
    if (err == -1) { \
	perror(msg) ; return EX_IOERR; \
    } } while (0)

#define BUFSZ getpagesize()

int
copy_file(
	const char * src,
	const char * dst)
{
    int err;
    void * buf = malloc(BUFSZ);
    int srcfd, dstfd;
    filesec_t fsec = filesec_init();
    struct stat statbuf;

    printf("copying %s => %s\n", src, dst);

    srcfd = open(src, O_RDONLY);
    CHECK(srcfd, "open src");

    err = fstatx_np(srcfd, &statbuf, fsec);
    CHECK(err, "fstatx_np srcfd");

    /* In theory, kauth_acl_inherit ought to take the acl from the source and
     * copy the direct ACEs onto the ACL that is created from the parent of the
     * destination. In practice, let's see ...
     */
    dstfd = openx_np(dst, O_CREAT | O_RDWR, fsec);
    CHECK(dstfd, "openx_np dst");

    do {
	err = read(srcfd, buf, BUFSZ);
	if (err > 0) {
	    write(dstfd, buf, BUFSZ);
	}
    } while (err > 0);

    return 0;
}

int main(int argc, const char ** argv)
{
    struct stat statbuf;
    char * src, * dst;

    if (argc < 3) {
	printf("usage: cpx SRC DST\n");
	return EX_USAGE;
    }

    src = (char *)argv[1];
    dst = (char *)argv[2];

    if (stat(dst, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
	const char * fname = strrchr(src, '/') + 1;
	asprintf(&dst, "%s/%s", dst, fname);
    }

    return copy_file(src, dst);
}
