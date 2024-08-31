#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <math.h>

#include <cmocka.h>

#include "bitter.h"


// Override MESSAGE_DEBUG from bitter.h here if needed.
//#define MESSAGE_DEBUG
#ifdef MESSAGE_DEBUG
    #define dbg_printf(...)  printf(__VA_ARGS__)
#else
    #define dbg_printf(...)
#endif


#define MESSAGE_SIZE 24 // 24 * 64 bits = 1536 bits


int fill_rand(uint8_t* buffer, int byte_len, int bit_len) {
    int bit_len_bytes = ceil(bit_len / 8.0);
    if(bit_len_bytes > byte_len)
        return -1;

    // Remaining bits in last byte.
    int br = (bit_len_bytes * 8) - bit_len;
    dbg_printf("fill_rand: byte_len(%d), bit_len(%d), bit_len_bytes(%d), "
        "bits_remaining(%d)\n",
            byte_len, bit_len, bit_len_bytes, br);

    // Create random field data.
    for(int i=0; i<bit_len_bytes; i++) {
        buffer[i] = rand_in_range(0, 255);
    }

    // Ensure that unused bits at end of field are set to zero as they are
    // also set to zero when returned decoded with get_... functions.
    if(br != 0) {
        int mask = (uint8_t)~(pow_i(2, br) - 1);
        dbg_printf("mask(0x%02x)\n", mask);
        buffer[bit_len_bytes-1] &= mask;
    }

#ifdef MESSAGE_DEBUG
    printf("Generated value:\n");
    dump_hex(stdout, buffer, byte_len, true, NULL);
#endif

    return bit_len_bytes;
}

void test_binary_messages(void **state) {
    WORD_T message[MESSAGE_SIZE] = {0};

    WORD_T f1 = 0xabc;                // 12
    WORD_T f2 = 0x123456;             // 24
    WORD_T f3 = 0x2828282;            // 28
    WORD_T f4 = 0xffffffff;           // 32
    WORD_T f5 = 0xaaaaaaaacccccccc;   // 64
    WORD_T f6 = 0x2;                  // 2
    WORD_T f7 = 0x1;                  // 2

    dbg_printf("f1: %lx\n", f1);
    dbg_printf("f2: %lx\n", f2);
    dbg_printf("f3: %lx\n", f3);
    dbg_printf("f4: %lx\n", f4);
    dbg_printf("f5: %lx\n", f5);
    dbg_printf("f6: %lx\n", f6);
    dbg_printf("f7: %lx\n", f7);

    // Pack the fields into the message array.
    int next_pos = 0;
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos, 12, f1, true, true);
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos, 24, f2, true, true);
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos, 28, f3, true, true);
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos, 32, f4, true, true);
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos, 64, f5, true, true);
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos,  2, f6, true, true);
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos,  2, f7, true, true);

#ifdef MESSAGE_DEBUG
    printf("===============================\n");
    dump_hex(stdout, message, sizeof(message), true, NULL);
    printf("===============================\n");
#endif

    // Copy message to simulate network transfer.
    WORD_T f1_2 = 0;
    WORD_T f2_2 = 0;
    WORD_T f3_2 = 0;
    WORD_T f4_2 = 0;
    WORD_T f5_2 = 0;
    WORD_T f6_2 = 0;
    WORD_T f7_2 = 0;

    uint8_t* message_tmp = malloc(sizeof(message));
    assert_non_null(message_tmp);
    memcpy(message_tmp, message, sizeof(message));
    WORD_T* message2 = (WORD_T*)message_tmp;

    // Decode copied message.
    next_pos = 0;
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos, 12, &f1_2, true);
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos, 24, &f2_2, true);
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos, 28, &f3_2, true);
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos, 32, &f4_2, true);
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos, 64, &f5_2, true);
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos,  2, &f6_2, true);
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos,  2, &f7_2, true);

    dbg_printf("f1_2: %lx\n", f1_2);
    dbg_printf("f2_2: %lx\n", f2_2);
    dbg_printf("f3_2: %lx\n", f3_2);
    dbg_printf("f4_2: %lx\n", f4_2);
    dbg_printf("f5_2: %lx\n", f5_2);
    dbg_printf("f6_2: %lx\n", f6_2);
    dbg_printf("f7_2: %lx\n", f7_2);

    assert_int_equal(f1, f1_2);
    assert_int_equal(f2, f2_2);
    assert_int_equal(f3, f3_2);
    assert_int_equal(f4, f4_2);
    assert_int_equal(f5, f5_2);
    assert_int_equal(f6, f6_2);
    assert_int_equal(f7, f7_2);

    free(message2);
}

void test_binary_messages2(void **state) {
    WORD_T message[MESSAGE_SIZE] = {0};

    WORD_T f1 = 0xabc;                // 12
    WORD_T f2 = 0x123456;             // 24
    WORD_T f3 = 0x2828282;            // 28
    WORD_T f4 = 0xffffffff;           // 32
    WORD_T f5 = 0xaaaaaaaacccccccc;   // 64
    WORD_T f6 = 0x2;                  // 2
    WORD_T f7 = 0x1;                  // 2
    WORD_T f8[] = {0x0102030405060708, 0xa0b0c0d0e0f0a1b2, 0xbb42100000000000}; // 148

    dbg_printf("f1: %lx\n", f1);
    dbg_printf("f2: %lx\n", f2);
    dbg_printf("f3: %lx\n", f3);
    dbg_printf("f4: %lx\n", f4);
    dbg_printf("f5: %lx\n", f5);
    dbg_printf("f6: %lx\n", f6);
    dbg_printf("f7: %lx\n", f7);
    dbg_printf("f8: %016lx %016lx %016lx\n", f8[0], f8[1], f8[2]);

    // Pack the fields into the message array.
    int next_pos = 0;
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,  12, f1, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,  24, f2, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,  28, f3, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,  32, f4, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,  64, f5, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,   2, f6, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,   2, f7, true, true);
    next_pos = set_message_bits2(message, MESSAGE_SIZE, next_pos, 148, f8, 3, true);

#ifdef MESSAGE_DEBUG
    printf("===============================\n");
    dump_hex(stdout, message, sizeof(message), true, NULL);
    printf("===============================\n");
#endif

    // Copy message to simulate network transfer.
    WORD_T f1_2 = 0;
    WORD_T f2_2 = 0;
    WORD_T f3_2 = 0;
    WORD_T f4_2 = 0;
    WORD_T f5_2 = 0;
    WORD_T f6_2 = 0;
    WORD_T f7_2 = 0;
    WORD_T f8_2[3] = {0};

    uint8_t* message_tmp = malloc(sizeof(message));
    assert_non_null(message_tmp);
    memcpy(message_tmp, message, sizeof(message));
    WORD_T* message2 = (WORD_T*)message_tmp;

    // Decode copied message.
    next_pos = 0;
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,  12, &f1_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,  24, &f2_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,  28, &f3_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,  32, &f4_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,  64, &f5_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,   2, &f6_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,   2, &f7_2, true);
    next_pos = get_message_bits2(message2, MESSAGE_SIZE, next_pos, 148,  f8_2, 3);

    dbg_printf("f1_2: %lx\n", f1_2);
    dbg_printf("f2_2: %lx\n", f2_2);
    dbg_printf("f3_2: %lx\n", f3_2);
    dbg_printf("f4_2: %lx\n", f4_2);
    dbg_printf("f5_2: %lx\n", f5_2);
    dbg_printf("f6_2: %lx\n", f6_2);
    dbg_printf("f7_2: %lx\n", f7_2);
    dbg_printf("f8_2: %016lx %016lx %016lx\n", f8_2[0], f8_2[1], f8_2[2]);

    assert_int_equal(f1, f1_2);
    assert_int_equal(f2, f2_2);
    assert_int_equal(f3, f3_2);
    assert_int_equal(f4, f4_2);
    assert_int_equal(f5, f5_2);
    assert_int_equal(f6, f6_2);
    assert_int_equal(f7, f7_2);

    assert_int_equal(f8[0], f8_2[0]);
    assert_int_equal(f8[1], f8_2[1]);
    assert_int_equal(f8[2], f8_2[2]);

    free(message2);
}

void test_binary_messages3(void **state) {
    WORD_T message[MESSAGE_SIZE] = {0};

    // This test adds a byte array in network byte order to a binary message.

    WORD_T f1 = 0xabc;                // 12
    WORD_T f2 = 0x123456;             // 24
    WORD_T f3 = 0x2828282;            // 28
    WORD_T f4 = 0xffffffff;           // 32
    WORD_T f5 = 0xaaaaaaaacccccccc;   // 64
    WORD_T f6 = 0x2;                  // 2
    WORD_T f7 = 0x1;                  // 2
    uint8_t f8_b[] = {                // 148
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0xa1, 0xb2,
        0xbb, 0x42, 0x10
    };

    dbg_printf("f1: %lx\n", f1);
    dbg_printf("f2: %lx\n", f2);
    dbg_printf("f3: %lx\n", f3);
    dbg_printf("f4: %lx\n", f4);
    dbg_printf("f5: %lx\n", f5);
    dbg_printf("f6: %lx\n", f6);
    dbg_printf("f7: %lx\n", f7);
    dbg_printf("f8:\n");
#ifdef MESSAGE_DEBUG
    dump_hex(stdout, f8_b, sizeof(f8_b), true, NULL);
#endif

    // Pack the fields into the message array.
    int next_pos = 0;
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,  12, f1, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,  24, f2, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,  28, f3, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,  32, f4, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,  64, f5, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,   2, f6, true, true);
    next_pos = set_message_bits (message, MESSAGE_SIZE, next_pos,   2, f7, true, true);
    next_pos = set_message_bits3(message, MESSAGE_SIZE, next_pos, 148, f8_b, 19, true);

#ifdef MESSAGE_DEBUG
    printf("===============================\n");
    dump_hex(stdout, message, sizeof(message), true, NULL);
    printf("===============================\n");
#endif

    // Copy message to simulate network transfer.
    WORD_T f1_2 = 0;
    WORD_T f2_2 = 0;
    WORD_T f3_2 = 0;
    WORD_T f4_2 = 0;
    WORD_T f5_2 = 0;
    WORD_T f6_2 = 0;
    WORD_T f7_2 = 0;
    uint8_t f8_2b[sizeof(f8_b)] = {0};

    uint8_t* message_tmp = malloc(sizeof(message));
    assert_non_null(message_tmp);
    memcpy(message_tmp, message, sizeof(message));
    WORD_T* message2 = (WORD_T*)message_tmp;

    // Decode copied message.
    next_pos = 0;
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,  12, &f1_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,  24, &f2_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,  28, &f3_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,  32, &f4_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,  64, &f5_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,   2, &f6_2, true);
    next_pos = get_message_bits (message2, MESSAGE_SIZE, next_pos,   2, &f7_2, true);
    next_pos = get_message_bits3(message2, MESSAGE_SIZE, next_pos, 148, f8_2b, sizeof(f8_2b));

    dbg_printf("f1_2: %lx\n", f1_2);
    dbg_printf("f2_2: %lx\n", f2_2);
    dbg_printf("f3_2: %lx\n", f3_2);
    dbg_printf("f4_2: %lx\n", f4_2);
    dbg_printf("f5_2: %lx\n", f5_2);
    dbg_printf("f6_2: %lx\n", f6_2);
    dbg_printf("f7_2: %lx\n", f7_2);

#ifdef MESSAGE_DEBUG
    printf("f8_2b:\n");
    dump_hex(stdout, f8_2b, sizeof(f8_2b), true, NULL);
#endif

    assert_int_equal(f1, f1_2);
    assert_int_equal(f2, f2_2);
    assert_int_equal(f3, f3_2);
    assert_int_equal(f4, f4_2);
    assert_int_equal(f5, f5_2);
    assert_int_equal(f6, f6_2);
    assert_int_equal(f7, f7_2);
    assert_memory_equal(f8_b, f8_2b, sizeof(f8_b));

    free(message2);
}

void test_binary_messages3_R(void **state) {
    WORD_T message[MESSAGE_SIZE] = {0};
    uint8_t value[sizeof(WORD_T)] = {0};
    uint8_t value2[sizeof(WORD_T)] = {0};

    // Tests all combinations of start position and field length up to
    // WORD_T bit length with random field values.
    for(int start_bit=0; start_bit<WORD_BIT_LEN; start_bit++) {
        for(int bit_len=1; bit_len<=WORD_BIT_LEN; bit_len++) {
            dbg_printf("check start_bit(%2d), bit_len(%2d)\n", start_bit, bit_len);
            memset(message, 0xff, sizeof(message));

            // Create random value and insert it into message.
            fill_rand(value, sizeof(value), bit_len);
#ifdef MESSAGE_DEBUG
            // Show random value to write.
            dbg_printf("Value to write:\n");
            dump_hex(stdout, value, sizeof(value), true, NULL);
#endif
            int rtc = set_message_bits3(message, MESSAGE_SIZE, start_bit, bit_len,
                value, ceil(bit_len / 8.0), true);
            dbg_printf("set rtc(%d)\n", rtc);
            assert_true(rtc >= 0);

#ifdef MESSAGE_DEBUG
            // Show message after random value was written.
            dbg_printf("Message:\n");
            dump_hex(stdout, message, sizeof(message), true, NULL);
#endif

            // Extract value from message and compare with written value.
            rtc = get_message_bits3(message, MESSAGE_SIZE, start_bit, bit_len,
                value2, ceil(bit_len / 8.0));
#ifdef MESSAGE_DEBUG
            // Show extracted field value from message.
            dbg_printf("Value read back:\n");
            dump_hex(stdout, value2, sizeof(value2), true, NULL);
#endif
            dbg_printf("get rtc(%d)\n", rtc);
            assert_true(rtc >= 0);

            assert_memory_equal(value, value2, sizeof(value));
        }
    }
}

void test_binary_messages_R(void **state) {
    // Create multiple fields with random lenghts and content to fill
    // message.

    // Structure which holds a message field definition, value which was
    // written to message and the value which was read back from message.
    typedef struct {
        char name[17];
        int start_bit;
        int bit_len;
        int byte_len;
        uint8_t* value;
        uint8_t* value2;
    } field_t, *field_p;

    // Randomly create bit fields with random length and content.
    int msg_max_bit_len = MESSAGE_SIZE * 64;
    int msg_max_byte_len __attribute__((unused)) = ceil(msg_max_bit_len / 8.0);
    WORD_T message[MESSAGE_SIZE] = {0};
    field_p fields = NULL;
    int field_cnt = 0;
    int next_start_bit = 0;
    int next_bit_len = rand_in_range(1, 1024);
    int next_byte_len = ceil(next_bit_len / 8.0);

    dbg_printf("initial start_bit(%d), bit_len(%d), msg_bit_len(%d)\n",
        next_start_bit, next_bit_len, msg_max_bit_len);

    while(next_start_bit + next_bit_len < msg_max_bit_len) {
        field_cnt++;

        field_t f;
        snprintf(f.name, sizeof(f.name), "field%d", field_cnt);
        f.start_bit = next_start_bit;
        f.bit_len = next_bit_len;
        f.byte_len = next_byte_len;

        f.value = malloc(next_byte_len);
        fill_rand(f.value, next_byte_len, next_bit_len);

        f.value2 = malloc(next_byte_len);
        memset(f.value2, 0, next_byte_len);

        if(fields == NULL)
            fields = (field_p)calloc(1, sizeof(field_t));
        else
            fields = (field_p)realloc(fields, field_cnt * sizeof(field_t));
        memcpy(&fields[field_cnt-1], &f, sizeof(f));

        next_start_bit = next_start_bit + next_bit_len;
        next_bit_len = rand_in_range(1, 1024);
        next_byte_len = ceil(next_bit_len / 8.0);
    }

    // Show created fields.
#ifdef MESSAGE_DEBUG
    int msg_bit_len = next_start_bit;
    int msg_byte_len = ceil(msg_bit_len / 8.0);

    dbg_printf("\nFields:\n");
    dbg_printf("    Name             Start   Len\n");
    dbg_printf("    ----------------------------\n");
    for(int i=0; i<field_cnt; i++) {
        field_t f = fields[i];
        dbg_printf("    %-16s %5d %5d\n", f.name, f.start_bit, f.bit_len);

        // int f_value_byte_len = ceil(f.bit_len / 8.0);
        // dbg_printf("Value:\n");
        // dump_hex(stdout, f.value, f_value_byte_len, true, NULL);
        // dbg_printf("Value2:\n");
        // dump_hex(stdout, f.value2, f_value_byte_len, true, NULL);
    }
    dbg_printf("    ----------------------------\n");
    dbg_printf("Message:\n");
    dbg_printf("    max bit_len(%5d), byte_len(%5d)\n",
        msg_max_bit_len, msg_max_byte_len);
    dbg_printf("    msg bit_len(%5d), byte_len(%5d)\n",
        msg_bit_len, msg_byte_len);
    dbg_printf("\n");
#endif

    // Encode fields.
    for(int i=0; i<field_cnt; i++) {
        field_t f = fields[i];
        int rtc = set_message_bits3(message, MESSAGE_SIZE, f.start_bit, f.bit_len,
            f.value, f.byte_len, true);
        // dbg_printf("rtc(%d)\n", rtc);
        assert_true(rtc >= 0);
    }

    // Dump final message.
#ifdef MESSAGE_DEBUG
    dbg_printf("\n==========\n");
    dbg_printf("Final message:\n");
    dump_hex(stdout, message, msg_byte_len, true, NULL);
    dbg_printf("==========\n");
#endif

    // Decode fields.
    for(int i=0; i<field_cnt; i++) {
        field_t f = fields[i];
        int rtc = get_message_bits3(message, MESSAGE_SIZE, f.start_bit, f.bit_len,
            f.value2, f.byte_len);
        // dbg_printf("rtc(%d)\n", rtc);
        assert_true(rtc >= 0);
    }

    // Compare results.
    for(int i=0; i<field_cnt; i++) {
        field_t f = fields[i];
#ifdef MESSAGE_DEBUG
        dbg_printf("\nCompare (%s):\n", f.name);
        dbg_printf("    Value:\n");
        dump_hex(stdout, f.value, f.byte_len, true, NULL);
        dbg_printf("    Value2:\n");
        dump_hex(stdout, f.value2, f.byte_len, true, NULL);
#endif

        assert_memory_equal(f.value, f.value2, f.byte_len);
    }

    // Free fields.
    for(int i=0; i<field_cnt; i++) {
        field_t f = fields[i];
        free(f.value);
        free(f.value2);
    }
    free(fields);
}

void test_binary_messages_A(void **state) {
    WORD_T message[MESSAGE_SIZE] = {0};

    WORD_T f1 = 0x48656c6c6f20576f;   // 64
    WORD_T f2 = 0x726c642020202020;   // 64
    WORD_T f3 = 0x2020202020202020;   // 64
    WORD_T f4 = 0x000002020595a000;   // 44

    dbg_printf("f1: %lx\n", f1);
    dbg_printf("f2: %lx\n", f2);
    dbg_printf("f3: %lx\n", f3);
    dbg_printf("f4: %lx\n", f4);

    // Pack the fields into the message array.
    int next_pos = 51;
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos, 64, f1, true, true);
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos, 64, f2, true, true);
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos, 64, f3, true, true);
    next_pos = set_message_bits(message, MESSAGE_SIZE, next_pos, 44, f4, true, true);

#ifdef MESSAGE_DEBUG
    dump_hex(stdout, message, 45, true, NULL);
#endif

    // Copy message to simulate network transfer.
    WORD_T f1_2 = 0;
    WORD_T f2_2 = 0;
    WORD_T f3_2 = 0;
    WORD_T f4_2 = 0;

    uint8_t* message_tmp = malloc(sizeof(message));
    assert_non_null(message_tmp);
    memcpy(message_tmp, message, sizeof(message));
    WORD_T* message2 = (WORD_T*)message_tmp;

    // Decode copied message.
    next_pos = 51;
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos, 64, &f1_2, true);
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos, 64, &f2_2, true);
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos, 64, &f3_2, true);
    next_pos = get_message_bits(message2, MESSAGE_SIZE, next_pos, 44, &f4_2, true);

    dbg_printf("f1_2: %lx\n", f1_2);
    dbg_printf("f2_2: %lx\n", f2_2);
    dbg_printf("f3_2: %lx\n", f3_2);
    dbg_printf("f4_2: %lx\n", f4_2);

    assert_int_equal(f1, f1_2);
    assert_int_equal(f2, f2_2);
    assert_int_equal(f3, f3_2);
    assert_int_equal(f4, f4_2);

    free(message2);
}

void test_binary_messages_B(void **state) {
    WORD_T message[MESSAGE_SIZE] = {0};

    uint8_t data[] = {
        0x80
    };
    int bit_len = 1;

#ifdef MESSAGE_DEBUG
    printf("\ndata:\n");
    dump_hex(stdout, data, sizeof(data), true, NULL);
#endif

    // Pack the fields into the message array.
    int next_pos = 0;
    next_pos = set_message_bits3(message, MESSAGE_SIZE, next_pos, bit_len, data, sizeof(data), true);

#ifdef MESSAGE_DEBUG
    printf("===============================\n");
    dump_hex(stdout, message, sizeof(message), true, NULL);
    printf("===============================\n");
#endif

    uint8_t data2[sizeof(data)] = {0};

    uint8_t* message_tmp = malloc(sizeof(message));
    assert_non_null(message_tmp);
    memcpy(message_tmp, message, sizeof(message));
    WORD_T* message2 = (WORD_T*)message_tmp;

    // Decode copied message.
    next_pos = 0;
    next_pos = get_message_bits3(message2, MESSAGE_SIZE, next_pos, bit_len, data2, sizeof(data2));

#ifdef MESSAGE_DEBUG
    printf("data2:\n");
    dump_hex(stdout, data2, sizeof(data2), true, NULL);
    printf("\n");
#endif

    assert_memory_equal(data, data2, sizeof(data));

    free(message2);
}

void example_1(void **state, bool start_low) {
    WORD_T message[2] = {0};
    memset(message, 0xdd, sizeof(message));

    WORD_T f1_value = 0xa800000000000023;
    int f1_bit_pos = 0;
    int f1_bit_len = 6;
    bool f1_start_low = start_low;

    dbg_printf("f1: 0x%016lx\n", f1_value);

    // Pack the fields into the message array.
    int next_pos = set_message_bits(message, MESSAGE_SIZE,
        f1_bit_pos, f1_bit_len, f1_value, true, f1_start_low);
    assert_true(next_pos >= 0);

#ifdef MESSAGE_DEBUG
    printf("===============================\n");
    dump_hex(stdout, message, sizeof(message), true, NULL);
    printf("===============================\n");
#endif

    // Copy message to simulate network transfer.
    WORD_T f1_value_2 = 0;

    uint8_t* message_tmp = malloc(sizeof(message));
    assert_non_null(message_tmp);
    memcpy(message_tmp, message, sizeof(message));
    WORD_T* message2 = (WORD_T*)message_tmp;

    // Decode copied message.
    next_pos = get_message_bits(message2, MESSAGE_SIZE,
        f1_bit_pos, f1_bit_len, &f1_value_2, f1_start_low);
    assert_true(next_pos >= 0);

    // Only compare relevant bits from original value with decoded value.
    WORD_T f1_mask = 0;
    if(f1_start_low)
        f1_mask = (WORD_T)(pow_i(2, f1_bit_len) - 1);
    else
        f1_mask = ((WORD_T)(pow_i(2, f1_bit_len) - 1) << (WORD_BIT_LEN - f1_bit_len));

    dbg_printf("mask: 0x%016lx\n", f1_mask);
    dbg_printf("f1_2: 0x%016lx\n", f1_value_2);

    assert_int_equal((f1_value & f1_mask), f1_value_2);

    free(message2);
}

void test_example_1_start_high(void **state) {
    example_1(state, false);
}

void test_example_1_start_low(void **state) {
    example_1(state, true);
}

void test_example_2(void **state) {
    WORD_T message[5] = {0};

#define f1_num_words    3
    WORD_T f1_value[f1_num_words] = {
        0x0102030405060708,
        0x1112131415161718,  // bits to insert ends somewhere in the middle of word 1
        0x2122232425262628
    };
    int f1_bit_pos = 61;
    int f1_bit_len = 77;
    int x = f1_bit_len - WORD_BIT_LEN;

    dbg_printf("f1:\n");
    for(int i=0; i<f1_num_words; i++)
        dbg_printf("  [%2d]: 0x%016lx\n", i, f1_value[i]);

    // Pack the fields into the message array.
    int next_pos = set_message_bits2(message, MESSAGE_SIZE,
        f1_bit_pos, f1_bit_len, f1_value, f1_num_words, true);
    assert_true(next_pos >= 0);

#ifdef MESSAGE_DEBUG
    printf("===============================\n");
    dump_hex(stdout, message, sizeof(message), true, NULL);
    printf("===============================\n");
#endif

    // Copy message to simulate network transfer.
    WORD_T f1_value_2[f1_num_words] = {0};

    uint8_t* message_tmp = malloc(sizeof(message));
    assert_non_null(message_tmp);
    memcpy(message_tmp, message, sizeof(message));
    WORD_T* message2 = (WORD_T*)message_tmp;

    // Decode copied message.
    next_pos = get_message_bits2(message2, MESSAGE_SIZE,
        f1_bit_pos, f1_bit_len, f1_value_2, f1_num_words);
    assert_true(next_pos >= 0);

    WORD_T f1_w1_mask = ((WORD_T)(pow_i(2, x) - 1) << (WORD_BIT_LEN - x));
    dbg_printf("mask: 0x%016lx\n", f1_w1_mask);
    dbg_printf("f1_2:\n");
    for(int i=0; i<f1_num_words; i++)
        dbg_printf("  [%2d]: 0x%016lx\n", i, f1_value_2[i]);

    assert_int_equal(f1_value[0], f1_value_2[0]);
    assert_int_equal((f1_value[1] & f1_w1_mask), f1_value_2[1]);
    assert_int_equal(f1_value_2[2], 0);

    free(message2);
}