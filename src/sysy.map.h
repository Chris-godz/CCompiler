#ifndef _SYSY_MAP_H_
#define _SYSY_MAP_H_
#include <stdbool.h>
#include <stdlib.h>
typedef struct HashNode
{
    void *key_;
    void *value_;
} HashNode;

typedef struct
{
    int size_;
    HashNode *buckets_;
    bool (*ComPareFunc_)(void *, void *); // 比较函数
    size_t (*HashCalc_)(void *);          // hash计算函数
} HashMap;

// 创建哈希表
HashMap *map_create();

// 插入键值对
void map_put(HashMap *map, void *key, void *value);

// 获取键对应的值
void *map_get(HashMap *map, void *key);

// 检查键是否存在
bool map_contains(HashMap *map, void *key);

// 检测键是否注册过
bool map_registered(HashMap *map, void *key);
#endif // _SYSY_MAP_H_