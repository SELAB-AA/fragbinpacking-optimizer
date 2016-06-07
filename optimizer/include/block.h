#ifndef BLOCK_H_
#define BLOCK_H_

#include <cstddef>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

class Problem;

class Block {
	friend std::ostream& operator<<(std::ostream& os, const Block& block);
	const Problem &problem_;
	size_t bin_count_;
	size_t size_;
	size_t item_count_;
	std::unordered_map<size_t, size_t> items_;

	size_t capacity() const;
public:
	Block(const Problem& problem);
	void put(size_t item);
	size_t slack() const;
	size_t score() const;
	const std::unordered_map<size_t, size_t> &items() const;
	const size_t bin_count() const;
	const size_t item_count() const;
	void swap(std::unordered_map<std::size_t, std::size_t> *f, size_t a, size_t b, size_t c, std::vector<std::shared_ptr<Block>> *out, size_t *slack, bool slack_swap = false) const;
	void swap(std::unordered_map<std::size_t, std::size_t> *f, size_t a, size_t b, size_t c, size_t d, std::vector<std::shared_ptr<Block>> *out, size_t *slack) const;
};

#endif
