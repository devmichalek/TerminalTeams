#pragma once
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

class TTUtilsSyscall {
public:
    TTUtilsSyscall() = default;
    virtual ~TTUtilsSyscall() {}
    TTUtilsSyscall(const TTUtilsSyscall&) = delete;
    TTUtilsSyscall(TTUtilsSyscall&&) = delete;
    TTUtilsSyscall& operator=(const TTUtilsSyscall&) = delete;
    TTUtilsSyscall& operator=(TTUtilsSyscall&&) = delete;

    virtual sem_t* sem_open(const char* name, int oflag) const {
        return ::sem_open(name, oflag);
    }

    virtual int sem_post(sem_t* sem) const {
        return ::sem_post(sem);
    }

    virtual int sem_timedwait(sem_t* sem, const struct timespec* abs_timeout) const {
        return ::sem_timedwait(sem, abs_timeout);
    }

    virtual void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) const {
        return ::mmap(addr, length, prot, flags, fd, offset);
    }

    virtual int shm_open(const char* name, int oflag, mode_t mode) const {
        return ::shm_open(name, oflag, mode);
    }

    virtual int shm_unlink(const char* name) const {
        return ::shm_unlink(name);
    }

    virtual int clock_gettime(clockid_t clk_id, struct timespec* tp) const {
        return ::clock_gettime(clk_id, tp);
    }
};
