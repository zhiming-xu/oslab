#include <stdio.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

struct kvdb {
  FILE *fp;
  pthread_mutex_t mutex;
  char filename[256];
};
typedef struct kvdb kvdb_t;

int kvdb_open(kvdb_t *db, const char *filename);
int kvdb_close(kvdb_t *db);
int kvdb_put(kvdb_t *db, const char *key, const char *value);
char *kvdb_get(kvdb_t *db, const char *key);
