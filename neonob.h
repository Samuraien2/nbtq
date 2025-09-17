// opinionated nob.h with colors

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/wait.h>

typedef enum {
    INFO,
    ERROR,
    REMOVE,
    COMPILE,
    LINK,
    FINISH
} LogType;

#define LOGPREFIX(CASE, CLR, TASK) case CASE: printf("\e[1;9" CLR "m" TASK "\e[m "); break;
#define streq(X, Y) !strcmp(X, Y)

typedef struct {
    const char **args;
    int count;
    int capacity;
} Cmd;

Cmd cmd = { NULL, 0, 50 };

#define cmd_append(...) cmd_append_(__VA_ARGS__, NULL)

void cmd_append_(const char *first, ...) {
    va_list args;
    va_start(args, first);

    const char *str = first;

    if (cmd.args == NULL) {
        cmd.args = (const char**)malloc(sizeof(char*) * cmd.capacity);
    }

    while (str != NULL) {
        cmd.args[cmd.count++] = str;
        str = va_arg(args, const char*);
    } 

    va_end(args);
}

int cmd_run() {
    pid_t pid = fork();

    if (pid == 0) {
        cmd.args[cmd.count] = NULL;
        execvp(cmd.args[0], (char *const *)cmd.args);
        perror("execvp");
        return 0;
    } else if (pid > 0) {
        wait(NULL);
        return 1;
    } else {
        perror("fork");
        return 0;
    }
}

void print(LogType type, const char *fmt, ...) {
    switch (type) { // pls dont kill me...
        LOGPREFIX(ERROR, "1", "ERROR")
        LOGPREFIX(REMOVE, "1", "REMOVE")
        LOGPREFIX(COMPILE, "1", "COMPILE")
        LOGPREFIX(LINK, "1", "LINK")
        LOGPREFIX(FINISH, "2", "FINISH")
        default: break;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    putchar('\n');
}

void delete_file(const char *file) {
    print(REMOVE, file);
    remove(file);
}

