#ifndef THREESUM_H_
#define THREESUM_H_

#include <cstddef>
#include <numeric>
#include <vector>

#include "item.h"

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) x
#define unlikely(x) x
#endif

namespace optimizer {

/*
 * A 3-Partition of an integer.
 */
class Partition {
    std::array<ItemCount *, 3u> items_;

 public:
    constexpr Partition(ItemCount *a, ItemCount *b, ItemCount *c)
        : items_{{a, b, c}} {}
    constexpr const std::array<ItemCount *, 3u> &items() const {
        return items_;
    }
};

/*
 * Computes the possible 3-partitions of a number indicating bin capacity.
 */
template <class RandIt, class T>
constexpr void threesum(RandIt begin, RandIt end, T *out,
                        std::uint32_t bin_count, std::uint32_t capacity) {
    const auto r = bin_count * capacity;

    assert(r && begin <= --end);

    while (end->size + 2u * begin->size < r) {
        if (--end < begin) {
            return;
        }
    }

    for (; begin <= end; --end) {
        while (begin->size + 2u * end->size > r) {
            if (++begin > end) {
                return;
            }
        }

        auto p_a = begin;
        auto p_b = end;
        const auto target = r - end->size;

        do {
            const auto t = p_a->size + p_b->size;
            if (t < target) {
                --p_b;
            } else {
                if (unlikely(t == target)) {
                    out->emplace_back(&*p_a, &*p_b, &*end);
                }
                ++p_a;
            }
        } while (p_a <= p_b);
    }
}

} // namespace optimizer

#undef likely
#undef unlikely

#endif
