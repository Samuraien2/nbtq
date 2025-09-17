#include "neonob.h"

#define CC "cc"

int main(int argc, char *argv[]) {
    bool opt_verbose = false;
    for (int i = 1; i < argc; i++) {
        if (streq(argv[i], "--help")) {
            printf(
                "%s [options]\n"
                "\n"
                "options:\n"
                "  clean  -c    cleans up build files\n"
                "  --help      prints this help\n"
                , argv[0]
            );
            return 0;
        }
        else if (streq(argv[i], "clean") || streq(argv[i], "-c")) {
            delete_file("nbtq");
            return 0;
        }
        else if (streq(argv[i], "-v")) {
            opt_verbose = true;
        }
    }

    cmd_append(CC, "-o", "nbtq", "-lz");
    cmd_append("src/main.c", "src/to_snbt.c");
    if (!cmd_run()) {
        return 1;
    }

    print(FINISH, "nbtq");
    
    return 0;
}
