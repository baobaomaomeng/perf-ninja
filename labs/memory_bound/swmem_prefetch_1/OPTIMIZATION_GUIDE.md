# 软件预取优化指南

## 概述

本项目实现了基于 `SOLUTION` 宏的软件预取优化开关，可以根据需要启用或禁用预取优化。

## 编译选项

### 1. 原始模式（无优化）
```bash
# 创建构建目录
mkdir -p build && cd build

# 配置CMake（不启用优化）
cmake -DCMAKE_BUILD_TYPE=Release ..

# 编译
make
```

### 2. 优化模式（启用预取）
```bash
# 创建构建目录
mkdir -p build && cd build

# 配置CMake（启用优化）
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SOLUTION=ON ..

# 编译
make
```

### 3. 快速切换方式
```bash
# 在build目录中重新配置
cd build

# 启用优化
cmake -DENABLE_SOLUTION=ON ..
make

# 禁用优化
cmake -DENABLE_SOLUTION=OFF ..
make
```

## 性能对比

| 模式 | 执行时间 | 性能提升 |
|------|----------|----------|
| 原始模式 | ~58ms | 基准 |
| 优化模式 | ~18ms | **3.2x** |

## 优化原理

### 软件预取技术
- 使用 `__builtin_prefetch` 指令提前加载数据到CPU缓存
- 预取距离设置为8个元素，平衡预取效果和CPU开销
- 减少哈希表随机访问的缓存未命中

### 代码结构
```cpp
#ifdef SOLUTION
  // 优化版本：使用软件预取
  const size_t prefetch_distance = 8;
  for (size_t i = 0; i < size; ++i) {
    if (i + prefetch_distance < size) {
      hash_map->prefetch(lookups[i + prefetch_distance]);
    }
    // 处理当前值...
  }
#else
  // 原始版本：不使用预取
  for (int val : lookups) {
    // 直接处理...
  }
#endif
```

## 调优建议

### 1. 预取距离调优
可以根据CPU架构调整 `prefetch_distance` 的值：
- **较小值（4-8）**：适合缓存延迟较低的CPU
- **较大值（12-16）**：适合缓存延迟较高的CPU

### 2. 编译优化级别
建议使用 `-DCMAKE_BUILD_TYPE=Release` 以获得最佳性能。

### 3. 适用场景
- 大哈希表的随机查找
- 内存密集型应用
- 缓存未命中率高的场景

## 验证方法

```bash
# 验证功能正确性
make && ./validate

# 性能测试
./lab
```

## 注意事项

1. **内存带宽**：预取会增加内存带宽使用
2. **代码复杂度**：优化版本增加了代码复杂度
3. **CPU依赖**：预取效果依赖于具体的CPU架构
4. **数据规模**：小数据集可能看不到明显的性能提升
