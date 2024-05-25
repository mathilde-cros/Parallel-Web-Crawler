#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>

// Non parallel Set List
class SetList {
private:
    std::vector<std::string> urls;
    int size;

public:
    SetList() : size(0) {}

    // Add a URL to the list
    bool addURL(const std::string& url) {
        if (containsURL(url)) return false;
        urls.push_back(url);
        size++;
        return true;
    }

    int getSize(){
        return size;
    }

    // Display all URLs in the list
    void displayList() const {
        for (const auto& url : urls) {
            std::cout << url << std::endl;
        }
    }

    // Check if a URL is present in the list
    bool containsURL(const std::string& url) const {
        return std::find(urls.begin(), urls.end(), url) != urls.end();
    }

    // Clear the list of URLs
    void clearList() {
        urls.clear();
    }
};

// Simple Parallel Hash Table
// https://dl.acm.org/doi/pdf/10.5555/2385452
template <typename T>
class BaseHashTable {
protected:
    std::vector<std::vector<T>> table;
    int setSize;

public:
    BaseHashTable(int capacity) : table(capacity), setSize(0) {
        for (int i = 0; i < capacity; i++){
            table[i] = std::vector<T>();
        }
    }

    int getSize(){
        return setSize;
    }

    // Add a URL to the hash table
    bool addURL(const T& url) {
        bool result = false;
        acquire(url);
        int table_index = (int) std::hash<T>{}(url) % table.size();
        std::vector<T>& bucket = table[table_index];
        if (std::find(bucket.begin(), bucket.end(), url) == bucket.end()) {
            bucket.push_back(url);
            result = true;
            setSize++;
        }
        release(url);

        if (policy()) resize();
        return result;
    }

    // Check if a URL is present in the hash table
    bool containsURL(const T& url) {
        acquire(url);
        int table_index = std::hash<T>{}(url) % table.size();
        std::vector<T>& bucket = table[table_index];
        release(url);
        return std::find(bucket.begin(), bucket.end(), url) != bucket.end();
    }

    // Display all URLs in the hash table
    void displayList() const {
        for (const auto& bucket : table) {
            for (const auto& url : bucket) {
                std::cout << url << std::endl;
            }
        }
    }

    // Clear the hash table of URLs
    void clearList() {
        setSize = 0;
        for (auto& bucket : table) {
            bucket.clear();
        }
    }

    virtual bool policy() = 0;
    virtual void resize() = 0;
    virtual void acquire(const T& x) = 0;
    virtual void release(const T& x) = 0;

};

// Coarse Grained Hash Set
template <typename T>
class CoarseHashTable : public BaseHashTable<T> {
private:
    std::mutex lock;

public:
    CoarseHashTable(int capacity) : BaseHashTable<T>(capacity) {}

    // Checks if the hash table is too big and has to be resized
    bool policy() {
        std::lock_guard<std::mutex> guard(lock);
        return this->setSize / this->table.size() > 4;
    }

    // Resize the hash table
    void resize(){
        std::lock_guard<std::mutex> guard(lock);
        int oldCapacity = this->table.size();
        if (oldCapacity != this->table.size()) {
            return;
        }
        int newCapacity = 2 * oldCapacity;
        std::vector<std::vector<T>> oldTable = this->table;
        this->table = std::vector<std::vector<T>>(newCapacity);
        for (int i = 0; i < newCapacity; i++){
            this->table[i] = std::vector<T>();
        }
        for (auto& bucket : oldTable){
            for (auto& x : bucket){
                int newBucket = std::hash<T>{}(x) % newCapacity;
                this->table[newBucket].push_back(x);
            }
        }
    }

    void acquire(const T& x) {
        lock.lock();
    }

    void release(const T& x) {
        lock.unlock();
    }
};

// Striped Hash Table
template <typename T>
class StripedHashTable : public BaseHashTable<T> {
private:
    std::vector<std::mutex> locks;

public:
    StripedHashTable(int capacity) : BaseHashTable<T>(capacity), locks(capacity) {}

    // Checks if the hash table is too big and has to be resized
    bool policy() {
        for (auto& lock : locks){
            lock.lock();
        }
        bool answer = this->setSize / this->table.size() > 4;
        for (auto& lock : locks){
            lock.unlock();
        }
        return answer;
    }

    // Resize the hash table
    void resize() {
        for (auto& lock : locks){
            lock.lock();
        }
        int oldCapacity = this->table.size();
        int newCapacity = 2 * oldCapacity;
        std::vector<std::vector<T>> oldTable = this->table;
        this->table = std::vector<std::vector<T>>(newCapacity);
        for (int i = 0; i < newCapacity; i++) {
            this->table[i] = std::vector<T>();
        }
        for (auto& bucket : oldTable){
            for (auto& x : bucket){
                int newBucket = std::hash<T>{}(x) % newCapacity;
                this->table[newBucket].push_back(x);
            }
        }
        for (auto& lock : locks){
            lock.unlock();
        }
    }

    void acquire(const T& x) {
        locks[std::hash<T>{}(x) % locks.size()].lock();
    }

    void release(const T& x) {
        locks[std::hash<T>{}(x) % locks.size()].unlock();
    }
};