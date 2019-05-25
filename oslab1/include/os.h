#ifndef __OS_H__
#define __OS_H__

#ifdef NDEBUG
    #define assert(ignore) ((void)0)
#else
    #define assert(cond) \
    do { \
        if (!(cond)) { \
            printf("Assertion fail at %s:%d\n", __FILE__, __LINE__); \
                _halt(1); \
        } \
    } while (0)
#endif

#include "kernel.h"
#define THREAD_SIZE 4096
#define MAX_THREAD_NUM 20
#define REGSET_SIZE sizeof(_RegSet)
#define PATH_LEN 32
#define MAX_FILE_NUM 64
#define MAX_FD_NUM 16
#define MAX_BLOCK_NUM 4
#define BLOCK_SIZE 4096
#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR 3

static inline void puts(const char *p) {
  for (; *p; p++) {
    _putc(*p);
  }
}

typedef struct spinlock {
    uint32_t locked;    // Locked or not ?
    const char *name;         // Name of the lock
} spinlock_t;

typedef struct semaphore {
    int count;
    const char *name;
    int id;
} sem_t;

typedef struct inode{
    int id;
    size_t size;
    int flags;
    int num_block;
    void *block[MAX_BLOCK_NUM];
} inode_t;

typedef struct filemap{
    char path[PATH_LEN];
    inode_t inode;
} filemap_t;

typedef struct filesystem{
    char root[PATH_LEN];
    int num_file;
    filemap_t FM[MAX_FILE_NUM];
} filesystem_t;

filesystem_t *procfs;
filesystem_t *devfs;
filesystem_t *kvfs;

typedef struct file{
    int fd;
    filesystem_t *mount;
    off_t offset;
    inode_t *inode;
} file_t;

typedef struct thread {
    _RegSet *regset;
    void* kstack;
    int pid;
    int running;        // Running or not
    int runnable;       // Can be run or not
    file_t fds[MAX_FD_NUM];
} thread_t;

thread_t thread_list[MAX_THREAD_NUM];
int last_thread;
#endif
