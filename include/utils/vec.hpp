#pragma once

#include "common.hpp"
#include <cstddef>
#include <cstdlib>
namespace phantom {
  namespace vec {
    template <typename T>
    class Vec {
  public:
      void init() {
        cap = VEC_INIT;
        data = (T*)malloc(sizeof(T) * cap);

        len = 0;
      }
      void init(size_t cap) {
        this->cap = cap;
        data = (T*)malloc(sizeof(T) * cap);

        len = 0;
      }
      void push(T element) {
        if (cap <= len) {
          cap *= 2;
          data = (T*)realloc(data, sizeof(T) * cap);
        }

        data[len] = element;
        len++;
      }
      void extend(size_t size) {
        cap += size;
        data = (T*)realloc(data, sizeof(T) * cap);
      }
      size_t size() {
        return len;
      }
      size_t capacity() {
        return capacity;
      }
      void dump() {
        free(data);
        cap = 0;
        len = 0;
      }
      T* begin() { return data; }
      T* end() { return data + len; }
      const T* begin() const { return data; }
      const T* end() const { return data + len; }

  private:
      T* data;
      size_t len;
      size_t cap;
    };
  } // namespace vec
} // namespace phantom
