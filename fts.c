#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fts.h>

static const char *
ftype(int f)
{
#define entry(x) case x: return #x
    static char msg[64];

    switch (f) {
    entry(FTS_D);       /* preorder directory */
    entry(FTS_DC);      /* directory that causes cycles */
    entry(FTS_DEFAULT); /* none of the above */
    entry(FTS_DNR);     /* unreadable directory */
    entry(FTS_DOT);     /* dot or dot-dot */
    entry(FTS_DP);      /* postorder directory */
    entry(FTS_ERR);     /* error; errno is set */
    entry(FTS_F);       /* regular file */
    entry(FTS_INIT);    /* initialized only */
    entry(FTS_NS);      /* stat(2) failed */
    entry(FTS_NSOK);    /* no stat(2) requested */
    entry(FTS_SL);      /* symbolic link */
    entry(FTS_SLNONE);  /* symbolic link without target */
    entry(FTS_W);       /* whiteout object */

    default:
        snprintf(msg, sizeof(msg), "UNKNOWN (%d)", f);
        return msg;
    }

#undef entry
}

static void
do_fts(const char * path)
{
    char * paths[] = { (char *)path, NULL };
    FTS * fts;
    int options = FTS_PHYSICAL | FTS_NOCHDIR;

    fts = fts_open(paths, options, NULL);
    if (fts == NULL) {
        printf("fts_open(%s): %s\n", path, strerror(errno));
        return;
    }

    assert(fts != NULL);

    for (FTSENT * e = fts_read(fts); e; e = fts_read(fts)) {
        printf("%-12s level %d %s\n",
                ftype(e->fts_info),
                e->fts_level,
                e->fts_path);
    }

    fts_close(fts);
}

int
main(int argc, const char ** argv)
{
    for (int i = 1; i < argc; ++i) {
        do_fts(argv[i]);
    }

    return 0;
}

