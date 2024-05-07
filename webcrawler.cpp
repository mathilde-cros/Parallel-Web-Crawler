#include <iostream>
#include <string>
#include <vector>
#include <thread>

// SetList data structure to store URLs
class SetList {
private:
    std::vector<std::string> urls;
public:
    // Add a URL to the list
    void addURL(const std::string& url) {
        urls.push_back(url);
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
    // Remove a specific URL from the list
    void removeURL(const std::string& url) {
        auto it = std::find(urls.begin(), urls.end(), url);
        if(it != urls.end()) {
            urls.erase(it);
            std::cout << "URL removed successfully." << std::endl;
        } else {
            std::cout << "URL not found." << std::endl;
        }
    }
};

// Downloads webpage using libcurl
std::string downloadWebpage(const std::string& url) {
    std::string downloaded_webpage;
    return downloaded_webpage;
};

// Crawls webpage and extracts links
void crawl(SetList& setList, const std::string& url) {
    // Do something to crawl the webpage
    // Add the links to the setList
};

// Main function to start crawling
int main(int argc, char *argv[]) {
    // Check if URL/s is/are provided and load them

    // Initialize a vector of threads to run in parallel
    std::vector<std::thread> threads;

    // Start crawling threads for each URL
    for (int i = 1; i < argc; ++i) {
    }

    // Wait for all crawling threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}