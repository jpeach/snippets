/*
 *  Copyright 2009 James Peach
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
#include <copyfile.h>

/* Trivial copyfile(3) driver. */

int main(int argc, const char ** argv)
{
    const char * srcpath;
    const char * dstpath;

    if (argc != 3) {
	fprintf(stderr, "usage: copyfile SRC DEST\n");
	return 1;
    }

    srcpath = argv[1];
    dstpath = argv[2];

    if (copyfile(srcpath, dstpath, NULL, COPYFILE_ALL) != 0) {
	perror("copyfile");
	return 1;
    }

    return 0;
}


