#ifndef REPLACERS_H_
#define REPLACERS_H_

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "solution.h"

template<class ForwardIt, typename Count, class BinaryPredicate>
ForwardIt dedup(ForwardIt first, ForwardIt last, Count count, BinaryPredicate p = [](const auto &l, const auto &r) { return l == r; }) {
	Count counter{};

	if (first == last || count == counter) {
		return last;
	}

	ForwardIt result = first;

	while (++first != last && counter < count) {
		if (!p(*result, *first)) {
			if (++result != first) {
				std::iter_swap(result, first);
			}
		} else {
			counter++;
		}
	}

	return ++result;
}

template<size_t NP, size_t NC, size_t NE>
void controlled_replacement_crossover(std::array<std::shared_ptr<Solution>, NP> *population, std::array<std::shared_ptr<Solution>, NC> *progeny, std::array<std::shared_ptr<Solution>, NC / 2> *r) {
	std::sort(r->begin(), r->end());
	std::sort(population->begin() + NE, population->end());

	std::array<std::shared_ptr<Solution>, NP - NC / 2 - NE> p_minus_r_minus_b;
	std::set_difference(population->begin() + NE, population->end(), r->begin(), r->end(), p_minus_r_minus_b.begin());

	std::sort(p_minus_r_minus_b.begin(), p_minus_r_minus_b.end(), [](const auto &left, const auto &right) { return left->size() > right->size(); });

	auto it = dedup(p_minus_r_minus_b.begin(), p_minus_r_minus_b.end(), NC / 2, [](const auto &left, const auto &right) { return left->size() == right->size(); });

	if (static_cast<size_t>(p_minus_r_minus_b.end() - it) < NC / 2) {
		it -= NC / 2 - (p_minus_r_minus_b.end() - it);
	}

	std::move(progeny->begin() + NC / 2, progeny->end(), it);

	std::move(progeny->begin(), progeny->begin() + NC / 2, population->begin() + NE);
	std::move(p_minus_r_minus_b.begin(), p_minus_r_minus_b.end(), population->begin() + NE + NC / 2);
	std::sort(population->begin() + NE, population->end(), [](const auto &left, const auto &right) { return left->size() > right->size(); });
	std::inplace_merge(population->begin(), population->begin() + NE, population->end(), [](const auto &left, const auto &right) { return left->size() > right->size(); });
}

template<size_t NP>
void controlled_replacement_mutation(std::array<std::shared_ptr<Solution>, NP> *population, std::vector<std::shared_ptr<Solution>> *cloned) {
	std::sort(cloned->begin(), cloned->end(), [](const auto &left, const auto &right) { return left->size() > right->size(); });

	auto it = dedup(population->begin(), population->end(), cloned->size(), [](const auto &l, const auto &r) { return l->size() == r->size(); });

	if (static_cast<size_t>(population->end() - it) < cloned->size()) {
		it -= cloned->size() - (population->end() - it);
	}

	std::move(cloned->begin(), cloned->end(), it);

	std::inplace_merge(population->begin(), it, it + cloned->size(), [](const auto &left, const auto &right) { return left->size() > right->size(); });
	std::inplace_merge(population->begin(), it + cloned->size(), population->end(), [](const auto &left, const auto &right) { return left->size() > right->size(); });
}

#endif
