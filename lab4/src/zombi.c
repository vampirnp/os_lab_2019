#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    // Создаем дочерний процесс
    pid = fork();

    if (pid < 0) {
        // Ошибка при создании процесса
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Дочерний процесс
        printf("Child process (PID: %d) is running...\n", getpid());
        sleep(5);
        printf("Child process (PID: %d) is exiting...\n", getpid());
        exit(0);
    } else {
        // Родительский процесс
        printf("Parent process (PID: %d) is running...\n", getpid());
        sleep(10);
        printf("Parent process (PID: %d) is exiting...\n", getpid());

        // Родительский процесс вызывает wait для получения статуса завершения дочернего процесса
       // wait(NULL);
    }

    return 0;
}
