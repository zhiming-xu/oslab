#include "kvdb.h"
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>

pthread_t thread[2];
kvdb_t db;
//pthread_mutex_t mu;

void* thread1()
{
    //pthread_mutex_lock(&mu);
    printf("Thread 1 begin\n");
    kvdb_open(&db, "a.db");
    for(int i=0;i<300;++i)
    {
        char *tmp = "AAAAA";
        kvdb_put(&db, "Th1", tmp);
    }
    printf("Thread 1 end\n");
    //pthread_mutex_unlock(&mu);
    pthread_exit(NULL);
}

void* thread2()
{
    //pthread_mutex_lock(&mu);
    printf("Thread 2 begin\n");
    kvdb_open(&db, "a.db");
    for(int i=0;i<300;++i)
    {
        char *tmp = "BBBBB";
        kvdb_put(&db, "Th2", tmp);
    }
    printf("Thread 2 end\n");
    //pthread_mutex_unlock(&mu);
    pthread_exit(NULL);
}

void thread_create()
{
    pthread_create(&thread[0], NULL, thread1, NULL);
    pthread_create(&thread[1], NULL, thread2, NULL);
}

void thread_wait()
{
    pthread_join(thread[0], NULL);
    pthread_join(thread[1], NULL);
}

int main() {
  //kvdb_t db;
  const char *key = "operating-systems";
  char *value;
  //pthread_mutex_init(&mu, NULL);
  kvdb_open(&db, "a.db");
  kvdb_put(&db, key, "three-easy-pieces");
  value = kvdb_get(&db, key);
  printf("[%s]: [%s]\n", key, value);
  free(value);
  thread_create();
  thread_wait();
  return 0;
}
