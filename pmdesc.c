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

#define PCP_CHECK_ERROR(expr, ret) \
	while ((ret = expr) < 0) \
	    { fprintf(stderr, "ERR: %s: %s\n", #expr, pmErrStr(ret)); exit(1); }

const char * semantics[] = 
{
    NULL,
    "PM_SEM_COUNTER",
    "PM_SEM_RATE",
    "PM_SEM_INSTANT",
    "PM_SEM_DISCRETE",
    NULL
};

void describe(const char * mname)
{
    pmID    pmid;
    pmDesc  pmdesc;
    int	    ret;

    PCP_CHECK_ERROR(pmLookupName(1, &mname, &pmid), ret);
    PCP_CHECK_ERROR(pmLookupDesc(pmid, &pmdesc), ret);

    printf("%s\n", mname);
    printf("\tpmid: %s\n", pmIDStr(pmid));
    printf("\ttype: %s\n", pmTypeStr(pmdesc.type));
    printf("\tindom: %s\n", pmInDomStr(pmdesc.indom));
    printf("\tsemantics: %s (%d)\n", semantics[pmdesc.sem], pmdesc.sem);
    printf("\tunits: %s\n", pmUnitsStr(&pmdesc.units));
}

int main(int argc, const char ** argv)
{
    int i;

    pmNewContext(PM_CONTEXT_HOST, "127.0.0.1");

    for (i = 1; i < argc; ++i)
    {
	describe(argv[i]);
    }

    return(0);
}
