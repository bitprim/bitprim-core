import copy
import os
import cpuid
import platform
from ci_utils import get_builder, handle_microarchs, copy_env_vars, filter_valid_exts, filter_marchs_tests

if __name__ == "__main__":
    full_build = os.getenv('BITPRIM_FULL_BUILD', '0') == '1'
    builder, name = get_builder()
    builder.add_common_builds(shared_option_name="%s:shared" % name)

    filtered_builds = []
    for settings, options, env_vars, build_requires, reference in builder.items:

        if settings["build_type"] == "Release" \
                and not options["%s:shared" % name]:

            copy_env_vars(env_vars)

            if os.getenv('BITPRIM_RUN_TESTS', 'false') == 'true':
                options["%s:with_tests" % name] = "True"
                options["%s:with_examples" % name] = "False"

            if full_build:
                marchs = filter_valid_exts(str(platform.system()), str(settings["compiler"]), float(str(settings["compiler.version"])), ['x86-64', 'haswell', 'skylake'])
                # marchs = ["x86-64"]
            else:
                marchs = ["x86-64"]

            ci_currency = os.getenv('BITPRIM_CI_CURRENCY', None)
            with_keoken = os.getenv('BITPRIM_WITH_KEOKEN', 'false') == 'true'

            if ci_currency is None:
                opts_bch = copy.deepcopy(options)
                opts_btc = copy.deepcopy(options)
                # opts_ltc = copy.deepcopy(options)

                opts_bch["%s:currency" % name] = "BCH"
                opts_bch["%s:keoken" % name] = with_keoken

                opts_btc["%s:currency" % name] = "BTC"
                # opts_ltc["%s:currency" % name] = "LTC"

                handle_microarchs("%s:microarchitecture" % name, marchs, filtered_builds, settings, opts_bch, env_vars, build_requires)
                handle_microarchs("%s:microarchitecture" % name, marchs, filtered_builds, settings, opts_btc, env_vars, build_requires)
                # handle_microarchs("%s:microarchitecture" % name, marchs, filtered_builds, settings, opts_ltc, env_vars, build_requires)
            else:
                options["%s:currency" % name] = ci_currency
                options["%s:keoken" % name] = with_keoken
                handle_microarchs("%s:microarchitecture" % name, marchs, filtered_builds, settings, options, env_vars, build_requires)

            filter_marchs_tests(name, filtered_builds, ["%s:with_tests" % name, "%s:with_examples" % name])

    builder.builds = filtered_builds
    builder.run()
