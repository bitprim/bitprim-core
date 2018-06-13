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

import os
from conans import ConanFile, CMake
from conans import __version__ as conan_version
from conans.model.version import Version

def option_on_off(option):
    return "ON" if option else "OFF"

def get_content(file_name):
    file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), file_name)
    with open(file_path, 'r') as f:
        return f.read().replace('\n', '').replace('\r', '')

def get_version():
    return get_content('conan_version')

def get_channel():
    return get_content('conan_channel')

def get_conan_req_version():
    return get_content('conan_req_version')


class BitprimCoreConan(ConanFile):
    name = "bitprim-core"
    version = get_version()
    license = "http://www.boost.org/users/license.html"
    url = "https://github.com/bitprim/bitprim-core"
    description = "Bitcoin Cross-Platform C++ Development Toolkit"
    settings = "os", "compiler", "build_type", "arch"

    if conan_version < Version(get_conan_req_version()):
        raise Exception ("Conan version should be greater or equal than %s" % (get_conan_req_version(), ))

    options = {"shared": [True, False],
               "fPIC": [True, False],
               "with_icu": [True, False],
               "with_qrencode": [True, False],
               "with_tests": [True, False],
               "with_examples": [True, False],
            #    "currency": "ANY" #["BCH", "BTC", "LTC", ...]
               "currency": ['BCH', 'BTC', 'LTC']
    }

        # "with_litecoin": [True, False],
    #    "with_png": [True, False],

    default_options = "shared=False", \
        "fPIC=True", \
        "with_icu=False", \
        "with_qrencode=False", \
        "with_tests=False", \
        "with_examples=False", \
        "currency=BCH"

        # "with_litecoin=False", \
        # "with_png=False", \

    generators = "cmake"
    exports = "conan_channel", "conan_version", "conan_req_version"
    exports_sources = "src/*", "CMakeLists.txt", "cmake/*", "bitprim-coreConfig.cmake.in", "bitprimbuildinfo.cmake", "include/*", "test/*", "examples/*"
    package_files = "build/lbitprim-core.a"
    build_policy = "missing"

    requires = (("boost/1.66.0@bitprim/stable"),
               ("secp256k1/0.3@bitprim/stable"))

    @property
    def msvc_mt_build(self):
        return "MT" in str(self.settings.compiler.runtime)

    @property
    def fPIC_enabled(self):
        if self.settings.compiler == "Visual Studio":
            return False
        else:
            return self.options.fPIC

    @property
    def is_shared(self):
        if self.options.shared and self.msvc_mt_build:
            return False
        else:
            return self.options.shared

    def requirements(self):
        # if self.options.with_png:
        #     self.requires("libpng/1.6.34@bitprim/stable")
            
        if self.options.currency == "LTC":
             self.requires("OpenSSL/1.0.2l@conan/stable")
            

        if self.options.with_qrencode:
            self.requires("libqrencode/4.0.0@bitprim/stable")

    def config_options(self):
        self.output.info('def config_options(self):')
        if self.settings.compiler == "Visual Studio":
            self.options.remove("fPIC")

            if self.options.shared and self.msvc_mt_build:
                self.options.remove("shared")

    def package_id(self):
        self.info.options.with_tests = "ANY"
        self.info.options.with_examples = "ANY"

        #For Bitprim Packages libstdc++ and libstdc++11 are the same
        if self.settings.compiler == "gcc" or self.settings.compiler == "clang":
            if str(self.settings.compiler.libcxx) == "libstdc++" or str(self.settings.compiler.libcxx) == "libstdc++11":
                self.info.settings.compiler.libcxx = "ANY"

    def build(self):
        for dep in self.deps_cpp_info.deps:
            # self.output.warn(self.deps_cpp_info["MyLib"].libdirs)
            print(dep)
            print(self.options[dep])

            # self.options["boost"]

        cmake = CMake(self)
        cmake.definitions["USE_CONAN"] = option_on_off(True)
        cmake.definitions["NO_CONAN_AT_ALL"] = option_on_off(False)
        
        # cmake.definitions["CMAKE_VERBOSE_MAKEFILE"] = option_on_off(False)
        cmake.verbose = True

        cmake.definitions["ENABLE_SHARED"] = option_on_off(self.is_shared)
        cmake.definitions["ENABLE_POSITION_INDEPENDENT_CODE"] = option_on_off(self.fPIC_enabled)

        cmake.definitions["WITH_TESTS"] = option_on_off(self.options.with_tests)
        cmake.definitions["WITH_EXAMPLES"] = option_on_off(self.options.with_examples)

        # cmake.definitions["WITH_LITECOIN"] = option_on_off(self.options.with_litecoin)

        # cmake.definitions["WITH_LTC"] = option_on_off(self.options.currency == 'LTC')
        # cmake.definitions["WITH_BTC"] = option_on_off(self.options.currency == 'LTC')
        # cmake.definitions["WITH_LITECOIN"] = option_on_off(self.options.currency == 'LTC')

        cmake.definitions["CURRENCY"] = self.options.currency

        cmake.definitions["WITH_ICU"] = option_on_off(self.options.with_icu)
        cmake.definitions["WITH_QRENCODE"] = option_on_off(self.options.with_qrencode)
        # cmake.definitions["WITH_PNG"] = option_on_off(self.options.with_png)
        cmake.definitions["WITH_PNG"] = option_on_off(self.options.with_qrencode)
        

        if self.settings.compiler != "Visual Studio":
            # cmake.definitions["CONAN_CXX_FLAGS"] += " -Wno-deprecated-declarations"
            cmake.definitions["CONAN_CXX_FLAGS"] = cmake.definitions.get("CONAN_CXX_FLAGS", "") + " -Wno-deprecated-declarations"

        if self.settings.compiler == "Visual Studio":
            cmake.definitions["CONAN_CXX_FLAGS"] = cmake.definitions.get("CONAN_CXX_FLAGS", "") + " /DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE"


        # self.output.info("------------------------------------------------------")
        # self.output.info(self.settings.compiler)
        # self.output.info(self.settings.compiler.libcxx)
        # self.output.info("------------------------------------------------------")

        # if self.settings.compiler != "Visual Studio"
        if self.settings.compiler == "gcc":
            if float(str(self.settings.compiler.version)) >= 5:
                cmake.definitions["NOT_USE_CPP11_ABI"] = option_on_off(False)
            else:
                cmake.definitions["NOT_USE_CPP11_ABI"] = option_on_off(True)
        elif self.settings.compiler == "clang":
            if str(self.settings.compiler.libcxx) == "libstdc++" or str(self.settings.compiler.libcxx) == "libstdc++11":
                cmake.definitions["NOT_USE_CPP11_ABI"] = option_on_off(False)

        # if self.settings.compiler == "clang":
        #     self.output.info("compiler is clang")

        # if str(self.settings.compiler.libcxx) == "libstdc++":
        #     self.output.info("libcxx is libstdc++")

        # if str(self.settings.compiler.libcxx) == "libstdc++11":
        #     self.output.info("libcxx is libstdc++11")

        cmake.definitions["BITPRIM_BUILD_NUMBER"] = os.getenv('BITPRIM_BUILD_NUMBER', '-')
        cmake.configure(source_dir=self.source_folder)

        # self.output.info("CONAN_CXX_FLAGS: %s" % (cmake.definitions["CONAN_CXX_FLAGS"], ))
        # self.output.info("cmake.command_line: %s" % (cmake.command_line, ))

        cmake.build()

        #TODO(fernando): Cmake Tests and Visual Studio doesn't work
        if self.options.with_tests:
            cmake.test()
            # cmake.test(target="tests")

    def imports(self):
        self.copy("*.h", "", "include")

    def package(self):
        self.copy("*.h", dst="include", src="include")
        self.copy("*.hpp", dst="include", src="include")
        self.copy("*.ipp", dst="include", src="include")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.includedirs = ['include']
        self.cpp_info.libs = ["bitprim-core"]

        if self.settings.os == "Linux" or self.settings.os == "FreeBSD":
            self.cpp_info.libs.append("pthread")

        if self.settings.os == "Windows" and self.settings.compiler == "gcc": # MinGW
            self.cpp_info.libs.append("ws2_32")
            self.cpp_info.libs.append("wsock32")

        if not self.is_shared:
            self.cpp_info.defines.append("BC_STATIC")


