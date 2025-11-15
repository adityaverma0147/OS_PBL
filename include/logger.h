#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>
#include <pthread.h>

typedef enum { INFO, ERROR, WARNING } LogLevel;

void init_logger(const char* filename);
void close_logger();
void log_message(LogLevel level, const char* format, ...);

// NEW for IPC
void set_log_pipe(int fd);
void send_log_ipc(const char* msg);
void run_logger_process(int read_fd);

#endif
