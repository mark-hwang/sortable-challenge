#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
typedef struct _trie_node_t trie_node_t;
typedef struct _product_t product_t;
typedef struct _listing_t listing_t;

struct _product_t {
    product_t* next;
    product_t* sibling;
    char name[512];
    char key[512];
    char condition[256];
    char option[256];
    listing_t* first;
    listing_t* last;
};

struct _trie_node_t {
    trie_node_t* child[256];
    product_t* data;
};

struct _listing_t {
    listing_t* next;
    char key[512];
    char condition[256];
    char raw[1024];
};



