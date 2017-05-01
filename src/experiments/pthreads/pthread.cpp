#include <stdio.h>
//Include needed to compile and link to pthread routines.
#include <pthread.h>

//Declare a function pointer that represents the routine that will be called
//when a thread is created.
void *hello(void *threadId) {
  int tid = *((int*)threadId);

  printf("Hello from thread #%d\n", tid);
}

int main(int argc, char *argv[]) {
  pthread_t thread1;
  pthread_t thread2;

  int threadId1 = 1;
  int threadId2 = 2;

  pthread_create(&thread1, NULL, hello, (void *) &threadId1);
  pthread_create(&thread2, NULL, hello, (void *) &threadId2);

  //This call has to be made to make sure the main method blocks
  //until the threads spawned from it complete! Otherwise, the main
  //thread could finish before any of the child threads and that's no good!
  pthread_exit(NULL);
}
