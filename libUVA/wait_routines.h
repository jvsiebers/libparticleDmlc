/* wait_routines.h
   Modification History:
      Feb 2, 1999: JVS: Created
*/
#ifndef WAIT_ROUTINES_H_INCLUDED
#define WAIT_ROUTINES_H_INCLUDED
/*  define status of waiting for child processes */
#define WAIT_FOR_CHILD    201
#define NO_WAIT_FOR_CHILD 202
int wait_for_child(int pid);
int wait_for_process_completion(int n_pids, pid_t *pid_array);
#endif
