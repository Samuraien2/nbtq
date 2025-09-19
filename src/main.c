#include "common.h"
#include "version.h"
#include "to_snbt.h"
#include <unistd.h>
#include <string.h>
#include <zlib.h>
#include <sys/stat.h>

void print_usage();
void print_version();
bool is_file_gzip(FILE *fp);

typedef struct Opts {
    bool compact;
    bool to_nbt;
    bool edit;
    bool no_gzip;
} Opts;

#define IS(X, Y) !strcmp(X, Y)

int main(int argc, char *argv[]) {
    char *filename = NULL;

    Opts opt = {0};
    
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (arg[0] == '-') {
            if (arg[1] == '-') {
                arg = arg + 2;
                
                if (IS(arg, "help")) {
                    print_usage();
                    return 0;
                }
                else if (IS(arg, "version")) {
                    print_version();
                    return 0;
                }
                else if (IS(arg, "to-snbt")) opt.to_nbt = false;
                else if (IS(arg, "to-nbt")) opt.to_nbt = true;
                else if (IS(arg, "no-gzip")) opt.no_gzip = true;
                else if (IS(arg, "compact")) opt.compact = true;
                else if (IS(arg, "edit")) opt.edit = true;
                else {
                    printf("unknown option: --%s\n", arg);
                    return 1;
                }
            }
            else if (arg[1] == '\0') {
                filename = NULL; // read stdin
            }
            else {
                for (int j = 1; arg[j] != '\0'; j++) {
                    switch (arg[j]) {
                        case 'v': print_version(); return 0;
                        case 'c': opt.compact = true; break;
                        case 'g': opt.no_gzip = true; break;
                        case 'e': opt.edit = true; break;
                        case 'n': opt.to_nbt = true; break;
                        case 's': opt.to_nbt = false; break;
                        case 'h': print_usage(); return 0;
                        default:
                            printf("unknown option: -%c\n", arg[i]);
                            return 1;
                    }
                }
            }
        } else {
            if (filename == NULL) {
                filename = arg;
            } else {
                error("More than one file passed");
                return 1;
            }
        }
    }

    NBTFile fp = {0};
    if (filename == NULL) {
        fp.fp = stdin;
    } else {
        struct stat path_stat;
        if (stat(filename, &path_stat) != 0) {
            perror("stat");
            return 1;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            error("A directory isn't a nbt file bro");
            return 1;
        }
    
        fp.fp = fopen(filename, "rb");
        if (!fp.fp) {
            perror("Opening file");
            return 1;
        }
    }

    if (!opt.no_gzip) {
        if (is_file_gzip(fp.fp)) {
            fp.gz = gzopen(filename, "rb");
            fp.is_gzip = true;
        }
    }

    int ret = nbt_to_snbt(fp, opt.compact);

    if (fp.is_gzip) {
        gzclose(fp.gz);
    } else {
        fclose(fp.fp);
    }
    return ret;
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
    printf("nbtq-" VERSION "\n");
}

void error(const char *fmt) {
    printf("\e[31mError: %s\e[m\n", fmt);
}
