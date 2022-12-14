//
// Created by fang on 2022/9/17.
//

#include <stdexcept>
#include <pystring.h>

#include "utils/singleton.hpp"

template <typename K, typename T>
struct Node {
    K key;
    T value;
    bool dirty = false;  // 标记为dirty的缓存项会在下次取缓存时刷新
    size_t count = 1;  // 引用计数，默认为1，每次缓存命中加1
    Node(K k, T v) : key(k), value(v) {}
};

/*
 * 支持引用计数的LRU缓存
 * 1. 若引用计数不为0则缓存永不失效，引用计数为0的缓存项按照LRU规则缓存
 * 2. 缓存中所有的Key和value都是唯一的，可以通过 cacheMap 与 valueMap 来互相查找
 * 3. key不能以 "_$dirty$" 结尾， 不包含左右引号
 * 4. 本地图片默认无法移除缓存
 */
template <typename K, typename T>
class LRUCache {
public:
    typedef typename std::list<Node<K, T>>::iterator CacheIter;
#define DIRTY "_$dirty$"
    inline static size_t DEFAULT_CAPACITY      = 400;
    inline static bool ALWAYS_CACHE_LOCAL_FILE = true;

    LRUCache(size_t c, T defaultValue)
        : capacity(c), defaultValue(defaultValue) {
        if (c < 1) throw std::logic_error("Cache capacity cannot less than 1.");
    }

    T get(K key) {
        if (cacheMap.find(key) == cacheMap.end() || cacheMap[key]->dirty) {
            // 缓存未命中
            return defaultValue;
        }

        // 缓存命中
        cacheList.splice(cacheList.begin(), cacheList, cacheMap[key]);
        cacheMap[key]                  = cacheList.begin();
        valueMap[cacheMap[key]->value] = cacheList.begin();
        // 开启缓存本地图片后，非本地图片时才计数，避免本地图片命中太多次而不删除导致计数值溢出
        if (!pystring::startswith(key, BRLS_RESOURCES) ||
            !ALWAYS_CACHE_LOCAL_FILE)
            cacheMap[key]->count++;
        return cacheMap[key]->value;
    }

    void set(K key, T value) {
        if (cacheMap.find(key) != cacheMap.end() && !cacheMap[key]->dirty) {
            // 缓存命中，修改已有的 Key，但不标记为dirty，避免缓存容量不够被删除，等待主动删除
            // 多次设置同一个key，可能会导致出现重复的 `dirty key`，不过不影响，因为这样的key已经没用了
            K newKey = key + DIRTY;
            cacheMap[key]->key += DIRTY;
            cacheMap[newKey] = cacheMap[key];
        }

        //检查容量限制
        if (cacheList.size() >= capacity) {
            deleteCache(cacheList.size() - capacity);
        }
        //添加新缓存
        cacheList.push_front(Node<K, T>(key, value));
        // 更新两个map的值
        cacheMap[key]                  = cacheList.begin();
        valueMap[cacheMap[key]->value] = cacheList.begin();
    }

    /*
     * 不是真的移除一个缓存，只是将对应缓存的计数器减1
     */
    void remove(T value) {
        if (valueMap.find(value) == valueMap.end() || valueMap[value]->dirty) {
            return;
        }
        valueMap[value]->count--;
    }

    void setCapacity(int c) {
        if (c < 1) throw std::logic_error("Cache capacity cannot less than 1.");
        // 这里避免修改麻烦直接给容量加了一个基础值，
        // 基础值的设定标准是全部页面所有图片数量的上限。
        c += DEFAULT_CAPACITY;
        this->capacity = c;
        if (cacheList.size() > capacity) {
            deleteCache(cacheList.size() - capacity);
        }
    }

    void markDirty() {
        for (auto& i : cacheList) {
            i.dirty = true;
            i.key += DIRTY;
        }
    }

    std::list<Node<K, T>>& getCacheList() { return cacheList; }

    void debug() {
        printf("===== cache size: %zu =====\n", cacheList.size());
        for (auto& i : cacheList) {
            printf("count: %zu, dirty: %d, value: %zu, key: %s\n", i.count,
                   i.dirty, i.value, i.key.c_str());
        }
    }

private:
    size_t capacity = 1;
    T defaultValue;
    std::list<Node<K, T>> cacheList;
    std::unordered_map<K, CacheIter> cacheMap;
    std::unordered_map<T, CacheIter> valueMap;

    /*
     * 从后向前删除N个缓存
     * 1. 只删除引用计数为0的缓存
     * 2. 删除成功返回0，否则返回数字代表未删除的数量
     */
    size_t deleteCache(size_t num) {
        //todo: 当前的数据结构 会导致在缓存满了之后，每次新增缓存时间复杂度从O(1)上升到O(N)
        if (num <= 0) return 0;
        auto vg = brls::Application::getNVGContext();
        for (auto i = cacheList.rbegin(); i != cacheList.rend(); i++) {
            if (i->count <= 0 || i->dirty) {
                num--;
                nvgDeleteImage(vg, i->value);
                cacheMap.erase(i->key);
                valueMap.erase(i->value);
                cacheList.erase(std::next(i).base());
                if (num == 0) break;
            }
        }
        return num;
    }
};

class TextureCache : public Singleton<TextureCache> {
public:
    TextureCache() {
        brls::Application::getWindowSizeChangedEvent()->subscribe(
            [this]() { this->cache.markDirty(); });
    }

    int getCache(const std::string& key) { return cache.get(key); }

    /*
     * 添加缓存
     */
    void addCache(const std::string& key, size_t texture) {
        if (texture <= 0) return;
        cache.set(key, texture);
    }

    /*
     * 将匹配的纹理缓存计数器减1，若计数器归零 则删除缓存
     */
    void removeCache(size_t texture) {
        if (texture <= 0) return;
        cache.remove(texture);
    }

    void clean() {
        auto vg = brls::Application::getNVGContext();
        for (auto& i : cache.getCacheList()) {
            nvgDeleteImage(vg, i.value);
        }
    }

    void debug() { cache.debug(); }

    LRUCache<std::string, size_t> cache = LRUCache<std::string, size_t>(200, 0);
};