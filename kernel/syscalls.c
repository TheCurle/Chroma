/*#include <sys/stat.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>

//prototypes; these are the bare minimum for newlib to compile.

void _exit();

int close(int file);

char **environ;

int execve(char *name, char **argv, char **env);

int fork();

int fstat(int file, struct stat *st);

int getpid();

int isatty(int file);

int kill(int pid, int sig);

int link(char *old, char *new);

int lseek(int file, int ptr, int dir);

int open(const char* name, int flags, ...);

int read(int file, char* ptr, int len);

caddr_t sbrk(int incr);

int stat(const char* file, struct stat *st);

clock_t times(struct tms* buf);

int unlink(char* name);

int wait(int file, char* ptr, int len);

int gettimeofday(struct timeval* p, struct timezone* z);
*/