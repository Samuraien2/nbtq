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

int read_tag();

int indentation = 0;
u8 last_tag;
NBTFile fp;
bool compact;

int nbt_to_snbt(NBTFile file, bool opt_compact) {
    fp = file;
    compact = opt_compact;
    while (1) {
        read_tag();
        if (fp.is_gzip) {
            if (gzeof(fp.gz))
                break;
        } else {
            if (feof(fp.fp))
                break;
        }
    }
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

void indent() {
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
            printf("%ldL", data);
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
            char *data = malloc(len + 1);
            fp_read(data, len);
            data[len] = '\0';
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
                if (i != len - 1) {
                    putchar(',');
                }
                print_data(type);
                newline();
            }
            indentation--;
            indent();
            putchar(']');
            break;
        }
        case TAG_COMPOUND: {
            putchar('{');
            indentation++;
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

int read_tag() {
    TagLen header;
    fp_read(&header, 3);

    if (indentation > 0) {
        if (header.tag != TAG_END && last_tag != TAG_COMPOUND) {
            putchar(',');
        }
        newline();
    }
    
    if (header.len > 0) {
        char *name = malloc(header.len + 1);
        fp_read(name, header.len);
        if (header.tag != TAG_END) {
            name[header.len] = '\0';
            indent();
            printf("%s:", name);
            if (!compact) putchar(' ');
        }
        free(name);
    }
    
    if (print_data(header.tag) != 0) {
        assert(0, "print_data(header.tag) failed");
        return 0;
    }
    last_tag = header.tag;

    return 0;
}


