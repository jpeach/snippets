#define ENABLE_GETTEXT

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <string>

#include <linux/quota.h>
#include <sys/quota.h>
#include <xfs/xfs.h>
#include <xfs/xqm.h>

// Usr libblkid to map dev_t to device paths.
#include <blkid/blkid.h>

typedef int errno_t;

errno_t
xfs_enable_quota(int * dirfd)
{
    return 0;
}

errno_t
xfs_getattr(int fd, struct fsxattr& attr)
{
    if (xfsctl(nullptr, fd, XFS_IOC_FSGETXATTR, &attr) == -1) {
        return errno;
    }

    return 0;
}

errno_t
xfs_setattr(int fd, struct fsxattr& attr)
{
    if (xfsctl(nullptr, fd, XFS_IOC_FSSETXATTR, &attr) == -1) {
        return errno;
    }

    return 0;
}

errno_t
xfs_setquota(const std::string& directory, prid_t projid, off_t nbytes)
{
  struct stat statbuf;
  char * devname = nullptr;

  if (::stat(directory.c_str(), &statbuf) == -1) {
    return errno;
  }

  if (!S_ISDIR(statbuf.st_mode)) {
    return ENOTDIR;
  }

  devname = blkid_devno_to_devname(statbuf.st_dev);
  if (devname == nullptr) {
    return ENODEV;
  }

  fs_disk_quota_t quota = {0};
  quota.d_version = FS_DQUOT_VERSION;
  quota.d_flags = XFS_PROJ_QUOTA;
  quota.d_fieldmask = FS_DQ_BSOFT | FS_DQ_BHARD;
  quota.d_id = projid;

  // Space limits are in basic blocks (512 bytes).
  quota.d_blk_hardlimit = nbytes / 512;
  quota.d_blk_softlimit = nbytes / 512;

  if (::quotactl(QCMD(Q_XSETQLIM, PRJQUOTA), devname,
        projid, reinterpret_cast<caddr_t>(&quota)) == -1) {
    free(devname);
    return errno;
  }

  free(devname);
  return 0;
}

errno_t
xfs_setproject(int dirfd, prid_t projid)
{
    struct fsxattr attr;
    errno_t error;

    error = xfs_getattr(dirfd, attr);
    if (error != 0) {
        perror("xfs getattr");
        return error;
    }

    // Set the project inherit flag on this directory.
    attr.fsx_xflags |= XFS_XFLAG_PROJINHERIT;
    attr.fsx_projid = projid;

    error = xfs_setattr(dirfd, attr);
    if (error != 0) {
        fprintf(stderr, "XFS setattr failed: %s (%d)\n", strerror(error), error);
        return error;
    }

    return ENOSYS;
}

int main(int argc, const char ** argv)
{
    prid_t projid = 4;
    off_t nbytes = 345 * 512;

    for (int i = 1; i < argc; ++i) {
        int dirfd = open(argv[i], O_RDONLY | O_DIRECTORY);
        if (dirfd == -1) {
            perror("open");
            continue;
        }

        if (platform_test_xfs_fd(dirfd) == 0) {
            fprintf(stderr, "%s is not XFS\n", argv[i]);
            close(dirfd);
            continue;
        }

        xfs_setproject(dirfd, projid);
        xfs_setquota(argv[i], projid, nbytes);
        xfs_setquota(argv[i], projid, 0);
        close(dirfd);
    }
    return EX_OK;
}
