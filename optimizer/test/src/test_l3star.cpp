#include <iostream>
#include <utility>

#include "environment.h"
#include "problem.h"

static std::uint32_t
test_lb(optimizer::Environment *env,
        const std::vector<std::pair<std::uint32_t, std::uint32_t>> &data,
        std::uint32_t c, std::uint32_t m) {
    std::vector<std::uint32_t> v;
    for (const auto &e : data) {
        std::fill_n(std::back_inserter(v), e.second, e.first);
    }
    return optimizer::Problem(env, v.cbegin(), v.cend(), c, m).lower_bound();
}

int main() {
    optimizer::Environment env;

    std::cout << test_lb(&env,
                         {std::make_pair(1u, 10u), std::make_pair(2u, 4u),
                          std::make_pair(3u, 22u), std::make_pair(4u, 1u)},
                         8u, 11u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(2u, 4u), std::make_pair(4u, 1u),
                                std::make_pair(7u, 4u)},
                         8u, 5u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(2u, 4u), std::make_pair(4u, 1u)},
                         8u, 2u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(2u, 4u), std::make_pair(4u, 1u)},
                         8u, 3u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(1u, 12u), std::make_pair(2u, 2u),
                                std::make_pair(3u, 40u)},
                         8u, 17u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(2u, 5u)}, 5u, 2u) << '\n';
    std::cout << test_lb(&env,
                         {std::make_pair(1u, 28u), std::make_pair(3u, 12u),
                          std::make_pair(6u, 8u)},
                         8u, 14u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(5u, 4u), std::make_pair(6u, 5u),
                                std::make_pair(7u, 2u)},
                         8u, 8u)
              << '\n';
    std::cout << test_lb(&env,
                         {std::make_pair(3u, 1u), std::make_pair(7u, 1u),
                          std::make_pair(11u, 1u), std::make_pair(33u, 3u),
                          std::make_pair(50u, 1u), std::make_pair(60u, 1u),
                          std::make_pair(70u, 1u)},
                         100u, 3u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(3u, 5u), std::make_pair(6u, 1u),
                                std::make_pair(7u, 5u)},
                         8u, 7u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(7u, 8u)}, 8u, 7u) << '\n';
    std::cout << test_lb(&env, {std::make_pair(3u, 2u), std::make_pair(7u, 6u)},
                         8u, 6u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(2u, 1u), std::make_pair(4u, 3u),
                                std::make_pair(6u, 6u), std::make_pair(8u, 5u)},
                         9u, 10u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(2u, 18u)}, 9u, 4u) << '\n';
    std::cout << test_lb(&env,
                         {std::make_pair(1u, 16u), std::make_pair(2u, 24u),
                          std::make_pair(3u, 64u)},
                         16u, 16u)
              << '\n';
    std::cout << test_lb(&env,
                         {std::make_pair(2u, 8u), std::make_pair(3u, 64u)}, 16u,
                         13u)
              << '\n';
    std::cout << test_lb(&env,
                         {std::make_pair(2u, 8u), std::make_pair(3u, 16u)}, 16u,
                         4u)
              << '\n';
    std::cout << test_lb(&env, {std::make_pair(5u, 16u)}, 16u, 5u) << '\n';
    std::cout << test_lb(&env, {std::make_pair(2u, 1000u),
                                std::make_pair(33u, 6000u)},
                         100u, 2000u)
              << '\n';
    std::cout << test_lb(&env,
                         {std::make_pair(3u, 200u), std::make_pair(5u, 200u)},
                         16u, 100u)
              << '\n';

    return 0;
}

