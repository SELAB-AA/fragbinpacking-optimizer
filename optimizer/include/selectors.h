#ifndef SELECTORS_H_
#define SELECTORS_H_

#include <assert.h>
#include <algorithm>
#include <array>
#include <memory>
#include <random>
#include <vector>

#include "environment.h"
#include "problem.h"
#include "solution.h"
#include "util.h"

template<size_t NP, size_t NC, size_t NE>
void controlled_selection_crossover(Problem *problem, std::array<std::shared_ptr<Solution>, NP> *population, std::array<std::shared_ptr<Solution>, NC / 2> *g, std::array<std::shared_ptr<Solution>, NC / 2> *r) {
	std::array<std::shared_ptr<Solution>, NC> p_copy1;
	std::array<std::shared_ptr<Solution>, NP - NE> p_copy2;
	std::copy_n(population->begin(), NC, p_copy1.begin());
	std::copy(population->begin() + NE, population->end(), p_copy2.begin());
	std::copy_n(random_unique(p_copy1.begin(), p_copy1.end(), g->size(), *problem->env()->rng()), g->size(), g->begin());
	std::copy_n(random_unique(p_copy2.begin(), p_copy2.end(), r->size(), *problem->env()->rng()), r->size(), r->begin());

	for (size_t i = 0; i < NC / 2; i++) {
		if ((*g)[i] == (*r)[i]) {
			std::swap((*g)[i], (*g)[(i + NC / 2 - 1) % (NC / 2)]);
		}
	}
}

template<size_t NP, size_t NM, size_t NE, size_t LS>
void controlled_selection_mutation(const std::array<std::shared_ptr<Solution>, NP> &population, std::vector<std::shared_ptr<Solution>> *clones, std::array<std::shared_ptr<Solution>, NM> *mutants) {
	std::copy_if(population.begin(), population.begin() + NE, std::back_inserter(*clones), [](const auto &sol) { return sol->age() < LS; });
	std::copy_n(population.begin(), NM, mutants->begin());
}

#endif
