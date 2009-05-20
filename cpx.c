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

#define SKIP_GROUP  0x0001
#define SKIP_OWNER  0x0002
#define SKIP_UUID   0x0004
#define SKIP_MODE   0x0008
#define SKIP_ACL    0x0010
#define SKIP_GRPUUID 0x0020

int
copy_file(
	const char * src,
	const char * dst,
	unsigned flags)
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

    if (flags & SKIP_GROUP) {
	filesec_set_property(fsec, FILESEC_GROUP, _FILESEC_UNSET_PROPERTY);
    }

    if (flags & SKIP_OWNER) {
	filesec_set_property(fsec, FILESEC_OWNER, _FILESEC_UNSET_PROPERTY);
    }

    if (flags & SKIP_UUID) {
	filesec_set_property(fsec, FILESEC_UUID, _FILESEC_UNSET_PROPERTY);
    }

    if (flags & SKIP_MODE) {
	filesec_set_property(fsec, FILESEC_MODE, _FILESEC_UNSET_PROPERTY);
    }

    if (flags & SKIP_ACL) {
	filesec_set_property(fsec, FILESEC_ACL, _FILESEC_UNSET_PROPERTY);
    }

    if (flags & SKIP_GRPUUID) {
	filesec_set_property(fsec, FILESEC_GRPUUID, _FILESEC_UNSET_PROPERTY);
    }

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

int main(int argc, char * const argv[])
{
    struct stat statbuf;
    char * src, * dst;
    unsigned flags = 0;
    int opt;

    while ((opt = getopt(argc, argv, "ogOGma")) != -1) {
	switch (opt) {
	    case 'o': flags |= SKIP_OWNER; break;
	    case 'O': flags |= SKIP_UUID; break;
	    case 'g': flags |= SKIP_GROUP; break;
	    case 'G': flags |= SKIP_GRPUUID; break;
	    case 'm': flags |= SKIP_MODE; break;
	    case 'a': flags |= SKIP_ACL; break;
	}
    }

    if (argc < 2) {
	printf("usage: cpx [-ogmaOG] SRC DST\n");
	return EX_USAGE;
    }

    argc -= optind;
    argv += optind;

    src = (char *)argv[0];
    dst = (char *)argv[1];

    if (stat(dst, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
	const char * fname = strrchr(src, '/') + 1;
	asprintf(&dst, "%s/%s", dst, fname);
    }

    return copy_file(src, dst, flags);
}
