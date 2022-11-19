#include <stdint.h>
#include <usrlib.h>

#define MINOR_WAIT 1000000     // TODO: To prevent a process from flooding the screen
#define WAIT       100000000    // TODO: Long enough to see theese processes beeing run at least twice

void endless_loop_prio();

uint64_t my_getpid_prio(){
  return getPID();
}

uint64_t my_create_process_prio(char * name){
  char * argv[] = {name};
  return initializeProccess((void (*)(int, char**))endless_loop_prio, 0, 1, argv, 0);
}

void my_nice_prio(uint64_t pid, uint64_t newPrio){
  return changeProccessPriority(pid, newPrio);
}

int my_kill_prio(uint64_t pid){
  return kill(pid);
}

int my_block_prio(uint64_t pid){
  return block(pid);
}

int my_unblock_prio(uint64_t pid){
  return unblock(pid);
}

void bussy_wait_prio(uint64_t n){
  uint64_t i;
  for (i = 0; i < n; i++);
}

void endless_loop_prio(){
  uint64_t pid = my_getpid_prio();

  while(1){
    // printf("%d ",pid);
    printint(pid); putchar(' ');
    bussy_wait_prio(MINOR_WAIT);
  }
}

#define TOTAL_PROCESSES 3

void test_prio(){
  uint64_t pids[TOTAL_PROCESSES];
  uint64_t i;

  for(i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = my_create_process_prio("endless_loop");

  bussy_wait_prio(WAIT);
  print("\nCHANGING PRIORITIES...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++){
    switch (i % 3){
      case 0:
        my_nice_prio(pids[i], 0); //lowest priority 
        break;
      case 1:
        my_nice_prio(pids[i], 2); //medium priority
        break;
      case 2:
        my_nice_prio(pids[i], 4); //highest priority
        break;
    }
  }

  bussy_wait_prio(WAIT);
  print("\nBLOCKING...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++)
    my_block_prio(pids[i]);

  print("CHANGING PRIORITIES WHILE BLOCKED...\n");
  for(i = 0; i < TOTAL_PROCESSES; i++){
    switch (i % 3){
      case 0:
        my_nice_prio(pids[i], 2); //medium priority 
        break;
      case 1:
        my_nice_prio(pids[i], 2); //medium priority
        break;
      case 2:
        my_nice_prio(pids[i], 2); //medium priority
        break;
    }
  }

  print("UNBLOCKING...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++)
    my_unblock_prio(pids[i]);

  bussy_wait_prio(WAIT);
  print("\nKILLING...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++)
    my_kill_prio(pids[i]);
}
