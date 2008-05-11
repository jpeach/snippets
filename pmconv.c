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
#include <pcp/impl.h>
#include <pcp/pmda.h>

typedef int (*conv_test)(void);

/* Convert the time scale but not the value nor the dimensionality. */
int convert_time_scale(void)
{
    pmAtomValue ival;
    pmAtomValue oval;

    pmUnits iunits = PMDA_PMUNITS(1, -1, 0, PM_SPACE_KBYTE, PM_TIME_SEC, 0);
    pmUnits ounits = PMDA_PMUNITS(1, -1, 0, PM_SPACE_KBYTE, PM_TIME_NSEC, 0);

    ival.l = oval.l = 100;

    return(pmConvScale(PM_TYPE_32, &ival, &iunits, &oval, &ounits));
}

struct test
{
    conv_test	    ctest;
    const char *    cdesc;
} testlist[] =
{
    { convert_time_scale, "whether the time scale alone can be converted" },
    { NULL, NULL}
};


int main(int argc, const char ** argv)
{
    struct test * t;
    int ret;

    pmNewContext(PM_CONTEXT_HOST, "127.0.0.1");

    for (t = testlist; *t->ctest; ++t)
    {
	printf("testing %s: ", t->cdesc);
	if ((ret = t->ctest()) == 0)
	    printf("yes\n");
	else
	    printf("no, %s\n", pmErrStr(ret));
    }

    return(0);
}
