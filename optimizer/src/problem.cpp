#include "problem.h"

#include <assert.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <type_traits>

#include "block.h"
#include "environment.h"
#include "operators.h"
#include "replacers.h"
#include "selectors.h"
#include "solution.h"

Problem::Problem(Environment *env, const std::vector<size_t> &item_sizes, size_t bin_capacity, size_t bin_count) :
	env_(env), bin_capacity_(bin_capacity), item_count_(0) {
	size_t sum = std::accumulate(item_sizes.begin(), item_sizes.end(), std::remove_reference_t<decltype(item_sizes)>::value_type{});

	auto rounded = sum - 1 - (sum - 1) % bin_capacity + bin_capacity;
	bin_count_ = rounded / bin_capacity;

	if (bin_count) {
		assert(bin_count >= bin_count_ && "bad bin count");
		bin_count_ = bin_count;
	}

	original_bin_count_ = bin_count_;
	original_item_count_ = item_sizes.size();
	original_slack_ = slack_ = bin_count_ * bin_capacity - sum;

	std::vector<size_t> item_sizes_copy(item_sizes);

	std::vector<size_t> weights;
	std::sort(item_sizes_copy.begin(), item_sizes_copy.end(), std::isgreater<size_t, size_t>);
	find_optimal(item_sizes_copy, &weights);

	std::sort(weights.begin(), weights.end(), std::isgreater<size_t, size_t>);

	lower_bound_ = fsbound(items_, slack_, bin_count_, bin_capacity_);

	if (items_.find(1) == items_.end() && slack_) {
		weights.push_back(1);
	}

	threesum(weights, bin_capacity, &initial_3_partitions_);
	threesum(weights, 2 * bin_capacity, &initial_3_partitions_);
}

Environment *Problem::env() {
	return env_;
}

const std::unordered_map<size_t, size_t> &Problem::items() const {
	return items_;
}

inline size_t u(size_t k, size_t x, size_t c) {
	size_t quot = x * (k + 1) / c;
	size_t rem = x * (k + 1) % c;
	return rem == 0 ? x * k : quot * c;
}

inline void Problem::find_optimal1(const std::vector<size_t> &v, size_t *l, size_t *r) {
	while (*l <= *r && v[*l] == bin_capacity_) {
		++*l;
	}

	optimal1_ = *l;
	bin_count_ -= *l;
}

inline void Problem::find_optimal2_ones(const std::vector<size_t> &v, size_t *l, size_t *r) {
	while (*l < *r && v[*l] == bin_capacity_ - 1 && v[*r] == 1) {
		++optimal2_[std::make_pair(v[(*r)--], v[(*l)++])];
		--bin_count_;
	}
}

inline void Problem::find_optimal2_slack(const std::vector<size_t> &v, size_t *l, size_t *r) {
	while (*l <= *r && slack_ && v[*l] == bin_capacity_ - 1) {
		++optimal2_[std::make_pair(0, v[(*l)++])];
		--slack_;
		--bin_count_;
	}
}

inline void Problem::find_optimal2_rest(const std::vector<size_t> &v, size_t *l, size_t *r, std::vector<size_t> *s) {
	while (*l < *r) {
		size_t sum = v[*l] + v[*r];
		if (sum == bin_capacity_) {
			++optimal2_[std::make_pair(v[(*l)++], v[(*r)--])];
			--bin_count_;
		} else if (sum < bin_capacity_) {
			size_t e = v[(*r)--];
			if (!items_[e]++) {
				s->push_back(e);
			}
			++item_count_;
		} else {
			size_t e = v[(*l)++];
			if (!items_[e]++) {
				s->push_back(e);
			}
			++item_count_;
		}
	}

	if (*l == *r) {
		if (!items_[v[*l]]++) {
			s->push_back(v[*l]);
		}
		++item_count_;
	}
}

inline void Problem::find_optimal(const std::vector<size_t> &v, std::vector<size_t> *s) {
	size_t l = 0;
	size_t r = v.size() - 1;
	find_optimal1(v, &l, &r);
	find_optimal2_ones(v, &l, &r);
	find_optimal2_slack(v, &l, &r);
	find_optimal2_rest(v, &l, &r, s);
}

template<size_t iterations>
size_t Problem::fsbound(const std::unordered_map<size_t, size_t> &items, size_t slack, size_t bin_count, size_t bin_capacity) {
	std::vector<size_t> weights(items.size());
	size_t idx = 0;
	for (auto entry : items) {
		weights[idx++] = entry.first;
	}
	std::sort(weights.begin(), weights.end());
	std::vector<size_t> counts(weights.size());
	std::transform(weights.begin(), weights.end(), counts.begin(), [&items](size_t w) { return items.at(w); });
	size_t right = std::upper_bound(weights.begin(), weights.end(), bin_capacity / 2) - weights.begin();
	size_t n = std::accumulate(counts.begin(), counts.end(), std::remove_reference_t<decltype(counts)>::value_type{}) + slack;

	if (n - slack <= bin_count) {
		return 0;
	}

	size_t possible_blocks = n / 3;
	size_t minsplit = bin_count > possible_blocks ? bin_count - possible_blocks : 0;

	if (right == 0 && slack == 0) {
		size_t diff = n - bin_count;
		return minsplit > diff ? minsplit : diff;
	}

	size_t kmax = 0;
	size_t r;

	for (size_t k = 2; k < iterations + 1; ++k) {
		r = weights.size() - 1;
		size_t total = 0;
		for (size_t i = 0; i < r + 1; ++i) {
			total += counts[i] * u(k, weights[i], bin_capacity);
		}
		size_t maximum = total ? 1 + ((total - 1) / (bin_capacity * k)) : 0;

		for (size_t i = 0; i < right; ++i) {
			while (weights[i] > bin_capacity - weights[r]) {
				total += counts[r] * (bin_capacity * k - u(k, weights[r], bin_capacity));
				--r;
			}
			if (i > 0) {
				total -= counts[i - 1] * u(k, weights[i - 1], bin_capacity);
			}
			size_t ceiling = total ? 1 + ((total - 1) / (bin_capacity * k)) : 0;
			if (ceiling < maximum) {
				break;
			}
			maximum = ceiling;
		}
		kmax = maximum > kmax ? maximum : kmax;
	}
	r = weights.size() - 1;

	size_t total = 0;

	for (size_t i = 0; i < r + 1; ++i) {
		total += counts[i] * weights[i];
	}

	size_t l2maximum = total ? 1 + ((total - 1) / bin_capacity) : 0;

	for (size_t i = 0; i < right; ++i) {
		while (weights[i] > bin_capacity - weights[r]) {
			total += counts[r] * (bin_capacity - weights[r]);
			--r;
		}
		if (i > 0) {
			total -= counts[i - 1] * weights[i - 1];
		}
		size_t ceiling = total ? 1 + ((total - 1) / bin_capacity) : 0;
		if (ceiling < l2maximum) {
			break;
		}
		l2maximum = ceiling;
	}

	size_t maxval = std::max(kmax, l2maximum);
	return maxval < bin_count ? minsplit : std::max(minsplit, maxval - bin_count);
}

size_t Problem::bin_count() const {
	return bin_count_;
}

size_t Problem::bin_capacity() const {
	return bin_capacity_;
}

size_t Problem::item_count() const {
	return item_count_;
}

size_t Problem::original_item_count() const {
	return original_item_count_;
}

size_t Problem::original_bin_count() const {
	return original_bin_count_;
}

size_t Problem::original_slack() const {
	return original_slack_;
}

size_t Problem::slack() const {
	return slack_;
}

size_t Problem::lower_bound() const {
	return lower_bound_;
}

void Problem::next_fit_fragmentation(const Problem &problem, const std::vector<size_t> &items, size_t slack, std::vector<std::shared_ptr<Block>> *blocks) {
	if (items.empty()) {
		return;
	}

	blocks->push_back(std::make_shared<Block>(problem));

	size_t current_block = blocks->size() - 1;

	for (auto s : items) {
		if (s > (*blocks)[current_block]->slack() && (*blocks)[current_block]->slack() <= slack) {
			slack -= (*blocks)[current_block++]->slack();
			blocks->push_back(std::make_shared<Block>(problem));
		}
		(*blocks)[current_block]->put(s);
	}
}

void Problem::b3(std::unordered_map<size_t, size_t> *item_map, size_t *slack, std::vector<std::shared_ptr<Block>> *blocks) const {
	std::vector<size_t> weights;
	std::vector<Partition<size_t>> partitions;
	if (item_map->find(1) == item_map->cend() && *slack) {
		weights.reserve(item_map->size() + 1);
		for (auto entry : *item_map) {
			weights.push_back(entry.first);
		}
		std::sort(weights.begin(), weights.end(), std::isgreater<size_t, size_t>);
		weights.push_back(1);
	} else {
		weights.reserve(item_map->size());
		for (auto entry : *item_map) {
			weights.push_back(entry.first);
		}
		std::sort(weights.begin(), weights.end(), std::isgreater<size_t, size_t>);
	}
	threesum(weights, bin_capacity_, &partitions);
	threesum(weights, 2 * bin_capacity_, &partitions);

	b3(item_map, slack, &partitions, blocks);
}

size_t Problem::find_packing(std::unordered_map<size_t, size_t> *item_map, size_t *slack, std::vector<Partition<size_t>> *partitions, std::vector<size_t> *packing) const {
	auto s = partitions->size();
	std::uniform_int_distribution<size_t> dist(0, s - 1);

	size_t blocks_made = 0;

	while (s > 0) {
		auto idx = dist(*env_->rng());
		auto allowed = true;
		size_t slack_use = 0;

		for (size_t i = 0; i < (*partitions)[idx].size; ++i) {
			const auto &ic = (*partitions)[idx].items[i];
			if (ic.item == 1) {
				auto it = item_map->find(1);
				slack_use = ic.count - (it == item_map->cend() ? 0 : std::min(ic.count, it->second));
				if (slack_use > *slack) {
					allowed = false;
					break;
				}
			} else {
				auto it = item_map->find(ic.item);
				if (it == item_map->cend() || it->second < ic.count) {
					allowed = false;
					break;
				}
			}
		}

		if (allowed) {
			for (size_t i = 0; i < (*partitions)[idx].size - (slack_use > 0); ++i) {
				const auto &it = (*partitions)[idx].items[i];
				if ((*item_map)[it.item] == it.count) {
					item_map->erase(it.item);
				} else {
					(*item_map)[it.item] -= it.count;
				}
				std::fill_n(std::back_inserter(*packing), it.count, it.item);
			}

			if (slack_use > 0) {
				const auto &size = (*partitions)[idx].size;
				const auto &count = (*partitions)[idx].items[size - 1].count;
				auto to_remove = count - slack_use;
				*slack -= slack_use;
				if ((*item_map)[1] == to_remove) {
					item_map->erase(1);
				} else {
					(*item_map)[1] -= to_remove;
				}

				if (count != slack_use) {
					std::fill_n(std::back_inserter(*packing), count - slack_use, 1);
				}
			}
			++blocks_made;
		} else {
			std::swap((*partitions)[idx], (*partitions)[s - 1]);
			dist.param(decltype(dist)::param_type{0, --s});
		}
	}

	return blocks_made;
}

void Problem::b3(std::unordered_map<size_t, size_t> *item_map, size_t *slack, std::vector<Partition<size_t>> *partitions, std::vector<std::shared_ptr<Block>> *blocks) const {
	if (!partitions->empty()) {
		std::vector<size_t> packing;
		find_packing(item_map, slack, partitions, &packing);

		auto size = std::accumulate(packing.cbegin(), packing.cend(), size_t{});
		auto bin_count = 1 + (size - 1) / bin_capacity_;
		auto p_slack = bin_count * bin_capacity_ - size;
		next_fit_fragmentation(*this, packing, p_slack, blocks);
	}
}

void Problem::g(std::unordered_map<size_t, size_t> *item_map, size_t *slack, std::vector<std::shared_ptr<Block>> *blocks) const {
	if (item_map->empty()) {
		return;
	}
	auto slack_in = *slack;
	*slack = 0;
	std::vector<size_t> items;
	for (const auto &entry : *item_map) {
		std::fill_n(std::back_inserter(items), entry.second, entry.first);
	}
	std::shuffle(items.begin(), items.end(), *env_->rng());
	next_fit_fragmentation(*this, items, slack_in, blocks);
}

std::shared_ptr<Solution> Problem::generate_individual(bool do_b3) {
	auto items(items_);
	auto slack(slack_);
	std::vector<std::shared_ptr<Block>> blocks;

	if (do_b3) {
		b3(&items, &slack, &initial_3_partitions_, &blocks);

		if (items.empty()) {
			std::sort(blocks.begin(), blocks.end(), [](const auto &l, const auto &r) { return l->score() < r->score(); });
			return std::make_shared<Solution>(*this, std::move(blocks));
		}

	}
	g(&items, &slack, &blocks);

	std::sort(blocks.begin(), blocks.end(), [](const auto &l, const auto &r) { return l->score() < r->score(); });
	return std::make_shared<Solution>(*this, std::move(blocks));
}
