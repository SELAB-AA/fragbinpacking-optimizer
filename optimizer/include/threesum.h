#ifndef THREESUM_H_
#define THREESUM_H_

#include <array>
#include <cstddef>
#include <vector>

template<typename T>
struct ItemCount {
	T item;
	T count;
};

template<typename T, size_t S = 3>
struct Partition {
	size_t size;
	std::array<ItemCount<T>, S> items;
};

void threesum(const std::vector<size_t> &s, size_t r, std::vector<Partition<size_t>> *out);

#endif
