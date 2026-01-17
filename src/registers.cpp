//
// Created by yax on 17/01/26.
//

#include <libsdb/registers.hpp>
#include <libsdb/bit.hpp>
#include <libsdb/process.hpp>

#include <iostream>

auto sdb::registers::read(const register_info &info) const -> value {
    const auto bytes = as_bytes(this->data_);

    if (info.format == register_format::uint) {
        switch (info.size) {
            case 1:
                return from_bytes<std::uint8_t>(bytes + info.offset);
            case 2:
                return from_bytes<std::uint16_t>(bytes + info.offset);
            case 4:
                return from_bytes<std::uint32_t>(bytes + info.offset);
            case 8:
                return from_bytes<std::uint64_t>(bytes + info.offset);
            default: error::send("Unexpected register size " __FILE__);
        }
    }
    if (info.format == register_format::double_float) {
        return from_bytes<double>(bytes + info.offset);
    }
    if (info.format == register_format::long_double) {
        return from_bytes<long double>(bytes + info.offset);
    }
    if (info.format == register_format::vector and info.size == 8) {
        return from_bytes<byte64>(bytes + info.offset);
    }
    return from_bytes<byte128>(bytes + info.offset);
}

auto sdb::registers::write(const register_info &info, value val) -> void {
    auto bytes = as_bytes(this->data_);

    std::visit([&](auto &v) {
        if (sizeof(v) == info.size) {
            auto val_bytes = as_bytes(v);
            std::copy(val_bytes, val_bytes + sizeof(v), bytes + info.offset);
        } else {
            std::clog << "sdb::register::write called with "
                    "mismatched register and value sizes. "
                    __FILE__;
            std::terminate();
        }
    }, val);

    const auto aligned_offset = info.offset & ~0b111;
    this->proc_->write_user_area(aligned_offset, from_bytes<std::uint64_t>(bytes + info.offset));
}
