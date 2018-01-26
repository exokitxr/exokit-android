#ifndef _UTIL_H
#define _UTIL_H

template<typename Type>
inline Type *cloneData(Type *data, size_t size) {
  Type *result = (Type *)malloc(size);
  memcpy(result, data, size);
  return result;
}

#endif
