#include "solution.hpp"
#include <algorithm>

#define SOLUTION

#ifdef SOLUTION
bool solution(MatrixOfDoubles& in, MatrixOfDoubles& out) {
  static constexpr int TILE_SIZE = 32;
  int size = in.size();
  for (int ii = 0; ii < size; ii+= TILE_SIZE) {
    for (int jj = 0; jj < size; jj+= TILE_SIZE) {
      for (int i = ii; i < std::min(ii + TILE_SIZE, size); i++) {
        for (int j = jj; j < std::min(jj + TILE_SIZE, size); j++) {
          out[i][j] = in[j][i];
        }
      }
    }
  }

  return out[0][size - 1];
}
#else
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}
#endif