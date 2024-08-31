/// @file dump.c

#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <strings.h>
#include <string.h>

#include "bitter.h"


/**
 * Writes a HEX dump
 * @param[in] fd        file to write the dump to
 * @param[in] data      pointer to data which should be dumped
 * @param[in] size      how much data should be dumped
 * @param[in] show_addr show or hide address information
 * @param[in] cb        Callback to receive one full line of hex dump output
 *                      for custom processing / logging
 */
void dump_hex(FILE* fd, const void* data, unsigned int size, bool show_addr,
        void (*cb)(const char*)) {
#define MAX_LINE_LEN    128
    char ascii[17];
    char line_buf[MAX_LINE_LEN];
    int line_len, line_ptr;
    unsigned int i;
    unsigned int addr = 0;
    int m8, m16;

    ascii[0] = '\0';
    line_len = MAX_LINE_LEN;
    line_ptr = 0;

    for(i = 0; i < size; i++) {
        m8 = i % 8;
        m16 = i % 16;

        if(i == 0 && show_addr) {
            if(cb == NULL)
                fprintf(fd, "%05x : ", addr);
            else {
                line_ptr += snprintf(line_buf + line_ptr, line_len, "%05x : ", addr);
                line_len -= line_ptr;
            }
        }
        if(i > 0 && (m16) == 0) {
            if(cb == NULL) {
                fprintf(fd, " |  %s\n", ascii);
                addr += 16;
                fprintf(fd, "%05x : ", addr);
            }
            else {
                line_ptr += snprintf(line_buf + line_ptr, line_len, " |  %s", ascii);
                cb(line_buf);

                addr += 16;

                line_ptr = 0;
                line_len = MAX_LINE_LEN;
                line_ptr += snprintf(line_buf + line_ptr, line_len, "%05x : ", addr);
                line_len -= line_ptr;
            }
        }
        else if(i > 0 && (m8) == 0) {
            if(cb == NULL)
                fprintf(fd, "-- ");
            else {
                line_ptr += snprintf(line_buf + line_ptr, line_len, "-- ");
                line_len -= line_ptr;
            }
        }

        if(cb == NULL)
            fprintf(fd, "%.2x ", ((unsigned char*)data)[i]);
        else {
            line_ptr += snprintf(line_buf + line_ptr, line_len, "%.2x ", ((unsigned char*)data)[i]);
            line_len -= line_ptr;
        }

		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~')
			ascii[m16] = ((unsigned char*)data)[i];
		else
			ascii[m16] = '.';
        ascii[m16 + 1] = '\0';
    }

    if(i > 0) {
        m16 = i % 16;
        if(m16 > 0) {
            if(cb == NULL)
                fprintf(fd, "%*s", (51 - ((m16 + (m16 > 8 ? 1 : 0)) * 3)), "");
            else {
                line_ptr += snprintf(line_buf + line_ptr, MAX_LINE_LEN, "%*s", (51 - ((m16 + (m16 > 8 ? 1 : 0)) * 3)), "");
                line_len -= line_ptr;
            }
        }
        if(cb == NULL)
            fprintf(fd, " |  %s\n", ascii);
        else {
            line_ptr += snprintf(line_buf + line_ptr, line_len, " |  %s", ascii);
            cb(line_buf);
        }
    }
}
