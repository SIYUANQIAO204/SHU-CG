#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>

namespace Game {

// 泛型对象池模板类
template<typename T>
class ObjectPool {
private:
    struct PooledObject {
        std::unique_ptr<T> object;
        bool active = false;
    };
    
    std::vector<PooledObject> pool_;
    size_t currentSize_ = 0;
    size_t nextIndex_ = 0; // 循环分配的索引
    
public:
    // 预分配指定数量的对象
    explicit ObjectPool(size_t initialSize = 100) {
        Resize(initialSize);
    }
    
    // 调整池大小
    void Resize(size_t newSize) {
        pool_.reserve(newSize);
        for (size_t i = currentSize_; i < newSize; ++i) {
            pool_.push_back({std::make_unique<T>(), false});
        }
        currentSize_ = newSize;
    }
    
    // 从池中获取一个可用对象
    template<typename... Args>
    T* Acquire(Args&&... args) {
        // 查找非激活对象
        for (size_t i = 0; i < currentSize_; ++i) {
            size_t index = (nextIndex_ + i) % currentSize_;
            if (!pool_[index].active) {
                pool_[index].active = true;
                pool_[index].object = std::make_unique<T>(std::forward<Args>(args)...);
                nextIndex_ = (index + 1) % currentSize_; // 下次从下一个开始查找
                return pool_[index].object.get();
            }
        }
        
        // 如果没有可用对象，扩展池
        Resize(currentSize_ * 2);
        return Acquire(std::forward<Args>(args)...);
    }




    // 释放对象回池（标记为非激活）
    void Release(T* object) {
        for (auto& pooled : pool_) {
            if (pooled.object.get() == object) {
                pooled.active = false;
                pooled.object->SetActive(false); // 同时设置游戏对象状态
                return;
            }
        }
    }
    
    // 遍历所有激活对象
    template<typename Func>
    void ForEachActive(Func&& func) {
        for (auto& pooled : pool_) {
            if (pooled.active) {
                func(*pooled.object);
            }
        }
    }
    
    // 获取激活对象数量
    size_t GetActiveCount() const {
        size_t count = 0;
        for (const auto& pooled : pool_) {
            if (pooled.active) ++count;
        }
        return count;
    }
    
    // 清理所有非激活对象（调用SetActive(false)的）
    void CleanupInactive() {
        for (auto& pooled : pool_) {
            if (pooled.active && !pooled.object->IsActive()) {
                pooled.active = false;
            }
        }
    }
};

} // namespace Game

#endif // OBJECT_POOL_H