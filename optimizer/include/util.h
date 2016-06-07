#ifndef UTIL_H_
#define UTIL_H_

#include <cstddef>
#include <iterator>
#include <random>

template<class bidiiter, class rng>
inline bidiiter random_unique(bidiiter begin, bidiiter end, size_t n, rng &&gen) {
        size_t left = std::distance(begin, end);
        std::uniform_int_distribution<size_t> dist(0, left - 1);

        while (n--) {
                bidiiter r = begin;
                std::advance(r, dist(gen));
                std::swap(*begin++, *r);
                dist.param(decltype(dist)::param_type(0, --left - 1));
        }

        return begin;
}

#endif
