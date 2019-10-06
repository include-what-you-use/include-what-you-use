#!/bin/bash
#
# Produce import files for Boost.
#

boost_headers_dir=${BOOST_ROOT:-/usr/include}
if [[ ! -d ${boost_headers_dir}/boost ]]; then
    cat <<EOF >&2
*** Error: Boost headers are not found at ${boost_headers_dir}/boost.

Please provide 'BOOST_ROOT' environment variable. Example:

    $ BOOST_ROOT=/usr/local/include $0

or for unpacked tarball

    $ BOOST_ROOT=~/work/boost_1_71_0 $0

EOF
    exit 1
fi

boost_version=$( \
    grep '#define BOOST_LIB_VERSION' "${boost_headers_dir}/boost/version.hpp" \
  | sed -e 's,#define BOOST_LIB_VERSION "\([^"]\+\)",\1,' -e 's,_,.,' \
  )
private_private="boost-${boost_version}-private-private.imp"
private_public="boost-${boost_version}-private-public.imp"
public_public="boost-${boost_version}-public-public.imp"
all="boost-${boost_version}-all.imp"

#
# ATTENTION The libraries below caused some sort of pain.
# E.g., for the Boost Test, IWYU also wants to include a lot of Boost PP
# library headers. That is why the `boost-public-public.imp` file is here:
# to reflect that *public headers* of some lib `#include` *public headers*
# of some other lib. But, you have to be **extremely** careful adding more
# libs here, cuz a lib w/ a lot of code like this:
#
#   #if blah-blah
#   #  include <boost/blah-blah...>
#   #endif
#
# gonna be a source of incorrect IWYU behavior. The public headers of the
# libraries below has reviewed for absence (or minimal impact of) these
# conditional compilation directives.
#
boost_fav_libs=( beast iterator program_options preprocessor test )

# Also, "parse" well known facade headers
boost_well_known_facade_headers=( \
  any.hpp \
  asio.hpp \
  asio/ssl.hpp \
  atomic.hpp \
  beast.hpp \
  beast/core.hpp \
  beast/http.hpp \
  beast/ssl.hpp \
  beast/websocket.hpp \
  beast/zlib.hpp \
  bimap.hpp \
  callable_traits.hpp \
  chrono.hpp \
  contract.hpp \
  cregex.hpp \
  dll.hpp \
  filesystem.hpp \
  flyweight.hpp \
  geometry.hpp \
  gil.hpp \
  hana.hpp \
  histogram.hpp \
  hof.hpp \
  locale.hpp \
  make_shared.hpp \
  make_unique.hpp \
  metaparse.hpp \
  mp11.hpp \
  parameter.hpp \
  phoenix.hpp \
  preprocessor.hpp \
  process.hpp \
  program_options.hpp \
  python.hpp \
  random.hpp \
  range.hpp \
  ratio.hpp \
  regex.hpp \
  scoped_array.hpp \
  scoped_ptr.hpp \
  shared_array.hpp \
  shared_ptr.hpp \
  signals2.hpp \
  smart_ptr.hpp \
  stacktrace.hpp \
  thread.hpp \
  type_traits.hpp \
  unordered_map.hpp \
  unordered_set.hpp \
  variant.hpp \
  wave.hpp \
  weak_ptr.hpp \
)

# Private to private
# ------------------
#
# Some headers cause a cycle-dependency and gonna be commented...
#
echo '[' > ${private_private}
( cd "${boost_headers_dir}" \
 && grep -r '^ *# *include' boost/ \
  | grep -e "boost/[^:]*/\(aux_\|detail\|impl\)/.*hp*:" \
  | grep -e "\:.*/\(aux_\|detail\|impl\)/" \
  | perl -nle 'm/^([^:]+).*["<]([^>]+)[">]/ && print qq@    { include: ["<$2>", private, "<$1>", private ] },@' \
  | grep -e '\["<boost/' \
  | sort -u \
  | sed -e '/boost\/contract\/detail\/inlined\/detail\/checking\.hpp/ s,^,#,' \
        -e '/boost\/local_function\/aux_\/macro\/code_\/functor\.hpp/ s,^,#,' \
        -e '/boost\/local_function\/aux_\/macro\/code_\/bind\.hpp/ s,^,#,' \
        -e '/boost\/numeric\/odeint\/iterator\/integrate\/detail\/integrate_adaptive\.hpp/ s,^,#,' \
        -e '/boost\/numeric\/odeint\/integrate\/detail\/integrate_const\.hpp/ s,^,#,' \
        -e '/boost\/python\/detail\/type_list\.hpp/ s,^,#,' \
        -e '/boost\/python\/detail\/type_list_impl\.hpp/ s,^,#,' \
        -e '/boost\/variant\/detail\/multivisitors_cpp14_based\.hpp/ s,^,#,' \
) >> ${private_private}
echo ']' >> ${private_private}

# Private to public
# -----------------
echo '[' > ${private_public}
( cd "${boost_headers_dir}" \
 && grep -r --exclude-dir={detail,impl,aux_} '^ *# *include' boost/ \
  | perl -nle 'm/^([^:]+).*["<]([^>]+)[">]/ && print qq@    { include: ["<$2>", private, "<$1>", public ] },@' \
  | grep -e '\/\(aux_\|detail\|impl\)\/' \
  | grep -e '\["<boost/' \
  | sed -e '/boost\/\(bind\|format\|function\|predef\)\//d' \
        -e '/workarounds*\.hpp/d' \
  | sort -u \
) >> ${private_public}
echo ']' >> ${private_public}

# Public to public
# ----------------
#
# - `boost/version.hpp` included internally as "implementation details",
#    so remove it from "interface"
# - `boost/preprocessor/control/while.hpp` has a "cycle dependency"
#
echo '[' > ${public_public}
( cd "${boost_headers_dir}" \
  && for lib in ${boost_fav_libs[@]} ${boost_well_known_facade_headers[@]}; do \
        grep -Hnr --exclude-dir={detail,impl,aux_} '^ *# *include' boost/${lib} \
      | grep -ve '\(detail\|impl\|aux_\)' \
      | grep -e "<boost/" \
      | grep -ve 'boost/\(bind\|format\|function\|predef\)' \
      | perl -nle 'm/^([^:]+).*["<]([^>]+)[">]/ && print qq@    { include: ["<$2>", public, "<$1>", public ] },@' \
      ;
     done \
  | sort -u \
  | sed -e '/boost\/preprocessor\/control\/while\.hpp/ s,^,#,' \
        -e '/boost\/version\.hpp/d'
) >> ${public_public}
echo ']' >> ${public_public}

# Consolidated import
# -------------------
cat <<EOF > ${all}
[
    # Ok, get the shipped Boost imports (autogenerated) into the scope.
    { ref: ${private_private} },
    { ref: ${private_public} },
    { ref: ${public_public} },

    # The following libraries are used via the corresponding "facade" header
    { include: ["@<boost/bind/.*>", private, "<boost/bind.hpp>", public ] },
    { include: ["@<boost/format/.*>", private, "<boost/format.hpp>", public ] },
    { include: ["@<boost/function/.*>", private, "<boost/function.hpp>", public ] },
    { include: ["@<boost/predef/.*>", private, "<boost/predef.h>", public ] },

    # Including \`boost/lexical_cast/*.hpp\` is Ok, but more traditional way is to use the facade header.
    { include: [ "<boost/lexical_cast/bad_lexical_cast.hpp>", public, "<boost/lexical_cast.hpp>", public ] },
    { include: [ "<boost/lexical_cast/try_lexical_convert.hpp>", public, "<boost/lexical_cast.hpp>", public ] },

    # Redirect \`using namespace boost::filesystem\` to the convenience header
    { symbol: ["boost::filesystem", public, "<boost/filesystem.hpp>", public] },
]
EOF
