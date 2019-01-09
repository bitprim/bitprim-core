#!/bin/bash
#
# Copyright (c) 2017 Bitprim developers (see AUTHORS)
#
# This file is part of Bitprim.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

hpps=$(find include -name \*\.hpp)
cpps=$(find src -name \*\.cpp)

for f in ${hpps}
do
#   echo $f
  diff -u <(cat ${f}) <(clang-format -style=file ${f})
done

for f in ${cpps}
do
#   echo $f
  diff -u <(cat ${f}) <(clang-format -style=file ${f})
done



# diff -u <(cat src/math/secp256k1_initializer.cpp) <(clang-format -style=file src/math/secp256k1_initializer.cpp)


# diff -u <(cat ${cpps}) <(clang-format ${cpps})
# diff -u <(cat src/*) <(clang-format src/*)
# diff -u <(cat include/*) <(clang-format include/*)

