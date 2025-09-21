#include <stdbool.h>
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

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ntohll(x) __builtin_bswap64(x)
#else
#define ntohll(x) x
#endif

#define printif(FMT, DATA, CONDITION) if (CONDITION) printf(FMT, DATA)

void read_compound();
int read_tag();
void fp_read(void *data, size_t size);
inline static void pchar(char ch);
inline static void newline();

int indentation = 0;
NBTFile fp;
bool compact;
bool suffix;
bool print = true;

char **simple_filter = NULL;
int simple_filter_count = 0;

// remember to free
char *substr(const char *start, int len) {
    char *dest = malloc(len + 1);
    if (dest == NULL) return NULL;

    strncpy(dest, start, len);
    dest[len] = '\0';
    return dest;
}

bool parse_filter(const char *filter) {
    if (filter != NULL) {
        // TODO: for now assume a valid input
        int start = 0;
        int capacity = 4;
        simple_filter = malloc(capacity * sizeof(char*));
        if (!simple_filter) return false;
        
        for (int i = 0; ; i++) {
            if (filter[i] == '.' || filter[i] == '\0') {
                if (simple_filter_count == capacity) {
                    capacity *= 2;
                    simple_filter = realloc(simple_filter, capacity * sizeof(char*));
                    if (!simple_filter) return false;
                }
                simple_filter[simple_filter_count++] = substr(filter + start, i - start);
                start = i + 1;
                if (filter[i] == '\0') break;
            }
        }
    }
    return true;
}

bool read_nbt(NBTFile file, bool opt_compact, bool opt_no_suffix, char *filter) {
    fp = file;
    compact = opt_compact;
    suffix = !opt_no_suffix;    
    
    u8 root_tag;
    fp_read(&root_tag, 1);
    if (root_tag != TAG_COMPOUND) {
        error("Root tag isn't a compound tag");
        return false;
    }

    u16 root_name_len;
    fp_read(&root_name_len, 2);
    if (root_name_len != 0) {
        error("Root tag has a name");
        return false;
    }

    parse_filter(filter);

    read_compound();
    read_tag();
    newline();

    for (int i = 0; i < simple_filter_count; i++) {
        printf("FILTER: '%s'\n", simple_filter[i]);
        free(simple_filter[i]);
    }
    free(simple_filter);
    
    return true;
}

void fp_read(void *data, size_t size) {
    if (fp.is_gzip) {
        gzread(fp.gz, data, size);
    } else {
        fread(data, size, 1, fp.fp);
    }
}

inline static void pchar(char ch) {
    if (print) putchar(ch);
}


static void indent() {
    assert(indentation >= 0, "Indentation less than 0");
    if (!compact && print) {
        // i love printf
        printf("%*s", indentation * 2, "");
    }
}

inline static void newline() {
    if (!compact) pchar('\n');
}

void print_array(char prefix, u8 type);

int read_data(TagID tag) {
    switch (tag) {
        case TAG_END: {
            pchar('}');
            indentation--;
            break;
        }
        case TAG_BYTE: {
            i8 data;
            fp_read(&data, 1);
            printif("%hhdb", data, print);
            break;
        }
        case TAG_SHORT: {
            i16 data;
            fp_read(&data, 2);
            printif("%hds", ntohs(data), print);
            break;
        }
        case TAG_INT: {
            i32 data;
            fp_read(&data, 4);
            printif("%d", ntohl(data), print);
            break;
        }
        case TAG_LONG: {
            i64 data;
            fp_read(&data, 8);
            printif("%lldL", (long long)ntohll(data), print);
            break;
        }
        case TAG_FLOAT: {
            u32 data;
            fp_read(&data, 4);
            if (print) {
                data = ntohl(data);

                float f;
                memcpy(&f, &data, sizeof(f));
                printf("%.1ff", f);
            }
            break;
        }
        case TAG_DOUBLE: {
            u64 data;
            fp_read(&data, 8);
            if (print) {
                data = ntohll(data);

                double d;
                memcpy(&d, &data, sizeof(d));
                printf("%.1f", d);
            }
            break;
        }
        case TAG_STRING: {
            u16 len;
            fp_read(&len, 2);
            len = ntohs(len);
            char *data = calloc(len + 1, 1);
            fp_read(data, len);
            printif("\"%s\"", data, print);
            free(data);
            break;
        }
        case TAG_LIST: {
            u8 type;
            fp_read(&type, 1);
            u32 len;
            fp_read(&len, 4);            

            if (type == TAG_END) {
                if (print)
                    printf("[]");
                break;
            }
            
            len = ntohl(len);
            pchar('[');
            newline();
            indentation++;
            for (u32 i = 0; i < len; i++) {
                indent();
                read_data(type);
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
        case TAG_COMPOUND: {
            
            read_compound();
            break;
        }
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
    printif("%c;", prefix, print);
    newline();

    for (i32 i = 0; i < size; i++) {
        indent();
        read_data(type);
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

        bool core = false;
        // if nested in a non-print
        if (print) {
            int lvl = indentation - 1;
            bool filter_exists = lvl < simple_filter_count;
            // if filter[lvl] != name
            if (filter_exists && strcmp(simple_filter[lvl], name)) {
                print = false;
                core = true;
            }
        }

        if (!is_first) {
            pchar(',');
        }
        newline();
        indent();
        if (print && core)
            printf("%s:", name);
        free(name);
        if (!compact) pchar(' ');
        
        int ret = read_data(tag);
        assert(ret == 0, "print_data()");
        is_first = false;

        // reset print at the end
        if (core && !print) {
            print = true;
        }
    }
    
    newline();
    indentation--;
    indent();
    pchar('}');
}
