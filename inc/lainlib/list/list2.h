#pragma once
#include <stdint.h>

 struct ListNode {
    ListNode(void* content) : content(content) {}

    void* content = nullptr;
    ListNode* prev = nullptr;
    ListNode* next = nullptr;
};

/**
 * A much simpler way to access arbitrary data.
 * TODO: some sort of iterative access; subsequent get() calls will re-iterate through the list.
 */
struct List {
    uint32_t size = 0;
    ListNode* root = nullptr;

    bool add(void* payload) {
        ListNode* node = new ListNode(payload);
        if (!node) {
            return false;
        }

        if (!root) {
            root = node;
        } else {
            ListNode* current = root;
            while(current->next)
                current = current->next;
            current->next = node;
            node->prev = current;
        }

        size++;

        return true;
    }

    void* get(uint32_t index) {
        if (size == 0 || size < index)
            return nullptr;

        ListNode* current = root;
        for (uint32_t idx = 0; (idx < index) && current; idx++)
            current = current->next;

        return current ? current->content : nullptr;
    }
};

