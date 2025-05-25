#include <php.h>
#include <zend_compile.h>
#include <zend_exceptions.h>
#include <zend_string.h>
#include <stdlib.h>

// Global counter
static int dump_counter = 1;

static zend_op_array *(*old_compile_string)(zend_string *source_string, const char *filename, zend_compile_position position);
static zend_op_array *(*old_compile_file)(zend_file_handle *file_handle, int type);

static void dump_code(const char *code, size_t len, const char *origin) {
    char filename[64];
    snprintf(filename, sizeof(filename), "/data/data/com.termux/usr/tmp/bintang_%d.php", dump_counter++);

    FILE *fp = fopen(filename, "wb");
    if (fp) {
        fwrite(code, 1, len, fp);
        fclose(fp);
        php_printf("[mydumper] dumped from %s to %s\n", origin, filename);
    }
}

static zend_op_array *my_compile_file(zend_file_handle *file_handle, int type) {
    if (file_handle->filename) {
        FILE *fp = fopen(ZSTR_VAL(file_handle->filename), "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long len = ftell(fp);
            rewind(fp);
            char *buffer = emalloc(len + 1);
            fread(buffer, 1, len, fp);
            buffer[len] = '\0';
            dump_code(buffer, len, ZSTR_VAL(file_handle->filename));
            efree(buffer);
            fclose(fp);
        }
    }
    return old_compile_file(file_handle, type);
}

static zend_op_array *my_compile_string(zend_string *source_string, const char *filename, zend_compile_position position) {
    dump_code(ZSTR_VAL(source_string), ZSTR_LEN(source_string), filename ? filename : "eval()");
    return old_compile_string(source_string, filename, position);
}

PHP_MINIT_FUNCTION(mydumper) {
    old_compile_file = zend_compile_file;
    zend_compile_file = my_compile_file;

    old_compile_string = zend_compile_string;
    zend_compile_string = my_compile_string;

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(mydumper) {
    zend_compile_file = old_compile_file;
    zend_compile_string = old_compile_string;
    return SUCCESS;
}

zend_module_entry mydumper_module_entry = {
    STANDARD_MODULE_HEADER,
    "mydumper",
    NULL,
    PHP_MINIT(mydumper),
    PHP_MSHUTDOWN(mydumper),
    NULL,
    NULL,
    NULL,
    "0.1",
    STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(mydumper)
