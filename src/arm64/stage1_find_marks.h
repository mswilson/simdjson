#ifndef SIMDJSON_ARM64_STAGE1_FIND_MARKS_H
#define SIMDJSON_ARM64_STAGE1_FIND_MARKS_H

#include "simdjson/portability.h"

#ifdef IS_ARM64

#include "arm64/bitmask.h"
#include "arm64/simd.h"
#include "simdjson/stage1_find_marks.h"

namespace simdjson::arm64 {

using namespace simd;

really_inline void find_whitespace_and_operators(
  const simd::simd8x64<uint8_t> in,
  uint64_t &whitespace, uint64_t &op) {

  auto v = in.map<uint8_t>([&](simd8<uint8_t> chunk) {
    auto nib_lo = chunk & 0xf;
    auto nib_hi = chunk.shr<4>();
    auto shuf_lo = nib_lo.lookup_16<uint8_t>(16, 0, 0, 0, 0, 0, 0, 0, 0, 8, 12, 1, 2, 9, 0, 0);
    auto shuf_hi = nib_hi.lookup_16<uint8_t>(8, 0, 18, 4, 0, 1, 0, 1, 0, 0, 0, 3, 2, 1, 0, 0);
    return shuf_lo & shuf_hi;
  });

  op = v.map([&](simd8<uint8_t> _v) { return _v.any_bits_set(0x7); }).to_bitmask();
  whitespace = v.map([&](simd8<uint8_t> _v) { return _v.any_bits_set(0x18); }).to_bitmask();
}

really_inline simd8<uint8_t> lookup_flipped_low_bits(
  simd8<uint8_t> flipped,
  uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4, uint8_t in5, uint8_t in6, uint8_t in7, uint8_t in8,
  uint8_t in9, uint8_t in10, uint8_t in11, uint8_t in12, uint8_t in13, uint8_t in14, uint8_t in15, uint8_t in16
) {
  return (flipped & 0x0F).lookup_16<uint8_t>(
    in1, in2, in3, in4, in5, in6, in7, in8,
    in9, in10, in11, in12, in13, in14, in15, in16
  );
}

#include "generic/utf8_lookup2_algorithm.h"
#include "generic/stage1_find_marks.h"

} // namespace simdjson::arm64

namespace simdjson {

template <>
int find_structural_bits<Architecture::ARM64>(const uint8_t *buf, size_t len, simdjson::ParsedJson &pj, bool streaming) {
  return arm64::stage1::find_structural_bits<64>(buf, len, pj, streaming);
}

} // namespace simdjson

#endif // IS_ARM64
#endif // SIMDJSON_ARM64_STAGE1_FIND_MARKS_H
