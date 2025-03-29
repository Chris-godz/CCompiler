#include "sysy.map.h"
#include <assert.h>
#include <stdio.h>
// 创建哈希表
HashMap *map_create(bool (*ComPareFunc)(void *, void *),
                    size_t (*HashCalc)(void *))
{
    int size = 1024;
    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    if (map == NULL)
    {
        return NULL;
    }

    map->size_ = size;
    map->ComPareFunc_ = ComPareFunc;
    map->HashCalc_ = HashCalc;
    map->buckets_ = (HashNode *)calloc(size, sizeof(HashNode));
    if (map->buckets_ == NULL)
    {
        free(map);
        return NULL;
    }
    // calloc 已将所有内存初始化为0，所有key已为NULL
    return map;
}
// 插入键值对
void map_put(HashMap *map, void *key, void *value)
{
    if (map == NULL || key == NULL)
        return; // 不允许NULL键

    unsigned int index = map->HashCalc_(key) % map->size_;
    unsigned int start_index = index;

    // 线性探测寻找位置
    do
    {
        // 如果找到空位置或相同的键
        if (map->buckets_[index].key_ == NULL || map->ComPareFunc_(map->buckets_[index].key_, key))
        {
            map->buckets_[index].key_ = key;
            map->buckets_[index].value_ = value;
            return;
        }

        // 移动到下一个位置
        index = (index + 1) % map->size_;
    } while (index != start_index);
    // 如果没有找到空位置，可能需要扩容
    fprintf(stderr, "map full\n");
    assert(false);
}

// 获取键对应的值
void *map_get(HashMap *map, void *key)
{
    if (map == NULL || key == NULL)
        return NULL;

    unsigned int index = map->HashCalc_(key)% map->size_;
    unsigned int start_index = index;

    do
    {
        // 如果遇到空位置，表示键不存在
        if (map->buckets_[index].key_ == NULL)
        {
            return NULL;
        }

        if (map->ComPareFunc_(map->buckets_[index].key_, key))
        {
            return map->buckets_[index].value_;
        }
        index = (index + 1) % map->size_;
    } while (index != start_index);
    assert(false);
    return NULL;
}

// 检查键是否存在
bool map_contains(HashMap *map, void *key)
{
    return map_get(map, key) != NULL;
}
bool map_registered(HashMap *map, void *key)
{
    assert(map != NULL);
    assert(key != NULL);
    unsigned int index = map->HashCalc_(key)% map->size_;
    unsigned int start_index = index;

    do
    {
        // 如果遇到空位置，表示键不存在
        if (map->buckets_[index].key_ == NULL)
        {
            return false;
        }
        if (map->ComPareFunc_(map->buckets_[index].key_, key))
        {
            return true;
        }
        index = (index + 1) % map->size_;
    } while (index != start_index);
    assert(false);
    return false;
}