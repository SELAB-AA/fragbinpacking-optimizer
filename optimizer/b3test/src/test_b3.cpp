#include <assert.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <vector>

#include "environment.h"
#include "problem.h"
#include "solution.h"
#include "solver.h"

int main() {
    optimizer::Environment env;
    auto item_counts =
        std::array<std::uint32_t, 6u>{{1000, 2000, 4000, 8000, 16000, 32000}};

    std::ofstream of;
    of.open("b3_time.tsv", std::ios::out | std::ios::trunc);
    assert(of.is_open());

    of << "M\tM'\tN\tN'\tW\tT\n";

    for (const auto &item_count : item_counts) {
        const auto &bin_capacity = item_count;
        std::uniform_int_distribution<std::uint32_t> size_dist(1u,
                                                               bin_capacity);
        for (auto i = 0u; i < 5u; ++i) {
            std::vector<std::uint32_t> item_sizes(item_count);
            std::generate(item_sizes.begin(), item_sizes.end(),
                          [&size_dist, &env] { return size_dist(*env.rng()); });
            optimizer::Problem problem(&env, item_sizes.cbegin(),
                                       item_sizes.cend(), bin_capacity);
            optimizer::Solution s{};
            auto items(problem.items());
            auto slack = problem.slack();
            auto actual_item_count = problem.item_count();
            s.items().reserve(problem.item_count());
            s.blocks().reserve((problem.item_count() + slack) / 3u);
            std::chrono::time_point<std::chrono::high_resolution_clock> start =
                std::chrono::high_resolution_clock::now();
            problem.b3(items.begin(), items.end(), &slack, &actual_item_count,
                       &s);
            std::chrono::duration<double> duration =
                std::chrono::high_resolution_clock::now() - start;
            of << problem.original_bin_count() << '\t' << problem.bin_count()
               << '\t' << problem.original_item_count() << '\t'
               << problem.item_count() << '\t' << problem.unique_size_count()
               << '\t' << duration.count() << '\n';
        }
    }
    of.close();
}

