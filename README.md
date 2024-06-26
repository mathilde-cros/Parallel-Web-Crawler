# Parallel-Web-Crawler

## Description

Crawling refers to the process of going to a webpage, following all the links from it, following all the links in the found pages, etc. This process is used, for example, by search engines to index the content of the Internet. The goal of this project is to crawl a website to create its “index”, a list of pages.

## Usage

### Download

You can download our repository by doing:
``` sh
git clone https://github.com/mathilde-cros/Parallel-Web-Crawler.git
```

### Requirements

To compile and run the code, you need [libcurl](https://curl.se/libcurl/) installed

### Usage

To compile and run the code, you can do:
``` sh
make 
./webcrawler <SET_OPTION> <URL>
``` 

With \<URL> the URL to crawl in the format: http://... or https://...

And with \<SET_OPTION> being 0, 1 or 2 according to the store method you wish to use for the URLs:
- 0 : SetList
- 1 : CoarseHashTable
- 2 : StripedHashTable

To use the parallel version, just replace webcrawler by webcrawler_parallel:
``` sh
make 
./webcrawler_parallel <SET_OPTION> <URL> <NUM_THREADS>
``` 

With \<NUM_THREADS> the number of threads to use

## Authors

Mathilde Cros (mathilde.cros@polytechnique.edu)

Jasmine Watissee (jasmine.watissee@polytechnique.edu)
