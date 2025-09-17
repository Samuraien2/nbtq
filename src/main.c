#include "common.h"
#include "to_snbt.h"
#include <unistd.h>
#include <string.h>
#include <zlib.h>

void print_usage();
void print_version();
bool is_file_gzip(FILE *fp);

typedef struct Opts {
    bool compact;
    bool to_nbt;  
    bool edit;
    bool no_gzip;
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

    Opts opts = {0};
    
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
                    opts.to_nbt = false;
                }
                else if (!strcmp(arg, "to-nbt")) {
                    opts.to_nbt = true;
                }
                else if (!strcmp(arg, "no-gzip")) {
                    opts.no_gzip = true;
                }
                else if (!strcmp(arg, "compact")) {
                    opts.compact = true;
                }
                else if (!strcmp(arg, "edit")) {
                    opts.edit = true;
                }
                else {
                    printf("unknown option: --%s\n", arg);
                    return 1;
                }
            }
            else if (arg[1] == '\0') { // read stdin
                filename = NULL;
            }
            else { // single-options
                for (int j = 2; arg[j] != '\0'; j++) {
                    switch (arg[i]) {
                        case 'h':
                            print_usage();
                            return 0;
                        case 'v':
                            print_version();
                            return 0;
                            
                    }
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

    NBTFile fp = {0};
    if (filename == NULL) {
        fp.fp = stdin;
    } else {
        fp.fp = fopen(filename, "rb");
        if (!fp.fp) {
            perror("Opening file");
            return 1;
        }
    }

    if (!opts.no_gzip) {
        if (is_file_gzip(fp.fp)) {
            fp.gz = gzopen(filename, "rb");
            fp.is_gzip = true;
        }
    }

    int ret = nbt_to_snbt(fp, opts.compact);
    if (ret != 0) {
        fprintf(stderr, "nbt_to_snbt(FILE) failed\n");
        return ret;
    }

    if (fp.is_gzip) {
        gzclose(fp.gz);
    } else {
        fclose(fp.fp);
    }
    return 0;
}

bool is_file_gzip(FILE *fp) {
    uint8_t magic[2];

    long pos = ftell(fp);
    if (pos == -1) return false;

    if (fread(magic, 1, 2, fp) != 2) {
        fseek(fp, pos, SEEK_SET);
        return false;
    }

    fseek(fp, pos, SEEK_SET);

    return magic[0] == 0x1F && magic[1] == 0x8B;
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
