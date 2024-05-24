#pragma once
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>

class TTUtilsSyscall {
public:
    TTUtilsSyscall() = default;
    virtual ~TTUtilsSyscall() {}
    TTUtilsSyscall(const TTUtilsSyscall&) = delete;
    TTUtilsSyscall(TTUtilsSyscall&&) = delete;
    TTUtilsSyscall& operator=(const TTUtilsSyscall&) = delete;
    TTUtilsSyscall& operator=(TTUtilsSyscall&&) = delete;

    virtual sem_t* sem_open(const char* name, int oflag, mode_t mode, unsigned int value) const {
        return ::sem_open(name, oflag, mode, value);
    }

    virtual sem_t* sem_open(const char* name, int oflag) const {
        return ::sem_open(name, oflag);
    }

    virtual int sem_post(sem_t* sem) const {
        return ::sem_post(sem);
    }

    virtual int sem_timedwait(sem_t* sem, const struct timespec* abs_timeout) const {
        return ::sem_timedwait(sem, abs_timeout);
    }

    virtual int sem_unlink(const char* name) {
        return ::sem_unlink(name);
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

    virtual mqd_t mq_open(const char* name, int oflag, mode_t mode, struct mq_attr* attr) const {
        return ::mq_open(name, oflag, mode, attr);
    }

    virtual int mq_unlink(const char* name) const {
        return ::mq_unlink(name);
    }

    virtual int mq_timedsend(mqd_t mqdes, const char* msg_ptr, size_t msg_len, unsigned msg_prio, const struct timespec* abs_timeout) const {
        return ::mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
    }

    virtual ssize_t mq_timedreceive(mqd_t mqdes, char* msg_ptr, size_t msg_len, unsigned* msg_prio, const struct timespec* abs_timeout) const {
        return ::mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
    }

    virtual int ftruncate(int fd, off_t length) {
        return ::ftruncate(fd, length);
    }

    virtual int open(const char* pathname, int flags) {
        return ::open(pathname, flags);
    }

    virtual int close(int fd) {
        return ::close(fd);
    }

    virtual int unlink(const char* pathname) {
        return ::unlink(pathname);
    }

    virtual int mkfifo(const char* pathname, mode_t mode) {
        return ::mkfifo(pathname, mode);
    }

    virtual ssize_t read(int fd, void* buf, size_t count) {
        return ::read(fd, buf, count);
    }

    virtual ssize_t write(int fd, const void* buf, size_t count) {
        return ::write(fd, buf, count);
    }
};
