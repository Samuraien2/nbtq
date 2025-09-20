#include "nob.h"

#define CC "cc"
#define BUILD_DIR "build/"

#define MAX_PATH 64

bool requires_rebuild(const char *name, const char *obj);

void comp_c(const char *path) {
    const char *name = path + 4;
    char obj[MAX_PATH];
    int len = sprintf(obj, BUILD_DIR"%s", name);
    obj[len - 1] = 'o';

    if (requires_rebuild(name, obj)) {
        print(COMPILE, "%s -> %s", path, obj);

        cmd_append(CC, "-c", path, "-o", obj, "-MMD");
        cmd_append("-Wall", "-Wextra");
        cmd_run();
    }
}

int main(int argc, char *argv[]) {
    bool opt_verbose = false;
    for (int i = 1; i < argc; i++) {
        if (streq(argv[i], "--help")) {
            printf(
                "%s [options]\n"
                "\n"
                "options:\n"
                "  clean  -c   cleans up build files\n"
                "  --help      prints this help\n"
                , argv[0]
            );
            return 0;
        }
        else if (streq(argv[i], "clean") || streq(argv[i], "-c")) {
            delete_file("nbtq");
            delete_dir_recursive(BUILD_DIR);
            return 0;
        }
        else if (streq(argv[i], "-v")) {
            opt_verbose = true;
        }
    }

    create_dir(BUILD_DIR);

    comp_c("src/main.c");
    comp_c("src/to_snbt.c");

    print(LINKING, "nbtq");
    cmd_append(CC, "-o", "nbtq", "-lz");
    cmd_append(BUILD_DIR "main.o");
    cmd_append(BUILD_DIR "to_snbt.o");
    cmd_run();
    return 0;
}

bool requires_rebuild(const char *name, const char *obj) {
    char dep[MAX_PATH];
    sprintf(dep, BUILD_DIR"%s", name);
    dep[strlen(dep) - 1] = 'd';

    time_t obj_time = mtime(obj);
    if (!obj_time) return true;
    
    FILE *fp = fopen(dep, "r");
    if (!fp) return true;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char *p = strchr(line, ':');
        if (p) p++; else continue;

        char *tok = strtok(p, " \t\n\\");
        while (tok) {
            if (mtime(tok) > obj_time) {
                fclose(fp);
                return true;
            }
            tok = strtok(NULL, " \t\n\\");
        }
    }
    fclose(fp);
    return false;
}
