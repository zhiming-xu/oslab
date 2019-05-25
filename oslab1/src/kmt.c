#include "os.h"
#include "kernel.h"
#include "mylib.h"

static int thread_ind = 0;
static int thread_now = -1;

static void kmt_sem_init(sem_t *sem, const char *name, int value)
{
    sem -> count = value;
    sem -> name = name;
    sem -> id = -1;
}

static void kmt_sem_wait(sem_t *sem)
{
    sem -> count--;
    if(sem -> count < 0)
    {
        sem -> id = thread_now;
        thread_list[thread_now].running = 0;
        thread_list[thread_now].runnable = 0;
        _yield();
    }
}

static void kmt_sem_signal(sem_t *sem)
{
    sem -> count++;
    if(sem -> id >= 0)
    {
        thread_list[sem -> id].runnable = 1;
        thread_list[sem -> id].running = 1;
        sem -> id = -1;
        _yield();
    }
}

static void kmt_init()
{
    printf("KMT init\n");
    for(int i = 0; i < MAX_THREAD_NUM; ++i)    
        thread_list[i].pid = -1;
}

static int kmt_create(thread_t *thread, void (*entry)(void *arg), void *arg)
{
    thread -> kstack = pmm -> alloc(THREAD_SIZE); 
    _Area tmp;
    tmp.start = thread -> kstack;
    tmp.end = (char*)tmp.start + THREAD_SIZE;
    thread -> regset = _make(tmp, entry, arg);
    assert(thread -> regset != NULL);
    thread -> running = 0;  // Current not running
    thread -> runnable = 1; // But can be run
    thread_ind++;
    thread -> pid = thread_ind;
    thread_list[thread_ind] = *thread;
    // for vfs
    for(int i=0; i<MAX_FD_NUM; ++i)
    {
        thread -> fds[i].fd = -1;
        thread -> fds[i].offset = -1;
        thread -> fds[i].mount = NULL;
        thread -> fds[i].inode = NULL;
    }
    char pid_s[5]; memset(pid_s, 0, 5);
    itoa(thread -> pid, pid_s);
    char path[20];  memset(path, 0, 20);
    strcpy(path, "/proc/");  strcat(path, pid_s);
    vfs -> create(procfs, path, O_RDONLY);
    return thread -> pid;
}

static void kmt_teardown(thread_t *thread)
{
    pmm -> free(thread -> kstack);
    thread -> regset = NULL;
    thread -> pid = -1;
    thread -> running = 0;
}

static thread_t* kmt_schedule()
{
    // printf("thread_now: %d, thread_ind: %d\n", thread_now, thread_ind);
    if(thread_now == -1)                //只有主线程
    {
        thread_now = thread_ind;
        assert(thread_list[thread_ind].pid != -1);
        // printf("thread_ind=%d\n", thread_now);
        return &thread_list[thread_ind];//返回刚刚创建的
    }
    else
    {
        thread_list[thread_now].running = 0;
        for(int i = thread_now + 1; i <= thread_ind; ++i)
        {
            if(thread_list[i].running)      //优先返回有信号量的
            {
                thread_now = i;
                return &thread_list[i];
                // printf("kmt_schedule, loop 1, return 1: i=%d\n", i);
            }
            else if(thread_list[i].pid == i && thread_list[i].runnable)
            {
                thread_now = i;
                return &thread_list[i];
                // printf("kmt_schedule, loop 1, return 2: i=%d\n", i);
            }
        }
        for(int i = 1; i < thread_now; ++i)
        {
            if(thread_list[i].running)      //优先返回有信号量的
            {
                thread_now = i;
                return &thread_list[i];
                // printf("kmt_schedule, loop 2, return 1: i=%d\n", i);
            }
            else if(thread_list[i].pid == i && thread_list[i].runnable)
            {
                thread_now = i;
                return &thread_list[i];
                // printf("kmt_schedule, loop 2, return 2: i=%d\n", i);
            }
        }
    }
    // printf("last return, thread_now=%d\n", thread_now);
    return &thread_list[thread_now];
}

static void kmt_spin_init(spinlock_t *lk, const char *name)
{
    lk -> name = name;
    lk -> locked = 0;
}

static void kmt_spin_lock(spinlock_t *lk)
{
    _intr_write(0);
    lk -> locked = 1;
}

static void kmt_spin_unlock(spinlock_t *lk)
{
    _intr_write(1);
    lk -> locked = 0;
}

MOD_DEF(kmt) {
    .init  = kmt_init,
    .create = kmt_create,
    .teardown = kmt_teardown,
    .schedule = kmt_schedule,
    .spin_init = kmt_spin_init,
    .spin_lock = kmt_spin_lock,
    .spin_unlock = kmt_spin_unlock,
    .sem_init = kmt_sem_init,
    .sem_wait = kmt_sem_wait,
    .sem_signal = kmt_sem_signal,
};
