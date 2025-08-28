#include "solution.hpp"

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;
  const size_t size = lookups.size();
  
#ifdef SOLUTION
  // 优化版本：使用软件预取
  // 预取距离 - 根据CPU缓存延迟调整
  const size_t prefetch_distance = 32;
  
  for (size_t i = 0; i < size; ++i) {
    // 预取未来的查找值
    if (i + prefetch_distance < size) {
      hash_map->prefetch(lookups[i + prefetch_distance]);
    }
    
    // 处理当前值
    int val = lookups[i];
    if (hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }
#else
  // 原始版本：不使用预取
  for (int val : lookups) {
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }
#endif

  return result;
}
