typedef uint64 pthread_t;

int pthread_create(pthread_t *thread_stack, void *(*start_routine)(void *, void *),\
                  void *arg1, void *arg2);
int pthread_join(pthread_t thread_stack, void *retval);
int pthread_cancel(pthread_t thread);
