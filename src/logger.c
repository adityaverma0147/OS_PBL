#include "logger.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static FILE* log_file = NULL;
static pthread_mutex_t log_lock;

static int log_fd = -1; // IPC pipe fd

void set_log_pipe(int fd) {
    log_fd = fd;
}

void send_log_ipc(const char* msg) {
    if (log_fd != -1) {
        write(log_fd, msg, strlen(msg));
        write(log_fd, "\n", 1);
    }
}

void init_logger(const char* filename) {
    pthread_mutex_init(&log_lock, NULL);
    log_file = fopen(filename, "a");
    if (!log_file) {
        perror("Log file open failed");
        exit(EXIT_FAILURE);
    }
}

void close_logger() {
    if (log_file) fclose(log_file);
    pthread_mutex_destroy(&log_lock);
}

static const char* level_to_string(LogLevel level) {
    switch (level) {
        case INFO: return "INFO";
        case ERROR: return "ERROR";
        case WARNING: return "WARN";
        default: return "LOG";
    }
}

void log_message(LogLevel level, const char* format, ...) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    pthread_mutex_lock(&log_lock);

    printf("[%s] %s: ", timebuf, level_to_string(level));

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");

    if (log_file) {
        fprintf(log_file, "[%s] %s: ", timebuf, level_to_string(level));
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }

    pthread_mutex_unlock(&log_lock);
}

void run_logger_process(int read_fd) {
    init_logger("server.log");

    char buffer[512];
    while (1) {
        int n = read(read_fd, buffer, sizeof(buffer) - 1);
        if (n <= 0) break;
        buffer[n] = '\0';
        log_message(INFO, "%s", buffer);
    }

    close_logger();
}
