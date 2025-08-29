
#include "solution.h"
#include <algorithm>
#include <immintrin.h>

void imageSmoothing(const InputVector &input, uint8_t radius,
                    OutputVector &output) {
  const int size = static_cast<int>(input.size());
  if (size == 0) {
    output.clear();
    return;
  }

  // Ensure output has the right size
  if (static_cast<int>(output.size()) != size) {
    output.resize(size);
  }

  // Build 32-bit prefix sums to avoid overflow, prefix[i] = sum(input[0..i])
  std::vector<uint32_t> prefix;
  prefix.resize(size);

#if defined(__AVX2__)
  {
    int idx = 0;
    uint32_t carry = 0;
    // Process 16 bytes per iteration
    for (; idx + 16 <= size; idx += 16) {
      __m128i bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(&input[idx]));
      __m256i s16 = _mm256_cvtepu8_epi16(bytes); // 16 lanes of uint16

      // Inclusive scan within each 128-bit lane (8 elements per lane)
      __m256i scan = s16;
      __m256i t;
      t = _mm256_slli_si256(scan, 2);
      scan = _mm256_add_epi16(scan, t);
      t = _mm256_slli_si256(scan, 4);
      scan = _mm256_add_epi16(scan, t);
      t = _mm256_slli_si256(scan, 8);
      scan = _mm256_add_epi16(scan, t);

      // Add lower-half total to upper-half lanes
      __m128i lo128 = _mm256_extracti128_si256(scan, 0);
      __m128i hi128 = _mm256_extracti128_si256(scan, 1);
      uint16_t half_sum0 = static_cast<uint16_t>(_mm_extract_epi16(lo128, 7));
      __m128i hi128_off = _mm_add_epi16(hi128, _mm_set1_epi16(static_cast<int>(half_sum0)));

      // Convert to 32-bit and add carry
      __m256i lo32 = _mm256_cvtepu16_epi32(lo128);
      __m256i hi32 = _mm256_cvtepu16_epi32(hi128_off);
      __m256i carry32 = _mm256_set1_epi32(static_cast<int>(carry));
      lo32 = _mm256_add_epi32(lo32, carry32);
      hi32 = _mm256_add_epi32(hi32, carry32);

      _mm256_storeu_si256(reinterpret_cast<__m256i *>(&prefix[idx]), lo32);
      _mm256_storeu_si256(reinterpret_cast<__m256i *>(&prefix[idx + 8]), hi32);

      // Update carry by total of this block (last element of hi128_off)
      uint16_t block_sum = static_cast<uint16_t>(_mm_extract_epi16(hi128_off, 7));
      carry += static_cast<uint32_t>(block_sum);
    }
    // Scalar tail
    for (; idx < size; ++idx) {
      carry += static_cast<uint32_t>(input[idx]);
      prefix[idx] = carry;
    }
  }
#else
  {
    uint32_t running = 0;
    for (int i = 0; i < size; ++i) {
      running += static_cast<uint32_t>(input[i]);
      prefix[i] = running;
    }
  }
#endif

  const int r = static_cast<int>(radius);

  const int last = size - 1;

  // Left border: positions 0..min(r,last)
  int pos = 0;
  const int left_end = std::min(r, last);
  for (; pos <= left_end; ++pos) {
    const int right = std::min(last, pos + r);
    output[pos] = static_cast<uint16_t>(prefix[right]);
  }

  // Main region: pos in [r+1, size-r)
  const int main_start = r + 1;
  const int main_end = size - r; // exclusive
  if (main_start < main_end) {
    pos = main_start;

#if defined(__AVX2__)
  // Process 16 outputs per iteration using 2 x 256-bit vectors of 8x int32
    for (; pos + 16 <= main_end; pos += 16) {
      const uint32_t *base_plus  = &prefix[pos + r];
      const uint32_t *base_minus = &prefix[pos - r - 1];

      __m256i a0 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(base_plus));
      __m256i b0 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(base_minus));
      __m256i d0 = _mm256_sub_epi32(a0, b0);

      __m256i a1 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(base_plus + 8));
      __m256i b1 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(base_minus + 8));
      __m256i d1 = _mm256_sub_epi32(a1, b1);

      __m128i d0_lo = _mm256_extracti128_si256(d0, 0);
      __m128i d0_hi = _mm256_extracti128_si256(d0, 1);
      __m128i d1_lo = _mm256_extracti128_si256(d1, 0);
      __m128i d1_hi = _mm256_extracti128_si256(d1, 1);

      __m128i pack0 = _mm_packus_epi32(d0_lo, d0_hi); // pos .. pos+7
      __m128i pack1 = _mm_packus_epi32(d1_lo, d1_hi); // pos+8 .. pos+15

      _mm_storeu_si128(reinterpret_cast<__m128i *>(&output[pos]), pack0);
      _mm_storeu_si128(reinterpret_cast<__m128i *>(&output[pos + 8]), pack1);
    }
#endif

    // Scalar main tail for remaining positions in main region
    for (; pos < main_end; ++pos) {
      uint32_t sum = prefix[pos + r] - prefix[pos - r - 1];
      output[pos] = static_cast<uint16_t>(sum);
    }
  }

  // Right border: positions where right edge is clipped at size-1
  int right_start = std::max(main_end, 0);
  for (pos = std::max(right_start, 0); pos < size; ++pos) {
    const int left = std::max(0, pos - r);
    const uint32_t left_prefix = (left > 0) ? prefix[left - 1] : 0u;
    const uint32_t sum = prefix[last] - left_prefix;
    output[pos] = static_cast<uint16_t>(sum);
  }
}

// Baseline scalar implementation copied from original version for comparison
void imageSmoothing_scalar(const InputVector &input, uint8_t radius,
                           OutputVector &output) {
  int pos = 0;
  int currentSum = 0;
  int size = static_cast<int>(input.size());

  for (int i = 0; i < std::min<int>(size, radius); ++i) {
    currentSum += input[i];
  }

  int limit = std::min(radius + 1, size - radius);
  for (pos = 0; pos < limit; ++pos) {
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

  limit = size - radius;
  for (; pos < limit; ++pos) {
    currentSum -= input[pos - radius - 1];
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

  limit = std::min(radius + 1, size);
  for (; pos < limit; pos++) {
    output[pos] = currentSum;
  }

  for (; pos < size; ++pos) {
    currentSum -= input[pos - radius - 1];
    output[pos] = currentSum;
  }
}
