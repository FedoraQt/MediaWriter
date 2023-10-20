/*
 * Copyright (C) 2001-2013 Red Hat, Inc.
 *
 * Michael Fulbright <msf@redhat.com>
 * Dustin Kirkland  <dustin.dirkland@gmail.com>
 *      Added support for checkpoint fragment sums;
 *      Exits media check as soon as bad fragment md5sum'ed
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define _LARGEFILE64_SOURCE

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "libcheckisomd5.h"
#include "md5.h"

#ifdef __APPLE__
#define lseek64 lseek
#endif

#ifdef _WIN32
#define lseek64 _lseeki64
#endif

#ifdef _WIN32
size_t getpagesize()
{
    return 2048; // not really necessary for Windows
}
#endif

#define APPDATA_OFFSET 883
#define SIZE_OFFSET 84

/* Length in characters of string used for fragment md5sum checking */
#define FRAGMENT_SUM_LENGTH 60

#define MAX(x, y) ((x > y) ? x : y)
#define MIN(x, y) ((x < y) ? x : y)

/* finds primary volume descriptor and returns info from it */
/* mediasum must be a preallocated buffer at least 33 bytes long */
/* fragmentsums must be a preallocated buffer at least FRAGMENT_SUM_LENGTH+1 bytes long */
static int parsepvd(int isofd, char *mediasum, int *skipsectors, long long *isosize, int *supported, char *fragmentsums, long long *fragmentcount)
{
    int pagesize = getpagesize();
    unsigned char *buf_unaligned = (unsigned char *)malloc((2048LL + pagesize) * sizeof(unsigned char));
    unsigned char *buf = (buf_unaligned + (pagesize - ((uintptr_t)buf_unaligned % pagesize)));
    char buf2[512];
    char tmpbuf[512];
    int skipfnd, md5fnd, supportedfnd, fragsumfnd, fragcntfnd;
    unsigned int loc;
    long long offset;
    char *p;

    if (lseek64(isofd, (16LL * 2048LL), SEEK_SET) == -1)
        goto fail;

    offset = (16LL * 2048LL);
    for (; 1;) {
        if (read(isofd, buf, 2048) <= 0)
            goto fail;

        if (buf[0] == 1)
            /* found primary volume descriptor */
            break;
        else if (buf[0] == 255)
            /* hit end and didn't find primary volume descriptor */
            goto fail;
        offset += 2048LL;
    }

    /* read out md5sum */
    memcpy(buf2, buf + APPDATA_OFFSET, 512);
    buf2[511] = '\0';

    *supported = 0;

    md5fnd = 0;
    skipfnd = 0;
    fragsumfnd = 0;
    fragcntfnd = 0;
    supportedfnd = 0;
    loc = 0;
    while (loc < 512) {
        if (!strncmp(buf2 + loc, "ISO MD5SUM = ", 13)) {
            /* make sure we dont walk off end */
            if ((loc + 32 + 13) > 511)
                goto fail;

            memcpy(mediasum, buf2 + loc + 13, 32);
            mediasum[32] = '\0';
            md5fnd = 1;
            loc += 45;
            for (p = buf2 + loc; loc < 512 && *p != ';'; p++, loc++)
                ;
        } else if (!strncmp(buf2 + loc, "SKIPSECTORS = ", 14)) {
            char *errptr;

            /* make sure we dont walk off end */
            if ((loc + 14) > 511)
                goto fail;

            loc = loc + 14;
            for (p = tmpbuf; loc < 512 && buf2[loc] != ';'; p++, loc++)
                *p = buf2[loc];

            *p = '\0';

            *skipsectors = strtol(tmpbuf, &errptr, 10);
            if (errptr && *errptr) {
                goto fail;
            } else {
                skipfnd = 1;
            }

            for (p = buf2 + loc; loc < 512 && *p != ';'; p++, loc++)
                ;
        } else if (!strncmp(buf2 + loc, "RHLISOSTATUS=1", 14)) {
            *supported = 1;
            supportedfnd = 1;
            for (p = buf2 + loc; loc < 512 && *p != ';'; p++, loc++)
                ;
        } else if (!strncmp(buf2 + loc, "RHLISOSTATUS=0", 14)) {
            *supported = 0;
            supportedfnd = 1;
            for (p = buf2 + loc; loc < 512 && *p != ';'; p++, loc++)
                ;
        } else if (!strncmp(buf2 + loc, "FRAGMENT SUMS = ", 16)) {
            /* make sure we dont walk off end */
            if ((loc + FRAGMENT_SUM_LENGTH) > 511)
                goto fail;

            memcpy(fragmentsums, buf2 + loc + 16, FRAGMENT_SUM_LENGTH);
            fragmentsums[FRAGMENT_SUM_LENGTH] = '\0';
            fragsumfnd = 1;
            loc += FRAGMENT_SUM_LENGTH + 16;
            for (p = buf2 + loc; loc < 512 && *p != ';'; p++, loc++)
                ;
        } else if (!strncmp(buf2 + loc, "FRAGMENT COUNT = ", 17)) {
            char *errptr;
            /* make sure we dont walk off end */
            if ((loc + 17) > 511)
                goto fail;

            loc = loc + 17;
            for (p = tmpbuf; loc < 512 && buf2[loc] != ';'; p++, loc++)
                *p = buf2[loc];

            *p = '\0';

            *fragmentcount = strtol(tmpbuf, &errptr, 10);
            if (errptr && *errptr) {
                goto fail;
            } else {
                fragcntfnd = 1;
            }

            for (p = buf2 + loc; loc < 512 && *p != ';'; p++, loc++)
                ;
        } else {
            loc++;
        }

        if ((skipfnd & md5fnd & fragsumfnd & fragcntfnd) & supportedfnd)
            break;
    }

    if (!(skipfnd & md5fnd))
        goto fail;

    /* get isosize */
    *isosize = (buf[SIZE_OFFSET] * 0x1000000 + buf[SIZE_OFFSET + 1] * 0x10000 + buf[SIZE_OFFSET + 2] * 0x100 + buf[SIZE_OFFSET + 3]) * 2048LL;

    free(buf_unaligned);
    return offset;

fail:
    free(buf_unaligned);
    return -1LL;
}

/* mediasum is the sum encoded in media, computedsum is one we compute   */
/* both strings must be pre-allocated at least 33 chars in length        */
static int checkmd5sum(int isofd, char *mediasum, char *computedsum, checkCallback cb, void *cbdata)
{
    long long nread;
    int i, j;
    int appdata_start_offset, appdata_end_offset;
    int nattempt;
    int skipsectors;
    int supported;
    int current_fragment = 0;
    int previous_fragment = 0;
    int pagesize = getpagesize();
    unsigned int bufsize = 32768;
    unsigned char md5sum[16];
    unsigned char fragmd5sum[16];
    unsigned int len;
    unsigned char *buf_unaligned = (unsigned char *)malloc((bufsize + pagesize) * sizeof(unsigned char));
    unsigned char *buf = (buf_unaligned + (pagesize - ((uintptr_t)buf_unaligned % pagesize)));
    long long isosize, offset, pvd_offset, apoff;
    char fragmentsums[FRAGMENT_SUM_LENGTH + 1];
    char thisfragsum[FRAGMENT_SUM_LENGTH + 1];
    long long fragmentcount = 0;
    MD5_CTX md5ctx, fragmd5ctx;

    if ((pvd_offset = parsepvd(isofd, mediasum, &skipsectors, &isosize, &supported, fragmentsums, &fragmentcount)) < 0) {
        free(buf_unaligned);
        return ISOMD5SUM_CHECK_NOT_FOUND;
    }

    /*    printf("Mediasum = %s\n",mediasum); */

    /* rewind, compute md5sum */
    lseek64(isofd, 0LL, SEEK_SET);

    ISOMD5_Init(&md5ctx);

    offset = 0;
    apoff = pvd_offset + APPDATA_OFFSET;

    if (cb)
        cb(cbdata, 0, isosize - skipsectors * 2048);

    while (offset < isosize - skipsectors * 2048) {
        nattempt = MIN(isosize - skipsectors * 2048 - offset, bufsize);

        /*      printf("%lld %lld %lld %d\n", offset, isosize, isosize-SKIPSECTORS*2048, nattempt); */

        nread = read(isofd, buf, nattempt);
        if (nread <= 0)
            break;

        if (nread > nattempt) {
            nread = nattempt;
            lseek64(isofd, offset + nread, SEEK_SET);
        }
        /* overwrite md5sum we implanted with original data */
        if (offset < apoff && offset + nread >= apoff) {
            appdata_start_offset = apoff - offset;
            appdata_end_offset = MIN(appdata_start_offset + MIN(nread, 512), offset + nread - apoff);
            len = appdata_end_offset - appdata_start_offset;
            memset(buf + appdata_start_offset, ' ', len);
        } else if (offset >= apoff && offset + nread < apoff + 512) {
            appdata_start_offset = 0;
            appdata_end_offset = nread;
            len = appdata_end_offset - appdata_start_offset;
            memset(buf + appdata_start_offset, ' ', len);
        } else if (offset < apoff + 512 && offset + nread >= apoff + 512) {
            appdata_start_offset = 0;
            appdata_end_offset = apoff + 512 - offset;
            len = appdata_end_offset - appdata_start_offset;
            memset(buf + appdata_start_offset, ' ', len);
        }

        ISOMD5_Update(&md5ctx, buf, nread);
        if (fragmentcount) {
            current_fragment = offset * (fragmentcount + 1) / (isosize - skipsectors * 2048);
            /* if we're onto the next fragment, calculate the previous sum and check */
            if (current_fragment != previous_fragment) {
                memcpy(&fragmd5ctx, &md5ctx, sizeof(MD5_CTX));
                ISOMD5_Final(fragmd5sum, &fragmd5ctx);
                *computedsum = '\0';
                j = (current_fragment - 1) * FRAGMENT_SUM_LENGTH / fragmentcount;
                for (i = 0; i < FRAGMENT_SUM_LENGTH / fragmentcount; i++) {
                    char tmpstr[2];
                    snprintf(tmpstr, 2, "%01x", fragmd5sum[i]);
                    strncat(computedsum, tmpstr, 1);
                    thisfragsum[i] = fragmentsums[j++];
                }
                thisfragsum[j] = '\0';
                previous_fragment = current_fragment;
                /* Exit immediately if current fragment sum is incorrect */
                if (strcmp(thisfragsum, computedsum) != 0) {
                    free(buf_unaligned);
                    return ISOMD5SUM_CHECK_FAILED;
                }
            }
        }
        offset = offset + nread;
        if (cb && offset / nread % 256 == 0) {
            if (cb(cbdata, offset, isosize - skipsectors * 2048)) {
                free(buf_unaligned);
                return ISOMD5SUM_CHECK_ABORTED;
            }
        }
    }

    if (cb)
        cb(cbdata, isosize, isosize - skipsectors * 2048);

    free(buf_unaligned);

    ISOMD5_Final(md5sum, &md5ctx);

    *computedsum = '\0';
    for (i = 0; i < 16; i++) {
        char tmpstr[4];
        snprintf(tmpstr, 4, "%02x", md5sum[i]);
        strncat(computedsum, tmpstr, 2);
    }

    /*    printf("mediasum, computedsum = %s %s\n", mediasum, computedsum); */

    if (strcmp(mediasum, computedsum))
        return ISOMD5SUM_CHECK_FAILED;
    else
        return ISOMD5SUM_CHECK_PASSED;
}

static int doMediaCheck(int isofd, char *mediasum, char *computedsum, long long *isosize, int *supported, checkCallback cb, void *cbdata)
{
    int rc;
    int skipsectors;
    long long fragmentcount = 0;
    char fragmentsums[FRAGMENT_SUM_LENGTH + 1];

    if (parsepvd(isofd, mediasum, &skipsectors, isosize, supported, fragmentsums, &fragmentcount) < 0) {
        return ISOMD5SUM_CHECK_NOT_FOUND;
    }

    rc = checkmd5sum(isofd, mediasum, computedsum, cb, cbdata);

    return rc;
}

int mediaCheckFile(const char *file, checkCallback cb, void *cbdata)
{
    int isofd;
    int rc;
    char mediasum[33], computedsum[33];
    long long isosize;
    int supported;

#ifdef _WIN32
    isofd = open(file, O_RDONLY | O_BINARY);
#else
    isofd = open(file, O_RDONLY);
#endif

    if (isofd < 0) {
        return ISOMD5SUM_FILE_NOT_FOUND;
    }

    rc = doMediaCheck(isofd, mediasum, computedsum, &isosize, &supported, cb, cbdata);

    close(isofd);

    /*    printf("isosize = %lld\n", isosize);
     *    printf("%s\n%s\n", mediasum, computedsum);
     */

    return rc;
}

int mediaCheckFD(int fd, checkCallback cb, void *cbdata)
{
    int rc;
    char mediasum[33], computedsum[33];
    long long isosize;
    int supported;

    if (fd < 0) {
        return ISOMD5SUM_FILE_NOT_FOUND;
    }

    rc = doMediaCheck(fd, mediasum, computedsum, &isosize, &supported, cb, cbdata);

    return rc;
}

int printMD5SUM(char *file)
{
    int isofd;
    char mediasum[64];
    long long isosize;
    char fragmentsums[FRAGMENT_SUM_LENGTH + 1];
    long long fragmentcount = 0;
    int supported;
    int skipsectors;

    isofd = open(file, O_RDONLY);

    if (isofd < 0) {
        return ISOMD5SUM_FILE_NOT_FOUND;
    }

    if (parsepvd(isofd, mediasum, &skipsectors, &isosize, &supported, fragmentsums, &fragmentcount) < 0) {
        return ISOMD5SUM_CHECK_NOT_FOUND;
    }

    close(isofd);

    printf("%s:   %s\n", file, mediasum);
    if ((strlen(fragmentsums) > 0) && (fragmentcount > 0)) {
        printf("Fragment sums: %s\n", fragmentsums);
#ifdef _WIN32
        printf("Fragment count: %" PRId64 "\n", fragmentcount);
#else
        printf("Fragment count: %lld\n", fragmentcount);
#endif
        printf("Supported ISO: %s\n", supported ? "yes" : "no");
    }

    return 0;
}
