#ifndef SELECTORS_H_
#define SELECTORS_H_

#include <assert.h>

#include <algorithm>
#include <array>
#include <memory>
#include <random>
#include <vector>

#include "environment.h"
#include "util.h"

namespace optimizer {

/*
 * Performs controlled selection for crossover. Selects `NC` individuals from
 * the population into sets `g` for good solutions and `r` for random solutions.
 * Parameters `NP` and `NE` are the size of the population and the size of the
 * elite set.
 */
template <std::uint32_t NP, std::uint32_t NC, std::uint32_t NE>
void controlled_selection_crossover(
    Problem *problem, std::array<std::unique_ptr<Solution>, NP> *population,
    std::array<Solution *, NC / 2u> *g, std::array<Solution *, NC / 2u> *r) {
    static_assert(NC >= 2u, "NC must be at least 2");
    std::array<Solution *, NC> p_copy1;
    std::array<Solution *, NP - NE> p_copy2;
    std::transform(population->cbegin(), population->cbegin() + NC,
                   p_copy1.begin(), [](const auto &sol) { return sol.get(); });
    std::transform(population->cbegin() + NE, population->cend(),
                   p_copy2.begin(), [](const auto &sol) { return sol.get(); });
    std::copy_n(sample_inplace(p_copy1.begin(), p_copy1.end(), g->size(),
                               *problem->env()->rng()),
                g->size(), g->begin());
    std::copy_n(sample_inplace(p_copy2.begin(), p_copy2.end(), r->size(),
                               *problem->env()->rng()),
                r->size(), r->begin());

    auto i = 0u;
    for (; i < NC / 2u - 1u; ++i) {
        if ((*g)[i] == (*r)[i]) {
            std::swap((*g)[i], (*g)[i + 1u]);
            ++i;
        }
    }
    if (i == NC / 2u - 1u && (*g)[NC / 2u - 1u] == (*r)[NC / 2u - 1u]) {
        std::swap((*g)[NC / 2u - 2u], (*g)[NC / 2u - 1u]);
    }
}

/*
 * Performs controlled selection for mutation. Selects `NM` individuals from the
 * population, possibly cloning some of them and mutating others.
 */
template <std::uint32_t NP, std::uint32_t NM, std::uint32_t NE,
          std::uint32_t LS>
constexpr void controlled_selection_mutation(
    const std::array<std::unique_ptr<Solution>, NP> &population,
    std::vector<Solution *> *clones, std::array<Solution *, NM> *mutants) {
    auto end = population.cbegin() + NE;
    for (auto it = population.cbegin(); it != end; ++it) {
        if ((*it)->age() < LS) {
            clones->push_back(it->get());
        }
    }
    std::transform(population.cbegin(), population.cbegin() + NM,
                   mutants->begin(), [](const auto &sol) { return sol.get(); });
}

} // namespace optimizer

#endif

