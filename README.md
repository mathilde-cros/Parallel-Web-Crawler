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

With \<SET_OPTION> a number designing the data structure used to store the URLs:
0 being the SetList, 1 being the Coarse Hash Table and 2 being the Striped Hash Table.

With \<URL> the URL to crawl in the format: http://... or https://...

To use the parallel version, just replace webcrawler by webcrawler_parallel:
``` sh
make 
./webcrawler_parallel <SET_OPTION> <URL>
``` 

## Authors

Mathilde Cros (mathilde.cros@polytechnique.edu)

Jasmine Watissee (jasmine.watissee@polytechnique.edu)