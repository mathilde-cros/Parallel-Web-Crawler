#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <curl/curl.h>
#include <chrono>
#include "hashtable.cpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

// Callback function to receive HTTP response
size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

// Function to check if URL exists
bool urlExists(const std::string& url) {
    CURL* curl;
    CURLcode res;
    long response_code = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // We don't need the body
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

        res = curl_easy_perform(curl);
        
        if(res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code == 200) {
                curl_easy_cleanup(curl);
                curl_global_cleanup();
                return true;
            }
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return false;
}

// Function to fetch HTML content from a URL
std::string fetchHTML(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string data;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            // std::cerr << "Failed to fetch URL: " << curl_easy_strerror(res) << std::endl;
            return ""; // Return empty string to indicate failure
        }
        curl_easy_cleanup(curl);
        return data;
    } else {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return "";
    }
}

// Thread pool to manage threads while crawling the website
class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

    void process_task_from_queue();

public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();
    void add_task_to_queue(const std::function<void()>& task);
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back(&ThreadPool::process_task_from_queue, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) {
        worker.join();
    }
}

void ThreadPool::add_task_to_queue(const std::function<void()>& task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(task);
    }
    condition.notify_one();
}

void ThreadPool::process_task_from_queue() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]() { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

// Parallel function to extract URLs from crawling the HTML content using regex
template <class T>
void crawl_parallel(std::string url, const std::string& base_url, T &urlSet, ThreadPool& threadPool, std::mutex& setMutex) {
    std::string html = fetchHTML(url);
    if (html.empty()) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(setMutex);
        urlSet.addURL(url);
    }
    // Regular expression to find URLs in HTML
    std::regex urlRegex(R"(href=["']([^"']+)["'])");
    auto words_begin = std::sregex_iterator(html.begin(), html.end(), urlRegex);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string url2 = match[1].str();

        // To ensure that the URL starts with the base URL
        if (url2.find(base_url) != 0) {
            if (url2.find('#') != std::string::npos || url2.find("//") != std::string::npos){
                continue;
            } else if (url2.find('/') == 0) {
                url2 = base_url + url2;
            } else if (url2.find('/') == std::string::npos) {
                url2 = base_url + '/' + url2;
            } else {
                continue;
            }
        }
        {
            std::lock_guard<std::mutex> lock(setMutex);
            if (urlSet.containsURL(url2)){
                continue;
            }
        }
        std::string newBaseUrl = (url2.back() == '/') ? url2.substr(0, url2.size() - 1) : url2;
        threadPool.add_task_to_queue([url2, base_url, &urlSet, &threadPool, &setMutex]() {
            crawl_parallel(url2, base_url, urlSet, threadPool, setMutex);
        });
    }
}

int main(int argc, char* argv[]) {

    if (argc != 3){
        std::cerr << "Invalid command, please give your command as:" << std::endl;
        std::cerr << "./webcrawler <opt_set> <url>" << std::endl;
        std::cerr << "opt_set being 0 (SetList), 1 (CoarsedHashTable), or 2 (StripedHashTable)" << std::endl;
        std::cerr << "\t\t defining the set you want to use to store the urls" << std::endl;
        std::cerr << "url being the url you want to crawl" << std::endl;
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();

    int option_urlset = std::stoi(argv[1]);
    std::string url = argv[2];

    // Keep only url starting like first one in order to avoid crawling the whole internet (ex. redirects to instagram.com ...)!
    std::regex baseUrlRegex(R"((https?://[^/]+))");
    std::smatch baseUrlMatch;
    std::string base_url;
    if (std::regex_search(url, baseUrlMatch, baseUrlRegex)) {
        base_url = baseUrlMatch[1].str();
    } else {
        std::cerr << "Failed to extract base URL." << std::endl;
        return 1;
    }
    std::cout << "Base URL: " << base_url << std::endl;

    // Validate the URL format
    std::regex urlFormat(R"(https?://\S+)");
    if (!std::regex_match(url, urlFormat)) {
        std::cerr << "Invalid URL format. Please enter a valid URL starting with http:// or https://" << std::endl;
        return 1;
    }

    std::mutex setMutex;
    ThreadPool threadPool(256);

    if (option_urlset == 0){
        SetList urlSet;
        threadPool.add_task_to_queue([url, base_url, &urlSet, &threadPool, &setMutex]() {
            crawl_parallel(url, base_url, urlSet, threadPool, setMutex);
        });
        std::this_thread::sleep_for(std::chrono::seconds(30));
        std::cout << "URLs found" << std::endl;
        urlSet.display();
        std::cout << "Number of URLs: " << urlSet.getSize() << std::endl;
    } else if (option_urlset == 1){
        CoarseHashTable<std::string> urlSet(32);
        threadPool.add_task_to_queue([url, base_url, &urlSet, &threadPool, &setMutex]() {
            crawl_parallel(url, base_url, urlSet, threadPool, setMutex);
        });
        std::this_thread::sleep_for(std::chrono::seconds(30));
        std::cout << "URLs found" << std::endl;
        urlSet.display();
        std::cout << "Number of URLs: " << urlSet.getSize() << std::endl;
    } else if (option_urlset == 2){
        StripedHashTable<std::string> urlSet(32);
        threadPool.add_task_to_queue([url, base_url, &urlSet, &threadPool, &setMutex]() {
            crawl_parallel(url, base_url, urlSet, threadPool, setMutex);
        });
        std::this_thread::sleep_for(std::chrono::seconds(30));
        std::cout << "URLs found" << std::endl;
        urlSet.display();
        std::cout << "Number of URLs: " << urlSet.getSize() << std::endl;
    } else {
        std::cerr << "Wrong url set option, please use 0 (SetList), 1 (CoarseHashTable) or 2 (StripedHashTable)" << std::endl;
        return 1;
    }

    auto stop = std::chrono::high_resolution_clock::now(); 
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "Execution time (seconds): ";
	std::cout << duration.count()/1000/1000 << std::endl;

    return 0;
}
