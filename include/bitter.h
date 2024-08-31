/// @file bitter.h

#ifndef _BITTER_H_
#define _BITTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


// Endian conversion for 64 bit values
#if __BIG_ENDIAN__
# define htonll(x) (x)
# define ntohll(x) (x)
#else
# define htonll(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
# define ntohll(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif


//#define MESSAGE_DEBUG

// Using 64bit words.
#define WORD_T          uint64_t
#define WORD_BIT_LEN    64
#define WORD_BYTE_LEN   sizeof(WORD_T)
#define WORD_HTON       htonll
#define WORD_NTOH       ntohll

// Using 32bit words.
// #define WORD_T          uint32_t
// #define WORD_BIT_LEN    32
// #define WORD_BYTE_LEN   sizeof(WORD_T)
// #define WORD_HTON       htonl
// #define WORD_NTOH       ntohl

extern int pow_i(int x, int n);
extern int rand_in_range(int min, int max);
extern void dump_hex(FILE* fd, const void* data, unsigned int size, bool show_addr,
    void (*cb)(const char*));


extern int set_message_bits(WORD_T message[], int message_len,
                     int start_bit, int bit_len,
                     const WORD_T value,
                     bool erase, bool start_low);
extern int get_message_bits(WORD_T message[], int message_len,
                     int start_bit, int bit_len, WORD_T* value,
                     bool start_low);


extern int set_message_bits2(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      const WORD_T value[], int value_len,
                      bool erase);
extern int get_message_bits2(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      WORD_T value[], int value_len);


extern int set_message_bits3(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      const uint8_t value[], int value_len,
                      bool erase);
extern int get_message_bits3(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      uint8_t value[], int value_len);

#endif
