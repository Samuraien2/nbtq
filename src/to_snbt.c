#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "common.h"
#include "to_snbt.h"

/* TODOS
TODO: quote keys containing invalid letters
TODO: filter
*/

void read_compound();
void fp_read(void *data, size_t size);
inline static void pchar(char ch);
inline static void newline();

int indentation = 0;
NBTFile fp;
bool compact;
bool suffix;

char **simple_filter = NULL;

// remember to free
char *substr(const char *start, int len) {
    char *dest = malloc(len + 1);
    if (dest == NULL) {
        return NULL;
    }

    strncpy(dest, start, len);

    dest[len] = '\0';

    return dest;
}

int nbt_to_snbt(NBTFile file, bool opt_compact, bool opt_no_suffix, char *filter) {
    fp = file;
    compact = opt_compact;
    suffix = !opt_no_suffix;

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

    if (filter != NULL) {
        // TODO: for now assume a valid input
        int start = 0;
        int count = 0;
        for (int i = 0; filter[i] != '\0'; i++) {
            char ch = filter[i];
            if (ch == '.') {
                count++;
                simple_filter = realloc(simple_filter, count * sizeof(char*));
                simple_filter[i] = substr(filter + start, i - start);
                start = i + 1;
            }
        }
    }

    int index = 0;
    while (1) {
        char *filter_name = simple_filter[index];
        printf("FILTER: '%s'\n", filter_name);
        
        index++;
    }
    
    read_compound();
    newline();
    return 0;
}

void fp_read(void *data, size_t size) {
    if (fp.is_gzip) {
        gzread(fp.gz, data, size);
    } else {
        fread(data, size, 1, fp.fp);
    }
}

inline static void pchar(char ch) {
    putchar(ch);
}

static void indent() {
    assert(indentation >= 0, "Indentation less than 0");
    if (!compact) {
        // i love printf
        printf("%*s", indentation * 2, "");
    }
}

inline static void newline() {
    if (!compact) pchar('\n');
}

void print_array(char prefix, u8 type);

int print_data(TagID tag) {
    switch (tag) {
        case TAG_END: {
            pchar('}');
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
                break;
            }
            
            len = ntohl(len);
            pchar('[');
            newline();
            indentation++;
            for (u32 i = 0; i < len; i++) {
                indent();
                print_data(type);
                if (i != len - 1) {
                    pchar(',');
                }
                newline();
            }
            indentation--;
            indent();
            pchar(']');
            break;
        }
        case TAG_COMPOUND:
            read_compound();
            break;
        case TAG_BYTE_ARRAY:
            print_array('B', TAG_BYTE);
            break;
        case TAG_INT_ARRAY:
            print_array('I', TAG_INT);
            break;
        case TAG_LONG_ARRAY:
            print_array('L', TAG_LONG);
            break;
        default:
            return 1;
    }
    return 0;
}

void print_array(char prefix, u8 type) {
    i32 size;
    fp_read(&size, 4);
    size = ntohl(size);

    pchar('[');
    newline();
    indentation++;
    indent();
    printf("%c;", prefix);
    newline();

    for (i32 i = 0; i < size; i++) {
        indent();
        print_data(type);
        if (i != size - 1) {
            pchar(',');
        }
        newline();
    }
    
    indentation--;
    indent();
    pchar(']');
}

void read_compound() {
    pchar('{');
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

        char *name = calloc(len + 1, 1);
        fp_read(name, len);

        if (!is_first) {
            pchar(',');
        }
        newline();
        indent();
        printf("%s:", name);
        free(name);
        if (!compact) pchar(' ');
        
        int ret = print_data(tag);
        assert(ret == 0, "print_data()");
        is_first = false;
    }
    
    newline();
    indentation--;
    indent();
    pchar('}');
}
