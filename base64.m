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

#import <stdint.h>
#import <stdbool.h>
#import <Foundation/Foundation.h>

static uint8_t base64_convert_char(const char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    else if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    else if (c >= '0' && c <= '9') return c - '0' + 52;

    /* Allow base64 variants. RFC3548 says that 62 may be encoded
     * using '-', but the emusic .emp base64 uses '.' for 62 and
     * '-' for the pad byte. This is ambiguous, so we prefer the
     * emusic encoding here.
     */
    switch (c) {
        case '.':
        case '+':
            return 62;
        case '_':
        case '/':
            return 63;
    }

    /* XXX raise exception? */
    return 0;
}

static bool base64_is_pad(const char c)
{
    switch(c) {
        case '-':
        case '=':
            return true;
        default:
            return false;
    }
}

NSData * base64_decode(NSData * input)
{
    NSMutableData * output;
    NSUInteger offset = 0;

    char inbytes[4];
    uint8_t outbytes[3];

    /* The actual amount we need is ((input length) / 4 ) * 3, but this is
     * close enough.
     */
    output = [NSMutableData dataWithCapacity: [input length]];

    /* Base64 is a sequence of 4-character code points. NSData will raise
     * NSRangeException if we walk off the end (ie. the data is not a multiple
     * of 4 bytes).
     */
    for (offset = 0; offset < [input length]; offset += 4) {
        [input getBytes: inbytes range: NSMakeRange(offset, 4)];

        /* Convert the base64 bytes to their 6-bit integer equivalents. */
        inbytes[0] = base64_convert_char(inbytes[0]);
        inbytes[1] = base64_convert_char(inbytes[1]);

        /* We now have 2 6-bit sequences, enough to concatenate 8 bits. */
        outbytes[0] = (inbytes[0] << 2) | ((inbytes[1] & 0x30) >> 4);

        /* Either the last byte or the last 2 bytes can be padding bytes. If
         * we hit a padding byte, then we are done, since they only come at
         * the end (by definition).
         */
        if (base64_is_pad(inbytes[2])) {
            [output appendBytes: outbytes length: 1];
            break;
        }

        inbytes[2] = base64_convert_char(inbytes[2]);
        outbytes[1] = ((inbytes[1] & 0x0F) << 4) | ((inbytes[2] & 0x3C) >> 2);

        if (base64_is_pad(inbytes[3])) {
            [output appendBytes: outbytes length: 2];
            break;
        }

        inbytes[3] = base64_convert_char(inbytes[3]);
        outbytes[2] = ((inbytes[2] & 0x03) << 6) | (inbytes[3] & 0x3F);

        /* Stash our 3 new bytes. */
        [output appendBytes: outbytes length: sizeof(outbytes)];
    }

    /* XXX must catch exception to avoid memory leak */
    return output;
}

#ifdef BASE64_TESTMAIN

int main(int argc, const char ** argv)
{
    int i;
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    for (i = 1; i < argc; ++i) {
        NSString * filename = [NSString stringWithCString: argv[i]
                                        encoding: NSUTF8StringEncoding];
        NSData * input = [NSData dataWithContentsOfFile: filename];

        NSData * output = base64_decode(input);

        NSString * printable = [output description];

        NSLog(@"%@\n", printable);

    }

    [pool drain];
    return 0;
}

#endif


/* vim: set et ts=4 sw=4: */
