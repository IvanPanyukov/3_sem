#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

pid_t pid;

int byte_char_rank;
int byte_char;

int writer_body (char ** argv, pid_t ppid) {

    sigset_t set_child;
    

    void custom_signal_EMPTY(int signo) {
    }
    struct sigaction act_empty;
    memset(&act_empty, 0, sizeof(act_empty));
    act_empty.sa_handler = custom_signal_EMPTY;
    sigfillset(&act_empty.sa_mask);
    sigaction(SIGUSR1, &act_empty, NULL);


    void custom_signal_PARENT_EXIT(int signo) {
        if (signo == SIGALRM)
            exit(0);
    }
    struct sigaction act_alarm;
    memset(&act_alarm, 0, sizeof(act_alarm));
    act_alarm.sa_handler = custom_signal_PARENT_EXIT;
    sigfillset(&act_alarm.sa_mask);
    sigaction(SIGALRM, &act_alarm, NULL);


    unsigned int fd = 0;
    char c = 0;

    if ((fd = open(argv[1], O_RDONLY)) < 0 ) {
        exit(-1);
    }

    int i;

    while (read(fd, &c, 1) > 0) {
        alarm(1);
        for ( i = 128; i >= 1; i /= 2) {
            if ( i & c )              // 1
                kill(ppid, SIGUSR1);
            else                      // 0
                kill(ppid, SIGUSR2);
            sigsuspend(&set_child);
        }
    }

    exit(0);
    
}

int reader_body (sigset_t* set) {

    errno = 0;

    printf("[x] File contents (beginning at new line):\n");
    
    do {
        if(byte_char_rank == 0) {
            
            write(STDOUT_FILENO, &byte_char, 1);
            fflush(stdout);
            byte_char_rank=128;
            byte_char = 0;
        }
        sigsuspend(set);
    } while (42 == 42);

}

int main(int argc, char ** argv) {
    if (argc != 2) {
        printf("[x] Specify input file name: %s <file>\n", argv[0]);
        exit(-1);
    }

    sigset_t set;


    void custom_signal_ZERO(int signo) {
        if (signo == SIGUSR2) {
            byte_char_rank/=2;
            kill(pid, SIGUSR1);
        }
    }
    struct sigaction act_zero;
    memset(&act_zero, 0, sizeof(act_zero));
    act_zero.sa_handler = custom_signal_ZERO;
    sigfillset(&act_zero.sa_mask);
    sigaction(SIGUSR2, &act_zero, NULL);
    sigaddset(&set, SIGUSR2);


    void custom_signal_ONE(int signo) {
        if (signo == SIGUSR1) {
            byte_char += byte_char_rank;
            byte_char_rank /= 2;
            kill(pid, SIGUSR1);
        }
    }
    struct sigaction act_one;
    memset(&act_one, 0, sizeof(act_one));
    act_one.sa_handler = custom_signal_ONE;
    sigfillset(&act_one.sa_mask);
    sigaction(SIGUSR1, &act_one, NULL);
    sigaddset(&set, SIGUSR1);


    void custom_signal_CHILD_EXIT(int signo) {
        if (signo == SIGCHLD)
            exit(0);
    }
    struct sigaction act_exit;
    memset(&act_exit, 0, sizeof(act_exit));
    act_exit.sa_handler = custom_signal_CHILD_EXIT;
    sigfillset(&act_exit.sa_mask);
    sigaction(SIGCHLD, &act_exit, NULL);
    sigaddset(&set, SIGCHLD);


    sigprocmask(SIG_BLOCK, &set, NULL );
    sigemptyset(&set);


    pid_t ppid = getpid();
    pid = fork();

    if (pid == 0) {
        writer_body(argv, ppid);
    } else {
        reader_body(&set);
    }
}
