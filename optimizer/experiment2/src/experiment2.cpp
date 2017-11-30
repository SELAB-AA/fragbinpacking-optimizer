#include <assert.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "environment.h"
#include "problem.h"
#include "solution.h"
#include "solver.h"

int main() {
    optimizer::Environment env;

    const std::uint32_t runs = 10;
    const std::uint32_t NP = 100;

    std::array<std::string, 6> names{
        {"bc_if", "bc_il", "bc_is", "bi_if", "bi_il", "bi_is"}};
    std::array<uint32_t, 3> counts{{10, 15, 20}};

    for (const auto &name : names) {
        for (const auto &n : counts) {
            for (auto y = 0u; y < 10u; ++y) {
                std::stringstream ss;
                std::stringstream ns;
                ns << name << "/bpp_" << n << '_' << y;
                std::string ns_s(ns.str());
                ss << "test_instances/" << ns_s << ".dat";
                std::vector<std::uint32_t> items;
                items.reserve(n);
                std::ifstream is(ss.str());
                std::string s;
                for (auto i = 0u; i < 3u; ++i) {
                    std::getline(is >> std::ws, s);
                }

                std::uint32_t bin_count, c;

                if (std::getline(is, s)) {
                    std::stringstream strstr(s);
                    std::string q;

                    strstr >> q >> q >> q >> bin_count;
                } else {
                    return -1;
                }

                if (std::getline(is, s)) {
                    std::stringstream strstr(s);
                    std::string q;

                    strstr >> q >> q >> q >> c;
                } else {
                    return -1;
                }

                for (; std::getline(is >> std::ws, s);) {
                    std::stringstream strstr(s);
                    auto p = strstr.peek();
                    if ('0' <= p && p <= '9') {
                        std::uint32_t x;
                        auto skip = false;

                        while (strstr >> x) {
                            skip = !skip;
                            if (!skip) {
                                items.push_back(x);
                            }
                        }
                    }
                }

                auto e1e2_start = std::chrono::high_resolution_clock::now();
                optimizer::Problem problem(&env, items.cbegin(), items.cend(),
                                           c, bin_count);
                std::chrono::duration<double> duration_e1e2 =
                    std::chrono::high_resolution_clock::now() - e1e2_start;
                for (auto i = 0u; i < runs; ++i) {
                    // reset seed
                    env.reseed();
                    auto seed = env.seed();

                    auto g_start = std::chrono::high_resolution_clock::now();
                    // run g
                    auto solution_g = problem.generate_individual<false>();
                    std::chrono::duration<double> duration_g =
                        std::chrono::high_resolution_clock::now() - g_start;

                    // reset seed
                    env.reseed(seed);

                    auto b3g_start = std::chrono::high_resolution_clock::now();
                    // run b3g
                    auto solution_b3g = problem.generate_individual();
                    std::chrono::duration<double> duration_b3g =
                        std::chrono::high_resolution_clock::now() - b3g_start;

                    // reset seed
                    env.reseed(seed);

                    // do genetic
                    std::array<std::unique_ptr<optimizer::Solution>, NP>
                        population;
                    optimizer::Solution solution_stage1;
                    auto found_optimal = false;

                    auto stage1_start =
                        std::chrono::high_resolution_clock::now();
                    for (auto j = 0u; j < population.size(); ++j) {
                        population[j] = problem.generate_individual();

                        if (problem.bin_count() - population[j]->size() ==
                            problem.lower_bound()) {
                            solution_stage1 = std::move(*population[j]);
                            found_optimal = true;
                            break;
                        }
                    }

                    std::chrono::duration<double> duration_stage1;

                    if (!found_optimal) {
                        std::sort(
                            population.begin(), population.end(),
                            [](const std::unique_ptr<optimizer::Solution> &left,
                               const std::unique_ptr<optimizer::Solution>
                                   &right) {
                                return left->size() > right->size();
                            });
                        solution_stage1 = *population[0];
                        duration_stage1 =
                            std::chrono::high_resolution_clock::now() -
                            stage1_start;
                    } else {
                        duration_stage1 =
                            std::chrono::high_resolution_clock::now() -
                            stage1_start;
                    }

                    auto solution_stage2(solution_stage1);

                    std::chrono::duration<double> duration_stage2{};

                    auto gen = std::uint32_t{};

                    std::vector<std::uint32_t> blocks_over_time;
                    blocks_over_time.push_back(solution_stage1.size());

                    if (!found_optimal) {
                        auto stage2_start =
                            std::chrono::high_resolution_clock::now();
                        solution_stage2 = optimizer::Solver<NP>(&problem).solve(
                            &population, &gen, &blocks_over_time);
                        duration_stage2 =
                            std::chrono::high_resolution_clock::now() -
                            stage2_start;
                    }

                    std::stringstream dat_name;
                    dat_name << "results/" << ns_s << '_' << i << ".dat";
                    std::stringstream gen_name;
                    gen_name << "results/" << ns_s << '_' << i << ".gen";

                    std::ofstream of;
                    of.open(dat_name.str(), std::ios::out | std::ios::trunc);

                    assert(of.is_open());

                    of << "# Seed: " << seed << '\n'
                       << "# Item count before reduction: " << n << '\n'
                       << "# Item count after reduction: "
                       << problem.item_count() << '\n'
                       << "# Time spent in reduction: " << duration_e1e2.count()
                       << '\n'
                       << "# Bin count: " << problem.bin_count() << '\n'
                       << "# Lower bound: " << problem.lower_bound() << '\n'
                       << "# Upper bound: " << problem.bin_count() - 1 << '\n'
                       << "# \n"
                       << "# Format:\n"
                       << "# blocks splits duration\n"
                       << "# \n"
                       << "# Order:\n"
                       << "# G\n"
                       << "# B3G\n"
                       << "# FFF Stage 1\n"
                       << "# FFF Stage 2\n"
                       << solution_g->size() << ' '
                       << problem.bin_count() - solution_g->size() << ' '
                       << duration_g.count() << '\n'
                       << solution_b3g->size() << ' '
                       << problem.bin_count() - solution_b3g->size() << ' '
                       << duration_b3g.count() << '\n'
                       << solution_stage1.size() << ' '
                       << problem.bin_count() - solution_stage1.size() << ' '
                       << duration_stage1.count() << '\n'
                       << solution_stage2.size() << ' '
                       << problem.bin_count() - solution_stage2.size() << ' '
                       << duration_stage2.count() << '\n';

                    of.close();

                    of.open(gen_name.str(), std::ios::out | std::ios::trunc);

                    assert(of.is_open());

                    of << "# Blocks for generations of FFF, including "
                          "generation 0\n";

                    for (const auto &count : blocks_over_time) {
                        of << count << '\n';
                    }

                    of.close();
                }
            }
        }
    }

    return 0;
}

