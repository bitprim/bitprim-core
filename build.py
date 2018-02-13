import os
from conan.packager import ConanMultiPackager
import copy

if __name__ == "__main__":
    # print('-*-*-*-*-* FROM PYTHON -*-*-*-*-*-*-*')
    # print(os.getenv('BITPRIM_BUILD_NUMBER', '-'))
    # print('-*-*-*-*-* FROM PYTHON -*-*-*-*-*-*-*')

    builder = ConanMultiPackager(username="bitprim", channel="testing",
                                 remotes="https://api.bintray.com/conan/bitprim/bitprim",
                                 archs=["x86_64"])

    builder.add_common_builds(shared_option_name="bitprim-core:shared")

    filtered_builds = []
    for settings, options, env_vars, build_requires in builder.builds:
        if settings["build_type"] == "Release" and not options["bitprim-core:shared"]:

            # print(env_vars)
            env_vars["BITPRIM_BUILD_NUMBER"] = os.getenv('BITPRIM_BUILD_NUMBER', '-')
            # print(env_vars)

            if os.getenv('BITPRIM_RUN_TESTS', 'false') == 'true':
                options["bitprim-core:with_tests"] = "True"
                options["bitprim-core:with_examples"] = "True"

            opts_bch = copy.deepcopy(options)
            opts_btc = copy.deepcopy(options)
            opts_ltc = copy.deepcopy(options)

            opts_bch["bitprim-core:currency"] = "BCH"
            opts_btc["bitprim-core:currency"] = "BTC"
            opts_ltc["bitprim-core:currency"] = "LTC"

            filtered_builds.append([settings, opts_bch, env_vars, build_requires])
            filtered_builds.append([settings, opts_btc, env_vars, build_requires])
            filtered_builds.append([settings, opts_ltc, env_vars, build_requires])


    builder.builds = filtered_builds
    builder.run()
