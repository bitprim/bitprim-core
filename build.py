# from conan.packager import ConanMultiPackager
# import copy
# import re
# import platform
import os
import cpuid
from ci_utils.utils import get_builder, handle_microarchs

if __name__ == "__main__":
    builder, name = get_builder()
    builder.add_common_builds(shared_option_name="%s:shared" % name)

    filtered_builds = []
    for settings, options, env_vars, build_requires, reference in builder.items:

        if settings["build_type"] == "Release" \
                and not options["%s:shared" % name]:

            env_vars["BITPRIM_BUILD_NUMBER"] = os.getenv('BITPRIM_BUILD_NUMBER', '-')

            if os.getenv('BITPRIM_RUN_TESTS', 'false') == 'true':
                options["%s:with_tests" % name] = "True"
                options["%s:with_examples" % name] = "True"

            opts_bch = copy.deepcopy(options)
            opts_btc = copy.deepcopy(options)
            # opts_ltc = copy.deepcopy(options)

            opts_bch["%s:currency" % name] = "BCH"
            opts_btc["%s:currency" % name] = "BTC"
            # opts_ltc["%s:currency" % name] = "LTC"

            filtered_builds.append([settings, opts_bch, env_vars, build_requires])
            filtered_builds.append([settings, opts_btc, env_vars, build_requires])
            # filtered_builds.append([settings, opts_ltc, env_vars, build_requires])


    builder.builds = filtered_builds
    builder.run()
