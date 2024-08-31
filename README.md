
# Bitter - a framework for constructing binary content

Bitter is a framework for constructing binary content
like messages used in network protocols.

It hides the bitter facts of handling single bits and bytes.

## Rationale

The mother of all things regarding computer processed
content is: The Byte - containing 8 Bits.

Based on that, combining bytes to words of various bit sizes like 16, 32, 64 ... have evolved over time.
But they are all based on - Bytes.

But what if you have to work with sub byte content?
Sure there are ways to set some bits in bytes or
words by using binary arithmetics (e.g., Shift, AND,
OR, ...) or use language constructs like (more or
less portable) bit-fields.

But what if you have to work with bit quantities on a
larger scale. For example constructing multi kBit
length messages with bit fields ranging from 1 up to
kBit in length?

So that's why this framework was written.

## Implementation

Internally, a n-bit binary message (or any other
binary content) is represented as an array of n-words
capable to hold at minimum n-bits. Depending on used
target hardware, word size [ *word_bit_len* ] can
vary, but is per default 64-bits.

*NOTE: The used word size can be configured in
`bitter.h` header file.*

Binary content of n-bits length can then be added or
extracted from this message word array at any bit
position.

The words in the message array are all automatically
encoded in Network-Byte-Order (Big-Endian) by the
`set-...` functions described below.

The framework hides the nitty gritty details of bit
handling (e.g., Shift's, OR's etc.) to place or
extract the bits to/from the correct position in the
word array.

## Bit Functions

Three `set-...` and `get-...` functions are
provided, each using differnt value types.

### set_message_bits

```C
int set_message_bits(WORD_T message[], int message_len,
                     int start_bit, int bit_len,
                     const WORD_T value,
                     bool erase, bool start_low);
```

Accepts a word as input `value` from which to place
`bit_len` bits (up to *word_bit_len*) into `message`
(which has a length of `message_len` words) at
bit position `start_bit`. Bits are ORed to the
message. If `erase` is `true`, the range of bits which
should be set in message are zeroed befor they are
set.

The maximum number of bits which could be set at once
using this method is *word_bit_len*, e.g., when using
32-bit words this is 32.

When `start_low` is `false`, `bit_len` bits are
inserted into `message` starting from `value` MSB, if
it is `false`, bits are inserted from `value` LSB.

Example: Assuming 16-bit words, a `value` of `0xa823`
hex (or `b1010_1000_0010_0011` in binary)
and a `bit_len` of 6-bits to insert into message.
When `start_low` is `false`, the 6 MSB bits `b1010_10`
will be added to the message. If it is `true`, the 6
LSB bits `b10_0011` will be added.

Have a look also in `/tests` folder file
`test_bitter.c` function `test_example_1`.

### get_message_bits

```C
int get_message_bits(WORD_T message[], int message_len,
                     int start_bit, int bit_len,
                     WORD_T* value,
                     bool start_low);
```

Extracts `bit_len` (up to *word_bit_len*) bits from
message at bit position `start_bit` into word
variable pointed to by `value` pointer. When
`start_low` is `false`, the extracted bits are placed
into `value` starting MSB, if `true`, starting LSB.

Example: Reversing the example shown for
`set_message_bits`, extracting 6-bits starting at
`start_bit` position 0 in `message`, with
`start_low` set to `false` will result in `0xa800`
placed in `value`. Setting `start_low` to `true`,
will result in `0x0023`.

### set_message_bits2

```C
int set_message_bits2(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      const WORD_T value[], int value_len,
                      bool erase);
```

This is the same as `set_message_bits` but accepts a
word array from which bits up to `value_len` *
*word_bit_len* can be inserted into `message` at
once. This function always starts inserting bits
into `message` from value word[0] MSB.

Example: (assuming 16-bit words)
```C
uint16_t value[] = {0x1234, 0x5678, 0x9abc};
```
From the above word array you want to insert 19-bits
into message. Value word[0] (`0x1234`) is completely
inserted and from value word[1] only the 3 MSB bits
`b010` are inserted. Bits 4-0 of
value word[1] and walue word[2] are ignored.

Have a look also in `/tests` folder file
`test_bitter.c` function `test_example_2`.

### get_message_bits2

```C
int get_message_bits2(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      WORD_T value[], int value_len);
```

### set_message_bits3

```C
int set_message_bits3(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      const uint8_t value[], int value_len,
                      bool erase);
```

This function inserts bits from a byte array value
into `message` starting with MSB bit 7 of byte[0].

Example: Suppose you have the follwoing byte array
and want to insert 11 bits from it somewhere into a
binary message:

```C
uint8_t byte_array[] = {0x12, 0x34, 0x56};

/*
 byte 0 (0x12)  | byte 1 (0x34) | byte 2 (0x56)
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|0 0 0 1 0 0 1 0|0 0 1 1 0 1 0 0|0 1 0 1 0 1 1 0|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
```

Using:
```C
set_message_bits3(msg, msg_len, start_bit_in_msg, 11,
                  byte_array, sizeof(byte_array),
                  true);
```
the first 11 bits (`b0001_0010_001` or `0x1220`) of
the byte array will be added to the message. Bits
4-0 of byte 1 and the rest of the array (byte 2) are
ignored.

### get_message_bits3

```C
int get_message_bits3(WORD_T message[], int message_len,
                      int start_bit, int bit_len,
                      uint8_t value[], int value_len);
```

## Tool Functions

### dump_hex

```C
void dump_hex(FILE* fd, const void* data, unsigned int size,
              bool show_addr, void (*cb)(const char*));
```

Creates a hex dump like shown below.

```
00000 : 00 00 00 00 00 00 00 00 -- 08 10 18 20 28 30 38 40  |  ........... (08@
00010 : 88 80 00 00 00 00 00 00 -- 00 00 00 00 00 00 00 00  |  ................
00020 : 00 00 00 00 00 00 00 00                             |  ........
```

Output can be directed to e.g., `stdout` or a file using
`fd` argument. Content to dump is specified via `data`
pointer. The length to dump is given by `size` argument.
Setting `show_addr` to `true` prepends an address to every
dumped line. To further process the output, `cb` can be set
to a callback function with signature
`void callback(const char* line);` which is called for every
dumped line.

## Prerequisites

This project uses Microsoft VS-Code as IDE and cmocka as unit-test framework.

* [VS-Code](https://code.visualstudio.com/)
* [cmocka](https://cmocka.org/)

## Misc

This project is by no means complete or error free or
production ready. Feel free to play with it and contribute if
you find a bug or thing which could be made better.

### License

This project is licensed under **BSD 3-Clause License**.
See [LICENSE](LICENSE) file.

### Contributing

If you would like to contribute (which is greatly
appreciated) please read the [Contributing Guide](CONTRIBUTING.md) beforehand.
