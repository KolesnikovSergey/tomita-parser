#include "info.h"
#include "oldfile.h"

#include <cstdlib>

#if defined(_linux_)
    #include <fcntl.h>
#endif

#if defined(_win_)
    #include "winint.h"
#else
    #include <unistd.h>
#endif

#if defined(_unix_)
    #include <sys/types.h>
    #include <sys/sysctl.h>
#endif

size_t NSystemInfo::NumberOfCpus() {
#if defined(_win_)
    SYSTEM_INFO info;

    GetSystemInfo(&info);

    return info.dwNumberOfProcessors;
#elif defined(_linux_)
    unsigned ret;
    int fd, nread, column;
    char buf[512];
    static const char matchstr[] = "processor\t:";

    fd = open("/proc/cpuinfo", O_RDONLY);

    if (fd == -1) {
        abort();
    }

    column = 0;
    ret = 0;

    while (true) {
        nread = read(fd, buf, sizeof(buf));

        if (nread <= 0) {
            break;
        }

        for (int i = 0; i < nread; ++i) {
            const char ch = buf[i];

            if (ch == '\n') {
                column = 0;
            } else if (column != -1) {
                if (ch == matchstr[column]) {
                    column++;

                    if (column == sizeof(matchstr) - 1) {
                        column = -1;
                        ret++;
                    }
                } else {
                    column = -1;
                }
            }
        }
    }

    if (ret == 0) {
        abort();
    }

    close(fd);

    return ret;
#elif defined(_freebsd_) || defined(_darwin_)
    int mib[2];
    size_t len;
    unsigned ncpus = 1;

    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    len = sizeof(ncpus);
    if (sysctl(mib, 2, &ncpus, &len, (void *) 0, 0) == -1) {
        abort();
    }

    return ncpus;
#elif defined(_SC_NPROCESSORS_ONLN)
    return sysconf(_SC_NPROCESSORS_ONLN);
#else
    #error todo
#endif
}

size_t NSystemInfo::LoadAverage(double* la, size_t len) {
#if defined(_win_) || defined(_musl_)
    int ret = -1;
#else
    for (size_t i = 0; i < len; ++i) {
        la[i] = 0;
    }
    int ret = getloadavg(la, len);
#endif

    if (ret < 0) {
        for (size_t i = 0; i < len; ++i) {
            la[i] = 0;
        }

        ret = len;
    }

    return (size_t)ret;
}

static size_t NCpus;

size_t NSystemInfo::CachedNumberOfCpus() {
    if (!NCpus) {
        NCpus = NumberOfCpus();
    }

    return NCpus;
}

size_t NSystemInfo::GetPageSize() throw () {
#if defined(_win_)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    return sysInfo.dwPageSize;
#else
    return getpagesize();
#endif
}
