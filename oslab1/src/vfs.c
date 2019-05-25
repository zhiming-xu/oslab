#include <os.h>
#include <mylib.h>

static void vfs_init();
static int vfs_access(const char* path, int mode);
static int vfs_mount(const char* path, filesystem_t *fs);
static int vfs_unmount(const char* path);
static int vfs_open(const char* path, int flags);
static void vfs_create(filesystem_t *fs, char *path, int flags);
static ssize_t vfs_read(int fd, void *buf, size_t bytes);
static ssize_t vfs_write(int fd, void *buf, size_t bytes);
static off_t vfs_lseek(int fd, off_t offset, int whence);
static int vfs_close(int fd);

MOD_DEF(vfs){
    .init = vfs_init,
    .access = vfs_access,
    .mount = vfs_mount,
    .unmount = vfs_unmount,
    .open = vfs_open,
    .create = vfs_create,
    .read = vfs_read,
    .write = vfs_write,
    .lseek = vfs_lseek,
    .close = vfs_close,
};

const char cpuinfo[] = "Intel Core(TM) Skylake, i7-6600U @ 2.8GHz\n";
const char meminfo[] = "[0x00500000, 0x08000000)\n";

static void vfs_init()
{
    procfs = (filesystem_t*)pmm -> alloc(sizeof(filesystem_t));
    devfs = (filesystem_t*)pmm -> alloc(sizeof(filesystem_t));
    kvfs = (filesystem_t*)pmm -> alloc(sizeof(filesystem_t));
    if(procfs == NULL || devfs == NULL || kvfs == NULL)
    {
        printf("Filesystem init error!\n");
        return;
    }
    vfs -> mount("/proc", procfs);
    vfs -> mount("/dev", devfs);
    vfs -> mount("/", kvfs);
    vfs ->create(procfs, "/proc/cpuinfo", O_RDONLY);
    vfs ->create(procfs, "/proc/meminfo", O_RDONLY);
    vfs ->create(devfs, "/dev/null", O_RDWR);
    vfs ->create(devfs, "/dev/zero", O_RDWR);
    vfs ->create(devfs, "/dev/random", O_RDWR);

    printf("VFS init\n");
}

static int vfs_access(const char* path, int mode)
{
    // TODO()
    return 1;
}

static int vfs_mount(const char* path, filesystem_t *fs)
{
    strcpy(fs -> root, path);
    fs -> num_file = 0;
    return 0;
}

static int vfs_unmount(const char* path)
{
    return -1;
}

static void vfs_create(filesystem_t *fs, char* path, int flags)
{
    strcpy(fs -> FM[fs -> num_file].path, path);
    fs -> FM[fs -> num_file].inode.size = 0;
    fs -> FM[fs -> num_file].inode.num_block = 0;
    fs -> FM[fs -> num_file].inode.id = fs -> num_file;
    fs -> FM[fs -> num_file].inode.flags = flags;
    for(int i = 0; i < MAX_BLOCK_NUM; ++i)
        fs -> FM[fs -> num_file].inode.block[i] = NULL;
    if(strcmp("/proc/cpuinfo", path) == 0)
    {
        fs -> FM[fs -> num_file].inode.num_block = 1;
        fs -> FM[fs -> num_file].inode.block[0] = (void*)pmm -> alloc(BLOCK_SIZE);
        memcpy(fs -> FM[fs -> num_file].inode.block[0], cpuinfo, strlen(cpuinfo));
        fs -> FM[fs -> num_file].inode.size = strlen(cpuinfo);
    }
    else if(strcmp("/proc/meminfo", path) == 0)
    {
        fs -> FM[fs -> num_file].inode.num_block = 1;
        fs -> FM[fs -> num_file].inode.block[0] = (void*)pmm -> alloc(BLOCK_SIZE);
        memcpy(fs -> FM[fs -> num_file].inode.block[0], meminfo, strlen(meminfo));
        fs -> FM[fs -> num_file].inode.size = strlen(meminfo);
    }
    else if(strstr(path, "/proc") != NULL)
    {
        fs -> FM[fs -> num_file].inode.num_block = 1;
        fs -> FM[fs -> num_file].inode.block[0] = (void*)pmm -> alloc(BLOCK_SIZE);
        char* pid_s = strcut(path, strlen("/proc/"));
        char  pid_info[50];
        strcpy(pid_info, "PID: ");strcat(pid_info, pid_s);
        strcat(pid_info, "\nStatus: Running\n");
        memcpy(fs -> FM[fs -> num_file].inode.block[0], pid_info, strlen(pid_info));
        fs -> FM[fs -> num_file].inode.size = strlen(pid_info);
    }
    /*
    else if(strcmp(path, "/dev/null") == 0)
    {
        fs -> FM[fs -> num_file].inode.num_block = 1;
        fs -> FM[fs -> num_file].inode.block[0] = (void*)pmm -> alloc(BLOCK_SIZE);
        memcpy(fs -> FM[fs -> num_file].inode.block[0], "\0\n", strlen("\0\n"));
        fs -> FM[fs -> num_file].inode.size = strlen("\0\n"); 
    }
    else if(strcmp(path, "/dev/zero") == 0)
    {
        fs -> FM[fs -> num_file].inode.num_block = 1;
        fs -> FM[fs -> num_file].inode.block[0] = (void*)pmm -> alloc(BLOCK_SIZE);
        memcpy(fs -> FM[fs -> num_file].inode.block[0], "0", strlen("0"));
        fs -> FM[fs -> num_file].inode.size = strlen("0"); 
    }
    else if(strcmp(path, "/dev/random") == 0)
    {
        fs -> FM[fs -> num_file].inode.num_block = 1;
        fs -> FM[fs -> num_file].inode.block[0] = (void*)pmm -> alloc(BLOCK_SIZE);
        char ran[33];   itoa(rand(), ran);
        memcpy(fs -> FM[fs -> num_file].inode.block[0], ran, strlen(ran));
        fs -> FM[fs -> num_file].inode.size = strlen(ran); 
    }
    */
    fs -> num_file++;
}

static int vfs_open(const char* path, int flags)
{
    filesystem_t *fs = NULL;
    if(strstr(path, "/proc") != NULL)
        fs = procfs;
    else if(strstr(path, "/dev") != NULL)
        fs = devfs;
    else if(strstr(path, "/") != NULL)
        fs = kvfs;
    else
    {
        printf("Invalid path!\n");
        return -1;
    }
    if(last_thread == -1)
    {
        printf("No current thread!\n");
        return -1;
    }
    for(int i = 0; i < fs -> num_file; ++i)
    {
        if(strcmp(fs -> FM[i].path, path) == 0)
        {
            int j = 0;
            while(thread_list[last_thread].fds[j].fd != -1)
                j++;
            assert(j < MAX_FD_NUM);
            thread_list[last_thread].fds[j].fd = j;
            thread_list[last_thread].fds[j].mount = fs;
            thread_list[last_thread].fds[j].offset = 0;
            thread_list[last_thread].fds[j].inode = &(fs -> FM[i].inode);
            char* pid_s = (char*)pmm -> alloc(20);
            strcpy(pid_s, path); pid_s = strcut(pid_s, strlen("/proc/"));
            if('0' <= pid_s[0] && pid_s[0] <= '9')
            {
                char pid_info[50];
                int tpid = atoi(pid_s);
                strcpy(pid_info, "PID: ");strcat(pid_info, pid_s);
                if(thread_list[tpid].running)
                    strcat(pid_info, "\nStatus: Running\n");
                else
                    strcat(pid_info, "\nStatus: Pending\n");
                memcpy(fs -> FM[i].inode.block[0], pid_info, strlen(pid_info));
            }
            return j;
        }
    }
    printf("vfs_open: Shouldn't reach here!\n");
    return -1;
}

static ssize_t vfs_read(int fd, void *buf, size_t bytes)
{
    if(last_thread == -1)
    {
        printf("No current thread!\n");
        return -1;
    }
    file_t *f = &(thread_list[last_thread].fds[fd]);
    if(f -> fd == -1)
        return -1;
    if(strcmp(f -> mount -> root, "/dev") == 0)
    {
        switch(f -> inode -> id)
        {
            case 0:
                return 0;
                break;
            case 1:
                memcpy(buf, "0\0", 2);
                return 1;
                break;
            case 2:
            {
                char s[33];
                itoa(rand(), s);
                memcpy(buf, s, strlen(s));
                return strlen(s);
                break;
            }
        }
    }
    if((f -> inode -> flags & O_RDONLY) == 0 || bytes <= 0)
        return 0;
    else if(bytes > f -> inode -> size)
        bytes = f -> inode -> size;
    if(bytes + f -> offset > f -> inode -> num_block * BLOCK_SIZE)
        bytes = f -> inode -> num_block * BLOCK_SIZE - f -> offset;
    int fblock = f -> offset / BLOCK_SIZE;
    int lblock = (bytes + f -> offset) / BLOCK_SIZE;
    void *dest = NULL; void *src = NULL;
    int len1 = 0;
    int len2 = bytes;
    for(int i = fblock; i <= lblock; ++i)
    {
        if(i == fblock && fblock == lblock)
        {
            dest = buf;
            src = f -> inode -> block[fblock] + f -> offset % BLOCK_SIZE;
            len1 = bytes;
        }
        else if(i == fblock)
        {
            dest = buf;
            src = f -> inode -> block[fblock] + f -> offset % BLOCK_SIZE;
            len1 = BLOCK_SIZE - f -> offset % BLOCK_SIZE;
        }
        else if(i == lblock)
        {
            src = f -> inode -> block[lblock];
            len1 = len2;
        }
        else
        {
            src = f -> inode -> block[i];
            len1 = BLOCK_SIZE;
        }
        memcpy(dest, src, len1);
        len2 -= len1;
        dest += len1;
    }
    return bytes;
}

static ssize_t vfs_write(int fd, void *buf, size_t bytes)
{
    if(last_thread == -1)
    {
        printf("No current thread!\n");
        return -1;
    }
    file_t *f = &(thread_list[last_thread].fds[fd]);
    if(f -> fd == -1 || bytes <= 0 || (f -> inode -> flags & O_WRONLY) == 0)
        return -1;
    if(strcmp(f -> mount -> root, "/dev") == 0)
        return 0;
    else if(bytes > f -> inode -> size)
        bytes = f -> inode -> size;
    int num_block = f -> inode -> num_block;
    while(num_block < (bytes + f -> offset) / BLOCK_SIZE + 1)
    {
        f -> inode -> block[num_block] = (void*)pmm -> alloc(BLOCK_SIZE);
        num_block++;
    }
    f -> inode -> num_block = num_block;
    f -> inode -> size += bytes;
    int lblock = f -> offset / BLOCK_SIZE;
    int rblock = (bytes + f -> offset) / BLOCK_SIZE;
    void *dest = NULL; void *src = NULL;
    int len1 = 0;
    int len2 = 0;
    for(int i = lblock; i <= rblock; ++i)
    {
        if(lblock == rblock)
        {
            dest = f -> inode -> block[lblock] + f -> offset % BLOCK_SIZE;
            src = dest;
            len1 = bytes;
        }
        else if(i == lblock)
        {
            dest = f -> inode -> block[lblock] + f -> offset % BLOCK_SIZE;
            src = dest;
            len1 = BLOCK_SIZE - f -> offset % BLOCK_SIZE;
        }
        else if(i == rblock)
        {
            dest = f -> inode -> block[rblock];
            src = buf + len2;
            len1 = bytes - len2;
        }
        else
        {
            dest = f -> inode -> block[i];
            src = buf + len2;
            len1 = BLOCK_SIZE;
        }
        memcpy(dest, src, len1);
        len2 += len1;
    }
    return bytes;
}

static off_t vfs_lseek(int fd, off_t offset, int whence)
{
    file_t *f = &(thread_list[last_thread].fds[fd]);
    switch(whence)
    {
        case SEEK_SET:
            f -> offset = offset;
            break;
        case SEEK_END:
            f -> offset = f -> inode -> size + offset;
            break;
        case SEEK_CUR:
            f -> offset += offset;
            break;
        default:
            printf("vfs_lseek: Shouldn't reach here!\n");
            return -1;
            break;
    }
    return f -> offset;
}

static int vfs_close(int fd)
{
    thread_list[last_thread].fds[fd].fd = -1;
    return 0;
}
