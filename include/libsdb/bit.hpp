//
// Created by yax on 17/01/26.
//

#ifndef SDB_BIT_HPP
#define SDB_BIT_HPP

#include <cstddef>
#include <cstring>

#include <libsdb/types.hpp>

namespace sdb {
    template<typename To>
    auto from_bytes(const std::byte *bytes) -> To {
        To ret;
        std::memcpy(&ret, bytes, sizeof(To));
        return ret;
    }

    template<typename From>
    auto as_bytes(From &from) -> std::byte * {
        return reinterpret_cast<std::byte *>(&from);
    }

    template<typename From>
    auto as_bytes(const From &from) -> const std::byte * {
        return reinterpret_cast<const std::byte *>(&from);
    }

    template<typename From>
    auto to_byte12(From src) -> byte128 {
        byte128 ret{};
        std::memcpy(&ret, &src, sizeof(From));
        return ret;
    }
    template<typename From>
    auto to_byte64(From src) -> byte64 {
        byte64 ret{};
        std::memcpy(&ret, &src, sizeof(From));
        return ret;
    }
}

#endif //SDB_BIT_HPP
