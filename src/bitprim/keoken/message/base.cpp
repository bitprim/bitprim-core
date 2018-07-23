/**
 * Copyright (c) 2017-2018 Bitprim Inc.
 *
 * This file is part of Bitprim.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitprim/keoken/message/base.hpp>

// #include <bitcoin/bitcoin/utility/container_sink.hpp>
// #include <bitcoin/bitcoin/utility/container_source.hpp>
// #include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>


namespace bitprim {
namespace keoken {
namespace message {
namespace base {

using namespace bc;

size_t serialized_size() {
    return sizeof(uint16_t) + sizeof(uint16_t);
}

void to_data(writer& sink, uint16_t version, uint16_t type) {
    sink.write_2_bytes_big_endian(version);
    sink.write_2_bytes_big_endian(type);
}

} // namespace base
} // namespace message
} // namespace keoken
} // namespace bitprim
