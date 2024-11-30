#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>

// Структура для передачи данных потокам
typedef struct {
    int thread_id;
    int start;
    int end;
    long long mod;
    long long partial_result;
} thread_data_t;

// Глобальная переменная для результата и мьютекс
long long result = 1;
pthread_mutex_t mutex;

// Функция, выполняемая каждым потоком
void* compute_partial_factorial(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    long long partial = 1;
    for (int i = data->start; i <= data->end; ++i) {
        partial = (partial * i) % data->mod;
    }

    // Блокировка мьютекса перед обновлением общего результата
    pthread_mutex_lock(&mutex);
    result = (result * partial) % data->mod;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    int k = -1;
    int pnum = -1;
    long long mod = -1;

    // Определение возможных опций
    static struct option options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };

    // Разбор аргументов командной строки
    while (1) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "k:p:m:", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 'k':
                k = atoi(optarg);
                break;
            case 'p':
                pnum = atoi(optarg);
                break;
            case 'm':
                mod = atoll(optarg);
                break;
            default:
                printf("Неправильные аргументы\n");
                return 1;
        }
    }

    // Проверка корректности введенных параметров
    if (k < 0 || pnum <= 0 || mod <= 0) {
        printf("Использование: %s --k <число> --pnum <количество потоков> --mod <модуль>\n", argv[0]);
        return 1;
    }

    // Инициализация мьютекса
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        return 1;
    }

    pthread_t* threads = malloc(sizeof(pthread_t) * pnum);
    if (threads == NULL) {
        perror("malloc");
        return 1;
    }

    thread_data_t* thread_data = malloc(sizeof(thread_data_t) * pnum);
    if (thread_data == NULL) {
        perror("malloc");
        free(threads);
        return 1;
    }

    // Определение диапазонов для каждого потока
    int numbers_per_thread = k / pnum;
    int remaining = k % pnum;
    int current = 1;

    for (int i = 0; i < pnum; ++i) {
        thread_data[i].thread_id = i;
        thread_data[i].start = current;
        thread_data[i].end = current + numbers_per_thread - 1;
        if (remaining > 0) {
            thread_data[i].end += 1;
            remaining -= 1;
        }
        current = thread_data[i].end + 1;
        thread_data[i].mod = mod;
        thread_data[i].partial_result = 1;

        // Создание потока
        if (pthread_create(&threads[i], NULL, compute_partial_factorial, &thread_data[i]) != 0) {
            perror("pthread_create");
            free(threads);
            free(thread_data);
            pthread_mutex_destroy(&mutex);
            return 1;
        }
    }

    // Ожидание завершения всех потоков
    for (int i = 0; i < pnum; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            free(threads);
            free(thread_data);
            pthread_mutex_destroy(&mutex);
            return 1;
        }
    }

    // Освобождение ресурсов
    pthread_mutex_destroy(&mutex);
    free(threads);
    free(thread_data);

    printf("Факториал %d по модулю %lld равен %lld\n", k, mod, result);

    return 0;
}
