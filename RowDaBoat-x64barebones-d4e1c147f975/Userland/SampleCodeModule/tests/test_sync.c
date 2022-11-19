#include <stdint.h>
#include <usrlib.h>
#include "test_util.h"

uint64_t my_create_process_sync(char * name, void (*function)(int, char**)){
  char * argv[] = {name};
  return initializeProccess(function, 0, 1, argv, 0);
}

int32_t my_sem_open_sync(char *sem_id, uint64_t initialValue){
  return createSem(sem_id, initialValue);
}

int my_sem_wait_sync(sem_t sem_id){
  return semWait(sem_id);
}

int my_sem_post_sync(sem_t sem_id){
  return semPost(sem_id);
}

void my_sem_close_sync(sem_t sem_id){
  removeSem(sem_id);
}

void printNumber(int64_t integer){
  if (integer < 0 ){
    putchar('-'); 
    printint(-1* integer);
  }else
    printint(integer);
}

#define N 100000
#define SEM_ID "sem"
#define TOTAL_PAIR_PROCESSES 2

int64_t global;  //shared memory

void slowInc(int64_t *p, int64_t inc){
  int64_t aux = *p;

  // Simula mala suerte
  if(GetUniform(100) % 31 == 1)
    skipTurn(); 
    
  aux += inc;
  *p = aux;
}

void my_process_inc(){
  uint64_t i;

  int semid = my_sem_open_sync(SEM_ID, 1);
  if (semid == -1){
    print("ERROR OPENING SEM\n");
    return;
  }
  
  for (i = 0; i < N; i++){
    my_sem_wait_sync(semid);
    slowInc(&global, 1);
    my_sem_post_sync(semid);
  }

  my_sem_close_sync(semid);
  
  //printf("Final value: %d\n", global);
  print("Final value: "); printNumber(global); putchar('\n');
}

void my_process_dec(){
  uint64_t i;

  int semid = my_sem_open_sync(SEM_ID, 1);
  if (semid == -1){
    print("ERROR OPENING SEM\n");
    return;
  }
  
  for (i = 0; i < N; i++){
    my_sem_wait_sync(semid);
    slowInc(&global, -1);
    my_sem_post_sync(semid);
  }

  my_sem_close_sync(semid);

  //printf("Final value: %d\n", global);
  print("Final value: "); printNumber(global); putchar('\n');
}

void test_sync(){
  uint64_t i;

  global = 0;

  print("CREATING PROCESSES...\n");

  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){
    my_create_process_sync("my_process_inc", (void (*)(int, char**))my_process_inc);
    my_create_process_sync("my_process_dec", (void (*)(int, char**))my_process_dec);
  }
  
  // The last one should print 0
}

void my_process_inc_no_sem(){
  uint64_t i;
  for (i = 0; i < N; i++){
    slowInc(&global, 1);
  }

  //printf("Final value: %d\n", global);
  print("Final value: "); printNumber(global); putchar('\n');
}

void my_process_dec_no_sem(){
  uint64_t i;
  for (i = 0; i < N; i++){
    slowInc(&global, -1);
  }

  //printf("Final value: %d\n", global);
  print("Final value: "); printNumber(global); putchar('\n');
}

void test_no_sync(){
  uint64_t i;

  global = 0;

  print("CREATING PROCESSES...\n");

  for(i = 0; i < TOTAL_PAIR_PROCESSES; i++){
    my_create_process_sync("my_process_inc_no_sem", (void (*)(int, char**))my_process_inc_no_sem);
    my_create_process_sync("my_process_dec_no_sem", (void (*)(int, char**))my_process_dec_no_sem);
  }

  // The last one should not print 0
}