#ifndef PROBLEM_H_
#define PROBLEM_H_

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

#include "threesum.h"

class Block;
class Environment;
class Solution;

template<class T> struct PairHash;
template<> struct PairHash<std::pair<size_t, size_t> > {
	size_t operator()(std::pair<size_t, size_t> const & p) const {
		size_t h1 = std::hash<size_t>()(p.first);
		size_t h2 = std::hash<size_t>()(p.second);

		return h1 ^ (h2 << 1);
	}
};

class Problem {
	Environment *env_;
	std::unordered_map<size_t, size_t> items_;
	size_t bin_count_;
	size_t bin_capacity_;
	size_t item_count_;
	size_t original_bin_count_;
	size_t original_item_count_;
	size_t original_slack_;
	size_t slack_;
	size_t lower_bound_;
	size_t optimal1_;
	std::unordered_map<std::pair<size_t, size_t>, size_t, PairHash<std::pair<size_t, size_t> > > optimal2_;
	std::vector<Partition<size_t>> initial_3_partitions_;
	void find_optimal1(const std::vector<size_t> &v, size_t *l, size_t *r);
	void find_optimal2_ones(const std::vector<size_t> &v, size_t *l, size_t *r);
	void find_optimal2_slack(const std::vector<size_t> &v, size_t *l, size_t *r);
	void find_optimal2_rest(const std::vector<size_t> &v, size_t *l, size_t *r, std::vector<size_t> *s);
	void find_optimal(const std::vector<size_t> &v, std::vector<size_t> *s);

	template<size_t iterations = 20>
	static size_t fsbound(const std::unordered_map<size_t, size_t> &items, size_t slack, size_t bin_count, size_t bin_capacity);
	size_t find_packing(std::unordered_map<size_t, size_t> *item_map, size_t *slack, std::vector<Partition<size_t>> *partitions, std::vector<size_t> *packing) const;
	void b3(std::unordered_map<size_t, size_t> *item_map, size_t *slack, std::vector<Partition<size_t>> *partitions, std::vector<std::shared_ptr<Block>> *blocks) const;
	static void next_fit_fragmentation(const Problem &problem, const std::vector<size_t> &items, size_t slack, std::vector<std::shared_ptr<Block>> *blocks);

public:
	Problem(Environment *env, const std::vector<size_t> &item_sizes, size_t bin_capacity, size_t bin_count = 0);
	Environment *env();
	const std::unordered_map<size_t, size_t> &items() const;
	size_t bin_count() const;
	size_t bin_capacity() const;
	size_t item_count() const;
	size_t original_item_count() const;
	size_t original_bin_count() const;
	size_t original_slack() const;
	size_t slack() const;
	size_t lower_bound() const;
	void b3(std::unordered_map<size_t, size_t> *item_map, size_t *slack, std::vector<std::shared_ptr<Block>> *blocks) const;
	void g(std::unordered_map<size_t, size_t> *item_map, size_t *slack, std::vector<std::shared_ptr<Block>> *blocks) const;
	std::shared_ptr<Solution> generate_individual(bool do_b3 = true);
};

#endif
