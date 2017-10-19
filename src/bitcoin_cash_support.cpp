/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
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

#include <iostream>
#include <bitcoin/bitcoin/bitcoin_cash_support.hpp>

namespace libbitcoin {

namespace {

static bool already_set_max_size = false;
static bool is_bitcoin_cash_ = false;
static size_t max_block_size = 1000000;
static size_t max_block_sigops = max_block_size / libbitcoin::max_sigops_factor;
} // namespace anonymous


//inline
bool is_bitcoin_cash() {
    return is_bitcoin_cash_;
}

//inline
void set_bitcoin_cash(bool value) {
    is_bitcoin_cash_ = value;
    if(is_bitcoin_cash_ && !already_set_max_size)
        set_max_block_size(8000000);
}

void set_max_block_size(size_t value) {
    max_block_size = value;
    max_block_sigops = max_block_size / libbitcoin::max_sigops_factor;
    already_set_max_size = true;
}

size_t get_max_block_size() {
    return max_block_size;
}

void set_max_block_sigops (size_t value ){
    max_block_sigops = value;
}

size_t get_max_block_sigops (){
    return max_block_sigops;
}

size_t get_next_block_size(const size_t ec_block_size){
    size_t next_block_size = max_block_size;
    while(next_block_size <  ec_block_size)
    {
        next_block_size = next_block_size * 2;    
    }

    return next_block_size;
}

size_t get_allowed_sigops(const size_t ec_block_size)
{
    return (get_next_block_size(ec_block_size) / libbitcoin::max_sigops_factor);
}

} /*namespace libbitcoin*/

