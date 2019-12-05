#ifndef SIMDJSON_WESTMERE_STAGE1_FIND_MARKS_H
#define SIMDJSON_WESTMERE_STAGE1_FIND_MARKS_H

#include "simdjson/portability.h"

#ifdef IS_X86_64

#include "westmere/bitmask.h"
#include "westmere/simd.h"
#include "simdjson/stage1_find_marks.h"

TARGET_WESTMERE
namespace simdjson::westmere {

using namespace simd;

really_inline void find_whitespace_and_operators(
  const simd8x64<uint8_t> in,
  uint64_t &whitespace, uint64_t &op) {

  // These lookups rely on the fact that anything < 127 will match the lower 4 bits, which is why
  // we can't use the generic lookup_16.
  auto whitespace_table = simd8<uint8_t>::repeat_16(' ', 100, 100, 100, 17, 100, 113, 2, 100, '\t', '\n', 112, 100, '\r', 100, 100);
  auto op_table = simd8<uint8_t>::repeat_16(',', '}', 0, 0, 0xc0u, 0, 0, 0, 0, 0, 0, 0, 0, 0, ':', '{');

  whitespace = in.map([&](simd8<uint8_t> _in) {
    return _in == simd8<uint8_t>(_mm_shuffle_epi8(whitespace_table, _in));
  }).to_bitmask();

  op = in.map([&](simd8<uint8_t> _in) {
    // | 32 handles the fact that { } and [ ] are exactly 32 bytes apart
    return (_in | 32) == simd8<uint8_t>(_mm_shuffle_epi8(op_table, _in-','));
  }).to_bitmask();
}

really_inline simd8<uint8_t> lookup_flipped_low_bits(
  simd8<uint8_t> flipped,
  uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4, uint8_t in5, uint8_t in6, uint8_t in7, uint8_t in8,
  uint8_t in9, uint8_t in10, uint8_t in11, uint8_t in12, uint8_t in13, uint8_t in14, uint8_t in15, uint8_t in16
) {
  // This is a super Intel-focused trick. Generally you have to mask out the high bits before doing
  // a table lookup, but on Intel the 16-bit table lookup ignores all bits except the bottom 4 and
  // the high bit. If the high bit is 1, it always returns 0. prev1_flipped will only be 1 if it's
  // ASCII, and we're not detecting any ASCII errors here anyway, so it's all good :)
  return flipped.lookup_16<uint8_t>(
    in1, in2, in3, in4, in5, in6, in7, in8,
    in9, in10, in11, in12, in13, in14, in15, in16
  );
}

#include "generic/utf8_lookup2_algorithm.h"
#include "generic/stage1_find_marks.h"

} // namespace westmere
UNTARGET_REGION

TARGET_WESTMERE
namespace simdjson {

template <>
int find_structural_bits<Architecture::WESTMERE>(const uint8_t *buf, size_t len, simdjson::ParsedJson &pj, bool streaming) {
  return westmere::stage1::find_structural_bits<64>(buf, len, pj, streaming);
}

} // namespace simdjson
UNTARGET_REGION

#endif // IS_X86_64
#endif // SIMDJSON_WESTMERE_STAGE1_FIND_MARKS_H
