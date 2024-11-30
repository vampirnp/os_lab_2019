#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

// Функция, выполняемая первым потоком
void* thread_function_1(void* arg) {
    pthread_mutex_lock(&mutex1);
    printf("Thread 1: Locked mutex 1\n");
    sleep(1); // Задержка для увеличения вероятности deadlock

    // Пытаемся захватить второй мьютекс
    pthread_mutex_lock(&mutex2);
    printf("Thread 1: Locked mutex 2\n");

    // Освобождаем мьютексы
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

// Функция, выполняемая вторым потоком
void* thread_function_2(void* arg) {
    pthread_mutex_lock(&mutex2);
    printf("Thread 2: Locked mutex 2\n");
    sleep(1); // Задержка для увеличения вероятности deadlock

    // Пытаемся захватить первый мьютекс
    pthread_mutex_lock(&mutex1);
    printf("Thread 2: Locked mutex 1\n");

    // Освобождаем мьютексы
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);

    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Инициализация мьютексов
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);

    // Создание потоков
    pthread_create(&thread1, NULL, thread_function_1, NULL);
    pthread_create(&thread2, NULL, thread_function_2, NULL);

    // Ожидание завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Освобождение ресурсов
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);

    return 0;
}
