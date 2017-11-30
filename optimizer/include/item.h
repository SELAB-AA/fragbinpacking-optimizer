#ifndef ITEM_H_
#define ITEM_H_

#include <cstdint>
#include <iostream>

namespace optimizer {

/*
 * Simple pair type with a key and a count.
 */
struct ItemCount {
    std::uint32_t size;
    std::uint32_t count;
    ItemCount() : size{}, count{} {}
    constexpr ItemCount(std::uint32_t s, std::uint32_t c) : size{s}, count{c} {}
    friend std::ostream &operator<<(std::ostream &os, const ItemCount &ic) {
        os << ic.size << '^' << ic.count;
        return os;
    }
};

} // namespace optimizer

#endif

