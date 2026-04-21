#ifndef BPTREE_H
#define BPTREE_H

#include <cstring>
#include <cstdio>

// Simple B+ Tree implementation for string keys
const int BPTREE_ORDER = 10;

struct BPTreeNode {
    bool isLeaf;
    int keyCount;
    char keys[BPTREE_ORDER][25];
    union {
        BPTreeNode* children[BPTREE_ORDER + 1];  // For internal nodes
        int values[BPTREE_ORDER];                // For leaf nodes
    };
    BPTreeNode* next;  // For leaf nodes (linked list)

    BPTreeNode(bool leaf = false) : isLeaf(leaf), keyCount(0), next(nullptr) {
        memset(keys, 0, sizeof(keys));
        if (!leaf) {
            memset(children, 0, sizeof(children));
        } else {
            memset(values, 0, sizeof(values));
        }
    }
};

class BPTree {
private:
    BPTreeNode* root;

    void insertInternal(char key[25], BPTreeNode* cursor, BPTreeNode* child);
    void removeInternal(char key[25], BPTreeNode* cursor, BPTreeNode* child);
    BPTreeNode* findParent(BPTreeNode* cursor, BPTreeNode* child);

public:
    BPTree() : root(nullptr) {}
    ~BPTree() {
        // TODO: Implement destructor
    }

    bool insert(char key[25], int value);
    bool remove(char key[25]);
    int* find(char key[25]);
    void traverse();
};

bool BPTree::insert(char key[25], int value) {
    if (root == nullptr) {
        root = new BPTreeNode(true);
        strcpy(root->keys[0], key);
        root->values[0] = value;
        root->keyCount = 1;
        return true;
    }

    BPTreeNode* cursor = root;
    BPTreeNode* parent = nullptr;

    // Find leaf node to insert into
    while (!cursor->isLeaf) {
        parent = cursor;
        for (int i = 0; i < cursor->keyCount; i++) {
            if (strcmp(key, cursor->keys[i]) < 0) {
                cursor = cursor->children[i];
                break;
            }
            if (i == cursor->keyCount - 1) {
                cursor = cursor->children[i + 1];
                break;
            }
        }
    }

    // Check if key already exists
    for (int i = 0; i < cursor->keyCount; i++) {
        if (strcmp(cursor->keys[i], key) == 0) {
            return false;  // Key already exists
        }
    }

    // Find position to insert
    int pos = 0;
    while (pos < cursor->keyCount && strcmp(cursor->keys[pos], key) < 0) {
        pos++;
    }

    // Make space for new key
    for (int i = cursor->keyCount; i > pos; i--) {
        strcpy(cursor->keys[i], cursor->keys[i - 1]);
        cursor->values[i] = cursor->values[i - 1];
    }

    // Insert new key
    strcpy(cursor->keys[pos], key);
    cursor->values[pos] = value;
    cursor->keyCount++;

    // Split leaf if overflow
    if (cursor->keyCount == BPTREE_ORDER) {
        BPTreeNode* newLeaf = new BPTreeNode(true);

        // Move half keys to new leaf
        int splitPos = cursor->keyCount / 2;
        for (int i = splitPos; i < cursor->keyCount; i++) {
            strcpy(newLeaf->keys[i - splitPos], cursor->keys[i]);
            newLeaf->values[i - splitPos] = cursor->values[i];
        }
        newLeaf->keyCount = cursor->keyCount - splitPos;
        cursor->keyCount = splitPos;

        // Update linked list
        newLeaf->next = cursor->next;
        cursor->next = newLeaf;

        // Insert new key in parent
        char newKey[25];
        strcpy(newKey, newLeaf->keys[0]);

        if (cursor == root) {
            BPTreeNode* newRoot = new BPTreeNode(false);
            strcpy(newRoot->keys[0], newKey);
            newRoot->children[0] = cursor;
            newRoot->children[1] = newLeaf;
            newRoot->keyCount = 1;
            root = newRoot;
        } else {
            insertInternal(newKey, parent, newLeaf);
        }
    }

    return true;
}

void BPTree::insertInternal(char key[25], BPTreeNode* cursor, BPTreeNode* child) {
    // Find position to insert
    int pos = 0;
    while (pos < cursor->keyCount && strcmp(cursor->keys[pos], key) < 0) {
        pos++;
    }

    // Make space
    for (int i = cursor->keyCount; i > pos; i--) {
        strcpy(cursor->keys[i], cursor->keys[i - 1]);
    }
    for (int i = cursor->keyCount + 1; i > pos + 1; i--) {
        cursor->children[i] = cursor->children[i - 1];
    }

    // Insert
    strcpy(cursor->keys[pos], key);
    cursor->children[pos + 1] = child;
    cursor->keyCount++;

    // Split if overflow
    if (cursor->keyCount == BPTREE_ORDER) {
        BPTreeNode* newInternal = new BPTreeNode(false);
        int splitPos = cursor->keyCount / 2;

        // Move half to new internal node
        for (int i = splitPos + 1; i < cursor->keyCount; i++) {
            strcpy(newInternal->keys[i - splitPos - 1], cursor->keys[i]);
        }
        for (int i = splitPos + 1; i <= cursor->keyCount; i++) {
            newInternal->children[i - splitPos - 1] = cursor->children[i];
        }
        newInternal->keyCount = cursor->keyCount - splitPos - 1;
        cursor->keyCount = splitPos;

        // Promote middle key
        char promoteKey[25];
        strcpy(promoteKey, cursor->keys[splitPos]);

        if (cursor == root) {
            BPTreeNode* newRoot = new BPTreeNode(false);
            strcpy(newRoot->keys[0], promoteKey);
            newRoot->children[0] = cursor;
            newRoot->children[1] = newInternal;
            newRoot->keyCount = 1;
            root = newRoot;
        } else {
            // Find parent and insert
            BPTreeNode* parent = findParent(root, cursor);
            insertInternal(promoteKey, parent, newInternal);
        }
    }
}

int* BPTree::find(char key[25]) {
    if (root == nullptr) {
        return nullptr;
    }

    BPTreeNode* cursor = root;
    while (!cursor->isLeaf) {
        bool found = false;
        for (int i = 0; i < cursor->keyCount; i++) {
            if (strcmp(key, cursor->keys[i]) < 0) {
                cursor = cursor->children[i];
                found = true;
                break;
            }
        }
        if (!found) {
            cursor = cursor->children[cursor->keyCount];
        }
    }

    for (int i = 0; i < cursor->keyCount; i++) {
        if (strcmp(cursor->keys[i], key) == 0) {
            return &cursor->values[i];
        }
    }

    return nullptr;
}

BPTreeNode* BPTree::findParent(BPTreeNode* cursor, BPTreeNode* child) {
    if (cursor == nullptr || cursor->isLeaf) {
        return nullptr;
    }

    for (int i = 0; i <= cursor->keyCount; i++) {
        if (cursor->children[i] == child) {
            return cursor;
        }
        BPTreeNode* result = findParent(cursor->children[i], child);
        if (result != nullptr) {
            return result;
        }
    }

    return nullptr;
}

#endif // BPTREE_H