#ifndef VALUE_H
#define VALUE_H

// JavaScript value
//
// Values are NaN-boxed double-precision floating point numbers. Some types
// have their data embedded as a part of the value, while more complex types
// include a pointer to some heap-allocated data
//
// Layout for NaN-boxed values:
//
//          12 bits reserved for NaN
//           |
//  1 pointer bit    3 tag bits       48 value bits
//   |       |        |                |
//   |.------+------..+.   .-----------+-----------.
//   ||             || |   |                       |
//   vv             vv v   v                       v
// | p1111111 | 11111ttt | vvvvvvvv | ... | vvvvvvvv |
//
// The pointer bit is set to indicate that the value is a pointer to
// heap-allocated data. Otherwise, the data is embedded in the value itself.
//
// The three tag bits distinguish how the data should be interpreted. The rest
// of the value, 48 bits in total, is used for the actual data.

// Table of tag values for pointers and inline values:
//
// | Value | Pointer            | Inline             |
// +-------+--------------------+--------------------+
// | 0b000 | (Reserved for NaN) | (Reserved for NaN) |
// | 0b001 |             Object |               Null |
// | 0b010 |             BigInt |          Undefined |
// | 0b011 |             String |               True |
// | 0b100 |             Symbol |              False |
// | 0b101 |           (Unused) |           (Unused) |
// | 0b110 |           (Unused) |           (Unused) |
// | 0b111 |           (Unused) |           (Unused) |

#define TAG_OBJECT      0b001
#define TAG_BIGINT      0b010
#define TAG_STRING      0b011
#define TAG_SYMBOL      0b100

#define TAG_UNDEFINED   0b001
#define TAG_NULL        0b010
#define TAG_TRUE        0b011
#define TAG_FALSE       0b100

#define POINTER_MASK    (1 << 0x3f)
#define NAN_MASK        (0xfff << 0x33)
#define TAG_MASK        (7 << 0x30)
#define VALUE_MASK      0xffffffffffff

#endif // VALUE_H
