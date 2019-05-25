#include "os.h"
#include "kernel.h"
#include "mylib.h"

static void os_init();
static void os_run();

static _RegSet *os_interrupt(_Event ev, _RegSet *regs);

MOD_DEF(os) {
  .init = os_init,
  .run = os_run,
  .interrupt = os_interrupt,
};

static void os_init() {
    printf("Hello world!\nOS init\n");
    last_thread = -1;
}

char s1[50], s2[50], s3[50], s4[50];
static void printa()
{
    while(1)
    {
    int fd = vfs -> open("/dev/random", O_RDONLY);
    if(fd == -1)
        printf("No such file or directory!\n");
    memset(s1, 0, 50);
    vfs -> read(fd, (void*)s1, 50);
    printf("PID 1: %s\n", s1);
    }
    while(1);
}

static void printb()
{
    while(1)
    {
    int fd = vfs -> open("/dev/random", O_RDONLY);
    if(fd == -1)
        printf("No such file or directory!\n");
    memset(s2, 0, 50);
    vfs -> read(fd, (void*)s2, 50);
    printf("PID 2: %s\n", s2);
    }
    while(1);
}

static void printc()
{
    while(1)
    {
    int fd = vfs -> open("/dev/ra", O_RDONLY);
    if(fd == -1)
        printf("No such file or directory!\n");
    memset(s3, 0, 50);
    vfs -> read(fd, (void*)s3, 50);
    printf("PID 3: %s\n", s3);
    }
    while(1);
}

static void printd()
{
    while(1)
    {
    int fd = vfs -> open("/dev/random", O_RDONLY);
    if(fd == -1)
        printf("No such file or directory!\n");
    memset(s4, 0, 50);
    vfs -> read(fd, (void*)s4, 50);
    printf("PID 4: %s\n", s4);
    }
    while(1);
}

thread_t t1, t2, t3, t4;
static void os_run() {
    kmt -> create(&t1, printa, NULL);
    kmt -> create(&t2, printb, NULL);
    kmt -> create(&t3, printc, NULL);
    kmt -> create(&t4, printd, NULL);
    _intr_write(1); // enable interrupt
    while (1) ; // should never return
}

static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {
    switch(ev.event)
    {
        case _EVENT_IRQ_TIMER:
            //_putc('T');
            break;
        case _EVENT_IRQ_IODEV:
            _putc('I');
            break;
        case _EVENT_ERROR:
            _putc('x');
            _halt(1);
            break;
    }
    if(last_thread != -1)   //上一个不是主线程时，保存现场
    {
        memcpy(thread_list[last_thread].regset, (const void*)regs, REGSET_SIZE);
        thread_list[last_thread].running = 0;
    }
    thread_t *th = kmt -> schedule();
    // printf("[DEBUG]th - thread_list = %d\n", (int)(th - thread_list));
    last_thread = th -> pid;
    th -> running = 1;
    assert(th -> pid != -1);
    // printf("[DEBUG]th -> pid: %d, regset: 0x%x\n", th -> pid, (int)(th -> regset));
    return th -> regset; // this is allowed by AM
}
