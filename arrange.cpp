#include "arrange.h"
#include "gason.h"

trie_node_t* create_trie() {
    trie_node_t* node = (trie_node_t*)malloc(sizeof(trie_node_t));
    if(node) {
        memset(node,0x00,sizeof(trie_node_t));
    }
    return node;
}

product_t* create_product() {
    product_t* node = (product_t*)malloc(sizeof(product_t));
    if(node) {
        memset(node,0x00,sizeof(product_t));
    }
    return node;
}

listing_t* create_listing() {
    listing_t* node = (listing_t*)malloc(sizeof(listing_t));
    if(node) {
        memset(node,0x00,sizeof(listing_t));
    }
    return node;
}

void tolower(const char* str, char* tmp) {
    while(*str) {
        if(*str >= 'A' && *str <= 'Z') {
            *tmp = *str + 32;
        } else {
            *tmp = *str;
        }
        ++str;
        ++tmp;
    }
    *tmp = 0x00;
}

void dumpString(FILE* file,const char *s) {
    fputc('"', file);
    while (*s) {
        int c = *s++;
        switch (c) {
        case '\b': fprintf(file, "\\b"); break;
        case '\f': fprintf(file, "\\f"); break;
        case '\n': fprintf(file, "\\n"); break;
        case '\r': fprintf(file, "\\r"); break;
        case '\t': fprintf(file, "\\t"); break;
        case '\\': fprintf(file, "\\\\"); break;
        case '"': fprintf(file, "\\\""); break;
        default: fputc(c, file);
        }
    }
    fprintf(file, "%s\"", s);
}

int parse_product(char* fname, product_t* root) {
    product_t* prev = root;
    FILE* file = fopen(fname, "r");
    if(!file) {
        fprintf(stderr,"produc file (%s) : fopen error\n",fname);
        return 0;
    }
    char buf[1024];
    while(fgets(buf,1024,file)) {
        char *endptr;
        JsonValue value;
        JsonAllocator allocator;
        int status = jsonParse(buf, &endptr, &value, allocator);
        if (status == JSON_OK) {
            if(value.getTag() == JSON_OBJECT) {
                product_t* prd = create_product();
                for (auto i : value) {
                    if(!strcmp(i->key,"product_name")) {
                        sprintf(prd->name,"%s",(i->value).toString());
                    } else if(!strcmp(i->key,"model")) {
                        tolower((i->value).toString(), prd->key);
                    } else if(!strcmp(i->key,"family")) {
                        tolower((i->value).toString(), prd->option);
                    } else if(!strcmp(i->key,"manufacturer")) {
                        tolower((i->value).toString(), prd->condition);
                    }
                }
                prev->next = prd;
                prev = prd;
            }
        }
    }
    fclose(file);
    return 1;
}

int parse_listing(char* fname, listing_t* root) {
    listing_t* prev = root;
    FILE* file = fopen(fname, "r");
    if(!file) {
        fprintf(stderr,"listing file (%s) : fopen error\n",fname);
        return 0;
    }
    char buf[1024],buf2[1024];
    while(fgets(buf,1024,file)) {
        strcpy(buf2,buf);
        char *endptr;
        JsonValue value;
        JsonAllocator allocator;
        int status = jsonParse(buf, &endptr, &value, allocator);
        if (status == JSON_OK) {
            if(value.getTag() == JSON_OBJECT) {
                listing_t* lst = create_listing();
                int len = strlen(buf2);
                if(buf2[len-1] == '\n') {
                    buf2[len-1] = 0x00;
                    if(buf[len-2] == '\r') {
                        buf[len-2] = 0x00;
                    }
                }
                sprintf(lst->raw,"%s",buf2);
                for (auto i : value) {
                    if(!strcmp(i->key,"title")) {
                        tolower((i->value).toString(),lst->key);
                    } else if(!strcmp(i->key,"manufacturer")) {
                        tolower((i->value).toString(), lst->condition);
                    }
                }
                prev->next = lst;
                prev = lst;
            }
        }
    }
    fclose(file);
    return 1;
}

void insert_trie(trie_node_t* root, product_t* data) {
    trie_node_t* cur = root;
    char* key = data->key;
    unsigned char ch = *key;
    while(ch) {
        if(!cur->child[ch]) {
            cur->child[ch] = create_trie();
        }
        cur = cur->child[ch];
        ch = *++key;
    }
    data->sibling = NULL;
    if(cur->data) {
        product_t* prev = cur->data;
        //printf("----duplicate : [%s]\n",data->key);
        while(prev->sibling) {
            prev = prev->sibling;
        }
        prev->sibling = data;
    } else {
        cur->data = data;
    }
}


int search_trie(trie_node_t* root, char* key, listing_t* list, product_t** pRet) {
    trie_node_t* cur = root;
    product_t* last = NULL;
    unsigned char ch = *key;
    while(ch) {
        if(cur->data) {
            last = cur->data;
        }
        if(!cur->child[ch]) {
            break;
        }
        cur = cur->child[ch];
        ch = *++key;
    }
    if(!last) {
        return 0;
    }
    int cnt = 0;
    product_t* prd = last;
    while(prd) {
        if(strstr(list->condition,prd->condition)) {
            *pRet = prd;
            cnt++;
        }
        prd = prd->sibling;
    }
    if(cnt == 1) {
        return 1;
    }
    while(last) {
        if(strstr(list->condition,last->condition) && strstr(list->key,last->option)) {
            *pRet = last;
            return 1;
        }
        last = last->sibling;
    }
    return 1;
}

void insert_products(trie_node_t* trie_root, product_t* prd_root) {
    product_t* cur = prd_root->next;
    while(cur) {
        insert_trie(trie_root,cur);
        cur = cur->next;
    }
}

void arrange_listing(trie_node_t* trie_root, listing_t* lst_root) {
    listing_t* prev = lst_root;
    listing_t* list = prev->next;
    while(list) {
        char* key = list->key;
        product_t* prd = NULL;
        while(*key) {
            if(search_trie(trie_root,key,list,&prd)) {
                break;
            }
            ++key;
        }
        if(prd) { //found product
            if(prd->last) {
                prd->last->next = list;
                prd->last = list;
            } else {
                prd->first = prd->last = list;
            }
            prev->next = list->next;
            prd->last->next = NULL;
        } else { // not found product
            prev = list;
        }
        list = prev->next;
    }
}

void print_result(char* fname, product_t* root) {
    FILE* file = fopen(fname,"w");
    if(!file) {
        fprintf(stderr,"result file (%s) : fopen error\n",fname);
        return;
    }
    product_t* cur = root->next;
    while(cur) {
        fprintf(file,"{\"product_name\":");
        dumpString(file,cur->name);
        fprintf(file,",\"listings\":[");
        listing_t* list = cur->first;
        while(list) {
            fprintf(file,"%s",list->raw);
            list = list->next;
            if(list) {
                fputc(',',file);
            }
        }
        fprintf(file,"]}\n");
        cur = cur->next;
    }
    fclose(file);
}

void print_unknown(char* fname, listing_t* root) {
    FILE* file = fopen(fname,"w");
    if(!file) {
        fprintf(stderr,"result file (%s) : fopen error\n",fname);
        return;
    }
    listing_t* list = root->next;
    fprintf(file,"{\"product_name\":\"UNKNOWN\",\"listings\":[");
    while(list) {
        fprintf(file,"%s",list->raw);
        list = list->next;
        if(list) {
            fprintf(file,"\n,");
        }
    }
    fprintf(file,"]}\n");
    fclose(file);
}

int main(int argc, char** argv) {
    if(argc < 4) {
        printf("Usage : %s [product-file] [listing-file] [result-file]",argv[0]);
        return 0;
    }
    product_t* prd_root = create_product();
    listing_t* lst_root = create_listing();
    trie_node_t* trie_root = create_trie();
    if(!parse_product(argv[1],prd_root)) {
        return 0;
    }
    if(!parse_listing(argv[2],lst_root)) {
        return 0;
    }
    insert_products(trie_root,prd_root);
    arrange_listing(trie_root,lst_root);
    print_result(argv[3],prd_root);
    //print_unknown(lst_root);

    //free_product(prd_root);
    //free_trie(trie_root);
    //free_listing(lst_root);
    return 1;
}

