#include "logger.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#ifndef LOGGER_BUF_SIZE
    #error LOGGER_BUF_SIZE IN BYTES MUST BE DEFINED
#endif

extern void log_write_ll(char* msg);

static char tx_str[LOGGER_BUF_SIZE];

uint8_t
log_write(char* msg, ...) {
    va_list ap;
    va_start(ap, msg);
    vsprintf(tx_str, msg, ap);
    va_end(ap);

    log_write_ll(tx_str);

    return 1;
}
