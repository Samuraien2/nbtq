#include "neonob.h"

#define CC "cc"
#define BUILD_DIR "build/"

void comp_c(const char *path) {
    const char *name = path + 4;
    char obj[64];
    int len = snprintf(obj, sizeof(obj), BUILD_DIR"%s", name);
    obj[len - 1] = 'o';
    print(COMPILE, "%s -> %s", path, obj);

    cmd_append(CC, "-c", path, "-o", obj);
    cmd_run();
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

    mkdir(BUILD_DIR, 0755);

    comp_c("src/main.c");
    comp_c("src/to_snbt.c");

    print(LINKING, "nbtq");
    cmd_append(CC, "-o", "nbtq", BUILD_DIR"to_snbt.o", BUILD_DIR"main.o", "-lz");
    cmd_run();
    return 0;
}
