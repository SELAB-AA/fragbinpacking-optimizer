#include "operators.h"

#include <assert.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <unordered_map>

#include "block.h"
#include "environment.h"
#include "problem.h"
#include "solution.h"
#include "util.h"

inline bool allowed(const std::shared_ptr<Block> &block, std::unordered_map<size_t, size_t> *available, size_t *slack) {
	if (block->slack() > *slack) {
		return false;
	}

	for (const auto &entry : block->items()) {
		if (entry.second > (*available)[entry.first]) {
			return false;
		}
	}

	for (const auto &entry : block->items()) {
		(*available)[entry.first] -= entry.second;
	}

	*slack -= block->slack();

	return true;
}

void gene_level_crossover(Problem *problem, const std::shared_ptr<Solution> &l, const std::shared_ptr<Solution> &r, std::shared_ptr<Solution> *p) {
	auto ll = l->blocks();
	auto rr = r->blocks();

	std::vector<std::shared_ptr<Block>> combination(ll.size() + rr.size());

	std::vector<std::shared_ptr<Block>> offspring;
	offspring.reserve(combination.size());

	auto aa = ll.begin();
	auto bb = rr.begin();
	auto cc = combination.begin();

	if (ll.size() > rr.size()) {
		cc = std::copy_n(aa, ll.size() - rr.size(), cc);
		aa += ll.size() - rr.size();
	} else if (rr.size() > ll.size()) {
		cc = std::copy_n(bb, rr.size() - ll.size(), cc);
		bb += rr.size() - ll.size();
	}

	while (aa != ll.end() || bb != rr.end()) {
		if ((*aa)->score() <= (*bb)->score()) {
			*cc++ = *aa++;
			*cc++ = *bb++;
		} else {
			*cc++ = *bb++;
			*cc++ = *aa++;
		}
	}

	auto available(problem->items());
	auto unplaced(problem->items());

	auto slack = problem->slack();

	for (const auto &c : combination) {
		if (allowed(c, &available, &slack)) {
			for (auto &entry : c->items()) {
				auto &item = unplaced[entry.first];
				item -= entry.second;
				if (item == 0) {
					unplaced.erase(entry.first);
				}
			}
			offspring.push_back(c);
		}
	}

	if (!unplaced.empty()) {
		const bool b3 = true;

		if (b3) {
			problem->b3(&unplaced, &slack, &offspring);
		}
		problem->g(&unplaced, &slack, &offspring);
	}

	std::sort(offspring.begin(), offspring.end(), [](const auto &left, const auto &right) { return left->score() < right->score(); });

	*p = std::make_shared<Solution>(*problem, std::move(offspring));
}

void update_free_list(Problem *problem, std::vector<size_t> *free_list, const std::unordered_map<std::size_t, std::size_t> &free_map) {
	free_list->clear();
	for (const auto &entry : free_map) {
		free_list->push_back(entry.first);
	}
	std::shuffle(free_list->begin(), free_list->end(), *problem->env()->rng());
}

void rearrangement_by_pairs(Problem *problem, std::vector<std::shared_ptr<Block>> *blocks, std::unordered_map<std::size_t, std::size_t> *free, std::vector<std::shared_ptr<Block>> *out, size_t *slack) {
	std::vector<size_t> free_items;
	free_items.reserve(free->size());

	for (const auto &entry : *free) {
		free_items.push_back(entry.first);
	}

	std::shuffle(free_items.begin(), free_items.end(), *problem->env()->rng());
	std::shuffle(blocks->begin(), blocks->end(), *problem->env()->rng());

	for (auto block = blocks->begin(); block < blocks->end(); ++block) {
		for (auto p = (*block)->items().cbegin(); p != (*block)->items().cend(); ++p) {
			for (auto q = p; q != (*block)->items().cend(); ++q) {
				if (q == p && q->second < 2) {
					continue;
				}
				for (auto r = free_items.begin(); r != free_items.end(); ++r) {
					for (auto s = r; s != free_items.end(); ++s) {
						if ((((s == r) && (((*free)[*s] < 2) || (q == p && q->first == *s)))) || (p->first == *r && q->first == *s) || (p->first == *s && q->first == *r)) {
							continue;
						}

						const auto block_sum = p->first + q->first;
						const auto block_slack = (*block)->slack();

						if (*r >= block_sum && *r <= block_sum + block_slack) {
							(*block)->swap(free, p->first, q->first, *r, out, slack);
							update_free_list(problem, &free_items, *free);
							goto end;
						}

						if (*s >= block_sum && *s <= block_sum + block_slack) {
							(*block)->swap(free, p->first, q->first, *s, out, slack);
							update_free_list(problem, &free_items, *free);
							goto end;
						}

						if (*r + *s >= block_sum && *r + *s <= block_sum + block_slack) {
							(*block)->swap(free, p->first, q->first, *r, *s, out, slack);
							update_free_list(problem, &free_items, *free);
							goto end;
						}

						if (*slack > 0 && p->first != 1 && q->first != 1) {
							if (*r + 1 == block_sum) {
								(*block)->swap(free, p->first, q->first, *r, out, slack, true);
								update_free_list(problem, &free_items, *free);
								goto end;
							}
							if (*s + 1 == block_sum) {
								(*block)->swap(free, p->first, q->first, *s, out, slack, true);
								update_free_list(problem, &free_items, *free);
								goto end;
							}

						}
					}
				}
			}
		}
		out->emplace_back(*block);
end:		;
	}
}

void adaptive_mutation(Problem *problem, const std::shared_ptr<Solution> &mutant, double k) {
	const bool use_reinsertion_procedure = false;
	const auto &m = mutant->size();
	const auto max_blocks = problem->bin_count() - problem->lower_bound();
	const auto incomplete = max_blocks - m;

	if (max_blocks == m) {
		return;
	}

	assert(max_blocks > m);

	auto scale = std::pow(incomplete, -1 / k);
	auto epsilon = (2 - static_cast<double>(incomplete) / max_blocks) * scale;
	std::uniform_real_distribution<> dist(0, scale);
	auto p_epsilon = 1.0 - dist(*problem->env()->rng());

	auto n_b = std::min(static_cast<decltype(m)>(std::ceil(m * epsilon * p_epsilon)), m);

	/*
	std::cout << "n_b: " << n_b << "\n" <<
		"m: " << m << "\n" <<
		"max_blocks: " << max_blocks << "\n" <<
		"n_b / m: " << static_cast<double>(n_b) / m << "\n" <<
		"epsilon: " << epsilon << "\n" <<
		"p_epsilon: " << p_epsilon << "\n";
	*/

	assert(n_b <= m);

	std::unordered_map<size_t, size_t> unplaced;

	size_t slack = 0;

	if (!use_reinsertion_procedure) {
		std::for_each(random_unique(mutant->blocks_.rbegin() + 1, mutant->blocks_.rend(), n_b - 1, *problem->env()->rng()) - 1, mutant->blocks_.rbegin() + n_b, [&slack, &unplaced](auto &block) {
			for (const auto &entry : block->items()) {
				unplaced[entry.first] += entry.second;
			}
			slack += block->slack();
		});
	} else {
		std::for_each(mutant->blocks_.rbegin(), mutant->blocks_.rbegin() + n_b, [&slack, &unplaced](const auto &block) {
			for (const auto &entry : block->items()) {
				unplaced[entry.first] += entry.second;
			}
			slack += block->slack();
		});
	}

	if (!use_reinsertion_procedure) {
		mutant->blocks_.resize(mutant->blocks_.size() - n_b);

		const bool b3 = true;
		if (b3) {
			problem->b3(&unplaced, &slack, &mutant->blocks_);
		}
		problem->g(&unplaced, &slack, &mutant->blocks_);
	} else {
		std::vector<std::shared_ptr<Block>> blocks;
		blocks.reserve(mutant->blocks_.size());

		mutant->blocks_.resize(mutant->blocks_.size() - n_b);

		rearrangement_by_pairs(problem, &mutant->blocks_, &unplaced, &blocks, &slack);

		const bool b3 = true;
		if (b3) {
			problem->b3(&unplaced, &slack, &blocks);
		}
		problem->g(&unplaced, &slack, &blocks);

		mutant->blocks_ = std::move(blocks);
	}

	std::sort(mutant->blocks_.begin(), mutant->blocks_.end(), [](const auto &l, const auto &r) { return l->score() < r->score(); });

	assert(mutant->blocks().size() <= max_blocks);

	mutant->age_ = 0;
}
