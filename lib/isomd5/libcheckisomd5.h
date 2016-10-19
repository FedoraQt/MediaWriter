#ifndef __LIBCHECKISOMD5_H__
#define  __LIBCHECKISOMD5_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ISOMD5SUM_CHECK_PASSED          1
#define ISOMD5SUM_CHECK_FAILED          0
#define ISOMD5SUM_CHECK_ABORTED         2
#define ISOMD5SUM_CHECK_NOT_FOUND       -1
#define ISOMD5SUM_FILE_NOT_FOUND        -2

/* for non-zero return value, check is aborted */
typedef int (*checkCallback)(void *, long long offset, long long total);

int mediaCheckFile(const char *iso, checkCallback cb, void *cbdata);
int mediaCheckFD(int fd, checkCallback cb, void *cbdata);
int printMD5SUM(char *file);

#ifdef __cplusplus
}
#endif
#endif
