/// @file messages.c

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <pcap.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <string.h>
#include <error.h>
#include <errno.h>

#include "bitter.h"


/**
 * set_message_bits - sets 1-n bits at an arbitrary position in a binary message
 * (where n is the word len e.g., 64 when using 64bit words).
 * A binary message is an array of words in network-byte-order.
 * @param[in] message       Message array of n words
 * @param[in] message_len   Number of words in message
 * @param[in] start_bit     Absolute bit position where value should be inserted
 * @param[in] bit_len       Number of bits (1-n) of value to insert at start_bit
 *                          position
 * @param[in] value         A single word value to insert bits from into message
 *                          starting at MSB of value
 * @param[in] erase         if true, set range in message to 0 before inserting
 *                          value, if false, value is just ORed without erasing
 *                          before
 * @param[in] start_low     If true, start at bit position <bit_len> of value
 * @returns                 Positive integer of bit position in message where
 *                          inserted value ends, negative value in case of error
 */
int set_message_bits(WORD_T message[], int message_len,
                     int start_bit, int bit_len,
                     const WORD_T value,
                     bool erase, bool start_low) {

    int mbi = start_bit / WORD_BIT_LEN; // Message byte index.
    int mo = start_bit % WORD_BIT_LEN;  // Bit offset in message word.
    int bits_left = bit_len;

    // Is start_bit outside message.
    if(start_bit < 0 || mbi >= message_len)
        return -1;
    // If bit length > message word len.
    if(bit_len > WORD_BIT_LEN)
        return -2;
    // If value should be added starting at last word in mesage but spans
    // over end of message.
    if((mbi == (message_len-1)) & ((mo + bit_len) > WORD_BIT_LEN))
        return -3;

    WORD_T v = value;
    // Internally, message words are stored in network-byte-order (big-endian).
    WORD_T m = WORD_NTOH(message[mbi]);

#ifdef MESSAGE_DEBUG
    printf("---- set_message_bits xx\n");
    printf("start(%3d), bits(%3d), value(0x%016lx), mi(%3d), mo(%2d), bl(%2d)\n",
        start_bit, bit_len, v, mbi, mo, bits_left);
#endif

    // Adjust value to be placed on correct bit position in message word.
    WORD_T v1 = v;
    WORD_T mask = 0;
    if(start_low == true)
        // Unwanted bits are set to 0 by left-shift.
        v1 = v << (WORD_BIT_LEN-bit_len);
    else {
        // Mask out unwanted bits.
        mask = ((WORD_T)(pow_i(2, bit_len) - 1) << (WORD_BIT_LEN - bit_len));
        v1 = v1 & mask;
    }
    v1 = v1 >> mo;

    int bits_in_m0 = (WORD_BIT_LEN - mo);

#ifdef MESSAGE_DEBUG
    printf("     m0 [%3d](0x%016lx), v1(0x%016lx),  b(%2d)\n", mbi, m, v1, bits_in_m0);
#endif

    // Ensure bits in message are zeroed out befor inserting value,
    // otherwise value is ORed over previous message content.
    mask = 0;
    if(erase) {
        mask = ~((WORD_T)(pow_i(2, bit_len) - 1) << (WORD_BIT_LEN - bit_len) >> mo);
        m &= mask;
#ifdef MESSAGE_DEBUG
        printf("         mask(0x%016lx)\n", mask);
        printf("     m0m[%3d](0x%016lx)\n", mbi, m);
#endif
    }

    // OR value over message word.
    m |= v1;

#ifdef MESSAGE_DEBUG
    printf("     m0'[%3d](0x%016lx)\n", mbi, m);
#endif

    // Handle message word crossing.
    bits_left = bit_len - bits_in_m0;
    message[mbi] = WORD_HTON(m);

    if(bits_left > 0) {
        mbi++;
        m = WORD_NTOH(message[mbi]);
        if(start_low)
            v1 = (v << (WORD_BIT_LEN - bits_left));
        else {
            mask = ((WORD_T)(pow_i(2, bits_left) - 1) << (WORD_BIT_LEN - bits_left));
            v1 = (v << bits_in_m0) & mask;
        }
#ifdef MESSAGE_DEBUG
        printf("     m1 [%3d](0x%016lx), v1(0x%016lx), bl(%2d)\n",
            mbi, m, v1, bits_left);
#endif
        if(erase) {
            mask = ~((WORD_T)(pow_i(2, bits_left) - 1) << (WORD_BIT_LEN - bits_left));
            m &= mask;
#ifdef MESSAGE_DEBUG
            printf("         mask(0x%016lx)\n", mask);
            printf("     m1m[%3d](0x%016lx)\n", mbi, m);
#endif
        }

        m |= v1;
#ifdef MESSAGE_DEBUG
        printf("     m1'[%3d](0x%016lx)\n", mbi, m);
#endif
        message[mbi] = WORD_HTON(m);
    }

    // Return next bit position.
    int next_bit = start_bit + bit_len;
#ifdef MESSAGE_DEBUG
    printf("     next(%d)\n", next_bit);
#endif
    return next_bit;
}


/**
 * get_message_bits extracts 1-64 bits from an arbitrary position of a binary message.
 * A binary message is an array of words in network-byte-order.
 * @param[in] message       Message array of n words
 * @param[in] message_len   Number of words in message
 * @param[in] start_bit     Absolute bit position where value should be extracted
 * @param[in] bit_len       Number of bits (1-64) of value to extract from
 *                          start_bit position
 * @param[out] value        word pointer to receive extracted value starting at MSB
 * @param[in] start_low     If true, start at bit position <bit_len> of value
 * @returns                 Positive integer of bit position in message where
 *                          read value ends, negative value in case of error
 */
int get_message_bits(WORD_T message[], int message_len,
                     int start_bit, int bit_len, WORD_T* value,
                     bool start_low) {

    int mbi = start_bit / WORD_BIT_LEN; // Message byte index.
    int mo = start_bit % WORD_BIT_LEN;  // Bit offset in message word.

    // Is start_bit outside message.
    if(mbi >= message_len)
        return -1;
    // If bite length > message word len.
    if(bit_len > WORD_BIT_LEN)
        return -2;
    // If value should be added starting last word in mesage but spans
    // over end of message.
    if((mbi == (message_len-1)) & ((mo + bit_len) > WORD_BIT_LEN))
        return -3;

    // Internally, message is stored in network-byte-order (big-endian).
    WORD_T m = WORD_NTOH(message[mbi]);

#ifdef MESSAGE_DEBUG
    printf("---- get_message_bits\n");
    printf("start(%3d), bits(%3d), mi(%3d), mo(%2d)\n",
        start_bit, bit_len, mbi, mo);
#endif

    // Adjust value to be placed on correct bit position in message word.

#ifdef MESSAGE_DEBUG
    printf("     m0[%3d](0x%016lx)\n", mbi, m);
#endif

    WORD_T v = m << mo;
#ifdef MESSAGE_DEBUG
        printf("     v0     (0x%016lx)\n", v);
#endif
    // Mask out unwanted bits.
    WORD_T mask = ((WORD_T)(pow_i(2, bit_len) - 1) << (WORD_BIT_LEN - bit_len));
    v = v & mask;
#ifdef MESSAGE_DEBUG
        printf("        mask(0x%016lx)\n", mask);
        printf("     v1     (0x%016lx)\n", v);
#endif

    if(bit_len > (WORD_BIT_LEN-mo)) {
        mbi++;
        m = WORD_NTOH(message[mbi]);
#ifdef MESSAGE_DEBUG
        printf("     m1[%3d](0x%016lx)\n", mbi, m);
#endif
        v |= m >> (WORD_BIT_LEN-mo);
#ifdef MESSAGE_DEBUG
        printf("     v2     (0x%016lx)\n", v);
#endif
    }
    if(start_low)
        v = v >> (WORD_BIT_LEN - bit_len);

#ifdef MESSAGE_DEBUG
    printf("     vh     (0x%016lx)\n", v);
#endif

    if(value != NULL)
        *value = v;

    // Return next bit position.
    int next_bit = start_bit + bit_len;
#ifdef MESSAGE_DEBUG
    printf("     next(%d)\n", next_bit);
#endif
    return next_bit;
}


/**
 * set_message_bits2 sets an arbitrary number of bits at an arbitrary position
 * in a binary message.
 * A binary message is an array of words in network-byte-order.
 * @param[in] message       Message array of n words
 * @param[in] message_len   Number of words in message
 * @param[in] start_bit     Absolute bit position where bits from value should
 *                          be inserted into message
 * @param[in] bit_len       Number of bits from value to insert at start_bit
 *                          position
 * @param[in] value         An array of word values to insert bits from. Each
 *                          word in value is in host byte order
 * @param[in] value_len     Number of words in value array
 * @param[in] erase         if true, set range in message to 0 before inserting
 *                          value, if false, value is just ORed without erasing
 *                          before
 * @returns                 Positive integer of bit position in message where
 *                          inserted value ends, negative value in case of error
 */
int set_message_bits2(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      const WORD_T value[], int value_len,
                      bool erase) {

    int mbi = start_bit / WORD_BIT_LEN;   // Message byte index.

    // Is start_bit outside message.
    if(start_bit < 0 || mbi >= message_len)
        return -1;

    int n = start_bit;
    int l = bit_len;
    if(l > WORD_BIT_LEN)
        l = WORD_BIT_LEN;
    int r = bit_len;

#ifdef MESSAGE_DEBUG
    printf("------------------------------------------------\n");
    printf("set_message_bits2\n");
    printf("start_bit(%4d),        bit_len(%4d)\n", start_bit, bit_len);
#endif

    for(int i=0; i<value_len; i++) {
        WORD_T v = value[i];
#ifdef MESSAGE_DEBUG
        printf("        n(%4d),              l(%4d),   v(0x%016lx)\n", n, l, v);
#endif
        int n2 = set_message_bits(message, message_len, n, l, v, erase, false);
        if(n2 < 0)
            return n2 * 10;
        r -= (n2 - n);
        if(r <= 0)
            break;
        n = n2;
        l = r;
        if(l > WORD_BIT_LEN)
            l = WORD_BIT_LEN;
    }

    // Return next bit position.
    int next_bit = start_bit + bit_len;
#ifdef MESSAGE_DEBUG
    printf("     next(%d)\n", next_bit);
#endif
    return next_bit;
}


/**
 * get_message_bits2 extracts an arbitrary number of bits from an arbitrary
 * position of a binary message as a value array containing words.
 * A binary message is an array of words in network-byte-order.
 * @param[in] message       Message array of n words
 * @param[in] message_len   Number of words in message
 * @param[in] start_bit     Absolute bit position where value should be extracted
 * @param[in] bit_len       Number of bits (1-64) of value to extract from
 *                          start_bit position
 * @param[out] value        word pointer to receive extracted value
 * @param[in] value_len     Number of words in value array
 * @returns                 Positive integer of bit position in message where
 *                          read value ends, negative value in case of error
 */
int get_message_bits2(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      WORD_T value[], int value_len) {

    int mbi = start_bit / WORD_BIT_LEN;   // Message byte index.

    // Is start_bit outside message.
    if(start_bit < 0 || mbi >= message_len)
        return -1;
    if(value == NULL)
        return -2;

    int n = start_bit;
    int l = bit_len;
    if(l > WORD_BIT_LEN)
        l = WORD_BIT_LEN;
    int r = bit_len;

    for(int i=0; i<value_len; i++) {
        int n2 = get_message_bits(message, message_len, n, l, &value[i], false);
        if(n2 < 0)
            return n2 * 10;
        r -= (n2 - n);
        if(r <= 0)
            break;
        n = n2;
        l = r;
        if(l > WORD_BIT_LEN)
            l = WORD_BIT_LEN;
    }

    // Return next bit position.
    int next_bit = start_bit + bit_len;
#ifdef MESSAGE_DEBUG
    printf("     next(%d)\n", next_bit);
#endif
    return next_bit;
}


/**
 * set_message_bits3 sets an arbitrary number of bits at an arbitrary position
 * in a binary message.
 * A binary message is an array of words in network-byte-order.
 * @param[in] message       Message array of n words
 * @param[in] message_len   Number of words in message
 * @param[in] start_bit     Absolute bit position where value should be inserted
 * @param[in] bit_len       Number of bits of value to insert at start_bit position
 * @param[in] value         An array of uint8_t values to insert. Consecutive
 *                          bytes in network byte order.
 * @param[in] value_len     Number of uint8_t bytes in value array
 * @param[in] erase         if true, set range in message to 0 before inserting
 *                          value, if false, value is just ORed without erasing
 *                          before
 * @returns                 Positive integer of bit position in message where
 *                          inserted value ends, negative value in case of error
 */
int set_message_bits3(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      const uint8_t value[], int value_len,
                      bool erase) {

    // If bits to set in message > then bits available in message; exit
    int value_len_bits = value_len * 8;
    if(bit_len > value_len_bits)
        return -1;
    int bit_len_bytes = ceil(bit_len / 8.0);

    // Convert byte array to word array.
    // Copy over only needed bytes and not all bytes from value array.
    int vawd_len = ceil(bit_len / (WORD_BIT_LEN*1.0));
    int vawd_len_bits __attribute__((unused)) = vawd_len * WORD_BIT_LEN;
    WORD_T* vawd = calloc(vawd_len, WORD_BYTE_LEN);
    if(vawd == NULL)
        return -2;
    memcpy(vawd, value, bit_len_bytes);

#ifdef MESSAGE_DEBUG
    printf("------------------------------------------------\n");
    printf("set_message_bits3\n");
    printf("start_bit(%4d),        bit_len(%4d), bit_len_bytes(%4d)\n",
        start_bit, bit_len, bit_len_bytes);
    printf("value_len(%4d), value_len_bits(%4d)\n",
        value_len, value_len_bits);
    printf(" vawd_len(%4d),  vawd_len_bits(%4d)\n",
        vawd_len, vawd_len_bits);

    printf("\nMem network-byte-order\n");
    dump_hex(stdout, vawd, vawd_len*8, true, NULL);
    printf("0x%016lx\n", vawd[vawd_len-1]);
#endif

    // Adjust endianess to host endianess.
    // It is assumed that content in value array is in network byte order.
    for(int i=0; i<vawd_len; i++)
        vawd[i] = WORD_NTOH(vawd[i]);

#ifdef MESSAGE_DEBUG
    printf("\nMem host-byte-order 1\n");
    dump_hex(stdout, vawd, vawd_len*8, true, NULL);
    printf("0x%016lx\n", vawd[vawd_len-1]);
#endif

//     // Pad last element.
//     if(bit_len != vawd_len_bits) {
//         int shift = (vawd_len_bits - bit_len);
// #ifdef MESSAGE_DEBUG
//         printf("Shift last word right by %d bits\n", shift);
// #endif
//         vawd[vawd_len-1] = vawd[vawd_len-1] >> shift;
//     }

// #ifdef MESSAGE_DEBUG
//     printf("\nMem host-byte-order 2\n");
//     dump_hex(stdout, vawd, vawd_len*8, true, NULL);
//     printf("0x%016lx\n", vawd[vawd_len-1]);
// #endif

    int next_pos = set_message_bits2(message, message_len, start_bit, bit_len,
                                     vawd, vawd_len, erase);
    free(vawd);
    if(next_pos < 0)
        next_pos *= 10;
    return next_pos;
}


/**
 * get_message_bits3 extracts an arbitrary number of bits from an arbitrary
 * position of a binary message as a value array containing consecurive uint8_t
 * bytes in network byte order.
 * A binary message is an array of words in network-byte-order.
 * @param[in] message       Message array of n words
 * @param[in] message_len   Number of words in message
 * @param[in] start_bit     Absolute bit position where value should be extracted
 * @param[in] bit_len       Number of bits (1-64) of value to extract from
 *                          start_bit position
 * @param[out] value        uint8_t pointer to receive extracted values
 * @param[in] value_len     Number of uint8_t bytes in value array
 * @returns                 Positive integer of bit position in message where
 *                          read value ends, negative value in case of error
 */
int get_message_bits3(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      uint8_t value[], int value_len) {

    // If bits to get from message > then bits available in message; exit
    int value_len_bits = value_len * 8;
    if(bit_len > value_len_bits)
        return -1;
    if( value == NULL)
        return -2;
    int bit_len_bytes = ceil(bit_len / 8.0);

    // Prepare word array to hold extracted values.
    int vawd_len = ceil(bit_len / (WORD_BIT_LEN*1.0));
    int vawd_len_bits = vawd_len * WORD_BIT_LEN;
    WORD_T* vawd = calloc(vawd_len, WORD_BYTE_LEN);
    if(vawd == NULL)
        return -3;
    int next_pos = get_message_bits2(message, message_len, start_bit, bit_len,
                                     vawd, vawd_len);
    if(next_pos < 0) {
        free(vawd);
        return next_pos * 10;
    }

#ifdef MESSAGE_DEBUG
    printf("------------------------------------------------\n");
    printf("get_message_bits3\n");
    printf("start_bit(%4d),        bit_len(%4d), bit_len_bytes(%4d)\n",
        start_bit, bit_len, bit_len_bytes);
    printf("value_len(%4d), value_len_bits(%4d)\n",
        value_len, value_len_bits);
    printf(" vawd_len(%4d),  vawd_len_bits(%4d)\n",
        vawd_len, vawd_len_bits);

    printf("\nMem network-byte-order\n");
    dump_hex(stdout, vawd, vawd_len*8, true, NULL);
    printf("0x%016lx\n", vawd[vawd_len-1]);
#endif

    // Clear unused bits of last element.
    if(bit_len != vawd_len_bits) {
        int br = (vawd_len_bits - bit_len);
        WORD_T mask = ((WORD_T)pow_i(2, bit_len) - 1) << br;
        WORD_T vtmp = vawd[vawd_len-1] &= mask;
#ifdef MESSAGE_DEBUG
        printf("Mask:\n");
        printf("bits_remaining(%d)\n", br);
        printf("0x%016lx\n", mask);
#endif
        vawd[vawd_len-1] = vtmp;
    }

#ifdef MESSAGE_DEBUG
    printf("\nMem network-byte-order 1\n");
    dump_hex(stdout, vawd, vawd_len*8, true, NULL);
    printf("0x%016lx\n", vawd[vawd_len-1]);
#endif

    // Convert word array to uint8 array in network byte order.
    for(int i=0; i<vawd_len; i++)
        vawd[i] = WORD_NTOH(vawd[i]);

#ifdef MESSAGE_DEBUG
    printf("\nMem host-byte-order 2\n");
    dump_hex(stdout, vawd, vawd_len*8, true, NULL);
    printf("0x%016lx\n", vawd[vawd_len-1]);
#endif

    // Copy over
    memcpy(value, vawd, bit_len_bytes);
    free(vawd);
    return next_pos;
}
