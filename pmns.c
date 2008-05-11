/*
 *  Copyright 2005 James Peach
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
#include <pcp/pmapi.h>

void pmns_lookup(const char * base)
{
    char ** children;
    int *   status;

    int	    count, i;

    count = pmGetChildrenStatus(base, &children, &status);
    if (count < 0)
	return;

    printf("%s\n", base);
    for (i = 0; i < count; ++i)
    {
	printf("\t%s (%sleaf)\n", children[i], 
		status[i] == PMNS_NONLEAF_STATUS ? "non " : "");
    }

}

int main(int argc, const char ** argv)
{
    int i;

    pmNewContext(PM_CONTEXT_HOST, "127.0.0.1");

    for (i = 1; i < argc; ++i)
    {
	pmns_lookup(argv[i]);
    }
}
