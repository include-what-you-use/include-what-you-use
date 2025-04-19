//===--- iwyu_include_picker.cc - map to canonical #includes for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_include_picker.h"

#include <algorithm>                    // for find
#include <cstddef>                      // for size_t
// not hash_map: it's not as portable and needs hash<string>.
#include <map>                          // for map, map<>::mapped_type, etc
#include <memory>
#include <numeric>                      // for accumulate
#include <string>                       // for string, basic_string, etc
#include <system_error>                 // for error_code
#include <utility>                      // for pair, make_pair
#include <vector>                       // for vector, vector<>::iterator

#include "clang/Tooling/Inclusions/StandardLibrary.h"
#include "iwyu_location_util.h"
#include "iwyu_path_util.h"
#include "iwyu_port.h"
#include "iwyu_regex.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "iwyu_verrs.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

// TODO: Clean out pragmas as IWYU improves.
// IWYU pragma: no_include <iterator>

using clang::OptionalFileEntryRef;
using llvm::MemoryBuffer;
using llvm::SourceMgr;
using llvm::yaml::KeyValueNode;
using llvm::yaml::MappingNode;
using llvm::yaml::Node;
using llvm::yaml::ScalarNode;
using llvm::yaml::SequenceNode;
using llvm::yaml::Stream;
using llvm::yaml::document_iterator;
using std::find;
using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::unique_ptr;
using std::vector;

namespace include_what_you_use {

// If we map from A to B, it means that every time we need a
// symbol from A, we can also get it from B.  Another way
// to think about it is that map_to "re-exports" all the
// symbols from map_from.
struct IncludeMapEntry {      // A POD so we can make the input static
  const char* map_from;       // A quoted-include or a symbol name
  IncludeVisibility from_visibility;
  const char* map_to;         // A quoted-include
  IncludeVisibility to_visibility;
};

namespace {

// Listed below are all IWYU's native symbol and include mappings,
// loosely based on GCC 4.4's libc and libstdc++.

// Symbol -> include mappings for GNU libc
const IncludeMapEntry libc_symbol_map[] = {
  // For library symbols that can be defined in more than one header
  // file, maps from symbol-name to legitimate header files.
  // This list was generated via
  // grep -R '__.*_defined' /usr/include | perl -nle 'm,/usr/include/([^:]*):#\s*\S+ __(.*)_defined, and print qq@    { "$2", kPublic, "<$1>", kPublic },@' | sort -u
  // I ignored all entries that only appeared once on the list (eg uint32_t).
  // I then added in NULL, which according to [diff.null] C.2.2.3, can
  // be defined in <clocale>, <cstddef>, <cstdio>, <cstdlib>,
  // <cstring>, <ctime>, or <cwchar>.  We also allow their C
  // equivalents.
  // In each case, I ordered them so <sys/types.h> was first, if it was
  // an option for this type.  That's the preferred #include all else
  // equal.  The visibility on the symbol-name is ignored; by convention
  // we always set it to kPrivate.
  { "_POSIX_VDISABLE", kPrivate, "<unistd.h>", kPublic },
  { "abort", kPrivate, "<stdlib.h>", kPublic },
  { "aiocb", kPrivate, "<aio.h>", kPublic },
  { "blkcnt_t", kPrivate, "<sys/types.h>", kPublic },
  { "blksize_t", kPrivate, "<sys/types.h>", kPublic },
  { "cc_t", kPrivate, "<termios.h>", kPublic },
  { "clock_t", kPrivate, "<time.h>", kPublic },
  { "clock_t", kPrivate, "<sys/types.h>", kPublic },
  { "clockid_t", kPrivate, "<sys/types.h>", kPublic },
  { "ctermid", kPrivate, "<stdio.h>", kPublic },
  { "daddr_t", kPrivate, "<sys/types.h>", kPublic },
  { "dev_t", kPrivate, "<sys/types.h>", kPublic },
  { "div_t", kPrivate, "<stdlib.h>", kPublic },
  { "double_t", kPrivate, "<math.h>", kPublic },
  { "error_t", kPrivate, "<errno.h>", kPublic },
  { "error_t", kPrivate, "<argp.h>", kPublic },
  { "error_t", kPrivate, "<argz.h>", kPublic },
  { "FD_CLR", kPrivate, "<sys/select.h>", kPublic },
  { "FD_ISSET", kPrivate, "<sys/select.h>", kPublic },
  { "FD_SET", kPrivate, "<sys/select.h>", kPublic },
  { "fd_set", kPrivate, "<sys/select.h>", kPublic },
  { "FD_SETSIZE", kPrivate, "<sys/select.h>", kPublic },
  { "FD_ZERO", kPrivate, "<sys/select.h>", kPublic },
  { "fenv_t", kPrivate, "<fenv.h>", kPublic },
  { "fexcept_t", kPrivate, "<fenv.h>", kPublic },
  { "FILE", kPrivate, "<stdio.h>", kPublic },
  { "float_t", kPrivate, "<math.h>", kPublic },
  { "fsblkcnt_t", kPrivate, "<sys/types.h>", kPublic },
  { "fsfilcnt_t", kPrivate, "<sys/types.h>", kPublic },
  { "getopt", kPrivate, "<unistd.h>", kPublic },
  { "gid_t", kPrivate, "<sys/types.h>", kPublic },
  { "htonl", kPrivate, "<arpa/inet.h>", kPublic },
  { "htons", kPrivate, "<arpa/inet.h>", kPublic },
  { "in_addr_t", kPrivate, "<netinet/in.h>", kPublic },
  { "in_port_t", kPrivate, "<netinet/in.h>", kPublic },
  { "id_t", kPrivate, "<sys/types.h>", kPublic },
  { "imaxdiv_t", kPrivate, "<inttypes.h>", kPublic },
  { "intmax_t", kPrivate, "<stdint.h>", kPublic },
  { "uintmax_t", kPrivate, "<stdint.h>", kPublic },
  { "ino64_t", kPrivate, "<sys/types.h>", kPublic },
  { "ino_t", kPrivate, "<sys/types.h>", kPublic },
  { "int8_t", kPrivate, "<stdint.h>", kPublic },
  { "int16_t", kPrivate, "<stdint.h>", kPublic },
  { "int32_t", kPrivate, "<stdint.h>", kPublic },
  { "int64_t", kPrivate, "<stdint.h>", kPublic },
  { "uint8_t", kPrivate, "<stdint.h>", kPublic },
  { "uint16_t", kPrivate, "<stdint.h>", kPublic },
  { "uint32_t", kPrivate, "<stdint.h>", kPublic },
  { "uint64_t", kPrivate, "<stdint.h>", kPublic },
  { "intptr_t", kPrivate, "<stdint.h>", kPublic },
  { "uintptr_t", kPrivate, "<stdint.h>", kPublic },
  { "iovec", kPrivate, "<sys/uio.h>", kPublic },
  { "itimerspec", kPrivate, "<time.h>", kPublic },
  { "key_t", kPrivate, "<sys/types.h>", kPublic },
  { "L_ctermid", kPrivate, "<stdio.h>", kPublic },
  { "lconv", kPrivate, "<locale.h>", kPublic },
  { "ldiv_t", kPrivate, "<stdlib.h>", kPublic },
  { "lldiv_t", kPrivate, "<stdlib.h>", kPublic },
  { "locale_t", kPrivate, "<locale.h>", kPublic },
  { "max_align_t", kPrivate, "<stddef.h>", kPublic },
  { "mbstate_t", kPrivate, "<wchar.h>", kPublic },
  { "mcontext_t", kPrivate, "<ucontext.h>", kPublic },
  { "mode_t", kPrivate, "<sys/types.h>", kPublic },
  { "nl_item", kPrivate, "<nl_types.h>", kPublic },
  { "nlink_t", kPrivate, "<sys/types.h>", kPublic },
  { "ntohl", kPrivate, "<arpa/inet.h>", kPublic },
  { "ntohs", kPrivate, "<arpa/inet.h>", kPublic },
  { "O_DSYNC", kPrivate, "<fcntl.h>", kPublic },
  { "O_SYNC", kPrivate, "<fcntl.h>", kPublic },
  { "off64_t", kPrivate, "<sys/types.h>", kPublic },
  { "off_t", kPrivate, "<sys/types.h>", kPublic },
  { "optarg", kPrivate, "<unistd.h>", kPublic },
  { "opterr", kPrivate, "<unistd.h>", kPublic },
  { "optind", kPrivate, "<unistd.h>", kPublic },
  { "optopt", kPrivate, "<unistd.h>", kPublic },
  { "pid_t", kPrivate, "<sys/types.h>", kPublic },
  { "posix_memalign", kPrivate, "<stdlib.h>", kPublic },
  { "printf", kPrivate, "<stdio.h>", kPublic },
  { "pthread_attr_t", kPrivate, "<pthread.h>", kPublic },
  { "pthread_cond_t", kPrivate, "<pthread.h>", kPublic },
  { "pthread_condattr_t", kPrivate, "<pthread.h>", kPublic },
  { "pthread_key_t", kPrivate, "<pthread.h>", kPublic },
  { "pthread_mutex_t", kPrivate, "<pthread.h>", kPublic },
  { "pthread_mutexattr_t", kPrivate, "<pthread.h>", kPublic },
  { "pthread_once_t", kPrivate, "<pthread.h>", kPublic },
  { "pthread_rwlock_t", kPrivate, "<pthread.h>", kPublic },
  { "pthread_rwlockattr_t", kPrivate, "<pthread.h>", kPublic },
  { "pthread_t", kPrivate, "<pthread.h>", kPublic },
  { "ptrdiff_t", kPrivate, "<stddef.h>", kPublic },
  { "regex_t", kPrivate, "<regex.h>", kPublic },
  { "regmatch_t", kPrivate, "<regex.h>", kPublic },
  { "regoff_t", kPrivate, "<regex.h>", kPublic },
  { "S_IFBLK", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IFCHR", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IFDIR", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IFIFO", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IFLNK", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IFMT", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IFREG", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IFSOCK", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IRGRP", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IROTH", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IRUSR", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IRWXG", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IRWXO", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IRWXU", kPrivate, "<sys/stat.h>", kPublic },
  { "S_ISGID", kPrivate, "<sys/stat.h>", kPublic },
  { "S_ISUID", kPrivate, "<sys/stat.h>", kPublic },
  { "S_ISVTX", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IWGRP", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IWOTH", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IWUSR", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IXGRP", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IXOTH", kPrivate, "<sys/stat.h>", kPublic },
  { "S_IXUSR", kPrivate, "<sys/stat.h>", kPublic },
  { "sa_family_t", kPrivate, "<sys/socket.h>", kPublic },
  { "SCHED_FIFO", kPrivate, "<sched.h>", kPublic },
  { "SCHED_OTHER", kPrivate, "<sched.h>", kPublic },
  { "SCHED_RR", kPrivate, "<sched.h>", kPublic },
  { "SEEK_CUR", kPrivate, "<stdio.h>", kPublic },
  { "SEEK_END", kPrivate, "<stdio.h>", kPublic },
  { "SEEK_SET", kPrivate, "<stdio.h>", kPublic },
  { "sig_atomic_t", kPrivate, "<signal.h>", kPublic },
  { "sigevent", kPrivate, "<signal.h>", kPublic },
  { "siginfo_t", kPrivate, "<signal.h>", kPublic },
  { "sigset_t", kPrivate, "<signal.h>", kPublic },
  { "sigval", kPrivate, "<signal.h>", kPublic },
  { "sockaddr", kPrivate, "<sys/socket.h>", kPublic },
  { "socklen_t", kPrivate, "<sys/socket.h>", kPublic },
  { "ssize_t", kPrivate, "<sys/types.h>", kPublic },
  { "stack_t", kPrivate, "<signal.h>", kPublic },
  { "stat", kPrivate, "<sys/stat.h>", kPublic },
  { "suseconds_t", kPrivate, "<sys/types.h>", kPublic },
  { "time_t", kPrivate, "<time.h>", kPublic },
  { "time_t", kPrivate, "<sys/types.h>", kPublic },
  { "timer_t", kPrivate, "<sys/types.h>", kPublic },
  { "timespec", kPrivate, "<time.h>", kPublic },
  { "timeval", kPrivate, "<sys/time.h>", kPublic },
  { "tm", kPrivate, "<time.h>", kPublic },
  { "u_char", kPrivate, "<sys/types.h>", kPublic },
  { "ucontext_t", kPrivate, "<ucontext.h>", kPublic },
  { "uid_t", kPrivate, "<sys/types.h>", kPublic },
  { "useconds_t", kPrivate, "<sys/types.h>", kPublic },
  { "wchar_t", kPrivate, "<stddef.h>", kPublic },
  { "wctrans_t", kPrivate, "<wctype.h>", kPublic },
  { "wctype_t", kPrivate, "<wctype.h>", kPublic },
  { "winsize", kPrivate, "<termios.h>", kPublic },
  { "wint_t", kPrivate, "<wchar.h>", kPublic },
  // It is unspecified if the cname headers provide ::size_t.
  // <locale.h> is the one header which defines NULL but not size_t.
  { "size_t", kPrivate, "<stddef.h>", kPublic },  // 'canonical' location for size_t
  { "size_t", kPrivate, "<signal.h>", kPublic },
  { "size_t", kPrivate, "<stdio.h>", kPublic },
  { "size_t", kPrivate, "<stdlib.h>", kPublic },
  { "size_t", kPrivate, "<string.h>", kPublic },
  { "size_t", kPrivate, "<time.h>", kPublic },
  { "size_t", kPrivate, "<uchar.h>", kPublic },
  { "size_t", kPrivate, "<wchar.h>", kPublic },
  // Macros that can be defined in more than one file, don't have the
  // same __foo_defined guard that other types do, so the grep above
  // doesn't discover them.  Until I figure out a better way, I just
  // add them in by hand as I discover them.
  { "EOF", kPrivate, "<stdio.h>", kPublic },
  { "FILE", kPrivate, "<stdio.h>", kPublic },
  { "IBSHIFT", kPrivate, "<asm/termbits.h>", kPublic },
  { "MAP_POPULATE", kPrivate, "<sys/mman.h>", kPublic },
  { "MAP_POPULATE", kPrivate, "<linux/mman.h>", kPublic },
  { "MAP_STACK", kPrivate, "<sys/mman.h>", kPublic },
  { "MAP_STACK", kPrivate, "<linux/mman.h>", kPublic },
  { "MAXHOSTNAMELEN", kPrivate, "<sys/param.h>", kPublic },
  { "MAXHOSTNAMELEN", kPrivate, "<protocols/timed.h>", kPublic },
  { "SIGABRT", kPrivate, "<signal.h>", kPublic },
  { "SIGCHLD", kPrivate, "<signal.h>", kPublic },
  { "va_arg", kPrivate, "<stdarg.h>", kPublic },
  { "va_copy", kPrivate, "<stdarg.h>", kPublic },
  { "va_end", kPrivate, "<stdarg.h>", kPublic },
  { "va_list", kPrivate, "<stdarg.h>", kPublic },
  { "va_start", kPrivate, "<stdarg.h>", kPublic },
  { "WEOF", kPrivate, "<wchar.h>", kPublic },
  // These are symbols that could be defined in either stdlib.h or
  // malloc.h, but we always want the stdlib location.
  { "malloc", kPrivate, "<stdlib.h>", kPublic },
  { "calloc", kPrivate, "<stdlib.h>", kPublic },
  { "realloc", kPrivate, "<stdlib.h>", kPublic },
  { "free", kPrivate, "<stdlib.h>", kPublic },
  // Entries for NULL
  { "NULL", kPrivate, "<stddef.h>", kPublic },  // 'canonical' location for NULL
  { "NULL", kPrivate, "<clocale>", kPublic },
  { "NULL", kPrivate, "<cstddef>", kPublic },
  { "NULL", kPrivate, "<cstdio>", kPublic },
  { "NULL", kPrivate, "<cstdlib>", kPublic },
  { "NULL", kPrivate, "<cstring>", kPublic },
  { "NULL", kPrivate, "<ctime>", kPublic },
  { "NULL", kPrivate, "<cwchar>", kPublic },
  { "NULL", kPrivate, "<locale.h>", kPublic },
  { "NULL", kPrivate, "<stdio.h>", kPublic },
  { "NULL", kPrivate, "<stdlib.h>", kPublic },
  { "NULL", kPrivate, "<string.h>", kPublic },
  { "NULL", kPrivate, "<time.h>", kPublic },
  { "NULL", kPrivate, "<wchar.h>", kPublic },
  { "offsetof", kPrivate, "<stddef.h>", kPublic },
};

// Common kludges for C++ standard libraries
const IncludeMapEntry stdlib_cxx_symbol_map[] = {
  // Almost all STL types take an allocator, but they
  // almost always use the default value.  Usually we detect that
  // and don't try to do IWYU, but sometimes it passes through.
  // For instance, when adding two strings, we end up calling
  //    template<_CharT,_Traits,_Alloc> ... operator+(
  //       basic_string<_CharT,_Traits,_Alloc>, ...)
  // These look like normal template args to us, so we see they're
  // used and declare an iwyu dependency, even though we don't need
  // to #include the traits or alloc type ourselves.  The surest way
  // to deal with this is to just say that everyone provides
  // std::allocator.  We can add more here at need.
  { "std::allocator", kPrivate, "<memory>", kPublic },
  { "std::allocator", kPrivate, "<string>", kPublic },
  { "std::allocator", kPrivate, "<vector>", kPublic },
  { "std::allocator", kPrivate, "<map>", kPublic },
  { "std::allocator", kPrivate, "<set>", kPublic },
  // A similar kludge for std::char_traits.  basic_string,
  // basic_ostream and basic_istream have this as a default template
  // argument, and sometimes it bleeds through when clang desugars the
  // string/ostream/istream type.
  { "std::char_traits", kPrivate, "<string>", kPublic },
  { "std::char_traits", kPrivate, "<ostream>", kPublic },
  { "std::char_traits", kPrivate, "<istream>", kPublic },

  // std::ptrdiff_t is often an architecture specific definition, force the
  // canonical location.
  { "std::ptrdiff_t", kPrivate, "<cstddef>", kPublic },

  { "std::size_t", kPrivate, "<cstddef>", kPublic },  // 'canonical' location for std::size_t
  { "std::size_t", kPrivate, "<cstdio>", kPublic },
  { "std::size_t", kPrivate, "<cstdlib>", kPublic },
  { "std::size_t", kPrivate, "<cstring>", kPublic },
  { "std::size_t", kPrivate, "<ctime>", kPublic },
  { "std::size_t", kPrivate, "<cuchar>", kPublic },
  { "std::size_t", kPrivate, "<cwchar>", kPublic },
};

// Symbol -> include mappings for GNU libstdc++
const IncludeMapEntry libstdcpp_symbol_map[] = {
  // GCC defines std::declval in <type_traits>, but the canonical location is <utility>
  { "std::declval", kPrivate, "<utility>", kPublic },
};

const IncludeMapEntry libc_include_map[] = {
  // Private -> public include mappings for GNU libc
  // ( cd /usr/include && grep '^ *# *include' {sys/,net/,}* | perl -nle 'm/^([^:]+).*<([^>]+)>/ && print qq@    { "<$2>", kPrivate, "<$1>", kPublic },@' | grep bits/ | sort )
  // When I saw more than one mapping for these, I typically picked
  // what I thought was the "best" one.
  { "<bits/a.out.h>", kPrivate, "<a.out.h>", kPublic },
  { "<bits/auxv.h>", kPrivate, "<sys/auxv.h>", kPublic },
  { "<bits/byteswap.h>", kPrivate, "<byteswap.h>", kPublic },
  { "<bits/cmathcalls.h>", kPrivate, "<complex.h>", kPublic },
  { "<bits/confname.h>", kPrivate, "<unistd.h>", kPublic },
  { "<bits/dirent.h>", kPrivate, "<dirent.h>", kPublic },
  { "<bits/dlfcn.h>", kPrivate, "<dlfcn.h>", kPublic },
  { "<bits/elfclass.h>", kPrivate, "<link.h>", kPublic },
  { "<bits/endian.h>", kPrivate, "<endian.h>", kPublic },
  { "<bits/environments.h>", kPrivate, "<unistd.h>", kPublic },
  { "<bits/epoll.h>", kPrivate, "<sys/epoll.h>", kPublic },
  { "<bits/errno.h>", kPrivate, "<errno.h>", kPublic },
  { "<bits/error.h>", kPrivate, "<error.h>", kPublic },
  { "<bits/eventfd.h>", kPrivate, "<sys/eventfd.h>", kPublic },
  { "<bits/fcntl.h>", kPrivate, "<fcntl.h>", kPublic },
  { "<bits/fcntl2.h>", kPrivate, "<fcntl.h>", kPublic },
  { "<bits/fenv.h>", kPrivate, "<fenv.h>", kPublic },
  { "<bits/fenvinline.h>", kPrivate, "<fenv.h>", kPublic },
  { "<bits/huge_val.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/huge_valf.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/huge_vall.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/hwcap.h>", kPrivate, "<sys/auxv.h>", kPublic },
  { "<bits/inf.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/inotify.h>", kPrivate, "<sys/inotify.h>", kPublic },
  { "<bits/ioctl-types.h>", kPrivate, "<sys/ioctl.h>", kPublic },
  { "<bits/ioctls.h>", kPrivate, "<sys/ioctl.h>", kPublic },
  { "<bits/ipc.h>", kPrivate, "<sys/ipc.h>", kPublic },
  { "<bits/ipctypes.h>", kPrivate, "<sys/ipc.h>", kPublic },
  { "<bits/libio-ldbl.h>", kPrivate, "<libio.h>", kPublic },
  { "<bits/link.h>", kPrivate, "<link.h>", kPublic },
  { "<bits/locale.h>", kPrivate, "<locale.h>", kPublic },
  { "<bits/math-finite.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mathcalls.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mathdef.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mathinline.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mman.h>", kPrivate, "<sys/mman.h>", kPublic },
  { "<bits/mman-shared.h>", kPrivate, "<sys/mman.h>", kPublic },
  { "<bits/monetary-ldbl.h>", kPrivate, "<monetary.h>", kPublic },
  { "<bits/mqueue.h>", kPrivate, "<mqueue.h>", kPublic },
  { "<bits/mqueue2.h>", kPrivate, "<mqueue.h>", kPublic },
  { "<bits/msq.h>", kPrivate, "<sys/msg.h>", kPublic },
  { "<bits/nan.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/netdb.h>", kPrivate, "<netdb.h>", kPublic },
  { "<bits/param.h>", kPrivate, "<sys/param.h>", kPublic },
  { "<bits/poll.h>", kPrivate, "<sys/poll.h>", kPrivate },
  { "<bits/poll2.h>", kPrivate, "<sys/poll.h>", kPrivate },
  { "<bits/posix1_lim.h>", kPrivate, "<limits.h>", kPublic },
  { "<bits/posix2_lim.h>", kPrivate, "<limits.h>", kPublic },
  { "<bits/posix_opt.h>", kPrivate, "<unistd.h>", kPublic },
  { "<bits/printf-ldbl.h>", kPrivate, "<printf.h>", kPublic },
  { "<bits/pthreadtypes.h>", kPrivate, "<pthread.h>", kPublic },
  { "<bits/resource.h>", kPrivate, "<sys/resource.h>", kPublic },
  { "<bits/sched.h>", kPrivate, "<sched.h>", kPublic },
  { "<bits/select.h>", kPrivate, "<sys/select.h>", kPublic },
  { "<bits/select2.h>", kPrivate, "<sys/select.h>", kPublic },
  { "<bits/sem.h>", kPrivate, "<sys/sem.h>", kPublic },
  { "<bits/semaphore.h>", kPrivate, "<semaphore.h>", kPublic },
  { "<bits/setjmp.h>", kPrivate, "<setjmp.h>", kPublic },
  { "<bits/setjmp2.h>", kPrivate, "<setjmp.h>", kPublic },
  { "<bits/shm.h>", kPrivate, "<sys/shm.h>", kPublic },
  { "<bits/sigaction.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/sigcontext.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/siginfo.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/signum.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/signum-arch.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/sigset.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/sigstack.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/sigthread.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/sockaddr.h>", kPrivate, "<sys/un.h>", kPublic },
  { "<bits/socket.h>", kPrivate, "<sys/socket.h>", kPublic },
  { "<bits/socket2.h>", kPrivate, "<sys/socket.h>", kPublic },
  { "<bits/socket_type.h>", kPrivate, "<sys/socket.h>", kPublic },
  { "<bits/stab.def>", kPrivate, "<stab.h>", kPublic },
  { "<bits/stat.h>", kPrivate, "<sys/stat.h>", kPublic },
  { "<bits/statfs.h>", kPrivate, "<sys/statfs.h>", kPublic },
  { "<bits/statvfs.h>", kPrivate, "<sys/statvfs.h>", kPublic },
  { "<bits/stdio-ldbl.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/stdio-lock.h>", kPrivate, "<libio.h>", kPublic },
  { "<bits/stdio.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/stdio2.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/stdio_lim.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/stdlib-bsearch.h>", kPrivate, "<stdlib.h>", kPublic },
  { "<bits/stdlib-float.h>", kPrivate, "<stdlib.h>", kPublic },
  { "<bits/stdlib-ldbl.h>", kPrivate, "<stdlib.h>", kPublic },
  { "<bits/stdlib.h>", kPrivate, "<stdlib.h>", kPublic },
  { "<bits/string.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/string2.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/string3.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/stropts.h>", kPrivate, "<stropts.h>", kPublic },
  { "<bits/struct_stat.h>", kPrivate, "<sys/stat.h>", kPublic },
  { "<bits/struct_stat.h>", kPrivate, "<ftw.h>", kPublic },
  { "<bits/sys_errlist.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/syscall.h>", kPrivate, "<sys/syscall.h>", kPublic },
  { "<bits/sysctl.h>", kPrivate, "<sys/sysctl.h>", kPublic },
  { "<bits/syslog-ldbl.h>", kPrivate, "<sys/syslog.h>", kPrivate },
  { "<bits/syslog-path.h>", kPrivate, "<sys/syslog.h>", kPrivate },
  { "<bits/syslog.h>", kPrivate, "<sys/syslog.h>", kPrivate },
  { "<bits/termios-c_lflag.h>", kPrivate, "<termios.h>", kPublic },
  { "<bits/termios-struct.h>", kPrivate, "<termios.h>", kPublic },
  { "<bits/termios-tcflow.h>", kPrivate, "<termios.h>", kPublic },
  { "<bits/termios.h>", kPrivate, "<termios.h>", kPublic },
  { "<bits/time.h>", kPrivate, "<time.h>", kPublic },
  { "<bits/time.h>", kPrivate, "<sys/time.h>", kPublic },
  { "<bits/timerfd.h>", kPrivate, "<sys/timerfd.h>", kPublic },
  { "<bits/timex.h>", kPrivate, "<sys/timex.h>", kPublic },
  { "<bits/types.h>", kPrivate, "<sys/types.h>", kPublic },
  { "<bits/types/siginfo_t.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/types/siginfo_t.h>", kPrivate, "<sys/wait.h>", kPublic },
  { "<bits/uio.h>", kPrivate, "<sys/uio.h>", kPublic },
  { "<bits/unistd.h>", kPrivate, "<unistd.h>", kPublic },
  { "<bits/ustat.h>", kPrivate, "<sys/ustat.h>", kPrivate },
  { "<bits/utmp.h>", kPrivate, "<utmp.h>", kPublic },
  { "<bits/utmpx.h>", kPrivate, "<utmpx.h>", kPublic },
  { "<bits/utsname.h>", kPrivate, "<sys/utsname.h>", kPublic },
  { "<bits/waitflags.h>", kPrivate, "<sys/wait.h>", kPublic },
  { "<bits/waitstatus.h>", kPrivate, "<sys/wait.h>", kPublic },
  { "<bits/wchar-ldbl.h>", kPrivate, "<wchar.h>", kPublic },
  { "<bits/wchar.h>", kPrivate, "<wchar.h>", kPublic },
  { "<bits/wchar2.h>", kPrivate, "<wchar.h>", kPublic },
  { "<bits/wordsize.h>", kPrivate, "<limits.h>", kPublic },
  { "<bits/xopen_lim.h>", kPrivate, "<limits.h>", kPublic },
  { "<bits/xtitypes.h>", kPrivate, "<stropts.h>", kPublic },
  // Sometimes libc tells you what mapping to do via an '#error':
  // # error "Never use <bits/dlfcn.h> directly; include <dlfcn.h> instead."
  // or
  // # error "Never include <bits/socket_type.h> directly; use <sys/socket.h> instead."
  // ( cd /usr/include && grep -R '^ *# *error "Never use\|include' * | perl -nle 'm/<([^>]+).*directly.*<([^>]+)/ && print qq@    { "<$1>", kPrivate, "<$2>", kPublic },@' | sort )
  { "<bits/a.out.h>", kPrivate, "<a.out.h>", kPublic },
  { "<bits/byteswap-16.h>", kPrivate, "<byteswap.h>", kPublic },
  { "<bits/byteswap.h>", kPrivate, "<byteswap.h>", kPublic },
  { "<bits/cmathcalls.h>", kPrivate, "<complex.h>", kPublic },
  { "<bits/confname.h>", kPrivate, "<unistd.h>", kPublic },
  { "<bits/dirent.h>", kPrivate, "<dirent.h>", kPublic },
  { "<bits/dlfcn.h>", kPrivate, "<dlfcn.h>", kPublic },
  { "<bits/elfclass.h>", kPrivate, "<link.h>", kPublic },
  { "<bits/endian.h>", kPrivate, "<endian.h>", kPublic },
  { "<bits/epoll.h>", kPrivate, "<sys/epoll.h>", kPublic },
  { "<bits/eventfd.h>", kPrivate, "<sys/eventfd.h>", kPublic },
  { "<bits/fcntl-linux.h>", kPrivate, "<fcntl.h>", kPublic },
  { "<bits/fcntl.h>", kPrivate, "<fcntl.h>", kPublic },
  { "<bits/fenv.h>", kPrivate, "<fenv.h>", kPublic },
  { "<bits/huge_val.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/huge_valf.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/huge_vall.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/in.h>", kPrivate, "<netinet/in.h>", kPublic },
  { "<bits/inf.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/inotify.h>", kPrivate, "<sys/inotify.h>", kPublic },
  { "<bits/ioctl-types.h>", kPrivate, "<sys/ioctl.h>", kPublic },
  { "<bits/ioctls.h>", kPrivate, "<sys/ioctl.h>", kPublic },
  { "<bits/ipc.h>", kPrivate, "<sys/ipc.h>", kPublic },
  { "<bits/ipctypes.h>", kPrivate, "<sys/ipc.h>", kPublic },
  { "<bits/locale.h>", kPrivate, "<locale.h>", kPublic },
  { "<bits/math-finite.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mathdef.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mathinline.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mman-linux.h>", kPrivate, "<sys/mman.h>", kPublic },
  { "<bits/mman.h>", kPrivate, "<sys/mman.h>", kPublic },
  { "<bits/mqueue.h>", kPrivate, "<mqueue.h>", kPublic },
  { "<bits/msq.h>", kPrivate, "<sys/msg.h>", kPublic },
  { "<bits/nan.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/param.h>", kPrivate, "<sys/param.h>", kPublic },
  { "<bits/poll.h>", kPrivate, "<sys/poll.h>", kPrivate },
  { "<bits/predefs.h>", kPrivate, "<features.h>", kPublic },
  { "<bits/resource.h>", kPrivate, "<sys/resource.h>", kPublic },
  { "<bits/select.h>", kPrivate, "<sys/select.h>", kPublic },
  { "<bits/semaphore.h>", kPrivate, "<semaphore.h>", kPublic },
  { "<bits/sigcontext.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/signalfd.h>", kPrivate, "<sys/signalfd.h>", kPublic },
  { "<bits/stdlib-float.h>", kPrivate, "<stdlib.h>", kPublic },
  { "<bits/string.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/string2.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/string3.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/timerfd.h>", kPrivate, "<sys/timerfd.h>", kPublic },
  { "<bits/typesizes.h>", kPrivate, "<sys/types.h>", kPublic },
  // Top-level #includes that just forward to another file:
  // $ for i in /usr/include/*; do [ -f $i ] && [ `wc -l < $i` = 1 ] && echo $i; done
  // (poll.h, syscall.h, syslog.h, ustat.h, wait.h).
  // For each file, I looked at the list of canonical header files --
  // https://pubs.opengroup.org/onlinepubs/9799919799/idx/head.html --
  // to decide which of the two files is canonical.  If neither is
  // on the POSIX.1-2024 list, I just choose the top-level one.
  { "<sys/aio.h>", kPrivate, "<aio.h>", kPublic },
  { "<sys/dirent.h>", kPrivate, "<dirent.h>", kPublic },
  { "<sys/errno.h>", kPrivate, "<errno.h>", kPublic },
  { "<sys/fcntl.h>", kPrivate, "<fcntl.h>", kPublic },
  { "<sys/poll.h>", kPrivate, "<poll.h>", kPublic },
  { "<sys/semaphore.h>", kPrivate, "<semaphore.h>", kPublic },
  { "<sys/signal.h>", kPrivate, "<signal.h>", kPublic },
  { "<sys/spawn.h>", kPrivate, "<spawn.h>", kPublic },
  { "<sys/stdio.h>", kPrivate, "<stdio.h>", kPublic },
  { "<sys/syslog.h>", kPrivate, "<syslog.h>", kPublic },
  { "<sys/termios.h>", kPrivate, "<termios.h>", kPublic },
  { "<sys/unistd.h>", kPrivate, "<unistd.h>", kPublic },
  { "<wait.h>", kPrivate, "<sys/wait.h>", kPublic },
  // These are all files in bits/ that delegate to asm/ and linux/ to
  // do all (or lots) of the work.  Note these are private->private.
  // $ for i in /usr/include/bits/*; do for dir in asm linux; do grep -H -e $dir/`basename $i` $i; done; done
  { "<linux/errno.h>", kPrivate, "<bits/errno.h>", kPrivate },
  { "<asm/ioctls.h>", kPrivate, "<bits/ioctls.h>", kPrivate },
  { "<asm/socket.h>", kPrivate, "<bits/socket.h>", kPrivate },
  { "<linux/socket.h>", kPrivate, "<bits/socket.h>", kPrivate },
  // Some asm files have 32- and 64-bit variants:
  // $ ls /usr/include/asm/*_{32,64}.h
  { "<asm/posix_types_32.h>", kPrivate, "<asm/posix_types.h>", kPublic },
  { "<asm/posix_types_64.h>", kPrivate, "<asm/posix_types.h>", kPublic },
  { "<asm/unistd_32.h>", kPrivate, "<asm/unistd.h>", kPrivate },
  { "<asm/unistd_64.h>", kPrivate, "<asm/unistd.h>", kPrivate },
  // I don't know what grep would have found these.  I found them
  // via user report.
  { "<asm/errno.h>", kPrivate, "<errno.h>", kPublic },
  { "<asm/errno-base.h>", kPrivate, "<errno.h>", kPublic },
  { "<asm/ptrace-abi.h>", kPrivate, "<asm/ptrace.h>", kPublic },
  { "<asm/unistd.h>", kPrivate, "<sys/syscall.h>", kPublic },
  { "<linux/limits.h>", kPrivate, "<limits.h>", kPublic },   // PATH_MAX
  { "<linux/prctl.h>", kPrivate, "<sys/prctl.h>", kPublic },
  { "<sys/ucontext.h>", kPrivate, "<ucontext.h>", kPublic },
  // System headers available on AIX, BSD, Solaris and other Unix systems
  { "<sys/dtrace.h>", kPrivate, "<dtrace.h>", kPublic },
  { "<sys/paths.h>", kPrivate, "<paths.h>", kPublic },
  { "<sys/syslimits.h>", kPrivate, "<limits.h>", kPublic },
  { "<sys/ttycom.h>", kPrivate, "<sys/ioctl.h>", kPublic },
  { "<sys/ustat.h>", kPrivate, "<ustat.h>", kPublic },
  // Exports guaranteed by the C standard
  { "<stdint.h>", kPublic, "<inttypes.h>", kPublic },
};

const IncludeMapEntry stdlib_c_include_map[] = {
  // Allow the C++ wrappers around C files.  Without these mappings,
  // if you #include <cstdio>, iwyu will tell you to replace it with
  // <stdio.h>, which is where the symbols are actually defined.  We
  // inhibit that behavior to keep the <cstdio> alone.  Note this is a
  // public-to-public mapping: we don't want to *replace* <assert.h>
  // with <cassert>, we just want to avoid suggesting changing
  // <cassert> back to <assert.h>.  (If you *did* want to replace
  // assert.h with cassert, you'd change it to a public->private
  // mapping.)  Here is how I identified the files to map:
  // $ for i in /usr/include/c++/4.4/c* ; do ls /usr/include/`basename $i | cut -b2-`.h /usr/lib/gcc/*/4.4/include/`basename $i | cut -b2-`.h 2>/dev/null ; done
  //
  // These headers are defined in [headers.cpp.c].
  // https://github.com/cplusplus/draft/blob/c+%2B20/source/lib-intro.tex
  //
  // $ curl -s -N https://raw.githubusercontent.com/cplusplus/draft/c%2B%2B20/source/lib-intro.tex | sed -n '/begin{multicolfloattable}.*{headers.cpp.c}/,/end{multicolfloattable}/p' | grep tcode | perl -nle 'm/tcode{<c(.*)>}/ && print qq@  { "<$1.h>", kPublic, "<c$1>", kPublic },@' | sort
  { "<assert.h>", kPublic, "<cassert>", kPublic },
  { "<complex.h>", kPublic, "<ccomplex>", kPublic },
  { "<ctype.h>", kPublic, "<cctype>", kPublic },
  { "<errno.h>", kPublic, "<cerrno>", kPublic },
  { "<fenv.h>", kPublic, "<cfenv>", kPublic },
  { "<float.h>", kPublic, "<cfloat>", kPublic },
  { "<inttypes.h>", kPublic, "<cinttypes>", kPublic },
  { "<iso646.h>", kPublic, "<ciso646>", kPublic },
  { "<limits.h>", kPublic, "<climits>", kPublic },
  { "<locale.h>", kPublic, "<clocale>", kPublic },
  { "<math.h>", kPublic, "<cmath>", kPublic },
  { "<setjmp.h>", kPublic, "<csetjmp>", kPublic },
  { "<signal.h>", kPublic, "<csignal>", kPublic },
  { "<stdalign.h>", kPublic, "<cstdalign>", kPublic },
  { "<stdarg.h>", kPublic, "<cstdarg>", kPublic },
  { "<stdbool.h>", kPublic, "<cstdbool>", kPublic },
  { "<stddef.h>", kPublic, "<cstddef>", kPublic },
  { "<stdint.h>", kPublic, "<cstdint>", kPublic },
  { "<stdio.h>", kPublic, "<cstdio>", kPublic },
  { "<stdlib.h>", kPublic, "<cstdlib>", kPublic },
  { "<string.h>", kPublic, "<cstring>", kPublic },
  { "<tgmath.h>", kPublic, "<ctgmath>", kPublic },
  { "<time.h>", kPublic, "<ctime>", kPublic },
  { "<uchar.h>", kPublic, "<cuchar>", kPublic },
  { "<wchar.h>", kPublic, "<cwchar>", kPublic },
  { "<wctype.h>", kPublic, "<cwctype>", kPublic },
};

const char* stdlib_cpp_public_headers[] = {
  // These headers are defined in [headers.cpp].
  // https://github.com/cplusplus/draft/blob/c+%2B20/source/lib-intro.tex
  //
  // $ curl -s -N https://raw.githubusercontent.com/cplusplus/draft/c%2B%2B23-wip/source/lib-intro.tex | sed -n '/begin{multicolfloattable}.*{headers.cpp}/,/end{multicolfloattable}/p' | grep tcode | perl -nle 'm/tcode{(.*)}/ && print qq@  "$1",@' | sort
  "<algorithm>",
  "<any>",
  "<array>",
  "<atomic>",
  "<barrier>",
  "<bit>",
  "<bitset>",
  "<charconv>",
  "<chrono>",
  "<codecvt>",
  "<compare>",
  "<complex>",
  "<concepts>",
  "<condition_variable>",
  "<coroutine>",
  "<deque>",
  "<exception>",
  "<execution>",
  "<expected>",
  "<filesystem>",
  "<flat_map>",
  "<flat_set>",
  "<format>",
  "<forward_list>",
  "<fstream>",
  "<functional>",
  "<future>",
  "<generator>",
  "<initializer_list>",
  "<iomanip>",
  "<ios>",
  "<iosfwd>",
  "<iostream>",
  "<istream>",
  "<iterator>",
  "<latch>",
  "<limits>",
  "<list>",
  "<locale>",
  "<map>",
  "<mdspan>",
  "<memory>",
  "<memory_resource>",
  "<mutex>",
  "<new>",
  "<numbers>",
  "<numeric>",
  "<optional>",
  "<ostream>",
  "<print>",
  "<queue>",
  "<random>",
  "<ranges>",
  "<ratio>",
  "<regex>",
  "<scoped_allocator>",
  "<semaphore>",
  "<set>",
  "<shared_mutex>",
  "<source_location>",
  "<span>",
  "<spanstream>",
  "<sstream>",
  "<stack>",
  "<stacktrace>",
  "<stdexcept>",
  "<stdfloat>",
  "<stop_token>",
  "<streambuf>",
  "<string>",
  "<string_view>",
  "<strstream>",
  "<syncstream>",
  "<system_error>",
  "<thread>",
  "<tuple>",
  "<typeindex>",
  "<typeinfo>",
  "<type_traits>",
  "<unordered_map>",
  "<unordered_set>",
  "<utility>",
  "<valarray>",
  "<variant>",
  "<vector>",
  "<version>",
};

// Common public -> public include mappings for C++ standard library
//
// Note: make sure to sync this setting with stl.public.imp
//
const IncludeMapEntry stdlib_cpp_include_map[] = {
  // The iostream .h files are confusing.  Lots of private headers,
  // which are handled below, but we also have public headers
  // #including each other (eg <iostream> #includes <istream>).  We
  // are pretty forgiving: if a user specifies any public header, we
  // generally don't require the others.
  // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && egrep '^ *# *include <(istream|ostream|iostream|fstream|sstream|streambuf|ios|iosfwd)>' *stream* ios | perl -nle 'm/^([^:]+).*[<"]([^>"]+)[>"]/ and print qq@    { "<$2>", kPublic, "<$1>", kPublic },@' | sort -u )
  { "<ios>", kPublic, "<istream>", kPublic },
  { "<ios>", kPublic, "<ostream>", kPublic },
  { "<iosfwd>", kPublic, "<ios>", kPublic },
  { "<iosfwd>", kPublic, "<streambuf>", kPublic },
  { "<istream>", kPublic, "<fstream>", kPublic },
  { "<istream>", kPublic, "<iostream>", kPublic },
  { "<istream>", kPublic, "<sstream>", kPublic },
  { "<ostream>", kPublic, "<fstream>", kPublic },
  { "<ostream>", kPublic, "<iostream>", kPublic },
  { "<ostream>", kPublic, "<istream>", kPublic },
  { "<ostream>", kPublic, "<sstream>", kPublic },
  { "<streambuf>", kPublic, "<ios>", kPublic },
};

// GNU libstdc++ mappings generated with:
//
// mapgen/iwyu-mapgen-libstdcxx.py --lang=c++ /usr/include/c++/11 \
//     /usr/include/x86_64-linux-gnu/c++/11
//
// Do not edit!
const IncludeMapEntry libstdcpp_include_map[] = {
  // Private-to-public #include mappings.
  { "<bits/atomic_futex.h>", kPrivate, "<future>", kPublic },
  { "<bits/atomic_word.h>", kPrivate, "<ext/atomicity.h>", kPublic },
  { "<bits/cxxabi_init_exception.h>", kPrivate, "<cxxabi.h>", kPublic },
  { "<bits/exception.h>", kPrivate, "<exception>", kPublic },
  { "<debug/bitset>", kPrivate, "<bitset>", kPublic },
  { "<debug/deque>", kPrivate, "<deque>", kPublic },
  { "<debug/forward_list>", kPrivate, "<forward_list>", kPublic },
  { "<debug/list>", kPrivate, "<list>", kPublic },
  { "<debug/map>", kPrivate, "<map>", kPublic },
  { "<debug/set>", kPrivate, "<set>", kPublic },
  { "<debug/unordered_map>", kPrivate, "<unordered_map>", kPublic },
  { "<debug/unordered_set>", kPrivate, "<unordered_set>", kPublic },
  { "<debug/vector>", kPrivate, "<vector>", kPublic },
  { "<ext/pb_ds/detail/branch_policy/branch_policy.hpp>", kPrivate, "<ext/pb_ds/tree_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/branch_policy/traits.hpp>", kPrivate, "<ext/pb_ds/assoc_container.hpp>", kPublic },
  { "<ext/pb_ds/detail/container_base_dispatch.hpp>", kPrivate, "<ext/pb_ds/assoc_container.hpp>", kPublic },
  { "<ext/pb_ds/detail/hash_fn/direct_mask_range_hashing_imp.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/hash_fn/direct_mod_range_hashing_imp.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/hash_fn/linear_probe_fn_imp.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/hash_fn/mask_based_range_hashing.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/hash_fn/mod_based_range_hashing.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/hash_fn/quadratic_probe_fn_imp.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/list_update_policy/lu_counter_metadata.hpp>", kPrivate, "<ext/pb_ds/list_update_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/priority_queue_base_dispatch.hpp>", kPrivate, "<ext/pb_ds/priority_queue.hpp>", kPublic },
  { "<ext/pb_ds/detail/resize_policy/cc_hash_max_collision_check_resize_trigger_imp.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/resize_policy/hash_exponential_size_policy_imp.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/resize_policy/hash_load_check_resize_trigger_imp.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/resize_policy/hash_load_check_resize_trigger_size_base.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/resize_policy/hash_prime_size_policy_imp.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/resize_policy/hash_standard_resize_policy_imp.hpp>", kPrivate, "<ext/pb_ds/hash_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/tree_policy/order_statistics_imp.hpp>", kPrivate, "<ext/pb_ds/tree_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/trie_policy/order_statistics_imp.hpp>", kPrivate, "<ext/pb_ds/trie_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/trie_policy/prefix_search_node_update_imp.hpp>", kPrivate, "<ext/pb_ds/trie_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/trie_policy/trie_policy_base.hpp>", kPrivate, "<ext/pb_ds/trie_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/trie_policy/trie_string_access_traits_imp.hpp>", kPrivate, "<ext/pb_ds/trie_policy.hpp>", kPublic },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/list_update_policy.hpp>", kPublic },
  { "<tr2/bool_set.tcc>", kPrivate, "<tr2/bool_set>", kPublic },
  // Private-to-private #include mappings.
  { "<bits/cxxabi_init_exception.h>", kPrivate, "<bits/exception_ptr.h>", kPrivate },
  { "<bits/enable_special_members.h>", kPrivate, "<bits/hashtable.h>", kPrivate },
  { "<bits/gthr-default.h>", kPrivate, "<bits/gthr.h>", kPrivate },
  { "<bits/gthr.h>", kPrivate, "<bits/atomic_wait.h>", kPrivate },
  { "<bits/gthr.h>", kPrivate, "<bits/c++io.h>", kPrivate },
  { "<bits/gthr.h>", kPrivate, "<bits/std_mutex.h>", kPrivate },
  { "<bits/gthr.h>", kPrivate, "<bits/std_thread.h>", kPrivate },
  { "<bits/stdc++.h>", kPrivate, "<bits/extc++.h>", kPrivate },
  { "<bits/stdc++.h>", kPrivate, "<bits/stdtr1c++.h>", kPrivate },
  { "<bits/stdtr1c++.h>", kPrivate, "<bits/extc++.h>", kPrivate },
  { "<bits/uses_allocator.h>", kPrivate, "<bits/stl_queue.h>", kPrivate },
  { "<bits/uses_allocator.h>", kPrivate, "<bits/stl_stack.h>", kPrivate },
  { "<debug/assertions.h>", kPrivate, "<bits/align.h>", kPrivate },
  { "<debug/assertions.h>", kPrivate, "<bits/stl_deque.h>", kPrivate },
  { "<debug/assertions.h>", kPrivate, "<bits/stl_iterator_base_funcs.h>", kPrivate },
  { "<debug/assertions.h>", kPrivate, "<bits/stl_vector.h>", kPrivate },
  { "<debug/assertions.h>", kPrivate, "<bits/unique_ptr.h>", kPrivate },
  { "<debug/assertions.h>", kPrivate, "<debug/debug.h>", kPrivate },
  { "<debug/assertions.h>", kPrivate, "<debug/safe_iterator.h>", kPrivate },
  { "<debug/assertions.h>", kPrivate, "<debug/safe_sequence.h>", kPrivate },
  { "<debug/assertions.h>", kPrivate, "<debug/safe_unordered_container.h>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<backward/auto_ptr.h>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<bits/basic_string.h>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<bits/stl_algobase.h>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<bits/stl_heap.h>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<bits/stl_numeric.h>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<bits/stl_queue.h>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<bits/stl_stack.h>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<bits/stream_iterator.h>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<bits/streambuf_iterator.h>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/point_iterators.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/binary_heap_/const_iterator.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/binary_heap_/point_const_iterator.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/binary_heap_/resize_policy.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/binomial_heap_/binomial_heap_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/debug_map_base.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/eq_fn/hash_eq_fn.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/hash_fn/ranged_hash_fn.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/hash_fn/ranged_probe_fn.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/const_iterator.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/point_const_iterator.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/node_iterators.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_base.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/rb_tree_map_/rb_tree_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/splay_tree_/splay_tree_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<debug/debug.h>", kPrivate, "<ext/vstring_util.h>", kPrivate },
  { "<debug/formatter.h>", kPrivate, "<debug/functions.h>", kPrivate },
  { "<debug/functions.h>", kPrivate, "<debug/debug.h>", kPrivate },
  { "<debug/functions.h>", kPrivate, "<debug/safe_iterator.h>", kPrivate },
  { "<debug/functions.h>", kPrivate, "<debug/safe_sequence.h>", kPrivate },
  { "<debug/functions.h>", kPrivate, "<debug/safe_unordered_container.h>", kPrivate },
  { "<debug/helper_functions.h>", kPrivate, "<debug/functions.h>", kPrivate },
  { "<debug/helper_functions.h>", kPrivate, "<debug/stl_iterator.h>", kPrivate },
  { "<debug/macros.h>", kPrivate, "<debug/debug.h>", kPrivate },
  { "<debug/macros.h>", kPrivate, "<debug/safe_iterator.h>", kPrivate },
  { "<debug/macros.h>", kPrivate, "<debug/safe_sequence.h>", kPrivate },
  { "<debug/macros.h>", kPrivate, "<debug/safe_unordered_container.h>", kPrivate },
  { "<debug/map.h>", kPrivate, "<debug/map>", kPrivate },
  { "<debug/multimap.h>", kPrivate, "<debug/map>", kPrivate },
  { "<debug/multiset.h>", kPrivate, "<debug/set>", kPrivate },
  { "<debug/safe_base.h>", kPrivate, "<debug/safe_iterator.h>", kPrivate },
  { "<debug/safe_base.h>", kPrivate, "<debug/safe_sequence.h>", kPrivate },
  { "<debug/safe_base.h>", kPrivate, "<debug/safe_unordered_base.h>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/deque>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/forward_list>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/list>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/map.h>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/multimap.h>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/multiset.h>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/set.h>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/string>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/unordered_map>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/unordered_set>", kPrivate },
  { "<debug/safe_container.h>", kPrivate, "<debug/vector>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/bitset>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/deque>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/forward_list>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/list>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/map.h>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/multimap.h>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/multiset.h>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/set.h>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/string>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/unordered_map>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/unordered_set>", kPrivate },
  { "<debug/safe_iterator.h>", kPrivate, "<debug/vector>", kPrivate },
  { "<debug/safe_iterator.tcc>", kPrivate, "<debug/safe_iterator.h>", kPrivate },
  { "<debug/safe_local_iterator.h>", kPrivate, "<debug/unordered_map>", kPrivate },
  { "<debug/safe_local_iterator.h>", kPrivate, "<debug/unordered_set>", kPrivate },
  { "<debug/safe_local_iterator.tcc>", kPrivate, "<debug/safe_local_iterator.h>", kPrivate },
  { "<debug/safe_sequence.h>", kPrivate, "<debug/bitset>", kPrivate },
  { "<debug/safe_sequence.h>", kPrivate, "<debug/deque>", kPrivate },
  { "<debug/safe_sequence.h>", kPrivate, "<debug/forward_list>", kPrivate },
  { "<debug/safe_sequence.h>", kPrivate, "<debug/list>", kPrivate },
  { "<debug/safe_sequence.h>", kPrivate, "<debug/map.h>", kPrivate },
  { "<debug/safe_sequence.h>", kPrivate, "<debug/multimap.h>", kPrivate },
  { "<debug/safe_sequence.h>", kPrivate, "<debug/multiset.h>", kPrivate },
  { "<debug/safe_sequence.h>", kPrivate, "<debug/set.h>", kPrivate },
  { "<debug/safe_sequence.h>", kPrivate, "<debug/string>", kPrivate },
  { "<debug/safe_sequence.h>", kPrivate, "<debug/vector>", kPrivate },
  { "<debug/safe_sequence.tcc>", kPrivate, "<debug/safe_sequence.h>", kPrivate },
  { "<debug/safe_unordered_base.h>", kPrivate, "<debug/safe_local_iterator.h>", kPrivate },
  { "<debug/safe_unordered_base.h>", kPrivate, "<debug/safe_unordered_container.h>", kPrivate },
  { "<debug/safe_unordered_container.h>", kPrivate, "<debug/unordered_map>", kPrivate },
  { "<debug/safe_unordered_container.h>", kPrivate, "<debug/unordered_set>", kPrivate },
  { "<debug/safe_unordered_container.tcc>", kPrivate, "<debug/safe_unordered_container.h>", kPrivate },
  { "<debug/set.h>", kPrivate, "<debug/set>", kPrivate },
  { "<debug/stl_iterator.h>", kPrivate, "<bits/stl_iterator.h>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate, "<ext/pb_ds/detail/container_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/find_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/info_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/iterators_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/node_iterators.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/point_iterators.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/policy_access_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/policy_access_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/rotate_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/split_join_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/bin_search_tree_/traits.hpp>", kPrivate, "<ext/pb_ds/detail/branch_policy/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate, "<ext/pb_ds/detail/priority_queue_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/entry_cmp.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/entry_pred.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/find_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/info_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/iterators_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/point_const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/point_const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/const_iterator.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/policy_access_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/resize_policy.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/split_join_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binary_heap_/trace_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_/binomial_heap_.hpp>", kPrivate, "<ext/pb_ds/detail/priority_queue_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_/binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_/binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_/binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_base_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_base_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_base_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_base_/find_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_base_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/binomial_heap_base_/split_join_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/branch_policy/branch_policy.hpp>", kPrivate, "<ext/pb_ds/detail/tree_trace_base.hpp>", kPrivate },
  { "<ext/pb_ds/detail/branch_policy/branch_policy.hpp>", kPrivate, "<ext/pb_ds/detail/trie_policy/trie_policy_base.hpp>", kPrivate },
  { "<ext/pb_ds/detail/branch_policy/null_node_metadata.hpp>", kPrivate, "<ext/pb_ds/detail/rb_tree_map_/node.hpp>", kPrivate },
  { "<ext/pb_ds/detail/branch_policy/null_node_metadata.hpp>", kPrivate, "<ext/pb_ds/detail/standard_policies.hpp>", kPrivate },
  { "<ext/pb_ds/detail/branch_policy/null_node_metadata.hpp>", kPrivate, "<ext/pb_ds/detail/tree_policy/node_metadata_selector.hpp>", kPrivate },
  { "<ext/pb_ds/detail/branch_policy/null_node_metadata.hpp>", kPrivate, "<ext/pb_ds/detail/tree_trace_base.hpp>", kPrivate },
  { "<ext/pb_ds/detail/branch_policy/null_node_metadata.hpp>", kPrivate, "<ext/pb_ds/detail/trie_policy/node_metadata_selector.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate, "<ext/pb_ds/detail/container_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/constructor_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/debug_no_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/debug_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/debug_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/debug_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/entry_list_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/erase_no_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/erase_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/erase_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/erase_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/find_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/insert_no_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/insert_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/insert_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/insert_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/iterators_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/policy_access_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/resize_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/resize_no_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/resize_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/resize_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/resize_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/size_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cc_hash_table_map_/trace_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_/binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/debug_map_base.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/debug_map_base.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/debug_map_base.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/debug_map_base.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/debug_map_base.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/debug_map_base.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/eq_fn/eq_by_less.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/eq_fn/eq_by_less.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/eq_fn/eq_by_less.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/eq_fn/hash_eq_fn.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/eq_fn/hash_eq_fn.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/constructor_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/debug_no_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/debug_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/debug_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/debug_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/erase_no_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/erase_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/erase_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/erase_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/find_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate, "<ext/pb_ds/detail/container_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/info_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/insert_no_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/insert_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/insert_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/insert_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/iterator_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/policy_access_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/resize_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/resize_no_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/resize_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/resize_store_hash_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/resize_fn_imps.hpp>", kPrivate },
  { "<ext/pb_ds/detail/gp_hash_table_map_/trace_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/hash_fn/ranged_hash_fn.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/hash_fn/ranged_probe_fn.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/info_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/iterators_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate, "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/node.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/point_const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/const_iterator.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/point_const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/policy_access_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/left_child_next_sibling_heap_/trace_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/list_update_map_/constructor_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/list_update_map_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/list_update_map_/entry_metadata_base.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/list_update_map_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/list_update_map_/find_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/list_update_map_/info_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/list_update_map_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/list_update_map_/iterators_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate, "<ext/pb_ds/detail/container_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/list_update_map_/trace_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/ov_tree_map_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/ov_tree_map_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/ov_tree_map_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/ov_tree_map_/info_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/ov_tree_map_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/ov_tree_map_/iterators_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/ov_tree_map_/node_iterators.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate, "<ext/pb_ds/detail/container_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/ov_tree_map_/split_join_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/ov_tree_map_/traits.hpp>", kPrivate, "<ext/pb_ds/detail/branch_policy/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pairing_heap_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pairing_heap_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pairing_heap_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pairing_heap_/find_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pairing_heap_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate, "<ext/pb_ds/detail/priority_queue_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pairing_heap_/split_join_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/find_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/info_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/insert_join_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/iterators_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate, "<ext/pb_ds/detail/container_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/pat_trie_base.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/pat_trie_base.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/policy_access_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/split_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/synth_access_traits.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/synth_access_traits.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/trace_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/traits.hpp>", kPrivate, "<ext/pb_ds/detail/branch_policy/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/pat_trie_/update_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rb_tree_map_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rb_tree_map_/rb_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rb_tree_map_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rb_tree_map_/rb_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rb_tree_map_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rb_tree_map_/rb_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rb_tree_map_/info_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rb_tree_map_/rb_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rb_tree_map_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rb_tree_map_/rb_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rb_tree_map_/node.hpp>", kPrivate, "<ext/pb_ds/detail/rb_tree_map_/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rb_tree_map_/rb_tree_.hpp>", kPrivate, "<ext/pb_ds/detail/container_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rb_tree_map_/split_join_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rb_tree_map_/rb_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rb_tree_map_/traits.hpp>", kPrivate, "<ext/pb_ds/detail/branch_policy/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rc_binomial_heap_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rc_binomial_heap_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rc_binomial_heap_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rc_binomial_heap_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rc_binomial_heap_/rc.hpp>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate, "<ext/pb_ds/detail/priority_queue_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rc_binomial_heap_/split_join_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/rc_binomial_heap_/trace_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/splay_tree_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/splay_tree_/splay_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/splay_tree_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/splay_tree_/splay_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/splay_tree_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/splay_tree_/splay_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/splay_tree_/find_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/splay_tree_/splay_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/splay_tree_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/splay_tree_/splay_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/splay_tree_/node.hpp>", kPrivate, "<ext/pb_ds/detail/splay_tree_/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/splay_tree_/splay_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/splay_tree_/splay_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/splay_tree_/splay_tree_.hpp>", kPrivate, "<ext/pb_ds/detail/container_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/splay_tree_/split_join_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/splay_tree_/splay_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/splay_tree_/traits.hpp>", kPrivate, "<ext/pb_ds/detail/branch_policy/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/standard_policies.hpp>", kPrivate, "<ext/pb_ds/detail/rb_tree_map_/rb_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/thin_heap_/constructors_destructor_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/thin_heap_/debug_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/thin_heap_/erase_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/thin_heap_/find_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/thin_heap_/insert_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/thin_heap_/split_join_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate, "<ext/pb_ds/detail/priority_queue_base_dispatch.hpp>", kPrivate },
  { "<ext/pb_ds/detail/thin_heap_/trace_fn_imps.hpp>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/tree_policy/node_metadata_selector.hpp>", kPrivate, "<ext/pb_ds/detail/branch_policy/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/tree_trace_base.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/tree_trace_base.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/trie_policy/node_metadata_selector.hpp>", kPrivate, "<ext/pb_ds/detail/branch_policy/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/binary_heap_/binary_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_/binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/binomial_heap_base_/binomial_heap_base_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/left_child_next_sibling_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/node_iterators.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/pairing_heap_/pairing_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/synth_access_traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/rc_binomial_heap_/rc_binomial_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/thin_heap_/thin_heap_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/type_utils.hpp>", kPrivate, "<ext/pb_ds/detail/types_traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/bin_search_tree_/bin_search_tree_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/branch_policy/branch_policy.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/branch_policy/null_node_metadata.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/branch_policy/traits.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/cond_dealtor.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/eq_fn/eq_by_less.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/eq_fn/hash_eq_fn.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/hash_fn/ranged_hash_fn.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/left_child_next_sibling_heap_/node.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/ov_tree_map_/ov_tree_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/pat_trie_/pat_trie_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/tree_policy/node_metadata_selector.hpp>", kPrivate },
  { "<ext/pb_ds/detail/types_traits.hpp>", kPrivate, "<ext/pb_ds/detail/trie_policy/node_metadata_selector.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/iterator.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/iterator.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/iterator.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/point_const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/point_const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/point_const_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/point_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/cc_hash_table_map_/cc_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/point_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/gp_hash_table_map_/gp_ht_map_.hpp>", kPrivate },
  { "<ext/pb_ds/detail/unordered_iterator/point_iterator.hpp>", kPrivate, "<ext/pb_ds/detail/list_update_map_/lu_map_.hpp>", kPrivate },
};

const IncludeMapEntry libcxx_symbol_map[] = {
    {"std::nullptr_t", kPrivate, "<cstddef>", kPublic},

    // For older MacOS libc++ (13.0.0), on macOS Ventura (13.2.1)
    {"std::string", kPrivate, "<string>", kPublic},
};

const IncludeMapEntry libcxx_include_map[] = {
    {"<__mutex_base>", kPrivate, "<mutex>", kPublic},
    {"<__tree>", kPrivate, "<set>", kPublic},
    {"<__tree>", kPrivate, "<map>", kPublic},

    // For the following entries:
    // cd llvm-project/libcxx/include ; find -type d -name "__*" | sort | sed -e 's#./__\(.*\)#  { "@<__\1/.*>", kPrivate, "<\1>", kPublic },#'
    //
    // tweak locale_dir entry, and comment out debug_utils, fwd, pstl, support
    {"@<__algorithm/.*>", kPrivate, "<algorithm>", kPublic},
    {"@<__atomic/.*>", kPrivate, "<atomic>", kPublic},
    {"@<__bit/.*>", kPrivate, "<bit>", kPublic},
    {"@<__charconv/.*>", kPrivate, "<charconv>", kPublic},
    {"@<__chrono/.*>", kPrivate, "<chrono>", kPublic},
    {"@<__compare/.*>", kPrivate, "<compare>", kPublic},
    {"@<__concepts/.*>", kPrivate, "<concepts>", kPublic},
    {"@<__condition_variable/.*>", kPrivate, "<condition_variable>", kPublic},
    {"@<__coroutine/.*>", kPrivate, "<coroutine>", kPublic},
    //{ "@<__debug_utils/.*>", kPrivate, "<debug_utils>", kPublic },
    {"@<__exception/.*>", kPrivate, "<exception>", kPublic},
    {"@<__expected/.*>", kPrivate, "<expected>", kPublic},
    {"@<__filesystem/.*>", kPrivate, "<filesystem>", kPublic},
    {"@<__format/.*>", kPrivate, "<format>", kPublic},
    {"@<__functional/.*>", kPrivate, "<functional>", kPublic},
    //{ "@<__fwd/.*>", kPrivate, "<fwd>", kPublic },
    {"@<__ios/.*>", kPrivate, "<ios>", kPublic},
    {"@<__iterator/.*>", kPrivate, "<iterator>", kPublic},
    {"@<__locale_dir/.*>", kPrivate, "<locale>", kPublic},
    {"@<__mdspan/.*>", kPrivate, "<mdspan>", kPublic},
    {"@<__memory/.*>", kPrivate, "<memory>", kPublic},
    {"@<__memory_resource/.*>", kPrivate, "<memory_resource>", kPublic},
    {"@<__mutex/.*>", kPrivate, "<mutex>", kPublic},
    {"@<__numeric/.*>", kPrivate, "<numeric>", kPublic},
    //{"@<__pstl/.*>", kPrivate, "<pstl>", kPublic},
    {"@<__random/.*>", kPrivate, "<random>", kPublic},
    {"@<__ranges/.*>", kPrivate, "<ranges>", kPublic},
    {"@<__stop_token/.*>", kPrivate, "<stop_token>", kPublic},
    {"@<__string/.*>", kPrivate, "<string>", kPublic},
    //{ "@<__support/.*>", kPrivate, "<support>", kPublic },
    {"@<__system_error/.*>", kPrivate, "<system_error>", kPublic},
    {"@<__thread/.*>", kPrivate, "<thread>", kPublic},
    {"@<__tuple/.*>", kPrivate, "<tuple>", kPublic},
    {"@<__type_traits/.*>", kPrivate, "<type_traits>", kPublic},
    {"@<__utility/.*>", kPrivate, "<utility>", kPublic},
    {"@<__variant/.*>", kPrivate, "<variant>", kPublic},


    // For the following entries:
    // cd llvm-project/libcxx/include ; find __fwd -type f -name "*.h" | sort | sed -E 's#__fwd/(.*).h#  { "<__fwd/\1.h>", kPrivate, "<\1>", kPublic },# ; s#<(fstream|ios|istream|ostream|sstream|streambuf)>#<iosfwd>#'
    //
    // tweak hash, pair, subrange entries, and comment out bit_reference, get
    {"<__fwd/array.h>", kPrivate, "<array>", kPublic},
    //{"<__fwd/bit_reference.h>", kPrivate, "<bit_reference>", kPublic},
    {"<__fwd/fstream.h>", kPrivate, "<iosfwd>", kPublic},
    //{"<__fwd/get.h>", kPrivate, "<get>", kPublic},
    {"<__fwd/hash.h>", kPrivate, "<functional>", kPublic},
    {"<__fwd/ios.h>", kPrivate, "<iosfwd>", kPublic},
    {"<__fwd/istream.h>", kPrivate, "<iosfwd>", kPublic},
    {"<__fwd/mdspan.h>", kPrivate, "<mdspan>", kPublic},
    {"<__fwd/memory_resource.h>", kPrivate, "<memory_resource>", kPublic},
    {"<__fwd/ostream.h>", kPrivate, "<iosfwd>", kPublic},
    {"<__fwd/pair.h>", kPrivate, "<utility>", kPublic},
    {"<__fwd/span.h>", kPrivate, "<span>", kPublic},
    {"<__fwd/sstream.h>", kPrivate, "<iosfwd>", kPublic},
    {"<__fwd/streambuf.h>", kPrivate, "<iosfwd>", kPublic},
    {"<__fwd/string.h>", kPrivate, "<string>", kPublic},
    {"<__fwd/string_view.h>", kPrivate, "<string_view>", kPublic},
    {"<__fwd/subrange.h>", kPrivate, "<ranges>", kPublic},
    {"<__fwd/tuple.h>", kPrivate, "<tuple>", kPublic},

    // For older MacOS libc++ (13.0.0), on macOS Ventura (13.2.1)
    {"<__functional_base>", kPrivate, "<functional>", kPublic},

    // For older libc++ (16.x)
    {"@<__tuple_dir/.*>", kPrivate, "<tuple>", kPublic},
};

// Returns true if str is a valid quoted filepath pattern (i.e. either
// a quoted filepath or "@" followed by a regex for matching a quoted
// filepath).
bool IsQuotedFilepathPattern(const string& str) {
  return IsQuotedInclude(str) || StartsWith(str, "@");
}

// Given a vector of nodes, augment each node with its children, as
// defined by m: nodes[i] is replaced by nodes[i] + m[nodes[i]],
// ignoring duplicates.  The input vector is modified in place.
void ExpandOnce(const IncludePicker::IncludeMap& m,
                vector<MappedInclude>* nodes) {
  vector<MappedInclude> nodes_and_children;
  set<string> seen_nodes_and_children;
  for (const MappedInclude& node : *nodes) {
    // First insert the node itself, then all its kids.
    if (!ContainsKey(seen_nodes_and_children, node.quoted_include)) {
      nodes_and_children.push_back(node);
      seen_nodes_and_children.insert(node.quoted_include);
    }
    if (const vector<MappedInclude>* children =
        FindInMap(&m, node.quoted_include)) {
      for (const MappedInclude& child : *children) {
        if (!ContainsKey(seen_nodes_and_children, child.quoted_include)) {
          nodes_and_children.push_back(child);
          seen_nodes_and_children.insert(child.quoted_include);
        }
      }
    }
  }
  nodes->swap(nodes_and_children);  // modify nodes in-place
}

enum TransitiveStatus { kUnused = 0, kCalculating, kDone };

// If the filename-map maps a.h to b.h, and also b.h to c.h, then
// there's a transitive mapping of a.h to c.h.  We want to add that
// into the filepath map as well, to make lookups easier.  We do this
// by doing a depth-first search for a single mapping, recursing
// whenever the value is itself a key in the map, and putting the
// results in a vector of all values seen.
// NOTE: This function updates values seen in filename_map, but
// does not invalidate any filename_map iterators.
void MakeNodeTransitive(IncludePicker::IncludeMap* filename_map,
                        map<string, TransitiveStatus>* seen_nodes,
                        vector<string>* node_stack,  // used for debugging
                        const string& key) {
  // If we've already calculated this node's transitive closure, we're done.
  const TransitiveStatus status = (*seen_nodes)[key];
  if (status == kCalculating) {  // means there's a cycle in the mapping
    // Note that cycles in mappings are generally benign, the cycle detection
    // here is only necessary to protect the recursive algorithm from infinite
    // regress. We will still expand all reachable nodes in the graph to a
    // plain sequence representing the transitive closure.
    // The expanded mappings are only used for simple lookup, never followed
    // recursively (which could have necessitated preserving cycles and handling
    // them in that traversal too).
    // Log cycles at a high verbosity level to aid debugging.
    VERRS(8) << "Ignored cycle in include mappings: ";
    for (const string& node : *node_stack)
      VERRS(8) << node << " -> ";
    VERRS(8) << key << "\n";
    return;
  }
  if (status == kDone)
    return;

  IncludePicker::IncludeMap::iterator node = filename_map->find(key);
  if (node == filename_map->end()) {
    (*seen_nodes)[key] = kDone;
    return;
  }

  // Keep track of node->second as we update it, to avoid duplicates.
  (*seen_nodes)[key] = kCalculating;
  for (const MappedInclude& child : node->second) {
    node_stack->push_back(child.quoted_include);
    MakeNodeTransitive(filename_map, seen_nodes, node_stack,
                       child.quoted_include);
    node_stack->pop_back();
  }
  (*seen_nodes)[key] = kDone;

  // Our transitive closure is just the union of the closure of our
  // children.  This routine replaces our value with this closure,
  // by replacing each of our values with its values.  Since our
  // values have already been made transitive, that is a closure.
  ExpandOnce(*filename_map, &node->second);
}

// Updates the values in filename_map based on its transitive mappings.
void MakeMapTransitive(IncludePicker::IncludeMap* filename_map) {
  // Insert keys of filename_map here once we know their value is
  // the complete transitive closure.
  map<string, TransitiveStatus> seen_nodes;
  vector<string> node_stack;
  for (const IncludePicker::IncludeMap::value_type& includes : *filename_map)
    MakeNodeTransitive(filename_map, &seen_nodes, &node_stack, includes.first);
}

// Get a scalar value from a YAML node.
// Returns empty string if it's not of type ScalarNode.
string GetScalarValue(Node* node) {
  ScalarNode* scalar = llvm::dyn_cast<ScalarNode>(node);
  if (scalar == nullptr)
    return string();

  llvm::SmallString<8> storage;
  return scalar->getValue(storage).str();
}

// Get a sequence value from a YAML node.
// Returns empty vector if it's not of type SequenceNode.
vector<string> GetSequenceValue(Node* node) {
  vector<string> result;

  SequenceNode* sequence = llvm::dyn_cast<SequenceNode>(node);
  if (sequence != nullptr) {
    for (Node& node : *sequence) {
      result.push_back(GetScalarValue(&node));
    }
  }

  return result;
}

// If new_path doesn't already exist in search_path, makes a copy of search_path
// and adds new_path to it.
// Returns the original or extended search path.
vector<string> ExtendMappingFileSearchPath(const vector<string>& search_path,
                                           const string& new_path) {
  CHECK_(IsAbsolutePath(new_path));

  if (std::find(search_path.begin(),
                search_path.end(),
                new_path) == search_path.end()) {
    vector<string> extended(search_path);
    extended.push_back(new_path);
    return extended;
  }

  return search_path;
}

// Scans search_path for existing files with filename.
// If filename is absolute and exists, return it.
// If filename is relative and exists based on cwd, return it in absolute form.
// If filename is relative and doesn't exist, try to find it along search_path.
// Returns an absolute filename if file is found, otherwise filename untouched.
string FindFileInSearchPath(const vector<string>& search_path,
                            const string& filename) {
  if (llvm::sys::fs::exists(filename)) {
    // If the file exists, no matter if its path is relative or absolute,
    // return it in absolute form.
    return MakeAbsolutePath(filename);
  } else if (!IsAbsolutePath(filename)) {
    // If it's relative, scan search path.
    for (const string& base_path : search_path) {
      string candidate = MakeAbsolutePath(base_path, filename);
      if (llvm::sys::fs::exists(candidate)) {
        return candidate;
      }
    }
  }

  // This is proven not to exist, so handle the error when
  // we attempt to open it.
  return filename;
}

void PrintMappings(const IncludePicker::IncludeMap& map, const char* name) {
  if (map.empty()) {
    llvm::errs() << name << ": empty, no statistics\n";
    return;
  }

  // Compute and print count/min/max/average.
  std::vector<size_t> lengths;
  for (const auto& m : map) {
    lengths.push_back(m.second.size());
  }

  size_t count = lengths.size();
  size_t min = *std::min_element(lengths.begin(), lengths.end());
  size_t max = *std::max_element(lengths.begin(), lengths.end());
  double avg = std::accumulate(lengths.begin(), lengths.end(), 0.0) / count;
  llvm::errs() << name << ": count=" << count << ", min=" << min
               << ", max=" << max << ", average=" << llvm::format("%.2f", avg)
               << "\n";

  // Sort all mappings by size, ascending, and print them.
  vector<pair<string, vector<MappedInclude>>> mappings(map.begin(), map.end());
  std::stable_sort(mappings.begin(), mappings.end(), [](auto& lhs, auto& rhs) {
    return rhs.second.size() < lhs.second.size();
  });

  llvm::errs() << "---\n";
  for (const auto& m : mappings) {
    llvm::errs() << m.first << ": [";
    llvm::interleaveComma(m.second, llvm::errs(), [](const MappedInclude& x) {
      llvm::errs() << x.quoted_include;
    });
    llvm::errs() << "]\n";
  }
  llvm::errs() << "---\n";
}

}  // anonymous namespace

MappedInclude::MappedInclude(const string& q, const string& p)
  : quoted_include(q)
  , path(p)
{
  CHECK_(IsQuotedInclude(quoted_include)) <<
    "Must be quoted include, was: " << quoted_include;
}

bool MappedInclude::HasAbsoluteQuotedInclude() const {
  if (!StartsWith(quoted_include, "\"") || quoted_include.size() < 2) {
    return false;
  }
  string path(quoted_include.begin() + 1, quoted_include.end() - 1);
  return IsAbsolutePath(path);
}

IncludePicker::IncludePicker(RegexDialect regex_dialect,
                             CStdLib cstdlib,
                             CXXStdLib cxxstdlib)
    : has_called_finalize_added_include_lines_(false),
      regex_dialect(regex_dialect) {
  AddDefaultMappings(cstdlib, cxxstdlib);
}

void IncludePicker::AddDefaultMappings(CStdLib cstdlib,
                                       CXXStdLib cxxstdlib) {
  using clang::tooling::stdlib::Header;
  using clang::tooling::stdlib::Lang;
  using clang::tooling::stdlib::Symbol;

  if (cstdlib == CStdLib::Glibc) {
    AddSymbolMappings(libc_symbol_map, IWYU_ARRAYSIZE(libc_symbol_map));
    AddIncludeMappings(libc_include_map, IWYU_ARRAYSIZE(libc_include_map));
  } else if (cstdlib == CStdLib::ClangSymbols) {
    // Get canonical C standard library mappings from clang tooling
    for (const Symbol& sym : Symbol::all(Lang::C)) {
      string name = sym.name().str();

      // the canonical header is returned first
      for (const Header& header : sym.headers()) {
        AddSymbolMapping(name, MappedInclude(header.name().str()), kPublic);
      }
    }
  }

  if (cxxstdlib == CXXStdLib::Libstdcxx) {
    AddSymbolMappings(libstdcpp_symbol_map,
                      IWYU_ARRAYSIZE(libstdcpp_symbol_map));
    AddIncludeMappings(libstdcpp_include_map,
                       IWYU_ARRAYSIZE(libstdcpp_include_map));
  } else if (cxxstdlib == CXXStdLib::Libcxx) {
    AddSymbolMappings(libcxx_symbol_map, IWYU_ARRAYSIZE(libcxx_symbol_map));
    AddIncludeMappings(libcxx_include_map, IWYU_ARRAYSIZE(libcxx_include_map));
  } else if (cxxstdlib == CXXStdLib::ClangSymbols) {
    // Get canonical C++ standard library mappings from clang tooling
    for (const Symbol& sym : Symbol::all(Lang::CXX)) {
      string name = sym.qualifiedName().str();

      // the canonical header is returned first
      for (const Header& header : sym.headers()) {
        AddSymbolMapping(name, MappedInclude(header.name().str()), kPublic);
      }
    }
  }

  if (cxxstdlib != CXXStdLib::None) {
    // Map C headers to associated C++ headers. The standard library
    // mappings shouldn't be mentioning the C headers.
    AddIncludeMappings(stdlib_c_include_map,
                       IWYU_ARRAYSIZE(stdlib_c_include_map));
    // C++ include mappings to allow different public headers that
    // generally include each other.
    AddIncludeMappings(stdlib_cpp_include_map,
                       IWYU_ARRAYSIZE(stdlib_cpp_include_map));

    // Add common C++ mappings to deal with generic C++ standard
    // library symbol issues (so the standard library doesn't have to
    // do this too). If it does that's ok.
    AddSymbolMappings(stdlib_cxx_symbol_map,
                      IWYU_ARRAYSIZE(stdlib_cxx_symbol_map));

    AddPublicIncludes(stdlib_cpp_public_headers,
                      IWYU_ARRAYSIZE(stdlib_cpp_public_headers));
  }
}

void IncludePicker::MarkVisibility(VisibilityMap* map,
                                   const string& key,
                                   IncludeVisibility visibility) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");

  // insert() leaves any old value alone, and only inserts if the key is new.
  map->insert(make_pair(key, visibility));
  CHECK_((*map)[key] == visibility)
      << " Same file seen with two different visibilities: "
      << key
      << " Old vis: " << (*map)[key]
      << " New vis: " << visibility;
}

// AddDirectInclude lets us use some hard-coded rules to add filepath
// mappings at runtime.  It includes, for instance, mappings from
// 'project/internal/foo.h' to 'project/public/foo_public.h' in google
// code (Google hides private headers in /internal/, much like glibc
// hides them in /bits/.)
void IncludePicker::AddDirectInclude(const string& includer_filepath,
                                     const string& includee_filepath,
                                     const string& quoted_include_as_written) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");

  // Note: the includer may be a .cc file, which is unnecessary to add
  // to our map, but harmless.
  const string quoted_includer = ConvertToQuotedInclude(includer_filepath);
  const string quoted_includee = ConvertToQuotedInclude(includee_filepath);
  MappedInclude mapped_includer(quoted_includer, includer_filepath);

  quoted_includes_to_quoted_includers_[quoted_includee].insert(quoted_includer);
  const pair<string, string> key(includer_filepath, includee_filepath);
  includer_and_includee_to_include_as_written_[key] = quoted_include_as_written;

  // Mark the clang fake-file "<built-in>" as private, so we never try
  // to map anything to it.
  if (includer_filepath == "<built-in>")
    MarkIncludeAsPrivate("\"<built-in>\"");

  // Automatically mark files in foo/internal/bar as private, and map them.
  // Then say that everyone else in foo/.* is a friend, who is allowed to
  // include the otherwise-private header.
  const size_t internal_pos = quoted_includee.find("internal/");
  if (internal_pos != string::npos &&
      (internal_pos == 0 || quoted_includee[internal_pos - 1] == '/')) {
    MarkIncludeAsPrivate(quoted_includee);
    // The second argument here is a regex for matching a quoted
    // filepath.  We get the opening quote from quoted_includee, and
    // the closing quote as part of the .*.
    AddFriendRegex(includee_filepath,
                   quoted_includee.substr(0, internal_pos) + ".*");
    VERRS(8) << "Adding dynamic mapping for internal/ header\n";
    AddMapping(quoted_includee, mapped_includer);
  }

  // Automatically mark <asm-FOO/bar.h> as private, and map to <asm/bar.h>.
  if (StartsWith(quoted_includee, "<asm-")) {
    MarkIncludeAsPrivate(quoted_includee);
    string public_header = quoted_includee;
    StripPast(&public_header, "/");   // read past "asm-whatever/"
    public_header = "<asm/" + public_header;   // now it's <asm/something.h>
    VERRS(8) << "Adding dynamic mapping for <asm-*> header\n";
    AddMapping(quoted_includee, MappedInclude(public_header));
  }
}

void IncludePicker::AddMapping(const string& map_from,
                               const MappedInclude& map_to) {
  VERRS(8) << "Adding mapping from " << map_from << " to "
           << map_to.quoted_include << "\n";
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  CHECK_(IsQuotedFilepathPattern(map_from)
         && "All map keys must be quoted filepaths or @ followed by regex");
  filepath_include_map_[map_from].push_back(map_to);
}

void IncludePicker::AddIncludeMapping(const string& map_from,
                                      IncludeVisibility from_visibility,
                                      const MappedInclude& map_to,
                                      IncludeVisibility to_visibility) {
  AddMapping(map_from, map_to);
  MarkVisibility(&include_visibility_map_, map_from, from_visibility);
  MarkVisibility(&include_visibility_map_, map_to.quoted_include,
                 to_visibility);
}

void IncludePicker::AddSymbolMapping(const string& map_from,
                                     const MappedInclude& map_to,
                                     IncludeVisibility to_visibility) {
  symbol_include_map_[map_from].push_back(map_to);

  MarkVisibility(&include_visibility_map_, map_to.quoted_include,
                 to_visibility);
}

void IncludePicker::AddIncludeMappings(const IncludeMapEntry* entries,
                                       size_t count) {
  for (size_t i = 0; i < count; ++i) {
    const IncludeMapEntry& e = entries[i];
    AddIncludeMapping(e.map_from, e.from_visibility, MappedInclude(e.map_to),
                      e.to_visibility);
  }
}

void IncludePicker::AddSymbolMappings(const IncludeMapEntry* entries,
                                      size_t count) {
  for (size_t i = 0; i < count; ++i) {
    const IncludeMapEntry& e = entries[i];
    AddSymbolMapping(e.map_from, MappedInclude(e.map_to), e.to_visibility);
  }
}

void IncludePicker::AddPublicIncludes(const char** includes, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    const char* include = includes[i];
    MarkVisibility(&include_visibility_map_, include, kPublic);
  }
}

void IncludePicker::MarkIncludeAsPrivate(
    const string& quoted_filepath_pattern) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  CHECK_(IsQuotedFilepathPattern(quoted_filepath_pattern)
         && "MIAP takes a quoted filepath pattern");
  MarkVisibility(&include_visibility_map_, quoted_filepath_pattern, kPrivate);
}

void IncludePicker::MarkPathAsPrivate(const string& path) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  MarkVisibility(&path_visibility_map_, path, kPrivate);
}

void IncludePicker::AddFriendRegex(const string& includee_filepath,
                                   const string& quoted_friend_regex) {
  friend_to_headers_map_["@" + quoted_friend_regex].insert(includee_filepath);
}

namespace {

// Given a map keyed by quoted filepath patterns, return a vector
// containing the @-regexes among the keys.
template <typename MapType>
vector<string> ExtractKeysMarkedAsRegexes(const MapType& m) {
  vector<string> regex_keys;
  for (const typename MapType::value_type& item : m) {
    if (StartsWith(item.first, "@"))
      regex_keys.push_back(item.first);
  }
  return regex_keys;
}

bool ContainsQuotedInclude(
    const vector<MappedInclude>& mapped_includes,
    const string& quoted_include) {
  for (const MappedInclude& mapped : mapped_includes) {
    if (mapped.quoted_include == quoted_include) {
      return true;
    }
  }
  return false;
}

}  // anonymous namespace

// Expands the regex keys in filepath_include_map_ and
// friend_to_headers_map_ by matching them against all source files
// seen by iwyu.  For each include that matches the regex, we add it
// to the map by copying the regex entry and replacing the key with
// the seen #include.
void IncludePicker::ExpandRegexes() {
  // First, get the regex keys.
  const vector<string> filepath_include_map_regex_keys =
      ExtractKeysMarkedAsRegexes(filepath_include_map_);
  const vector<string> friend_to_headers_map_regex_keys =
      ExtractKeysMarkedAsRegexes(friend_to_headers_map_);

  // Then, go through all #includes to see if they match the regexes,
  // discarding the identity mappings.  TODO(wan): to improve
  // performance, don't construct more than one Regex object for each
  // element in the above vectors.
  for (const auto& incmap : quoted_includes_to_quoted_includers_) {
    const string& hdr = incmap.first;
    for (const string& regex_key : filepath_include_map_regex_keys) {
      const string regex = regex_key.substr(1);
      const vector<MappedInclude>& map_to = filepath_include_map_[regex_key];
      if (RegexMatch(regex_dialect, hdr, regex) &&
          !ContainsQuotedInclude(map_to, hdr)) {
        for (const MappedInclude& target : map_to) {
          filepath_include_map_[hdr].push_back(MappedInclude(
              RegexReplace(regex_dialect, hdr, regex, target.quoted_include)));
        }
        MarkVisibility(&include_visibility_map_, hdr,
                       include_visibility_map_[regex_key]);
      }
    }
    for (const string& regex_key : friend_to_headers_map_regex_keys) {
      if (RegexMatch(regex_dialect, hdr, regex_key.substr(1))) {
        InsertAllInto(friend_to_headers_map_[regex_key],
                      &friend_to_headers_map_[hdr]);
      }
    }
  }
}

// Handle work that's best done after we've seen all the mappings
// (including dynamically-added ones) and all the include files.
// For instance, we can now expand all the regexes we've seen in
// the mapping-keys, since we have the full list of #includes to
// match them again.  We also transitively-close the maps.
void IncludePicker::FinalizeAddedIncludes() {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't call FAI twice");

  // The map keys may be regular expressions.
  // Match those to seen #includes now.
  ExpandRegexes();

  // If a.h maps to b.h maps to c.h, we'd like an entry from a.h to c.h too.
  MakeMapTransitive(&filepath_include_map_);
  // Now that filepath_include_map_ is transitively closed, it's an
  // easy task to get the values of symbol_include_map_ closed too.
  for (IncludeMap::value_type& symbol_include : symbol_include_map_) {
    ExpandOnce(filepath_include_map_, &symbol_include.second);
  }

  has_called_finalize_added_include_lines_ = true;

  // Print some mapping statistics.
  if (ShouldPrint(9)) {
    PrintMappings(filepath_include_map_, "filepath_include_map_");
    PrintMappings(symbol_include_map_, "symbol_include_map_");
  }
}

// For the given key, return the vector of values associated with that
// key, or an empty vector if the key does not exist in the map.
// *However*, we filter out all values that have private visibility
// before returning the vector.  *Also*, if the key is public in
// the map, we insert the key as the first of the returned values,
// this is an implicit "self-map."
vector<MappedInclude> IncludePicker::GetPublicValues(
    const IncludePicker::IncludeMap& m, const string& key) const {
  CHECK_(!StartsWith(key, "@"));
  vector<MappedInclude> retval;
  const vector<MappedInclude>* values = FindInMap(&m, key);
  if (!values || values->empty())
    return retval;

  for (const MappedInclude& value : *values) {
    CHECK_(!StartsWith(value.quoted_include, "@"));
    if (GetVisibility(value, kPublic) == kPublic)
      retval.push_back(value);
  }
  return retval;
}

string IncludePicker::MaybeGetIncludeNameAsWritten(
    const string& includer_filepath, const string& includee_filepath) const {
  const pair<string, string> key(includer_filepath, includee_filepath);
  // I want to use GetOrDefault here, but it has trouble deducing tpl args.
  const string* value = FindInMap(&includer_and_includee_to_include_as_written_,
                                  key);
  return value ? *value : "";
}

vector<string> IncludePicker::BestQuotedIncludesForIncluder(
    const vector<MappedInclude>& includes,
    const string& including_filepath) const {
  // Convert each MappedInclude to a quoted include, according to the
  // following priorities:
  // 1. If the file is already included, use whatever name it's already
  //    included via.  This is better to use than ConvertToQuotedInclude
  //    because it avoids trouble when the same file is accessible via
  //    different include search-paths, or is accessed via a symlink.
  // 2. If the quoted include in the MappedInclude object is an absolute path,
  //    that's unlikely to be what's wanted.  Try to convert it to a relative
  //    include via ConvertToQuotedInclude.
  // 3. Otherwise, use the quoted include present in the MappedInclude.
  const string including_path =
      MakeAbsolutePath(GetParentPath(including_filepath));
  vector<string> retval;
  for (const MappedInclude& mapped_include : includes) {
    const string& quoted_include_as_written =
        MaybeGetIncludeNameAsWritten(including_filepath, mapped_include.path);
    if (!quoted_include_as_written.empty()) {
      retval.push_back(quoted_include_as_written);
    } else if (mapped_include.HasAbsoluteQuotedInclude() &&
               !mapped_include.path.empty()) {
      retval.push_back(ConvertToQuotedInclude(mapped_include.path,
                                              including_path));
    } else {
      retval.push_back(mapped_include.quoted_include);
    }
  }
  return retval;
}

vector<MappedInclude> IncludePicker::GetCandidateHeadersForSymbol(
    const string& symbol) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  return GetPublicValues(symbol_include_map_, symbol);
}

vector<string> IncludePicker::GetCandidateHeadersForSymbolUsedFrom(
    const string& symbol, const string& including_filepath) const {
  return BestQuotedIncludesForIncluder(
      GetCandidateHeadersForSymbol(symbol), including_filepath);
}

vector<MappedInclude> IncludePicker::GetCandidateHeadersForFilepath(
    const string& filepath, const string& including_filepath) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  string absolute_quoted_header = ConvertToQuotedInclude(filepath);
  vector<MappedInclude> retval =
      GetPublicValues(filepath_include_map_, absolute_quoted_header);

  // We also need to consider the header itself.  Make that an option if it's
  // public or there's no other option.
  string quoted_header;
  if (including_filepath.empty()) {
    quoted_header = absolute_quoted_header;
  } else {
    quoted_header = ConvertToQuotedInclude(
        filepath, MakeAbsolutePath(GetParentPath(including_filepath)));
  }
  MappedInclude default_header(quoted_header, filepath);
  if (retval.empty() || GetVisibility(default_header, kPublic) == kPublic) {
    // Insert at front so it's the preferred option
    retval.insert(retval.begin(), default_header);
  }
  return retval;
}

// Except for the case that the includer is a 'friend' of the includee
// (via an '// IWYU pragma: friend XXX'), the same as
// GetCandidateHeadersForFilepath.
vector<string> IncludePicker::GetCandidateHeadersForFilepathIncludedFrom(
    const string& included_filepath, const string& including_filepath) const {
  vector<MappedInclude> mapped_includes;
  // We pass the own files path to ConvertToQuotedInclude so the quoted include
  // for the case that there is no matching `-I` option is just the filename
  // (e.g. "foo.cpp") instead of the absolute file path.
  const string including_path =
      MakeAbsolutePath(GetParentPath(including_filepath));
  const string quoted_includer = ConvertToQuotedInclude(
      including_filepath, including_path);
  const string quoted_includee = ConvertToQuotedInclude(
      included_filepath, including_path);
  const set<string>* headers_with_includer_as_friend =
      FindInMap(&friend_to_headers_map_, quoted_includer);
  if (headers_with_includer_as_friend != nullptr &&
      ContainsKey(*headers_with_includer_as_friend, included_filepath)) {
    mapped_includes.push_back(
        MappedInclude(quoted_includee, including_filepath));
  } else {
    mapped_includes =
        GetCandidateHeadersForFilepath(included_filepath, including_filepath);
    if (mapped_includes.size() == 1) {
      if (GetVisibility(mapped_includes[0]) == kPrivate) {
        VERRS(0) << "Warning: "
                 << "No public header found to replace the private header "
                 << included_filepath << "\n";
      }
    }
  }

  return BestQuotedIncludesForIncluder(mapped_includes, including_filepath);
}

bool IncludePicker::HasMapping(const string& map_from_filepath,
                               const string& map_to_filepath) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  const string quoted_from = ConvertToQuotedInclude(map_from_filepath);
  const string quoted_to = ConvertToQuotedInclude(map_to_filepath);
  // We can't use GetCandidateHeadersForFilepath since includer might be private
  const vector<MappedInclude>* all_mappers =
      FindInMap(&filepath_include_map_, quoted_from);
  if (all_mappers) {
    if (ContainsQuotedInclude(*all_mappers, quoted_to)) {
      return true;
    }
  }
  return quoted_to == quoted_from;   // indentity mapping, why not?
}

bool IncludePicker::IsPublic(OptionalFileEntryRef file) const {
  CHECK_(file && "Need existing FileEntry");
  const string path = GetFilePath(file);
  const string quoted_file = ConvertToQuotedInclude(path);
  const MappedInclude include(quoted_file, path);
  return (GetVisibility(include) == kPublic);
}

// Parses a YAML/JSON file containing mapping directives of various types.
void IncludePicker::AddMappingsFromFile(const string& filename) {
  vector<string> default_search_path;
  return AddMappingsFromFile(filename, default_search_path);
}

vector<string> IncludePicker::GetMappedPublicHeaders(
    const string& symbol_name,
    const string& use_path,
    const string& decl_filepath) const {
  // If the symbol has a special mapping, use it, otherwise map its file.
  vector<string> symbol_headers =
      GetCandidateHeadersForSymbolUsedFrom(symbol_name, use_path);
  if (!symbol_headers.empty())
    return symbol_headers;
  return GetCandidateHeadersForFilepathIncludedFrom(decl_filepath, use_path);
}

// Parses a YAML/JSON file containing mapping directives of various types:
//  symbol   - symbol name -> quoted include
//  include  - private quoted include -> public quoted include
//  ref      - include mechanism for mapping files, to allow project-specific
//             groupings
// This private implementation method is recursive and builds the search path
// incrementally.
void IncludePicker::AddMappingsFromFile(const string& filename,
                                        const vector<string>& search_path) {
  string absolute_path = FindFileInSearchPath(search_path, filename);

  llvm::ErrorOr<unique_ptr<MemoryBuffer>> bufferOrError =
      MemoryBuffer::getFile(absolute_path);
  if (std::error_code error = bufferOrError.getError()) {
    VERRS(0) << "Cannot open mapping file '" << absolute_path
             << "': " << error.message() << ".\n";
    return;
  }

  VERRS(5) << "Adding mappings from file '" << absolute_path << "'.\n";

  SourceMgr source_manager;
  Stream json_stream(bufferOrError.get()->getMemBufferRef(), source_manager);

  document_iterator stream_begin = json_stream.begin();
  if (stream_begin == json_stream.end())
    return;

  // Get root sequence.
  Node* root = stream_begin->getRoot();
  SequenceNode *array = llvm::dyn_cast<SequenceNode>(root);
  if (array == nullptr) {
    json_stream.printError(root, "Root element must be an array.");
    return;
  }

  for (Node& array_item_node : *array) {
    Node* current_node = &array_item_node;

    // Every item must be a JSON object ("mapping" in YAML terms.)
    MappingNode* mapping = llvm::dyn_cast<MappingNode>(current_node);
    if (mapping == nullptr) {
      json_stream.printError(current_node,
          "Mapping directives must be objects.");
      return;
    }

    for (KeyValueNode &mapping_item_node : *mapping) {
      // General form is { directive: <data> }.
      const string directive = GetScalarValue(mapping_item_node.getKey());

      if (directive == "symbol") {
        // Symbol mapping.
        vector<string> mapping = GetSequenceValue(mapping_item_node.getValue());
        if (mapping.size() != 4) {
          json_stream.printError(current_node,
              "Symbol mapping expects a value on the form "
              "'[from, visibility, to, visibility]'.");
          return;
        }

        // Ignore unused from-visibility, at some point maybe remove it from the
        // mapping file format.

        IncludeVisibility to_visibility = ParseVisibility(mapping[3]);
        if (to_visibility == kUnusedVisibility) {
          json_stream.printError(current_node,
              "Unknown visibility '" + mapping[3] + "'.");
          return;
        }

        if (!IsQuotedInclude(mapping[2])) {
          json_stream.printError(
              current_node,
              "Expected to-entry to be quoted include, but was '" + mapping[2] +
                  "'");
          return;
        }

        AddSymbolMapping(mapping[0], MappedInclude(mapping[2]), to_visibility);
      } else if (directive == "include") {
        // Include mapping.
        vector<string> mapping = GetSequenceValue(mapping_item_node.getValue());
        if (mapping.size() != 4) {
          json_stream.printError(current_node,
              "Include mapping expects a value on the form "
              "'[from, visibility, to, visibility]'.");
          return;
        }

        IncludeVisibility from_visibility = ParseVisibility(mapping[1]);
        if (from_visibility == kUnusedVisibility) {
          json_stream.printError(current_node,
              "Unknown visibility '" + mapping[1] + "'.");
          return;
        }

        IncludeVisibility to_visibility = ParseVisibility(mapping[3]);
        if (to_visibility == kUnusedVisibility) {
          json_stream.printError(current_node,
              "Unknown visibility '" + mapping[3] + "'.");
          return;
        }

        if (!IsQuotedFilepathPattern(mapping[0])) {
          json_stream.printError(
              current_node,
              "Expected from-entry to be quoted filepath or @regex, but was '" +
                  mapping[0] + "'");
          return;
        }

        if (!IsQuotedInclude(mapping[2])) {
          json_stream.printError(
              current_node,
              "Expected to-entry to be quoted include, but was '" + mapping[2] +
                  "'");
          return;
        }

        AddIncludeMapping(mapping[0],
            from_visibility,
            MappedInclude(mapping[2]),
            to_visibility);
      } else if (directive == "ref") {
        // Mapping ref.
        string ref_file = GetScalarValue(mapping_item_node.getValue());
        if (ref_file.empty()) {
          json_stream.printError(current_node,
              "Mapping ref expects a single filename value.");
          return;
        }

        // Add the path of the file we're currently processing
        // to the search path. Allows refs to be relative to referrer.
        vector<string> extended_search_path =
            ExtendMappingFileSearchPath(search_path,
                                        GetParentPath(absolute_path));

        // Recurse.
        AddMappingsFromFile(ref_file, extended_search_path);
      } else {
        json_stream.printError(current_node,
            "Unknown directive '" + directive + "'.");
        return;
      }
    }
  }
}

IncludeVisibility IncludePicker::ParseVisibility(
    const string& visibility) const {
  if (visibility == "private")
    return kPrivate;
  else if (visibility == "public")
    return kPublic;

  return kUnusedVisibility;
}

IncludeVisibility IncludePicker::GetVisibility(
    const MappedInclude& include, IncludeVisibility default_value) const {
  const IncludeVisibility* include_visibility =
    FindInMap(&include_visibility_map_, include.quoted_include);
  if (include_visibility) {
    return *include_visibility;
  }
  return GetOrDefault(path_visibility_map_, include.path, default_value);
}

}  // namespace include_what_you_use
