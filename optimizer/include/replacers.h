#ifndef REPLACERS_H_
#define REPLACERS_H_

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "solution.h"
#include "util.h"

namespace optimizer {

/*
 * Performs controlled replacement of solutions in the population from
 * progeny produced by grouping crossover and the random individuals.
 * Template parameters are `NP`, the population size, `NC` the number
 * of individuals to undergo crossover, and `NE` is the size of the elite set.
 */
template <std::uint32_t NP, std::uint32_t NC, std::uint32_t NE>
void controlled_replacement_crossover(
    std::array<std::unique_ptr<Solution>, NP> *population,
    std::array<std::unique_ptr<Solution>, NC> *progeny,
    std::array<Solution *, NC / 2u> *r) {
    std::sort(r->begin(), r->end());
    std::sort(population->begin() + NE, population->end());

    std::array<std::unique_ptr<Solution>, NP - NC / 2u - NE> p_minus_r_minus_b;
    std::set_difference(
        std::make_move_iterator(population->begin() + NE),
        std::make_move_iterator(population->end()), r->cbegin(), r->cend(),
        p_minus_r_minus_b.begin(),
        [](const auto &left, const auto &right) { return &*left < &*right; });

    std::sort(p_minus_r_minus_b.begin(), p_minus_r_minus_b.end(),
              [](const auto &left, const auto &right) {
                  return left->size() > right->size();
              });

    auto it = dedup(p_minus_r_minus_b.begin(), p_minus_r_minus_b.end(), NC / 2u,
                    [](const auto &left, const auto &right) {
                        return left->size() == right->size();
                    });

    if (static_cast<std::uint32_t>(p_minus_r_minus_b.end() - it) < NC / 2u) {
        it -= NC / 2u - (p_minus_r_minus_b.end() - it);
    }

    std::move(progeny->begin() + NC / 2u, progeny->end(), it);

    std::move(progeny->begin(), progeny->begin() + NC / 2u,
              population->begin() + NE);
    std::move(p_minus_r_minus_b.begin(), p_minus_r_minus_b.end(),
              population->begin() + NE + NC / 2u);
    std::sort(population->begin() + NE, population->end(),
              [](const auto &left, const auto &right) {
                  return left->size() > right->size();
              });
    std::inplace_merge(population->begin(), population->begin() + NE,
                       population->end(),
                       [](const auto &left, const auto &right) {
                           return left->size() > right->size();
                       });
}

/*
 * Performs controlled replacement of cloned individuals into the population.
 * The parameter `NP` indicates the size of the population.
 */
template <std::uint32_t NP>
void controlled_replacement_mutation(
    std::array<std::unique_ptr<Solution>, NP> *population,
    std::vector<std::unique_ptr<Solution>> *cloned) {
    std::sort(cloned->begin(), cloned->end(),
              [](const auto &left, const auto &right) {
                  return left->size() > right->size();
              });

    auto it = dedup(
        population->begin(), population->end(), cloned->size(),
        [](const auto &l, const auto &r) { return l->size() == r->size(); });

    if (static_cast<std::uint32_t>(population->end() - it) < cloned->size()) {
        it -= cloned->size() - (population->end() - it);
    }

    std::move(cloned->begin(), cloned->end(), it);

    std::inplace_merge(population->begin(), it, it + cloned->size(),
                       [](const auto &left, const auto &right) {
                           return left->size() > right->size();
                       });
    std::inplace_merge(population->begin(), it += cloned->size(),
                       population->end(),
                       [](const auto &left, const auto &right) {
                           return left->size() > right->size();
                       });
}

} // namespace optimizer

#endif
