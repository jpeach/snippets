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

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

const char PROGNAME[] = "scnotify";

CFStringRef cfstring(const char * str)
{
    return CFStringCreateWithCStringNoCopy(NULL,
		    str, kCFStringEncodingASCII, kCFAllocatorNull);
}

const char * utf8string(CFStringRef cfstr)
{
    return CFStringGetCStringPtr(cfstr, CFStringGetFastestEncoding(cfstr));
}

void sc_perror(const char * msg)
{
    fprintf(stderr, "%s: %s: %s\n",
	    PROGNAME, msg, SCErrorString(SCError()));
}

void sc_notify(SCDynamicStoreRef ds, CFArrayRef changes, void * ctx)
{
}

CFRange array_range(CFArrayRef array)
{
    return CFRangeMake(0, CFArrayGetCount(array));
}

void print_key(const void * data, void * context)
{
    CFStringRef key = (CFStringRef)data;
    printf("\t%s\n", utf8string(key));
}

void array_print(CFArrayRef array)
{
    CFArrayApplyFunction (array, array_range(array), print_key, NULL);
}

CFArrayRef sc_list_keys(SCDynamicStoreRef sc, const char * key)
{
    CFArrayRef keys;
    CFStringRef cfkey;

    cfkey = cfstring(key);

    keys = SCDynamicStoreCopyKeyList(sc, cfkey);
    if (!keys) {
	sc_perror(key);
	CFRelease(cfkey);
	return NULL;
    }

    CFRelease(cfkey);
    return keys;
}

void print_addresses(SCDynamicStoreRef sc)
{
    CFArrayRef links;

    links = sc_list_keys(sc, "State:/Network/Interface/.*/Link");
    if (links == NULL) {
	return;
    }

    array_print(links);
    CFRelease(links);
}

SCDynamicStoreRef sc_connect(void)
{
    SCDynamicStoreRef ref;
    SCDynamicStoreContext context = { 0 /* version */,
	NULL /* retain */, NULL /* release */, NULL /* copy */};
    
    ref = SCDynamicStoreCreate(NULL /* default allocator */,
		    cfstring(PROGNAME), sc_notify, &context);
    if (ref == NULL) {
	    sc_perror("SCDynamicStoreCreate");
    }

    return ref;
}

int main(int argc, const char ** argv)
{
    SCDynamicStoreRef sc;

    sc = sc_connect();

    print_addresses(sc);
    CFRelease(sc);
    return 0;
}

