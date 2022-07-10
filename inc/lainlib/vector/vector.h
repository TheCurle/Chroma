#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel/system/memory.h>

/************************
 *** Team Kitty, 2022 ***
 ***    Lainlib       ***
 ***********************/

// integer typedefs
using u32 = uint32_t;

namespace lainlib {

    template<typename T>
    struct vector {
        T& operator [] (u32 i) {
            return this->data[i];
        }

        T operator [] (u32 i) const {
            return this->data[i];
        }

        u32 size() {
            return bytes / sizeof(T);
        }

        template<typename access_type>
        access_type handle_read(const void *buf) {
            access_type v;
            memcpy(&v,buf,sizeof(access_type));
            return v;
        }

        template<typename access_type>
        void handle_write(void *buf, access_type v) {
            memcpy(buf,&v,sizeof(access_type));
        }

        // insert at end as a var as a raw set of bytes into the Array
        template<typename Y>
        void emplace_back(Y v) {
            const u32 len = sizeof(v);

            reserve(len);

            // actually write in the data
            handle_write(&data[size()], v);
            bytes += len;
        }

        T pop() {
            const T v = data[size() - 1];
            bytes -= sizeof(v);

            return v;
        }

        void reserve_raw(u32 size) {
            capacity = size;
            data = (T*)krealloc(data, size);
        }

        void resize(u32 in) {
            const u32 len = in * sizeof(T);

            const u32 old_len = size();

            reserve_raw( bytes);
            bytes = len;

            // default initialize the new elements
            for(u32 i = old_len; i < size(); i++) {
                data[i] = {};
            }
        }

        // make sure there is enough left for the allocation we are doing
        void reserve(u32 size) {
            const u32 free_size = capacity - bytes;

            // we have room to just dump this in
            if(free_size >= size) {
                return;
            } else {
                const u32 new_capacity = (capacity + size) * 2;
                reserve_raw(new_capacity);
            }
        }

        // raw mem read and writes over the array
        T read_var(u32 idx) {
            return handle_read<T>(data[idx]);
        }

        template<typename Y>
        void write_var(u32 idx, T v) {
            return handle_write(data[idx], v);
        }

        // insert raw memory block into the array
        void push_mem(const void* newData, u32 size)
        {
            reserve(size);

            memcpy(data[bytes], newData, size);
            bytes += size;
        }

        void destroy() {
            if(data) {
                kfree(data);
                data = nullptr;
            }

            bytes = 0;
            capacity = 0;
        }

        T* data = nullptr;

        // in raw bytes
        u32 bytes = 0;
        u32 capacity = 0;
    };
}