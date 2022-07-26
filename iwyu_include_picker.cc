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
// TODO(wan): make sure IWYU doesn't suggest <iterator>.
#include <iterator>                     // for find
// not hash_map: it's not as portable and needs hash<string>.
#include <map>                          // for map, map<>::mapped_type, etc
#include <memory>
#include <regex>
#include <string>                       // for string, basic_string, etc
#include <system_error>                 // for error_code
#include <utility>                      // for pair, make_pair
#include <vector>                       // for vector, vector<>::iterator

#include "iwyu_location_util.h"
#include "iwyu_path_util.h"
#include "iwyu_port.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "iwyu_verrs.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"
#include "clang/Basic/FileManager.h"

using std::find;
using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::unique_ptr;
using std::vector;

using llvm::MemoryBuffer;
using llvm::SourceMgr;
using llvm::yaml::KeyValueNode;
using llvm::yaml::MappingNode;
using llvm::yaml::Node;
using llvm::yaml::ScalarNode;
using llvm::yaml::SequenceNode;
using llvm::yaml::Stream;
using llvm::yaml::document_iterator;

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
  { "aiocb", kPrivate, "<aio.h>", kPublic },
  { "blkcnt_t", kPrivate, "<sys/types.h>", kPublic },
  { "blkcnt_t", kPrivate, "<sys/stat.h>", kPublic },
  { "blksize_t", kPrivate, "<sys/types.h>", kPublic },
  { "blksize_t", kPrivate, "<sys/stat.h>", kPublic },
  { "cc_t", kPrivate, "<termios.h>", kPublic },
  { "clock_t", kPrivate, "<sys/types.h>", kPublic },
  { "clock_t", kPrivate, "<sys/time.h>", kPublic },
  { "clock_t", kPrivate, "<time.h>", kPublic },
  { "clockid_t", kPrivate, "<sys/types.h>", kPublic },
  { "clockid_t", kPrivate, "<time.h>", kPublic },
  { "daddr_t", kPrivate, "<sys/types.h>", kPublic },
  { "daddr_t", kPrivate, "<rpc/types.h>", kPublic },
  { "dev_t", kPrivate, "<sys/types.h>", kPublic },
  { "dev_t", kPrivate, "<sys/stat.h>", kPublic },
  { "div_t", kPrivate, "<stdlib.h>", kPublic },
  { "double_t", kPrivate, "<math.h>", kPublic },
  { "error_t", kPrivate, "<errno.h>", kPublic },
  { "error_t", kPrivate, "<argp.h>", kPublic },
  { "error_t", kPrivate, "<argz.h>", kPublic },
  { "fd_set", kPrivate, "<sys/select.h>", kPublic },
  { "fd_set", kPrivate, "<sys/time.h>", kPublic },
  { "fenv_t", kPrivate, "<fenv.h>", kPublic },
  { "fexcept_t", kPrivate, "<fenv.h>", kPublic },
  { "FILE", kPrivate, "<stdio.h>", kPublic },
  { "FILE", kPrivate, "<wchar.h>", kPublic },
  { "float_t", kPrivate, "<math.h>", kPublic },
  { "fsblkcnt_t", kPrivate, "<sys/types.h>", kPublic },
  { "fsblkcnt_t", kPrivate, "<sys/statvfs.h>", kPublic },
  { "fsfilcnt_t", kPrivate, "<sys/types.h>", kPublic },
  { "fsfilcnt_t", kPrivate, "<sys/statvfs.h>", kPublic },
  { "gid_t", kPrivate, "<sys/types.h>", kPublic },
  { "gid_t", kPrivate, "<grp.h>", kPublic },
  { "gid_t", kPrivate, "<pwd.h>", kPublic },
  { "gid_t", kPrivate, "<signal.h>", kPublic },
  { "gid_t", kPrivate, "<stropts.h>", kPublic },
  { "gid_t", kPrivate, "<sys/ipc.h>", kPublic },
  { "gid_t", kPrivate, "<sys/stat.h>", kPublic },
  { "gid_t", kPrivate, "<unistd.h>", kPublic },
  { "htonl", kPrivate, "<arpa/inet.h>", kPublic },
  { "htons", kPrivate, "<arpa/inet.h>", kPublic },
  { "id_t", kPrivate, "<sys/types.h>", kPublic },
  { "id_t", kPrivate, "<sys/resource.h>", kPublic },
  { "imaxdiv_t", kPrivate, "<inttypes.h>", kPublic },
  { "intmax_t", kPrivate, "<stdint.h>", kPublic },
  { "uintmax_t", kPrivate, "<stdint.h>", kPublic },
  { "ino64_t", kPrivate, "<sys/types.h>", kPublic },
  { "ino64_t", kPrivate, "<dirent.h>", kPublic },
  { "ino_t", kPrivate, "<sys/types.h>", kPublic },
  { "ino_t", kPrivate, "<dirent.h>", kPublic },
  { "ino_t", kPrivate, "<sys/stat.h>", kPublic },
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
  { "iovec", kPrivate, "<sys/socket.h>", kPublic },
  { "key_t", kPrivate, "<sys/types.h>", kPublic },
  { "key_t", kPrivate, "<sys/ipc.h>", kPublic },
  { "lconv", kPrivate, "<locale.h>", kPublic },
  { "ldiv_t", kPrivate, "<stdlib.h>", kPublic },
  { "lldiv_t", kPrivate, "<stdlib.h>", kPublic },
  { "max_align_t", kPrivate, "<stddef.h>", kPublic },
  { "mode_t", kPrivate, "<sys/types.h>", kPublic },
  { "mode_t", kPrivate, "<fcntl.h>", kPublic },
  { "mode_t", kPrivate, "<ndbm.h>", kPublic },
  { "mode_t", kPrivate, "<spawn.h>", kPublic },
  { "mode_t", kPrivate, "<sys/ipc.h>", kPublic },
  { "mode_t", kPrivate, "<sys/mman.h>", kPublic },
  { "mode_t", kPrivate, "<sys/stat.h>", kPublic },
  { "nlink_t", kPrivate, "<sys/types.h>", kPublic },
  { "nlink_t", kPrivate, "<sys/stat.h>", kPublic },
  { "ntohl", kPrivate, "<arpa/inet.h>", kPublic },
  { "ntohs", kPrivate, "<arpa/inet.h>", kPublic },
  { "off64_t", kPrivate, "<sys/types.h>", kPublic },
  { "off64_t", kPrivate, "<unistd.h>", kPublic },
  { "off_t", kPrivate, "<sys/types.h>", kPublic },
  { "off_t", kPrivate, "<aio.h>", kPublic },
  { "off_t", kPrivate, "<fcntl.h>", kPublic },
  { "off_t", kPrivate, "<stdio.h>", kPublic },
  { "off_t", kPrivate, "<sys/mman.h>", kPublic },
  { "off_t", kPrivate, "<sys/stat.h>", kPublic },
  { "off_t", kPrivate, "<unistd.h>", kPublic },
  { "pid_t", kPrivate, "<sys/types.h>", kPublic },
  { "pid_t", kPrivate, "<fcntl.h>", kPublic },
  { "pid_t", kPrivate, "<sched.h>", kPublic },
  { "pid_t", kPrivate, "<signal.h>", kPublic },
  { "pid_t", kPrivate, "<spawn.h>", kPublic },
  { "pid_t", kPrivate, "<sys/msg.h>", kPublic },
  { "pid_t", kPrivate, "<sys/sem.h>", kPublic },
  { "pid_t", kPrivate, "<sys/shm.h>", kPublic },
  { "pid_t", kPrivate, "<sys/wait.h>", kPublic },
  { "pid_t", kPrivate, "<termios.h>", kPublic },
  { "pid_t", kPrivate, "<time.h>", kPublic },
  { "pid_t", kPrivate, "<unistd.h>", kPublic },
  { "pid_t", kPrivate, "<utmpx.h>", kPublic },
  { "ptrdiff_t", kPrivate, "<stddef.h>", kPublic },
  { "regex_t", kPrivate, "<regex.h>", kPublic },
  { "regmatch_t", kPrivate, "<regex.h>", kPublic },
  { "regoff_t", kPrivate, "<regex.h>", kPublic },
  { "sigevent", kPrivate, "<signal.h>", kPublic },
  { "sigevent", kPrivate, "<aio.h>", kPublic },
  { "sigevent", kPrivate, "<mqueue.h>", kPublic },
  { "sigevent", kPrivate, "<time.h>", kPublic },
  { "siginfo_t", kPrivate, "<signal.h>", kPublic },
  { "siginfo_t", kPrivate, "<sys/wait.h>", kPublic },
  { "sigset_t", kPrivate, "<signal.h>", kPublic },
  { "sigset_t", kPrivate, "<spawn.h>", kPublic },
  { "sigset_t", kPrivate, "<sys/select.h>", kPublic },
  { "sigval", kPrivate, "<signal.h>", kPublic },
  { "sockaddr", kPrivate, "<sys/socket.h>", kPublic },
  { "socklen_t", kPrivate, "<sys/socket.h>", kPublic },
  { "socklen_t", kPrivate, "<netdb.h>", kPublic },
  { "ssize_t", kPrivate, "<sys/types.h>", kPublic },
  { "ssize_t", kPrivate, "<aio.h>", kPublic },
  { "ssize_t", kPrivate, "<monetary.h>", kPublic },
  { "ssize_t", kPrivate, "<mqueue.h>", kPublic },
  { "ssize_t", kPrivate, "<stdio.h>", kPublic },
  { "ssize_t", kPrivate, "<sys/msg.h>", kPublic },
  { "ssize_t", kPrivate, "<sys/socket.h>", kPublic },
  { "ssize_t", kPrivate, "<sys/uio.h>", kPublic },
  { "ssize_t", kPrivate, "<unistd.h>", kPublic },
  { "stat", kPrivate, "<sys/stat.h>", kPublic },
  { "stat", kPrivate, "<ftw.h>", kPublic },
  { "suseconds_t", kPrivate, "<sys/types.h>", kPublic },
  { "suseconds_t", kPrivate, "<sys/select.h>", kPublic },
  { "suseconds_t", kPrivate, "<sys/time.h>", kPublic },
  { "time_t", kPrivate, "<time.h>", kPublic },
  { "time_t", kPrivate, "<sched.h>", kPublic },
  { "time_t", kPrivate, "<sys/msg.h>", kPublic },
  { "time_t", kPrivate, "<sys/select.h>", kPublic },
  { "time_t", kPrivate, "<sys/sem.h>", kPublic },
  { "time_t", kPrivate, "<sys/shm.h>", kPublic },
  { "time_t", kPrivate, "<sys/stat.h>", kPublic },
  { "time_t", kPrivate, "<sys/time.h>", kPublic },
  { "time_t", kPrivate, "<sys/types.h>", kPublic },
  { "time_t", kPrivate, "<utime.h>", kPublic },
  { "timer_t", kPrivate, "<sys/types.h>", kPublic },
  { "timer_t", kPrivate, "<time.h>", kPublic },
  { "timespec", kPrivate, "<time.h>", kPublic },
  { "timespec", kPrivate, "<aio.h>", kPublic },
  { "timespec", kPrivate, "<mqueue.h>", kPublic },
  { "timespec", kPrivate, "<sched.h>", kPublic },
  { "timespec", kPrivate, "<signal.h>", kPublic },
  { "timespec", kPrivate, "<sys/select.h>", kPublic },
  { "timespec", kPrivate, "<sys/stat.h>", kPublic },
  { "timeval", kPrivate, "<sys/time.h>", kPublic },
  { "timeval", kPrivate, "<sys/resource.h>", kPublic },
  { "timeval", kPrivate, "<sys/select.h>", kPublic },
  { "timeval", kPrivate, "<utmpx.h>", kPublic },
  { "tm", kPrivate, "<time.h>", kPublic },
  { "u_char", kPrivate, "<sys/types.h>", kPublic },
  { "u_char", kPrivate, "<rpc/types.h>", kPublic },
  { "uid_t", kPrivate, "<sys/types.h>", kPublic },
  { "uid_t", kPrivate, "<pwd.h>", kPublic },
  { "uid_t", kPrivate, "<signal.h>", kPublic },
  { "uid_t", kPrivate, "<stropts.h>", kPublic },
  { "uid_t", kPrivate, "<sys/ipc.h>", kPublic },
  { "uid_t", kPrivate, "<sys/stat.h>", kPublic },
  { "uid_t", kPrivate, "<unistd.h>", kPublic },
  { "useconds_t", kPrivate, "<sys/types.h>", kPublic },
  { "useconds_t", kPrivate, "<unistd.h>", kPublic },
  { "wchar_t", kPrivate, "<stddef.h>", kPublic },
  { "wchar_t", kPrivate, "<stdlib.h>", kPublic },
  // It is unspecified if the cname headers provide ::size_t.
  // <locale.h> is the one header which defines NULL but not size_t.
  { "size_t", kPrivate, "<stddef.h>", kPublic },  // 'canonical' location for size_t
  { "size_t", kPrivate, "<aio.h>", kPublic },
  { "size_t", kPrivate, "<glob.h>", kPublic },
  { "size_t", kPrivate, "<grp.h>", kPublic },
  { "size_t", kPrivate, "<iconv.h>", kPublic },
  { "size_t", kPrivate, "<monetary.h>", kPublic },
  { "size_t", kPrivate, "<mqueue.h>", kPublic },
  { "size_t", kPrivate, "<ndbm.h>", kPublic },
  { "size_t", kPrivate, "<pwd.h>", kPublic },
  { "size_t", kPrivate, "<regex.h>", kPublic },
  { "size_t", kPrivate, "<search.h>", kPublic },
  { "size_t", kPrivate, "<signal.h>", kPublic },
  { "size_t", kPrivate, "<stdio.h>", kPublic },
  { "size_t", kPrivate, "<stdlib.h>", kPublic },
  { "size_t", kPrivate, "<string.h>", kPublic },
  { "size_t", kPrivate, "<strings.h>", kPublic },
  { "size_t", kPrivate, "<sys/mman.h>", kPublic },
  { "size_t", kPrivate, "<sys/msg.h>", kPublic },
  { "size_t", kPrivate, "<sys/sem.h>", kPublic },
  { "size_t", kPrivate, "<sys/shm.h>", kPublic },
  { "size_t", kPrivate, "<sys/socket.h>", kPublic },
  { "size_t", kPrivate, "<sys/types.h>", kPublic },
  { "size_t", kPrivate, "<sys/uio.h>", kPublic },
  { "size_t", kPrivate, "<time.h>", kPublic },
  { "size_t", kPrivate, "<uchar.h>", kPublic },
  { "size_t", kPrivate, "<unistd.h>", kPublic },
  { "size_t", kPrivate, "<wchar.h>", kPublic },
  { "size_t", kPrivate, "<wordexp.h>", kPublic },
  // Macros that can be defined in more than one file, don't have the
  // same __foo_defined guard that other types do, so the grep above
  // doesn't discover them.  Until I figure out a better way, I just
  // add them in by hand as I discover them.
  { "EOF", kPrivate, "<stdio.h>", kPublic },
  { "EOF", kPrivate, "<libio.h>", kPublic },
  { "FILE", kPrivate, "<stdio.h>", kPublic },
  { "MAP_POPULATE", kPrivate, "<sys/mman.h>", kPublic },
  { "MAP_POPULATE", kPrivate, "<linux/mman.h>", kPublic },
  { "MAP_STACK", kPrivate, "<sys/mman.h>", kPublic },
  { "MAP_STACK", kPrivate, "<linux/mman.h>", kPublic },
  { "MAXHOSTNAMELEN", kPrivate, "<sys/param.h>", kPublic },
  { "MAXHOSTNAMELEN", kPrivate, "<protocols/timed.h>", kPublic },
  { "SIGABRT", kPrivate, "<signal.h>", kPublic },
  { "SIGCHLD", kPrivate, "<signal.h>", kPublic },
  { "SIGCHLD", kPrivate, "<linux/signal.h>", kPublic },
  { "va_list", kPrivate, "<stdarg.h>", kPublic },
  { "va_list", kPrivate, "<stdio.h>", kPublic },
  { "va_list", kPrivate, "<wchar.h>", kPublic },
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
  { "NULL", kPrivate, "<unistd.h>", kPublic },
  { "NULL", kPrivate, "<wchar.h>", kPublic },
};

// Symbol -> include mappings for GNU libstdc++
const IncludeMapEntry libstdcpp_symbol_map[] = {
  // Kludge time: almost all STL types take an allocator, but they
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

  { "std::size_t", kPrivate, "<cstddef>", kPublic },  // 'canonical' location for std::size_t
  { "std::size_t", kPrivate, "<cstdio>", kPublic },
  { "std::size_t", kPrivate, "<cstdlib>", kPublic },
  { "std::size_t", kPrivate, "<cstring>", kPublic },
  { "std::size_t", kPrivate, "<ctime>", kPublic },
  { "std::size_t", kPrivate, "<cuchar>", kPublic },
  { "std::size_t", kPrivate, "<cwchar>", kPublic },
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
  { "<bits/sys_errlist.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/syscall.h>", kPrivate, "<sys/syscall.h>", kPrivate },
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
  { "<bits/syscall.h>", kPrivate, "<sys/syscall.h>", kPrivate },
  { "<bits/timerfd.h>", kPrivate, "<sys/timerfd.h>", kPublic },
  { "<bits/typesizes.h>", kPrivate, "<sys/types.h>", kPublic },
  // Top-level #includes that just forward to another file:
  // $ for i in /usr/include/*; do [ -f $i ] && [ `wc -l < $i` = 1 ] && echo $i; done
  // (poll.h, syscall.h, syslog.h, ustat.h, wait.h).
  // For each file, I looked at the list of canonical header files --
  // http://www.opengroup.org/onlinepubs/9699919799/idx/head.html --
  // to decide which of the two files is canonical.  If neither is
  // on the POSIX.1 1998 list, I just choose the top-level one.
  { "<sys/poll.h>", kPrivate, "<poll.h>", kPublic },
  { "<sys/syscall.h>", kPrivate, "<syscall.h>", kPublic },
  { "<sys/syslog.h>", kPrivate, "<syslog.h>", kPublic },
  { "<sys/ustat.h>", kPrivate, "<ustat.h>", kPublic },
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
  { "<asm/unistd.h>", kPrivate, "<syscall.h>", kPublic },
  { "<linux/limits.h>", kPrivate, "<limits.h>", kPublic },   // PATH_MAX
  { "<linux/prctl.h>", kPrivate, "<sys/prctl.h>", kPublic },
  { "<sys/ucontext.h>", kPrivate, "<ucontext.h>", kPublic },
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
  // $ curl -s -N https://raw.githubusercontent.com/cplusplus/draft/c%2B%2B20/source/lib-intro.tex | sed -n '/begin{multicolfloattable}.*{headers.cpp.c}/,/end{multicolfloattable}/p' lib-intro.tex | grep tcode | perl -nle 'm/tcode{<c(.*)>}/ && print qq@  { "<$1.h>", kPublic, "<c$1>", kPublic },@' | sort
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
  // $ curl -s -N https://raw.githubusercontent.com/cplusplus/draft/c%2B%2B20/source/lib-intro.tex | sed -n '/begin{multicolfloattable}.*{headers.cpp}/,/end{multicolfloattable}/p' lib-intro.tex | grep tcode | perl -nle 'm/tcode{(.*)}/ && print qq@  "$1",@' | sort
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
  "<filesystem>",
  "<format>",
  "<forward_list>",
  "<fstream>",
  "<functional>",
  "<future>",
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
  "<memory>",
  "<memory_resource>",
  "<mutex>",
  "<new>",
  "<numbers>",
  "<numeric>",
  "<optional>",
  "<ostream>",
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
  "<sstream>",
  "<stack>",
  "<stdexcept>",
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

// Private -> public include mappings for GNU libstdc++
//
// Note: make sure to sync this setting with gcc.stl.headers.imp
//
const IncludeMapEntry libstdcpp_include_map[] = {
  // cd /usr/include/c++/10 && grep -r headername | perl -nle 'm/^([^:]+).*@headername\{([^,]*)\}/ && print qq@  { "<$1>", kPrivate, "<$2>", kPublic },@' | sort -u
  { "<backward/auto_ptr.h>", kPrivate, "<memory>", kPublic },
  { "<backward/backward_warning.h>", kPrivate, "<iosfwd>", kPublic },
  { "<backward/binders.h>", kPrivate, "<functional>", kPublic },
  { "<bits/algorithmfwd.h>", kPrivate, "<algorithm>", kPublic },
  { "<bits/allocated_ptr.h>", kPrivate, "<memory>", kPublic },
  { "<bits/allocator.h>", kPrivate, "<memory>", kPublic },
  { "<bits/alloc_traits.h>", kPrivate, "<memory>", kPublic },
  { "<bits/atomic_base.h>", kPrivate, "<atomic>", kPublic },
  { "<bits/atomic_lockfree_defines.h>", kPrivate, "<atomic>", kPublic },
  { "<bits/basic_ios.h>", kPrivate, "<ios>", kPublic },
  { "<bits/basic_ios.tcc>", kPrivate, "<ios>", kPublic },
  { "<bits/basic_string.h>", kPrivate, "<string>", kPublic },
  { "<bits/basic_string.tcc>", kPrivate, "<string>", kPublic },
  { "<bits/boost_concept_check.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/c++0x_warning.h>", kPrivate, "<iosfwd>", kPublic },
  { "<bits/charconv.h>", kPrivate, "<charconv>", kPublic },
  { "<bits/char_traits.h>", kPrivate, "<string>", kPublic },
  { "<bits/codecvt.h>", kPrivate, "<locale>", kPublic },
  { "<bits/concept_check.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/cpp_type_traits.h>", kPrivate, "<ext/type_traits>", kPublic },
  { "<bits/cxxabi_forced.h>", kPrivate, "<cxxabi.h>", kPublic },
  { "<bits/deque.tcc>", kPrivate, "<deque>", kPublic },
  { "<bits/exception_defines.h>", kPrivate, "<exception>", kPublic },
  { "<bits/exception_ptr.h>", kPrivate, "<exception>", kPublic },
  { "<bits/forward_list.h>", kPrivate, "<forward_list>", kPublic },
  { "<bits/forward_list.tcc>", kPrivate, "<forward_list>", kPublic },
  { "<bits/fs_dir.h>", kPrivate, "<filesystem>", kPublic },
  { "<bits/fs_fwd.h>", kPrivate, "<filesystem>", kPublic },
  { "<bits/fs_ops.h>", kPrivate, "<filesystem>", kPublic },
  { "<bits/fs_path.h>", kPrivate, "<filesystem>", kPublic },
  { "<bits/fstream.tcc>", kPrivate, "<fstream>", kPublic },
  { "<bits/functexcept.h>", kPrivate, "<exception>", kPublic },
  { "<bits/functional_hash.h>", kPrivate, "<functional>", kPublic },
  { "<bits/gslice_array.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/gslice.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/hash_bytes.h>", kPrivate, "<functional>", kPublic },
  { "<bits/indirect_array.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/int_limits.h>", kPrivate, "<limits>", kPublic },
  { "<bits/invoke.h>", kPrivate, "<functional>", kPublic },
  { "<bits/ios_base.h>", kPrivate, "<ios>", kPublic },
  { "<bits/istream.tcc>", kPrivate, "<istream>", kPublic },
  { "<bits/iterator_concepts.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/list.tcc>", kPrivate, "<list>", kPublic },
  { "<bits/locale_classes.h>", kPrivate, "<locale>", kPublic },
  { "<bits/locale_classes.tcc>", kPrivate, "<locale>", kPublic },
  { "<bits/locale_conv.h>", kPrivate, "<locale>", kPublic },
  { "<bits/locale_facets.h>", kPrivate, "<locale>", kPublic },
  { "<bits/locale_facets_nonio.h>", kPrivate, "<locale>", kPublic },
  { "<bits/locale_facets_nonio.tcc>", kPrivate, "<locale>", kPublic },
  { "<bits/locale_facets.tcc>", kPrivate, "<locale>", kPublic },
  { "<bits/localefwd.h>", kPrivate, "<locale>", kPublic },
  { "<bits/mask_array.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/memoryfwd.h>", kPrivate, "<memory>", kPublic },
  { "<bits/move.h>", kPrivate, "<utility>", kPublic },
  { "<bits/nested_exception.h>", kPrivate, "<exception>", kPublic },
  { "<bits/ostream_insert.h>", kPrivate, "<ostream>", kPublic },
  { "<bits/ostream.tcc>", kPrivate, "<ostream>", kPublic },
  { "<bits/parse_numbers.h>", kPrivate, "<chrono>", kPublic },
  { "<bits/postypes.h>", kPrivate, "<iosfwd>", kPublic },
  { "<bits/predefined_ops.h>", kPrivate, "<algorithm>", kPublic },
  { "<bits/ptr_traits.h>", kPrivate, "<memory>", kPublic },
  { "<bits/quoted_string.h>", kPrivate, "<iomanip>", kPublic },
  { "<bits/random.h>", kPrivate, "<random>", kPublic },
  { "<bits/random.tcc>", kPrivate, "<random>", kPublic },
  { "<bits/range_access.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/range_cmp.h>", kPrivate, "<functional>", kPublic },
  { "<bits/ranges_algobase.h>", kPrivate, "<algorithm>", kPublic },
  { "<bits/ranges_algo.h>", kPrivate, "<algorithm>", kPublic },
  { "<bits/ranges_uninitialized.h>", kPrivate, "<memory>", kPublic },
  { "<bits/refwrap.h>", kPrivate, "<functional>", kPublic },
  { "<bits/regex_automaton.h>", kPrivate, "<regex>", kPublic },
  { "<bits/regex_automaton.tcc>", kPrivate, "<regex>", kPublic },
  { "<bits/regex_compiler.h>", kPrivate, "<regex>", kPublic },
  { "<bits/regex_compiler.tcc>", kPrivate, "<regex>", kPublic },
  { "<bits/regex_constants.h>", kPrivate, "<regex>", kPublic },
  { "<bits/regex_error.h>", kPrivate, "<regex>", kPublic },
  { "<bits/regex_executor.h>", kPrivate, "<regex>", kPublic },
  { "<bits/regex_executor.tcc>", kPrivate, "<regex>", kPublic },
  { "<bits/regex.h>", kPrivate, "<regex>", kPublic },
  { "<bits/regex_scanner.h>", kPrivate, "<regex>", kPublic },
  { "<bits/regex_scanner.tcc>", kPrivate, "<regex>", kPublic },
  { "<bits/regex.tcc>", kPrivate, "<regex>", kPublic },
  { "<bits/shared_ptr_atomic.h>", kPrivate, "<memory>", kPublic },
  { "<bits/shared_ptr_base.h>", kPrivate, "<memory>", kPublic },
  { "<bits/shared_ptr.h>", kPrivate, "<memory>", kPublic },
  { "<bits/slice_array.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/specfun.h>", kPrivate, "<cmath>", kPublic },
  { "<bits/sstream.tcc>", kPrivate, "<sstream>", kPublic },
  { "<bits/std_function.h>", kPrivate, "<functional>", kPublic },
  { "<bits/std_mutex.h>", kPrivate, "<mutex>", kPublic },
  { "<bits/stl_algobase.h>", kPrivate, "<algorithm>", kPublic },
  { "<bits/stl_algo.h>", kPrivate, "<algorithm>", kPublic },
  { "<bits/stl_bvector.h>", kPrivate, "<vector>", kPublic },
  { "<bits/stl_construct.h>", kPrivate, "<memory>", kPublic },
  { "<bits/stl_deque.h>", kPrivate, "<deque>", kPublic },
  { "<bits/stl_function.h>", kPrivate, "<functional>", kPublic },
  { "<bits/stl_heap.h>", kPrivate, "<queue>", kPublic },
  { "<bits/stl_iterator_base_funcs.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/stl_iterator_base_types.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/stl_iterator.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/stl_list.h>", kPrivate, "<list>", kPublic },
  { "<bits/stl_map.h>", kPrivate, "<map>", kPublic },
  { "<bits/stl_multimap.h>", kPrivate, "<map>", kPublic },
  { "<bits/stl_multiset.h>", kPrivate, "<set>", kPublic },
  { "<bits/stl_numeric.h>", kPrivate, "<numeric>", kPublic },
  { "<bits/stl_pair.h>", kPrivate, "<utility>", kPublic },
  { "<bits/stl_queue.h>", kPrivate, "<queue>", kPublic },
  { "<bits/stl_raw_storage_iter.h>", kPrivate, "<memory>", kPublic },
  { "<bits/stl_relops.h>", kPrivate, "<utility>", kPublic },
  { "<bits/stl_set.h>", kPrivate, "<set>", kPublic },
  { "<bits/stl_stack.h>", kPrivate, "<stack>", kPublic },
  { "<bits/stl_tempbuf.h>", kPrivate, "<memory>", kPublic },
  { "<bits/stl_uninitialized.h>", kPrivate, "<memory>", kPublic },
  { "<bits/stl_vector.h>", kPrivate, "<vector>", kPublic },
  { "<bits/streambuf_iterator.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/streambuf.tcc>", kPrivate, "<streambuf>", kPublic },
  { "<bits/stream_iterator.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/stringfwd.h>", kPrivate, "<string>", kPublic },
  { "<bits/string_view.tcc>", kPrivate, "<string_view>", kPublic },
  { "<bits/uniform_int_dist.h>", kPrivate, "<random>", kPublic },
  { "<bits/unique_lock.h>", kPrivate, "<mutex>", kPublic },
  { "<bits/unique_ptr.h>", kPrivate, "<memory>", kPublic },
  { "<bits/unordered_map.h>", kPrivate, "<unordered_map>", kPublic },
  { "<bits/unordered_set.h>", kPrivate, "<unordered_set>", kPublic },
  { "<bits/valarray_after.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/valarray_array.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/valarray_array.tcc>", kPrivate, "<valarray>", kPublic },
  { "<bits/valarray_before.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/vector.tcc>", kPrivate, "<vector>", kPublic },
  { "<decimal/decimal.h>", kPrivate, "<decimal>", kPublic },
  { "<experimental/bits/fs_dir.h>", kPrivate, "<experimental/filesystem>", kPublic },
  { "<experimental/bits/fs_fwd.h>", kPrivate, "<experimental/filesystem>", kPublic },
  { "<experimental/bits/fs_ops.h>", kPrivate, "<experimental/filesystem>", kPublic },
  { "<experimental/bits/fs_path.h>", kPrivate, "<experimental/filesystem>", kPublic },
  { "<experimental/bits/net.h>", kPrivate, "<experimental/net>", kPublic },
  { "<experimental/bits/shared_ptr.h>", kPrivate, "<experimental/memory>", kPublic },
  { "<experimental/bits/string_view.tcc>", kPrivate, "<experimental/string_view>", kPublic },
  { "<ext/cast.h>", kPrivate, "<ext/pointer.h>", kPublic },
  { "<ext/random.tcc>", kPrivate, "<ext/random>", kPublic },
  { "<ext/rc_string_base.h>", kPrivate, "<ext/vstring.h>", kPublic },
  { "<ext/ropeimpl.h>", kPrivate, "<ext/rope>", kPublic },
  { "<ext/sso_string_base.h>", kPrivate, "<ext/vstring.h>", kPublic },
  { "<ext/vstring_fwd.h>", kPrivate, "<ext/vstring.h>", kPublic },
  { "<ext/vstring.tcc>", kPrivate, "<ext/vstring.h>", kPublic },
  { "<ext/vstring_util.h>", kPrivate, "<ext/vstring.h>", kPublic },
  { "<tr1/bessel_function.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/beta_function.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/ell_integral.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/exp_integral.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/functional_hash.h>", kPrivate, "<tr1/functional>", kPublic },
  { "<tr1/gamma.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/hypergeometric.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/legendre_function.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/modified_bessel_func.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/poly_hermite.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/poly_laguerre.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/random.h>", kPrivate, "<tr1/random>", kPublic },
  { "<tr1/random.tcc>", kPrivate, "<tr1/random>", kPublic },
  { "<tr1/riemann_zeta.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/shared_ptr.h>", kPrivate, "<tr1/memory>", kPublic },
  { "<tr1/special_function_util.h>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/unordered_map.h>", kPrivate, "<tr1/unordered_map>", kPublic },
  { "<tr1/unordered_set.h>", kPrivate, "<tr1/unordered_set>", kPublic },
  { "<tr2/dynamic_bitset.tcc>", kPrivate, "<tr2/dynamic_bitset>", kPublic },
  // cd /usr/include/x86_64-linux-gnu/c++/10 && grep -r headername | perl -nle 'm/^([^:]+).*@headername\{([^,]*)\}/ && print qq@  { "<$1>", kPrivate, "<$2>", kPublic },@' | sort -u
  { "<bits/basic_file.h>", kPrivate, "<ios>", kPublic },
  { "<bits/c++allocator.h>", kPrivate, "<memory>", kPublic },
  { "<bits/c++config.h>", kPrivate, "<iosfwd>", kPublic },
  { "<bits/c++config.h>", kPrivate, "<version>", kPublic },
  { "<bits/c++io.h>", kPrivate, "<ios>", kPublic },
  { "<bits/c++locale.h>", kPrivate, "<locale>", kPublic },
  { "<bits/cpu_defines.h>", kPrivate, "<iosfwd>", kPublic },
  { "<bits/ctype_base.h>", kPrivate, "<locale>", kPublic },
  { "<bits/ctype_inline.h>", kPrivate, "<locale>", kPublic },
  { "<bits/cxxabi_tweaks.h>", kPrivate, "<cxxabi.h>", kPublic },
  { "<bits/error_constants.h>", kPrivate, "<system_error>", kPublic },
  { "<bits/messages_members.h>", kPrivate, "<locale>", kPublic },
  { "<bits/opt_random.h>", kPrivate, "<random>", kPublic },
  { "<bits/os_defines.h>", kPrivate, "<iosfwd>", kPublic },
  { "<bits/time_members.h>", kPrivate, "<locale>", kPublic },
  { "<ext/opt_random.h>", kPrivate, "<ext/random>", kPublic },
  // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && grep '^ *# *include' {ext/,tr1/,}* | perl -nle 'm/^([^:]+).*<([^>]+)>/ && print qq@    { "<$2>", kPrivate, "<$1>", kPublic },@' | grep -e bits/ -e tr1_impl/ | sort -u)
  // I removed a lot of 'meaningless' dependencies -- for instance,
  // <functional> #includes <bits/stringfwd.h>, but if someone is
  // using strings, <functional> isn't enough to satisfy iwyu.
  // We may need to add other dirs in future versions of gcc.
  { "<bits/atomic_word.h>", kPrivate, "<ext/atomicity.h>", kPublic },
  { "<bits/basic_file.h>", kPrivate, "<fstream>", kPublic },
  { "<bits/boost_sp_shared_count.h>", kPrivate, "<memory>", kPublic },
  { "<bits/c++io.h>", kPrivate, "<ext/stdio_sync_filebuf.h>", kPublic },
  { "<bits/c++config.h>", kPrivate, "<cstddef>", kPublic },
  { "<bits/cmath.tcc>", kPrivate, "<cmath>", kPublic },
  { "<bits/codecvt.h>", kPrivate, "<fstream>", kPublic },
  { "<bits/cxxabi_tweaks.h>", kPrivate, "<cxxabi.h>", kPublic },
  { "<bits/functional_hash.h>", kPrivate, "<unordered_map>", kPublic },
  { "<bits/hashtable.h>", kPrivate, "<unordered_map>", kPublic },
  { "<bits/hashtable.h>", kPrivate, "<unordered_set>", kPublic },
  { "<bits/ios_base.h>", kPrivate, "<iostream>", kPublic },
  { "<bits/ios_base.h>", kPrivate, "<iomanip>", kPublic },
  { "<bits/postypes.h>", kPrivate, "<iostream>", kPublic },
  { "<bits/stl_pair.h>", kPrivate, "<tr1/utility>", kPublic },
  { "<bits/stl_tree.h>", kPrivate, "<map>", kPublic },
  { "<bits/stl_tree.h>", kPrivate, "<set>", kPublic },
  { "<tr1_impl/array>", kPrivate, "<array>", kPublic },
  { "<tr1_impl/array>", kPrivate, "<tr1/array>", kPublic },
  { "<tr1_impl/boost_shared_ptr.h>", kPrivate, "<memory>", kPublic },
  { "<tr1_impl/boost_shared_ptr.h>", kPrivate, "<tr1/memory>", kPublic },
  { "<tr1_impl/boost_sp_counted_base.h>", kPrivate, "<memory>", kPublic },
  { "<tr1_impl/boost_sp_counted_base.h>", kPrivate, "<tr1/memory>", kPublic },
  { "<tr1_impl/cctype>", kPrivate, "<cctype>", kPublic },
  { "<tr1_impl/cctype>", kPrivate, "<tr1/cctype>", kPublic },
  { "<tr1_impl/cfenv>", kPrivate, "<cfenv>", kPublic },
  { "<tr1_impl/cfenv>", kPrivate, "<tr1/cfenv>", kPublic },
  { "<tr1_impl/cinttypes>", kPrivate, "<cinttypes>", kPublic },
  { "<tr1_impl/cinttypes>", kPrivate, "<tr1/cinttypes>", kPublic },
  { "<tr1_impl/cmath>", kPrivate, "<cmath>", kPublic },
  { "<tr1_impl/cmath>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1_impl/complex>", kPrivate, "<complex>", kPublic },
  { "<tr1_impl/complex>", kPrivate, "<tr1/complex>", kPublic },
  { "<tr1_impl/cstdint>", kPrivate, "<cstdint>", kPublic },
  { "<tr1_impl/cstdint>", kPrivate, "<tr1/cstdint>", kPublic },
  { "<tr1_impl/cstdio>", kPrivate, "<cstdio>", kPublic },
  { "<tr1_impl/cstdio>", kPrivate, "<tr1/cstdio>", kPublic },
  { "<tr1_impl/cstdlib>", kPrivate, "<cstdlib>", kPublic },
  { "<tr1_impl/cstdlib>", kPrivate, "<tr1/cstdlib>", kPublic },
  { "<tr1_impl/cwchar>", kPrivate, "<cwchar>", kPublic },
  { "<tr1_impl/cwchar>", kPrivate, "<tr1/cwchar>", kPublic },
  { "<tr1_impl/cwctype>", kPrivate, "<cwctype>", kPublic },
  { "<tr1_impl/cwctype>", kPrivate, "<tr1/cwctype>", kPublic },
  { "<tr1_impl/functional>", kPrivate, "<functional>", kPublic },
  { "<tr1_impl/functional>", kPrivate, "<tr1/functional>", kPublic },
  { "<tr1_impl/random>", kPrivate, "<random>", kPublic },
  { "<tr1_impl/random>", kPrivate, "<tr1/random>", kPublic },
  { "<tr1_impl/regex>", kPrivate, "<regex>", kPublic },
  { "<tr1_impl/regex>", kPrivate, "<tr1/regex>", kPublic },
  { "<tr1_impl/type_traits>", kPrivate, "<tr1/type_traits>", kPublic },
  { "<tr1_impl/type_traits>", kPrivate, "<type_traits>", kPublic },
  { "<tr1_impl/unordered_map>", kPrivate, "<tr1/unordered_map>", kPublic },
  { "<tr1_impl/unordered_map>", kPrivate, "<unordered_map>", kPublic },
  { "<tr1_impl/unordered_set>", kPrivate, "<tr1/unordered_set>", kPublic },
  { "<tr1_impl/unordered_set>", kPrivate, "<unordered_set>", kPublic },
  { "<tr1_impl/utility>", kPrivate, "<tr1/utility>", kPublic },
  { "<tr1_impl/utility>", kPrivate, "<utility>", kPublic },
  // Hash and hashtable-based containers.
  { "<tr1_impl/functional_hash.h>", kPrivate, "<tr1/functional>", kPublic },
  { "<tr1_impl/functional_hash.h>", kPrivate, "<tr1/unordered_map>", kPublic },
  { "<tr1_impl/functional_hash.h>", kPrivate, "<tr1/unordered_set>", kPublic },
  { "<tr1/functional_hash.h>", kPrivate, "<tr1/unordered_map>", kPublic },
  { "<tr1/functional_hash.h>", kPrivate, "<tr1/unordered_set>", kPublic },
  { "<tr1_impl/hashtable>", kPrivate, "<tr1/unordered_map>", kPublic },
  { "<tr1_impl/hashtable>", kPrivate, "<tr1/unordered_set>", kPublic },
  { "<tr1/hashtable.h>", kPrivate, "<tr1/unordered_map>", kPublic },
  { "<tr1/hashtable.h>", kPrivate, "<tr1/unordered_set>", kPublic },
  // All .tcc files are gcc internal-include files.  We get them from
  // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && grep -R '^ *# *include.*tcc' * | perl -nle 'm/^([^:]+).*[<"]([^>"]+)[>"]/ && print qq@    { "<$2>", kPrivate, "<$1>", kPublic },@' | sort )
  // I had to manually edit some of the entries to say the map-to is private.
  { "<bits/cmath.tcc>", kPrivate, "<cmath>", kPublic },
  { "<debug/safe_iterator.tcc>", kPrivate, "<debug/safe_iterator.h>", kPublic },
  { "<tr1_impl/random.tcc>", kPrivate, "<tr1_impl/random>", kPrivate },
  // Some bits->bits #includes: A few files in bits re-export
  // symbols from other files in bits.
  // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && grep '^ *# *include.*bits/' bits/* | perl -nle 'm/^([^:]+).*<([^>]+)>/ && print qq@  { "<$2>", kPrivate, "<$1>", kPrivate },@' | grep bits/ | sort -u)
  // and carefully picked reasonable-looking results (algorithm
  // *uses* pair but doesn't *re-export* pair, for instance).
  { "<bits/c++allocator.h>", kPrivate, "<bits/allocator.h>", kPrivate },
  { "<bits/ctype_base.h>", kPrivate, "<bits/locale_facets.h>", kPrivate },
  { "<bits/ctype_inline.h>", kPrivate, "<bits/locale_facets.h>", kPrivate },
  { "<bits/messages_members.h>", kPrivate,
    "<bits/locale_facets_nonio.h>", kPrivate },
  { "<bits/stl_move.h>", kPrivate, "<bits/stl_algobase.h>", kPrivate },
  // I don't think we want to be having people move to 'backward/'
  // yet.  (These hold deprecated STL classes that we still use
  // actively.)  These are the ones that turned up in an analysis of
  { "<backward/hash_fun.h>", kPrivate, "<hash_map>", kPublic },
  { "<backward/hash_fun.h>", kPrivate, "<hash_set>", kPublic },
  { "<backward/hashtable.h>", kPrivate, "<hash_map>", kPublic },
  { "<backward/hashtable.h>", kPrivate, "<hash_set>", kPublic },
  { "<backward/strstream>", kPrivate, "<strstream>", kPublic },
  // We have backward as part of the -I search path now, so have the
  // non-backwards-prefix version as well.
  { "<auto_ptr.h>", kPrivate, "<memory>", kPublic },
  { "<binders.h>", kPrivate, "<functional>", kPublic },
  { "<hash_fun.h>", kPrivate, "<hash_map>", kPublic },
  { "<hash_fun.h>", kPrivate, "<hash_set>", kPublic },
  { "<hashtable.h>", kPrivate, "<hash_map>", kPublic },
  { "<hashtable.h>", kPrivate, "<hash_set>", kPublic },
  // (This one should perhaps be found automatically somehow.)
  { "<ext/sso_string_base.h>", kPrivate, "<string>", kPublic },
  // The iostream .h files are confusing.  Lots of private headers,
  // which are handled above, but we also have public headers
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
  // The location of exception_defines.h varies by GCC version.  It should
  // never be included directly.
  { "<exception_defines.h>", kPrivate, "<exception>", kPublic },

  // post libstdc++-10 stuff which is not automatically caught by commands above
  { "<bits/exception.h>", kPrivate, "<exception>", kPublic },
  { "<pstl/execution_defs.h>", kPrivate, "<execution>", kPublic },
  { "<pstl/glue_algorithm_impl.h>", kPrivate, "<execution>", kPublic },
  { "<pstl/glue_execution_defs.h>", kPrivate, "<execution>", kPublic },
  { "<pstl/parallel_backend_tbb.h>", kPrivate, "<execution>", kPublic },
  { "<tbb/tbb_stddef.h>", kPrivate, "<execution>", kPublic },
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
  if (status == kCalculating) {   // means there's a cycle in the mapping
    // TODO: Reconsider cycle handling; the include_cycle test fails without
    // this special-casing, but it seems we should handle this more generally.
    if (key.find("internal/") != string::npos) {
      VERRS(4) << "Ignoring a cyclical mapping involving " << key << "\n";
      return;
    }
  }
  if (status == kCalculating) {
    VERRS(0) << "Cycle in include-mapping:\n";
    for (const string& node : *node_stack)
      VERRS(0) << "  " << node << " ->\n";
    VERRS(0) << "  " << key << "\n";
    CHECK_UNREACHABLE_("Cycle in include-mapping");  // cycle is a fatal error
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

IncludePicker::IncludePicker(bool no_default_mappings)
    : has_called_finalize_added_include_lines_(false) {
  if (!no_default_mappings) {
    AddDefaultMappings();
  }
}

void IncludePicker::AddDefaultMappings() {
  AddSymbolMappings(libc_symbol_map, IWYU_ARRAYSIZE(libc_symbol_map));
  AddSymbolMappings(libstdcpp_symbol_map, IWYU_ARRAYSIZE(libstdcpp_symbol_map));

  AddIncludeMappings(libc_include_map,
      IWYU_ARRAYSIZE(libc_include_map));
  AddIncludeMappings(stdlib_c_include_map,
      IWYU_ARRAYSIZE(stdlib_c_include_map));
  AddIncludeMappings(libstdcpp_include_map,
      IWYU_ARRAYSIZE(libstdcpp_include_map));

  AddPublicIncludes(stdlib_cpp_public_headers,
      IWYU_ARRAYSIZE(stdlib_cpp_public_headers));
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
    AddMapping(quoted_includee, mapped_includer);
  }

  // Automatically mark <asm-FOO/bar.h> as private, and map to <asm/bar.h>.
  if (StartsWith(quoted_includee, "<asm-")) {
    MarkIncludeAsPrivate(quoted_includee);
    string public_header = quoted_includee;
    StripPast(&public_header, "/");   // read past "asm-whatever/"
    public_header = "<asm/" + public_header;   // now it's <asm/something.h>
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
      const vector<MappedInclude>& map_to = filepath_include_map_[regex_key];
      // Enclose the regex in ^(...)$ for full match.
      std::regex regex(std::string("^(" + regex_key.substr(1) + ")$"));
      if (std::regex_match(hdr, regex) &&
          !ContainsQuotedInclude(map_to, hdr)) {
        Extend(&filepath_include_map_[hdr], filepath_include_map_[regex_key]);
        MarkVisibility(&include_visibility_map_, hdr,
                       include_visibility_map_[regex_key]);
      }
    }
    for (const string& regex_key : friend_to_headers_map_regex_keys) {
      std::regex regex(std::string("^(" + regex_key.substr(1) + ")$"));
      if (std::regex_match(hdr, regex)) {
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

bool IncludePicker::IsPublic(const clang::FileEntry* file) const {
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
