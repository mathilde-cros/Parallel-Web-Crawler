#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <curl/curl.h>

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
    void displayURLs() {
        for(std::vector<std::string>::const_iterator it = urls.begin(); it != urls.end(); ++it) {
            const std::string& url = *it;
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
        std::vector<std::string>::const_iterator it = std::find(urls.begin(), urls.end(), url);
        if(it != urls.end()) {
            urls.erase(it);
            std::cout << "URL removed successfully." << std::endl;
        } else {
            std::cout << "URL not found." << std::endl;
        }
    }
};

// Auxiliary function of downloadWebpages
std::string downloadWebpageAUX(const std::string& url) {
    CURL *curl;
    CURLcode libcurl_res;
    std::string webpage_res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* ptr, size_t size, size_t nmemb, std::string* data) -> size_t {
            data->append(ptr, size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &webpage_res);

        libcurl_res = curl_easy_perform(curl);
        if (libcurl_res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(libcurl_res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    return webpage_res;
}

// Downloads each URL page and creates an associated SetList
void downloadWebpages(const std::vector<std::string>& urls) {
    std::vector<std::thread> threads;
    std::vector<SetList> SetLists(urls.size());

    for (size_t i = 0; i < urls.size(); ++i) {
    threads.emplace_back([&, i]() {
        std::string webpage = downloadWebpageAUX(urls[i]);
        SetLists[i].addURL(urls[i]);
    });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}


// Crawls webpage and extracts links
void crawl(SetList& setList, const std::string& url) {
    // Do something to crawl the webpage
    // Add the links to the setList
};

// // Main function to start crawling
// int main(int argc, char *argv[]) {
//     // Check if URL/s is/are provided and load them

//     // Initialize a vector of threads to run in parallel
//     std::vector<std::thread> threads;

//     // Start crawling threads for each URL
//     for (int i = 1; i < argc; ++i) {
//     }

//     // Wait for all crawling threads to finish
//     for (auto& thread : threads) {
//         thread.join();
//     }

//     return 0;
// }

// Temp main function to test if current implementations work
int main() {
    std::vector<std::string> urls;
    urls.push_back("https://www.tripadvisor.com/Tourism-g147293-Punta_Cana_La_Altagracia_Province_Dominican_Republic-Vacations.html");
    urls.push_back("https://www.pagesjaunes.fr/pros/08380256");
    urls.push_back("https://www.yelp.fr/search?find_desc=Restaurants&find_loc=Paris");

    downloadWebpages(urls);

    return 0;
}