#include "solution.h"

#include <algorithm>

#include "block.h"
#include "problem.h"

Solution::Solution(const Problem &problem) : problem_(std::make_shared<const Problem>(problem)), blocks_{std::make_shared<Block>(Block(problem))}, age_(0) {}

Solution::Solution(const Problem &problem, std::vector<std::shared_ptr<Block>> &&blocks) : problem_(std::make_shared<const Problem>(problem)), blocks_(blocks), age_(0) {}

Solution::Solution(const Solution &solution) : problem_(solution.problem_), age_(solution.age_) {
	blocks_.reserve(solution.blocks_.size());

	for (const auto &block : solution.blocks_) {
		blocks_.push_back(std::make_shared<Block>(*block));
	}
}

Solution &Solution::operator=(const Solution &solution) {
	Solution sol(solution);
	swap(sol);
	return *this;

}

size_t Solution::size() const {
	return blocks_.size();
}

const std::vector<std::shared_ptr<Block>> &Solution::blocks() const {
	return blocks_;
}

const unsigned int Solution::age() const {
	return age_;
}

void Solution::increase_age(unsigned int increment) {
	age_ += increment;
}

std::ostream& operator<<(std::ostream &os, const Solution &solution) {
	std::vector<const std::shared_ptr<Block> *> blocks;
	blocks.reserve(solution.size());
	for (const auto &block : solution.blocks()) {
		blocks.push_back(&block);
	}
	std::sort(blocks.begin(), blocks.end(), [](const auto &l, const auto &r) { return (*l)->score() < (*r)->score(); });
	auto it = blocks.cbegin();
	os << ***it;
	for (++it; it != blocks.cend(); ++it) {
		os << ", " << ***it;
	}
	return os;
}
