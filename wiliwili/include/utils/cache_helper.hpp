//
// Created by fang on 2022/9/17.
//

#include "utils/singleton.hpp"

template <typename K, typename T>
struct Node {
    K key;
    T value;
    Node(K k, T v) : key(k), value(v) {}
};
template <typename K, typename T>
class LRUCache {
public:
    LRUCache(int c, T defaultValue) : capacity(c), defaultValue(defaultValue) {}

    T get(K key) {
        if (cacheMap.find(key) == cacheMap.end()) {
            return defaultValue;
        }
        cacheList.splice(cacheList.begin(), cacheList, cacheMap[key]);
        cacheMap[key] = cacheList.begin();
        return cacheMap[key]->value;
    }
    void set(K key, T value) {
        if (cacheMap.find(key) == cacheMap.end()) {
            if (cacheList.size() == capacity) {
                cacheMap.erase(cacheList.back().key);
                cacheList.pop_back();
            }
            cacheList.push_front(Node<K, T>(key, value));
            cacheMap[key] = cacheList.begin();
        } else {
            cacheMap[key]->value = value;
            cacheList.splice(cacheList.begin(), cacheList, cacheMap[key]);
            cacheMap[key] = cacheList.begin();
        }
    }

    std::list<Node<K, T>>& getCacheList() { return cacheList; }

private:
    int capacity;
    T defaultValue;
    std::list<Node<K, T>> cacheList;
    std::unordered_map<K, typename std::list<Node<K, T>>::iterator> cacheMap;
};

class TextureCache : public Singleton<TextureCache> {
public:
    int getCache(const std::string& url) { return cache.get(url); }

    void addCache(const std::string& url, int texture) {
        if (texture <= 0) return;

        cache.set(url, texture);
    }

    void clean() {
        auto vg = brls::Application::getNVGContext();
        for (auto& i : cache.getCacheList()) {
            nvgDeleteImage(vg, i.value);
        }
    }

    LRUCache<std::string, int> cache = LRUCache<std::string, int>(200, 0);
};