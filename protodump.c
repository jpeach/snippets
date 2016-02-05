#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

/*
 * https://developers.google.com/protocol-buffers/docs/encoding?hl=en
 */

#define FIELD_TYPE_VARINT       0
#define FIELD_TYPE_FIXED64      1
#define FIELD_TYPE_BYTES        2
#define FIELD_TYPE_GRP_START    3
#define FIELD_TYPE_GRP_END      4
#define FIELD_TYPE_FIXED32      5

static uint8_t
next(int fd)
{
    uint8_t byte;
    read(fd, &byte, 1);
    printf("read 0x%x\n", byte);
    return byte;
}

static uint64_t
varint(int fd)
{
    union {
        uint8_t bytes[8];
        uint64_t    u64;
    } value;
    unsigned count = 0;

    value.u64 = 0;
    do {
        uint8_t byte = next(fd);

        // Fill the bytes from the end since we are x86 (LSB).
        value.bytes[count] = byte & 0x7f;
        printf("value is 0x%0x\n", value.bytes[count]);
        printf("byte is 0x%0x\n", byte);

        // If MSB is set, keep reading otherwise we are done.
        if (!(byte & 0x80)) {
            break;
        }
    } while (++count < sizeof(value));

    return value.u64;
}

// read a field type.
static int
field(int fd)
{
    uint64_t varint = varint(fd);
    int wiretype;

    // The last three bits of the number store the wire type
    wiretype = varint & 0x00000007;
    printf("wiretype is %d\n");

}

int main(int argc, const char ** argv)
{
    printf("%llu\n", varint(STDIN_FILENO));
}

