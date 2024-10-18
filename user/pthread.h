
/* data types */
typedef uint64 pthread_t;

typedef struct __mutex_t
{
  int locked;
} pthread_mutex_t;


/* interfaces */

int pthread_create(pthread_t *thread_stack, void *(*start_routine)(void *, void *),\
                  void *arg1, void *arg2);
int pthread_join(pthread_t thread_stack, void *retval);
int pthread_cancel(pthread_t thread);

void pthread_mutex_init(pthread_mutex_t *);
void pthread_mutex_lock(pthread_mutex_t *);
void pthread_mutex_unlock(pthread_mutex_t *);
