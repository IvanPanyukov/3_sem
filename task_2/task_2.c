#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


//Write a duplex pipe implementation.
typedef struct {
    int txd[2];
    int rxd[2];
} dpipe_t;
// 1) This pipe should connect some process with his child, for continuous communication.
// 2) Be careful with opened descriptors.
// 3) Monitor the integrity of transmitted data.
// 4) When one process is terminated, the other should also exit.

int parent_body(pid_t pid, dpipe_t* duplexpipe) {
    size_t size;
    char buf[4096];
    printf("\n> ");
    while((size = read(0, buf, sizeof(buf) - 1)) > 0) {
        buf[size] = 0; // the text string data is expected
        buf[size - 1] = 0; // useless '\n' in the end of the read string

        write(duplexpipe->txd[1], (void*) buf, size);
        printf("[x] <parent> sent: \"%s\"\n", buf);

        read(duplexpipe->rxd[0], (void*) buf, size);
        printf("[x] <parent> recv: \"%s\"\n", buf);

        printf("> ");
    }
    return 0;
}

int child_body(dpipe_t* duplexpipe, pid_t pid) {
    size_t size_r;
    char buf[4096];
    while((size_r = read(duplexpipe->txd[0], (void*) buf, sizeof(buf)-1) > 0)) {
        printf("[x] <child> recv:  \"%s\"\n", buf);

        write(duplexpipe->rxd[1], (void*) buf, size_r);
        printf("[x] <child> sent:  \"%s\"\n", buf);
    }
    return 0;
}

int main()
{
    setbuf(stdout, NULL);
    dpipe_t duplexpipe;

    if((pipe(duplexpipe.txd) < 0) || (pipe(duplexpipe.rxd) < 0)) {
        printf("[x] Pipe creation has failed\n");
        return -1;
    }

    pid_t ppid_before_fork = getpid();
    pid_t pid = fork();
    if (pid < 0) {
        printf("[x] Fork failed\n");
        return -1;
    }

    if (pid) {
        close(duplexpipe.txd[0]);
        close(duplexpipe.rxd[1]);
        parent_body(pid, &duplexpipe);
    } else {
        close(duplexpipe.txd[1]);
        close(duplexpipe.rxd[0]);
        child_body(&duplexpipe, pid);
    }
    return 0;
}
