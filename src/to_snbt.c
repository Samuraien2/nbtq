#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "common.h"
#include "to_snbt.h"

/*
NBT -> SNBT

TODO: quote keys containing invalid letters
TODO: complex structures like lists of compounds
*/

int read_compound();
void fp_read(void *data, size_t size);

int indentation = 0;
NBTFile fp;
bool compact;

int nbt_to_snbt(NBTFile file, bool opt_compact) {
    fp = file;
    compact = opt_compact;

    u8 root_tag;
    fp_read(&root_tag, 1);
    if (root_tag != TAG_COMPOUND) {
        error("Root tag isn't a compound tag");
        return 100;
    }

    u16 root_name_len;
    fp_read(&root_name_len, 2);
    if (root_name_len != 0) {
        error("Root tag has a name");
        return 101;
    }
    
    read_compound();
    putchar('\n');
    return 0;
}

void fp_read(void *data, size_t size) {
    if (fp.is_gzip) {
        gzread(fp.gz, data, size);
    } else {
        fread(data, size, 1, fp.fp);
    }
}

static void indent() {
    assert(indentation >= 0, "Indentation less than 0");
    if (!compact) {
        // i love printf
        printf("%*s", indentation * 2, "");
    }
}

inline static void newline() {
    if (!compact) putchar('\n');
}

void print_array(char prefix, u8 type);

int print_data(TagID tag) {
    switch (tag) {
        case TAG_END: {
            putchar('}');
            indentation--;
            break;
        }
        case TAG_BYTE: {
            i8 data;
            fp_read(&data, 1);
            printf("%hhdb", data);
            break;
        }
        case TAG_SHORT: {
            i16 data;
            fp_read(&data, 2);
            printf("%hds", ntohs(data));
            break;
        }
        case TAG_INT: {
            i32 data;
            fp_read(&data, 4);
            printf("%d", ntohl(data));
            break;
        }
        case TAG_LONG: {
            i64 data;
            fp_read(&data, 8);
            // TODO: only on little-endian
            data = __builtin_bswap64(data);
            printf("%lldL", (long long)data);
            break;
        }
        case TAG_FLOAT: {
            u32 data;
            fp_read(&data, 4);
            data = ntohl(data);

            float f;
            memcpy(&f, &data, sizeof(f));
            printf("%.1ff", f);
            break;
        }
        case TAG_DOUBLE: {
            u64 data;
            fp_read(&data, 8);
            data = __builtin_bswap64(data);

            double d;
            memcpy(&d, &data, sizeof(d));
            printf("%.1f", d);
            break;
        }
        case TAG_STRING: {
            u16 len;
            fp_read(&len, 2);
            len = ntohs(len);
            char *data = calloc(len + 1, 1);
            fp_read(data, len);
            printf("\"%s\"", data);
            free(data);
            break;
        }
        case TAG_LIST: {
            u8 type;
            fp_read(&type, 1);
            u32 len;
            fp_read(&len, 4);            

            if (type == TAG_END) {
                printf("[]");
                return 0;
            }
            
            len = ntohl(len);
            putchar('[');
            newline();
            indentation++;
            for (u32 i = 0; i < len; i++) {
                indent();
                print_data(type);
                if (i != len - 1) {
                    putchar(',');
                }
                newline();
            }
            indentation--;
            indent();
            putchar(']');
            break;
        }
        case TAG_COMPOUND: {
            read_compound();
            break;
        }
        case TAG_BYTE_ARRAY: {
            print_array('B', TAG_BYTE);
            break;
        }
        case TAG_INT_ARRAY: {
            print_array('I', TAG_INT);
            break;
        }
        case TAG_LONG_ARRAY: {
            print_array('L', TAG_LONG);
            break;
        }
    }
    return 0;
}

void print_array(char prefix, u8 type) {
    i32 size;
    fp_read(&size, 4);
    size = ntohl(size);

    putchar('[');
    newline();
    indentation++;
    indent();
    printf("%c;", prefix);
    newline();

    for (i32 i = 0; i < size; i++) {
        indent();
        print_data(type);
        if (i != size - 1) {
            putchar(',');
        }
        newline();
    }
    
    indentation--;
    indent();
    putchar(']');
}

int read_compound() {
    putchar('{');
    indentation++;

    bool is_first = true;
    while (1) {
        u8 tag;
        fp_read(&tag, 1);

        if (tag == TAG_END)
            break;

        u16 len;
        fp_read(&len, 2);
        len = ntohs(len);

        char *name = malloc(len + 1);
        fp_read(name, len);
        name[len] = '\0';

        if (!is_first) {
            putchar(',');
        }
        newline();
        indent();
        printf("%s:", name);
        free(name);
        if (!compact) putchar(' ');

        print_data(tag);
        is_first = false;
    }
    
    newline();
    indentation--;
    indent();
    putchar('}');
    return 0;
}
