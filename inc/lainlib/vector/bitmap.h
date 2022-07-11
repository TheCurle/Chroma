#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel/system/memory.h>

/************************
 *** Team Kitty, 2022 ***
 ***    Lainlib       ***
 ***********************/

namespace lainlib {
    struct bitmap {
        size_t size;
        uint8_t* data;
        size_t lastFreeSpace = 0;

        size_t getSize() const { return size; }

        void set(size_t idx, bool val) {
            if (idx > size) {
                SerialPrintf("[BITMP] Attempted to set data out of bounds; index %u, size %u.\r\n", idx, size);
                return;
            }

            size_t byte = idx / 8, bit = idx % 8;
            if (val)
                data[byte] |= (1 << bit);
            else
                data[byte] &= ~(1 << bit);
        }

        bool get(size_t idx) const {
            if (idx > size) {
                SerialPrintf("[BITMP] Attempted to get data out of bounds; index %u, size %u.\r\n", idx, size);
                return false;
            }

            size_t byte = idx / 8, bit = idx % 8;
            return (data[byte] & (1 << bit));
        }

        bitmap(uint8_t* buffer, size_t sizeIn) {
            size = sizeIn;
            memset(buffer, 0xFF, size / 8);
            data = buffer;
            resetLastFreeSpace();
        }

        bitmap() {
            size = 0;
            data = nullptr;
            resetLastFreeSpace();
        }

        size_t findSpace(size_t length) {
            size_t currentLen = 0, currentIdx = 0;

            for (size_t i = lastFreeSpace; i < size; i++) {
                if (i == 0) continue;

                if (!get(i)) {
                    if (currentLen == 0)
                        currentIdx = i;
                    currentLen++;
                } else {
                    currentLen = 0;
                    currentIdx = 0;
                }

                if (currentLen == length) {
                    lastFreeSpace = currentIdx + currentLen;
                    return currentIdx;
                }
            }

            if (lastFreeSpace == 0) {
                SerialPrintf("[BITMP] Fatal error: no space left.\r\n");
                return 0;
            } else {
                resetLastFreeSpace();
                return findSpace(length);
            }
        }

        size_t allocate(size_t length) {
            size_t idx = findSpace(length);
            if (idx == 0) {
                SerialPrintf("[BITMP] Unable to allocate length.\r\n");
                return 0;
            }

            if (setUsed(idx, length) == 0) {
                SerialPrintf("[BITMP] Unable to allocate length.\r\n");
                return 0;
            }

            return idx;
        }

        size_t setFree(size_t idx, size_t length) {
            for (size_t i = idx; i < length; i++)
                set(i, false);

            lastFreeSpace = idx;
            return 1;
        }

        size_t setUsed(size_t idx, size_t length) {
            for (size_t i = idx; i < length; i++)
                set(i, true);

            lastFreeSpace = idx;
            return 1;
        }

        void resetLastFreeSpace() { lastFreeSpace = 0; }
    };
}