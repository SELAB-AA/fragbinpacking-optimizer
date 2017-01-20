#include <assert.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <vector>

#include "environment.h"
#include "problem.h"
#include "solution.h"
#include "solver.h"

const std::uint32_t POPULATION_SIZE = 100;

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Too few arguments.\n";
        return -1;
    }

    if (argc > 3) {
        std::cerr << "Too many arguments.\n";
        return -1;
    }

    auto item_count = std::strtoul(argv[1], nullptr, 0);

    if (!item_count) {
        std::cerr << "Bad number of items.\n";
        return -1;
    }

    auto bin_capacity = std::strtoul(argv[2], nullptr, 0);

    if (!bin_capacity) {
        std::cerr << "Bad bin capacity.\n";
        return -1;
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    optimizer::Environment env;
    std::cout << "Seed: " << env.seed() << '\n';

    std::uniform_int_distribution<std::uint32_t> size_dist(1, bin_capacity);

    std::array<std::unique_ptr<optimizer::Solution>, POPULATION_SIZE>
        population;

    std::vector<std::uint32_t> item_sizes(item_count);

    std::generate(item_sizes.begin(), item_sizes.end(),
                  [&] { return size_dist(*env.rng()); });

    // env.reseed();
    // std::cout << "random: " << (*env.rng())() << "\n";

    optimizer::Solution best_solution;
    auto gen = std::uint32_t{};

    start = std::chrono::high_resolution_clock::now();

    optimizer::Problem problem(&env, item_sizes.cbegin(), item_sizes.cend(),
                               bin_capacity);

    if (problem.item_count()) {
        auto found_optimal = false;

        for (auto i = 0u; i < population.size(); ++i) {
            population[i] = problem.generate_individual();

            if (problem.bin_count() - population[i]->size() ==
                problem.lower_bound()) {
                best_solution = std::move(*population[i]);
                found_optimal = true;
                break;
            }
        }

        if (!found_optimal) {
            std::sort(population.begin(), population.end(),
                      [](const std::unique_ptr<optimizer::Solution> &l,
                         const std::unique_ptr<optimizer::Solution> &r) {
                          return l->size() > r->size();
                      });
            best_solution = optimizer::Solver<POPULATION_SIZE>(&problem).solve(
                &population, &gen);
        }
    }

    end = std::chrono::high_resolution_clock::now();

    std::cout << best_solution << '\n';

    std::cout << "Generations: " << gen << '\n';

    std::cout << "Best: " << problem.bin_count() - best_solution.size()
              << " cuts (" << best_solution.size() << " blocks)\n";
    std::cout << "lower bound: " << problem.lower_bound() << '\n';

    std::chrono::duration<double> elapsed_seconds = end - start;

    std::cout << "Elapsed time: " << elapsed_seconds.count() << " s\n";

    std::cout << "OptGap: "
              << static_cast<double>(problem.original_item_count() +
                                     problem.original_slack() +
                                     problem.lower_bound()) /
                     (problem.original_item_count() + problem.original_slack() +
                      problem.bin_count() - best_solution.size())
              << '\n';
    auto nominator =
        problem.item_count() + problem.slack() + problem.lower_bound();
    auto denominator = problem.item_count() + problem.slack() +
                       problem.bin_count() - best_solution.size();
    std::cout << "OptGap (reduced): "
              << static_cast<double>(nominator ? nominator : 1) /
                     (denominator ? denominator : 1)
              << '\n';

    if (problem.bin_count() - best_solution.size() == problem.lower_bound()) {
        std::cout << "===OPTIMAL==\n";
    }
}
