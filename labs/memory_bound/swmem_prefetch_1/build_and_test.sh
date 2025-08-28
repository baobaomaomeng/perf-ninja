#!/bin/bash

# 软件预取优化构建和测试脚本

echo "=== 软件预取优化测试 ==="
echo

# 创建构建目录
mkdir -p build
cd build

echo "1. 测试原始模式（无优化）..."
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SOLUTION=OFF .. > /dev/null
make > /dev/null
echo "   验证功能..."
./validate
echo "   性能测试..."
ORIGINAL_TIME=$(./lab 2>&1 | grep "bench1" | awk '{print $2}' | sed 's/ms//')
echo "   执行时间: ${ORIGINAL_TIME}ms"
echo

echo "2. 测试优化模式（启用预取）..."
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_SOLUTION=ON .. > /dev/null
make > /dev/null
echo "   验证功能..."
./validate
echo "   性能测试..."
OPTIMIZED_TIME=$(./lab 2>&1 | grep "bench1" | awk '{print $2}' | sed 's/ms//')
echo "   执行时间: ${OPTIMIZED_TIME}ms"
echo

# 计算性能提升
if [ ! -z "$ORIGINAL_TIME" ] && [ ! -z "$OPTIMIZED_TIME" ]; then
    SPEEDUP=$(echo "scale=2; $ORIGINAL_TIME / $OPTIMIZED_TIME" | bc)
    echo "=== 性能对比 ==="
    echo "原始模式: ${ORIGINAL_TIME}ms"
    echo "优化模式: ${OPTIMIZED_TIME}ms"
    echo "性能提升: ${SPEEDUP}x"
    echo
    echo "✅ 测试完成！"
else
    echo "❌ 性能测试失败，请检查输出"
fi
