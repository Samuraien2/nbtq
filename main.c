#define _GNU_SOURCE
#include "common.h"
#include "to_snbt.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

void print_usage();
void print_version();

typedef struct Opts {
    bool compact;
    bool to_nbt;  
    bool edit;
} Opts;

enum OptionIDs {
    OPT_TO_SNBT,
    OPT_TO_NBT,
    OPT_COMPACT,
    OPT_EDIT,
    OPT_HELP,
    OPT_VERSION,
    OPTIONS_TOTAL
};

typedef struct {
    char *name;
    char ch;
} FullOption;

int main(int argc, char *argv[]) {
    char *filename = NULL;

    FullOption full_options[] = {
        { "to-snbt", 's' },
        { "to-nbt", 'n' },
        { "compact", 'c' },
        { "edit", 'e' },
        { "help", 'h' },
        { "version", 'v' }
    };
    
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (arg[0] == '-') {
            if (arg[1] == '-') { // long-option
                arg = arg + 2;
                
                if (!strcmp(arg, "help")) {
                    print_usage();
                    return 0;
                }
                else if (!strcmp(arg, "version")) {
                    print_version();
                    return 0;
                }
                else if (!strcmp(arg, "to-snbt")) {
                    
                }
            }
            else if (arg[1] == '\0') { // read stdin
                filename = NULL;
            }
            else { // single-options
                for (int j = 2; arg[j] != '\0'; j++) {
                    printf("OPT: %c\n", arg[i]);
                }
            }
        } else {
            if (filename == NULL) {
                filename = arg;
            } else {
                fprintf(stderr, "\e[31mError: More than one file passed\e[m\n");
                return 1;
            }
        }
    }

    FILE *fp;
    if (filename == NULL) {
        fp = stdin;
    } else {
        fp = fopen(filename, "rb");
        if (!fp) {
            perror("Opening file");
            return 1;
        }
    }

    int ret = nbt_to_snbt(fp);
    if (ret != 0) {
        fprintf(stderr, "nbt_to_snbt(FILE) failed\n");
        return ret;
    }

    fclose(fp);
    return 0;
}

void print_usage() {
    printf(
        "Usage: nbtq [OPTIONS] [FILE | -] [FILTER]\n"
        "nbtq - parses NBT data and prints to stdout\n"
        "\n"
        "With no FILE, or when FILE is -, read standard input.\n"
        "\n"
        "Options:\n"
        "  -s --to-snbt   convert NBT into SBNT (default)\n"
        "  -n --to-nbt    convert SNBT into NBT\n"
        "  -c --compact   gets rid of whitespace in SNBT\n"
        "  -e --edit      edit NBT file in $EDITOR\n"
        "  -h --help      prints this help info and exits\n"
        "  -v --version   prints version info and exits\n"
        "\n"
        "Filter:\n"
        "  filter matches the path matching from Minecraft (/data get entity @p <path>)\n"
        "  read more: https://minecraft.wiki/w/NBT_path\n"
        "\n"
        "Examples\n"
        "  nbtq level.dat > level.dat.snbt\n"
        "  nbtq level.dat.snbt -n > level.dat\n"
        "  nbtq player.dat 'bukkit.lastKnownName'\n"
    );
}

void print_version() {
    printf("nbtq-1.0.0\n");
}
