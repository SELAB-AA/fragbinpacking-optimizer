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
    if (unlikely(!r || begin == end)) {
        return;
    }

    const auto ub_a = r - 2u * (--end)->size;
    const auto lb_a = 1u + (r - 1u) / 3u;

    for (begin += begin->size > ub_a ? 1u : 0u; begin < end; ++begin) {
        if (unlikely(begin->size < lb_a)) {
            break;
        }

        auto left = begin;
        auto right = end;
        const auto trg = r - begin->size;

        do {
            const auto t = left->size + right->size;
            if (unlikely(t == trg)) {
                if (begin->size == left->size) {
                    if (left->size == right->size) {
                        out->emplace_back(&*left, &*left, &*left);
                    } else {
                        out->emplace_back(&*left, &*left, &*right);
                    }
                } else {
                    if (left->size == right->size) {
                        out->emplace_back(&*begin, &*left, &*left);
                    } else {
                        out->emplace_back(&*begin, &*left, &*right);
                    }
                }
                ++left;
            } else if (t > trg) {
                ++left;
            } else {
                --right;
            }
        } while (left <= right);
    }
}

} // namespace optimizer

#undef likely
#undef unlikely

#endif
