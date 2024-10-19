#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // This is the child process
        char *args[] = {"./sequential_min_max", argv[1], argv[2], NULL};
        execv(args[0], args);
        
        // If execv returns, it must have failed
        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        // This is the parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Child process exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("Child process did not exit normally\n");
        }
    }

    return 0;
}