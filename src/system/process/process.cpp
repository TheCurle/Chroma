#include <kernel/chroma.h>
#include <kernel/system/process/process.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

// An array of pointers to the header of each active process.
Process** processes;
// An array of pointers to the header of each process active on the current core.
Process* processesPerCore[MAX_CORES];

// A lock "counter". This can be increased and decreased at will.
// Positive values mean the process is being locked by another, zero and negative means it is active.
int locked = 0;
// Whether the current process is fully loaded into memory and ready to run.
bool loaded = false;

// A ticketlock that prevents two threads being created at the same time.
ticketlock_t creatorlock;
// A ticketlock that prevents two threads from being switched into at the same time.
ticketlock_t switcherlock;

// The PID of the next process to be created. The kernel is always 0.
size_t nextPID = 1;

