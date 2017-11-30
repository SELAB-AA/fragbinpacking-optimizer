#ifndef UTIL_H_
#define UTIL_H_

#include <cstddef>
#include <iterator>
#include <limits>
#include <random>
#include <type_traits>

namespace optimizer {

/*
 * Similar to std::partition, but works on an already sorted range and the
 * predicate is inverted and there is an argument `count` for the maximum
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
            ++counter;
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
 * Generates a bounded uniform random variate.
 */
template <typename Rng>
constexpr std::uint32_t bounded_rand(std::uint32_t n, Rng &&gen) {
    static_assert(std::remove_reference<Rng>::type::max() ==
                          std::numeric_limits<std::uint32_t>::max() &&
                      std::remove_reference<Rng>::type::min() ==
                          std::numeric_limits<std::uint32_t>::min(),
                  "Range of Rng must be full");
    auto p = static_cast<std::uint64_t>(gen()) * n;
    if (static_cast<std::uint32_t>(p) < n) {
        const auto t = -n % n;
        while (static_cast<std::uint32_t>(p) < t) {
            p = static_cast<std::uint64_t>(gen()) * n;
        }
    }
    return static_cast<std::uint32_t>(p >> 32u);
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
    typedef typename std::remove_reference<Rng>::type::result_type result_t;
    for (auto d = std::distance(begin, end);
         n > Count() &&
         d > typename std::iterator_traits<RandIt>::difference_type();
         --n, --d, ++begin) {
        auto r = begin;
        std::advance(r, bounded_rand(static_cast<result_t>(d), gen));
        std::swap(*begin, *r);
    }
    return begin;
}

/*
 *  Reimplementation of std::shuffle.
 */
template <typename RandIt, typename Rng>
constexpr void shuffle(RandIt begin, RandIt end, Rng &&gen) {
    typedef typename std::iterator_traits<RandIt>::difference_type delta_t;
    typedef typename std::remove_reference<Rng>::type::result_type result_t;
    static_assert(std::is_same<std::uint32_t, result_t>::value, "Wrong type");
    auto count = std::distance(begin, end);
    while (count > typename std::iterator_traits<RandIt>::difference_type(1)) {
        const auto chosen = static_cast<delta_t>(
            bounded_rand(static_cast<result_t>(count--), gen));
        using std::swap;
        swap(*(begin + chosen), *--end);
    }
}

} // namespace optimizer

#endif

