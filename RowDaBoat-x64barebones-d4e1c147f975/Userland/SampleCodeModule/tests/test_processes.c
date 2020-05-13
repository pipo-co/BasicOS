#include "test_util.h"
#include <usrlib.h>

//TO BE INCLUDED
void endless_loop(){
  while(1);
}

uint64_t my_create_process(char * name){
  char * argv[1];
  argv[0] = name;
  return initializeProccess((int (*)(int, char**))endless_loop, 0, 1, argv, 0);
}

uint64_t my_kill(uint64_t pid){
  kill(pid);
  return 0;
}

uint64_t my_block(uint64_t pid){
  block(pid);
  return 0;
}

uint64_t my_unblock(uint64_t pid){
  unblock(pid);
  return 0;
}

#define MAX_PROCESSES 10 //Should be around 80% of the the processes handled by the kernel //32000

enum State {ERROR, RUNNING, BLOCKED, KILLED};

typedef struct P_rq{
  uint64_t pid;
  enum State state;
}p_rq;

void test_processes(){
  p_rq p_rqs[MAX_PROCESSES];
  uint8_t rq;
  uint8_t alive = 0;
  uint8_t action;

  while (1){

    skipTurn(); // Le damos la oportunidad al scheduler para que limpie los procesos KILLED

    // Create MAX_PROCESSES processes
    for(rq = 0; rq < MAX_PROCESSES; rq++){
      p_rqs[rq].pid = my_create_process("endless_loop");  // TODO: Port this call as required

      if (p_rqs[rq].pid == 0){                           // TODO: Port this as required
        print("Error creating process\n");               // TODO: Port this as required
        return;
      }else{
        p_rqs[rq].state = RUNNING;
        alive++;
      }
    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0){

      for(rq = 0; rq < MAX_PROCESSES; rq++){
        action = GetUniform(2) % 2; 

        switch(action){
          case 0:
            if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED){
              if (my_kill(p_rqs[rq].pid) == -1){          // TODO: Port this as required
                print("Error killing process\n");        // TODO: Port this as required
                return;
              }
              p_rqs[rq].state = KILLED; 
              alive--;
            }
            break;

          case 1:
            if (p_rqs[rq].state == RUNNING){
              if(my_block(p_rqs[rq].pid) == -1){          // TODO: Port this as required
                print("Error blocking process\n");       // TODO: Port this as required
                return;
              }
              p_rqs[rq].state = BLOCKED; 
            }
            break;
        }
      }

      // Randomly unblocks processes
      for(rq = 0; rq < MAX_PROCESSES; rq++)
        if (p_rqs[rq].state == BLOCKED && GetUniform(2) % 2){
          if(my_unblock(p_rqs[rq].pid) == -1){            // TODO: Port this as required
            print("Error unblocking process\n");         // TODO: Port this as required
            return;
          }
          p_rqs[rq].state = RUNNING; 
        }
    } 
  }
}

