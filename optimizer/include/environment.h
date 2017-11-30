#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <climits>
#include <cstdint>
#include <random>

#include <pcg_random.hpp>

namespace optimizer {

/*
 * The `Environment` class contains random state and random bit generators.
 */
class Environment {
    std::random_device rd_;
    pcg32_fast::state_type seed_;
    pcg32_fast rng_;

 public:
    explicit Environment(pcg32_fast::state_type seed)
        : rd_{}, seed_{seed}, rng_{seed_} {}
#if UINT_MAX >= UINT64_MAX
    Environment() : rd_{}, seed_(rd_()), rng_{seed_} {}
    void reseed() { reseed(rd_()); }
#elif UINT_MAX >= UINT32_MAX
    Environment()
        : rd_{},
          seed_(static_cast<pcg32_fast::state_type>(rd_()) << 32u | rd_()),
          rng_{seed_} {}
    void reseed() {
        reseed(static_cast<pcg32_fast::state_type>(rd_()) << 32u | rd_());
    }
#else
#error Does not compute
#endif
    pcg32_fast *rng() { return &rng_; }
    pcg32_fast::state_type seed() const { return seed_; }
    void reseed(pcg32_fast::state_type seed) {
        seed_ = seed;
        rng_.seed(seed);
    }
};

} // namespace optimizer

#endif

