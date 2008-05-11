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

#import <Foundation/Foundation.h>

NSURLDownload * download(NSString * source, NSString * target)
{
    NSURL * url;
    NSURLRequest * request;
    NSURLDownload * download;

    url = [NSURL URLWithString: source];

    request = [NSURLRequest requestWithURL: url
		       cachePolicy: NSURLRequestReloadIgnoringLocalCacheData
		   timeoutInterval: 60.0];

    download = [[NSURLDownload alloc] initWithRequest: request
                                             delegate: nil];
    if (download) {
	[download setDeletesFileUponFailure: YES];
        [download setDestination:target allowOverwrite: YES];
    }

    return download;
}

int main(void)
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    download(@"http://slashdot.org/", @"foo1");
    download(@"http://slashdot.org/", @"foo2");
    download(@"http://slashdot.org/", @"foo3");
    download(@"http://slashdot.org/", @"foo4");
    download(@"http://slashdot.org/", @"foo5");
    download(@"http://slashdot.org/", @"foo6");
    download(@"http://slashdot.org/", @"foo7");
    download(@"http://slashdot.org/", @"foo8");
    download(@"http://slashdot.org/", @"foo9");
    download(@"http://slashdot.org/", @"foo10");
    download(@"http://slashdot.org/", @"foo11");

    [[NSRunLoop currentRunLoop] run];

    [pool drain];
    return 0;
}
