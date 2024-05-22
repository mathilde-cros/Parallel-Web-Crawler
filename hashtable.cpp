#include <iostream>
#include <string>
#include <vector>
#include <mutex>

// Non parallel Set List
class SetList {
private:
    std::vector<std::string> urls;
public:
    // Add a URL to the list
    bool addURL(const std::string& url) {
        if (containsURL(url)) return false;
        urls.push_back(url);
        return true;
    }

    // Display all URLs in the list
    void displayList() {
        for(const auto& url : urls) {
            std::cout << url << std::endl;
        }
    }

    // Check if a URL is present in the list
    bool containsURL(const std::string& url) {
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
class HashTable {
private:
    std::vector<std::vector<T>> table;
    int setSize;
    std::mutex lock;

    // Checks if the hash table is too big and has to be resized
    bool policy() {
        return setSize / table.size() > 4;
    }

    // resize the hash table
    void resize(){
        std::lock_guard<std::mutex> guard(lock);
        int oldCapacity = table.size();
        if (oldCapacity != table.size()){
            return;
        }
        int newCapacity = 2 * oldCapacity;
        std::vector<std::vector<T>> oldTable = table;
        table = std::vector<std::vector<T>>(newCapacity);
        for (int i = 0; i < newCapacity; i++){
            table[i] = std::vector<T>();
        }
        for (auto& bucket : oldTable){
            for (auto& x : bucket){
                int newBucket = std::hash<T>{}(x) % newCapacity;
                table[newBucket].push_back(x);
            }
        }
    }

    void acquire(T x){
        lock.lock();
    }

    void release(T x){
        lock.unlock();
    }

public: 
    HashTable(int capacity) : setSize(0), table(capacity) {
        for (int i = 0; i < capacity; i++){
            table[i] = std::vector<T>();
        }
    }

    // Add a URL to the hash table
    bool addURL(const T& url) {
        bool result = false;
        acquire(url);
        int table_index = std::abs((int)std::hash<T>{}(url) % table.size());
        std::vector& bucket = table[table_index];
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
    bool containsURL(const T& url){
        std::lock_guard<std::mutex> guard(lock);
        int table_index = std::hash<T>{}(url) % table.size();
        std::vector& bucket = table[table_index];
        return std::find(bucket.begin(), bucket.end(), url) != bucket.end();
    }

    // Display all URLs in the hash table
    void displayList() {
        for (auto& bucket : table){
            for (auto& url : bucket){
                std::cout << url << std::endl;
            }
        }
    }

    // Clear the hash table of URLs
    void clearList() {
        setSize = 0;
        for (auto& bucket : table){
            bucket.clear();
        }
    }
};

// Striped Hash Table
template <typename T>
class StripedHashTable : public HashTable {
private:
    std::vector<std::mutex> locks; 

    // resize the hash table
    void resize(){
        int oldCapacity = table.size();
        for (auto& lock : locks){
            lock.lock();
        }
        if (oldCapacity != table.size()){
            return;
        }
        int newCapacity = 2 * oldCapacity;
        std::vector<std::vector<T>> oldTable = table;
        table = std::vector<std::vector<T>>(newCapacity);
        for (int i = 0; i < newCapacity; i++){
            table[i] = std::vector<T>();
        }
        for (auto& bucket : oldTable){
            for (auto& x : bucket){
                int newBucket = std::hash<T>{}(x) % newCapacity;
                table[newBucket].push_back(x);
            }
        }
        for (auto& lock : locks){
            lock.unlock();
        }
    }

    void acquire(T x){
        locks[std::hash<T>{}(x) % table.size()].lock();
    }

    void release(T x){
        locks[std::hash<T>{}(x) % table.size()].unlock();
    }

public: 
    StripedHashTable(int capacity) : setSize(0), table(capacity) {
        for (int i = 0; i < capacity; i++){
            table[i] = std::vector<T>();
        }
    }
};
