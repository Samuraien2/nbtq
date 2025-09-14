#include "common.h"
#include "to_snbt.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf(
            "nbtq - jq for NBT data\n"
            "usage: nbtq <file> [path/filter] [options]\n"
            "\n"
            "options:\n"
            "  -s --to-snbt   turns NBT into SBNT (default)\n"
            "  -n --to-nbt    turns SNBT into NBT\n"
            "  -c --compact   gets rid of whitespace in SNBT\n"
        );
        return 0;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Opening file");
        return 1;
    }
    
    nbt_to_snbt(fp);

    fclose(fp);
    return 0;
}
