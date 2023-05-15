# object_pool

一个简单高效的对象池实现。

- 支持初始化单个大块(chunk)内存支持最大对象数量
- 分配对象时间复杂度:
  - 当chunk未用完时： O(1)
  - 当chunk用完，会创建新chunk并链表链接起来，O(max_objs + 1), max_objs时单个chunk允许分配最大对象数
- 回收对象复杂度: O(1)
- 回收池内存: O(chunk_num)
- 支持线程安全用法
- 分配对象地址按`sizeof(std::max_align_t)`对齐

## 简单用法

```cpp
#include "object_pool.hpp"
yasio::object_pool<int, std::mutex> thread_safe_pool(128); // 创建线程安全对象池
yasio::object_pool<int> pool(128); // 创建非线程安全对象池

auto value1 = pool.create(2023);
auto value2 = pool.create(2024);
auto value3 = pool.create(2025);
pool.destroy(value3);
pool.destroy(value1);
pool.destroy(value2);

```
