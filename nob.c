#include "nob.h"

#define CC "cc"
#define BUILD_DIR "build/"

#define MAX_PATH 64

bool opt_release = false;
bool opt_debug = false;
bool opt_native = false;
bool opt_rebuild_anyways = false;

void print_help() {
    printf(
        "./nob [options]\n"
        "\n"
        "options:\n"
        "  clean  -c   cleans up build files\n"
        "  --help -h   prints this help\n"
        "  -r          build in release mode\n"
        "  -d          build in debug mode\n"
        "  -n          compile for this cpu architecture\n"
        "  -q          stay quiet\n"
        "  -B          rebuild anyways\n"
    );
}

void comp_c(const char *path) {
    const char *name = path + 4;
    char obj[MAX_PATH], dep[MAX_PATH];
    int len = sprintf(obj, BUILD_DIR"%s", name);
    strcpy(dep, obj);
    obj[len - 1] = 'o';
    dep[len - 1] = 'd';

    if (opt_rebuild_anyways || requires_rebuild(dep, obj)) {
        print(COMPILE, "%s -> %s", path, obj);

        cmd_append(CC, "-c", path, "-o", obj, "-MMD");

        if (opt_native)
            cmd_append("-march=native");
        if (opt_release)
            cmd_append("-flto");
        else
            cmd_append("-Wall", "-Wextra");

        if (opt_debug)
            cmd_append("-g", "-fsanitize=address", "-fsanitize=undefined", "-fsanitize=leak", "-fanalyzer");
        cmd_run();
    }
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (streq(arg, "--help")) {
            print_help();
            return 0;
        }
        if (arg[0] == '-') {
            for (int j = 1; arg[j] != '\0'; j++) {
                switch (arg[j]) {
                    case 'r': opt_release = true; break;
                    case 'd': opt_debug = true; break;
                    case 'n': opt_native = true; break;
                    case 'q': is_quiet = true; break;
                    case 'B': opt_rebuild_anyways = true; break;
                    case 'h': print_help(); return 0;
                    case 'c':
                        delete_file("nbtq");
                        delete_dir_recursive(BUILD_DIR);
                        return 0;
                }
            }
        }
    }

    create_dir(BUILD_DIR);

    comp_c("src/main.c");
    comp_c("src/to_snbt.c");

    print(LINKING, "nbtq");
    cmd_append(CC, "-o", "nbtq", "-lz");
    cmd_append(BUILD_DIR "main.o");
    cmd_append(BUILD_DIR "to_snbt.o");

    if (opt_release)
        cmd_append("-O2", "-s", "-flto");
    if (opt_debug)
        cmd_append("-fsanitize=address", "-fsanitize=undefined", "-fsanitize=leak");

    cmd_run();
    return 0;
}
