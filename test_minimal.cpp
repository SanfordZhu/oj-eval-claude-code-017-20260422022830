#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;

const int HASH_SIZE = 100007;

template<typename T>
class SimpleHashMap {
private:
    struct Node {
        char key[25];
        T value;
        Node* next;
        Node(const char* k, const T& v) : value(v), next(nullptr) {
            strcpy(key, k);
        }
    };

    Node* table[HASH_SIZE];

    unsigned int hash(const char* str) {
        unsigned int h = 0;
        while (*str) {
            h = h * 131 + *str++;
        }
        return h % HASH_SIZE;
    }

public:
    SimpleHashMap() {
        memset(table, 0, sizeof(table));
    }

    void insert(const char* key, const T& value) {
        unsigned int h = hash(key);
        Node* node = new Node(key, value);
        node->next = table[h];
        table[h] = node;
        printf("Inserted key: %s at hash: %u\n", key, h);
    }

    T* find(const char* key) {
        unsigned int h = hash(key);
        printf("Looking for key: %s, hash: %u\n", key, h);
        Node* node = table[h];
        while (node) {
            if (strcmp(node->key, key) == 0) {
                return &node->value;
            }
            node = node->next;
        }
        return nullptr;
    }
};

int main() {
    SimpleHashMap<int> test;
    test.insert("A", 1);
    test.insert("B", 2);

    int* val = test.find("A");
    if (val) printf("Found: %d\n", *val);

    val = test.find("B");
    if (val) printf("Found: %d\n", *val);

    val = test.find("C");
    if (!val) printf("Not found\n");

    return 0;
}