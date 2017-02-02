/* dio.c: Probe XFS XFS_IOC_DIOINFO direct IO parameters. */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <xfs/xfs.h>

static void
dio(const char * path)
{
    struct dioattr attr = {0};
    int fd = open(path, O_DIRECT | O_RDONLY);

    if (fd == -1) {
        fprintf(stderr, "open(%s): %s\n", path, strerror(errno));
        return;
    }

    if (xfsctl(NULL, fd, XFS_IOC_DIOINFO, &attr) == -1) {
        perror("xfsctl");
        close(fd);
        return;
    }

    close(fd);

    printf("%s\n", path);
    printf("\td_mem     %d\n", attr.d_mem);
    printf("\td_miniosz %d\n", attr.d_miniosz);
    printf("\td_maxiosz %d\n", attr.d_maxiosz);
}

int
main(int argc, const char ** argv)
{
    for (int i = 1; i < argc; ++i) {
        dio(argv[i]);
    }

    return EX_OK;
}
