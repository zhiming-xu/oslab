#include "kvdb.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <sys/file.h>
#include <unistd.h>

int kvdb_open(kvdb_t *db, const char *filename)
{
    FILE * tmp;
    pthread_mutex_lock(&(db -> mutex));
    tmp = fopen(filename, "a+");
    db -> fp = tmp;
    pthread_mutex_unlock(&(db -> mutex));
    if(tmp == NULL)
    {
        perror("File open error");
        return -1;
    }
    return 0;
}

int kvdb_close(kvdb_t* db)
{
    int success = 0;
    pthread_mutex_lock(&(db -> mutex));
    int fd = fileno(db -> fp);
    if(flock(fd, LOCK_EX) == -1)
    {
        perror("File lock error in close");
        success = -1;
    }
    if(fclose(db -> fp) != 0)
    {
        perror("File close error");
        success = -1;
    }
    system("sync");
    if(flock(fd, LOCK_UN) == -1)
    {
        perror("File unlock error in close");
        success = -1;
    }
    pthread_mutex_unlock(&(db -> mutex));
    return success;
}

int kvdb_put(kvdb_t *db, const char* key, const char* value)
{
    int success = 0;
    pthread_mutex_lock(&(db -> mutex));
    int fd = fileno(db -> fp);
    flock(fd, LOCK_EX);

    fseek(db->fp, 0, SEEK_END);
    fwrite(key,   1, strlen(key),   db->fp);
    fwrite("\n",  1, 1,             db->fp);
    fwrite(value, 1, strlen(value), db->fp);
    fwrite("\n",  1, 1,             db->fp);
    
    system("sync");
    if(flock(fd, LOCK_UN) == -1)
    {
        perror("File unlock error in put");
        success = -1;
    }
    pthread_mutex_unlock(&(db -> mutex));
    return success;
}

char* kvdb_get(kvdb_t *db, const char* key)
{
    char* ret = NULL;
    int success = 1;
    pthread_mutex_lock(&(db -> mutex));
    int fd = fileno(db -> fp);
    if(flock(fd, LOCK_EX) == -1)
    {    
        perror("File lock error in get");
        success = 0;
    }
    if(fseek(db -> fp, 0, SEEK_SET) == -1)
    {
        perror("File seek error in get");
        success = 0;
    }
    static char _key[1 << 20], _value[1 << 20];
    fseek(db->fp, 0, SEEK_SET);
    while (1 && success) {
        if (!fgets(_key, sizeof(_key), db->fp)) break;
        if (!fgets(_value, sizeof(_value), db->fp)) break;
        _key[strlen(_key) - 1] = '\0';
        _value[strlen(_value) - 1] = '\0';
        if (strcmp(key, _key) == 0) {
            if (!ret) free(ret);
            ret = strdup(_value);
        }
    }
    system("sync");
    if(flock(fd, LOCK_UN) == -1)
        perror("File unlock error in get");
    pthread_mutex_unlock(&(db -> mutex));
    return ret;
}
