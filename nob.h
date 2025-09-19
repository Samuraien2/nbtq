// opinionated nob.h with colors made by Samuraien2

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>

typedef enum {
    INFO,
    ERROR,
    REMOVE,
    COMPILE,
    LINKING,
    FINISH
} LogType;

#define LOGPREFIX(CASE, CLR, TASK) case CASE: printf("\e[1;9" CLR "m" TASK "\e[m "); break;
#define streq(X, Y) !strcmp(X, Y)

typedef struct {
    const char **args;
    int count;
} Cmd;

Cmd cmd = { NULL, 0 };

#define cmd_append(...) cmd_append_(__VA_ARGS__, NULL)

void cmd_append_(const char *first, ...) {
    va_list args;
    va_start(args, first);

    const char *str = first;

    if (cmd.args == NULL) {
        cmd.args = (const char**)malloc(sizeof(char*) * 100);
    }

    while (str != NULL) {
        cmd.args[cmd.count++] = str;
        str = va_arg(args, const char*);
    } 

    va_end(args);
}

int cmd_run() {
    cmd.args[cmd.count] = NULL;
    cmd.count = 0;
    
    pid_t pid = fork();
    if (pid == 0) {
        execvp(cmd.args[0], (char *const *)cmd.args);
        perror("execvp");
        return 0;
    } else if (pid > 0) {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return -1;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
        return 1;
    } else {
        perror("fork");
        return -1;
    }
}

void print(LogType type, const char *fmt, ...) {
    switch (type) { // pls dont kill me...
        LOGPREFIX(ERROR, "1", "ERROR")
        LOGPREFIX(REMOVE, "1", "REMOVE")
        LOGPREFIX(COMPILE, "2", "COMPILE")
        LOGPREFIX(LINKING, "6", "LINKING")
        LOGPREFIX(FINISH, "2", "FINISH")
        default: break;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    putchar('\n');
}


int _delete_dir_recursive(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        char fullpath[4096];
        struct stat statbuf;

        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (lstat(fullpath, &statbuf) == -1) {
            perror("lstat");
            closedir(dir);
            return -1;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            if (_delete_dir_recursive(fullpath) == -1) {
                closedir(dir);
                return -1;
            }
        } else {
            if (remove(fullpath) == -1) {
                perror("remove");
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);

    if (rmdir(path) == -1) {
        perror("rmdir");
        return -1;
    }
    
    return 0;
}

void delete_dir_recursive(const char *path) {
    print(REMOVE, "%s*", path);
    _delete_dir_recursive(path);
}

void create_dir(const char *file) {
    mkdir(file, 0755);
}

void delete_file(const char *file) {
    print(REMOVE, file);
    remove(file);
}
