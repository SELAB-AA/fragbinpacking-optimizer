#ifndef UTIL_H_
#define UTIL_H_

#include <cstddef>
#include <iterator>
#include <random>

namespace optimizer {

/*
 * Similar to std::partition, but works on an already sorted range and the
 * predicate is inverted and there is an argument `cpimt` for the maximum
 * nummber of unique elements to find before returning.
*
 * Input sorted range and count of maximum number of unique entries to find.
 * Optional binary predicate for determining if values are equal.
 *
 * Returns an iterator past the range of unique items.
 */
template <class ForwardIt, typename Count, class BinaryPredicate>
constexpr ForwardIt dedup(ForwardIt first, ForwardIt last, Count count,
                          BinaryPredicate p = [](const auto &l, const auto &r) {
                              return l == r;
                          }) {
    Count counter{};

    if (first == last || count == counter) {
        return last;
    }

    auto result = first;

    while (++first != last && counter < count) {
        if (!p(*result, *first)) {
            std::swap(*++result, *first);
        } else {
            counter++;
        }
    }

    return ++result;
}

/*
 * Frequency counter for sorted ranges of items.
 *
 * Input sorted range of items.
 *
 * Output std::vector<ItemCount> of unique items and counts.
 */
template <class RandIt>
std::vector<ItemCount> fcount(RandIt begin, RandIt end) {
    std::vector<ItemCount> result;
    result.reserve(std::distance(begin, end));
    while (begin != end) {
        auto r = std::equal_range(
            begin, end, *begin,
            [](const auto &lhs, const auto &rhs) { return lhs > rhs; });
        result.emplace_back(*r.first, std::distance(r.first, r.second));
        begin = r.second;
    }
    result.shrink_to_fit();
    return result;
}

/*
 * Similar to std::partition and std::shuffle. Randomly moves n elements
 * in order to the beginning of the range.
 *
 * Input range of elements, desired number of unique elements, and a random
 * number generator.
 *
 * Returns an iterator to the end of the unique range.
 */
template <class RandIt, class Count, class Rng>
constexpr RandIt sample_inplace(RandIt begin, RandIt end, Count n, Rng &&gen) {
    for (auto d = std::distance(begin, end); n > 0 && d > 0;
         --n, --d, ++begin) {
        auto r = begin;
        std::advance(r, gen(d));
        std::swap(*begin, *r);
    }
    return begin;
}

} // namespace optimizer

#endif
