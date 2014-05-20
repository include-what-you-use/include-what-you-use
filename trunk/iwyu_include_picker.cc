//===--- iwyu_include_picker.cc - map to canonical #includes for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_include_picker.h"

#include <stddef.h>                     // for size_t
#include <algorithm>                    // for find
// TODO(wan): make sure IWYU doesn't suggest <iterator>.
#include <iterator>                     // for find
// not hash_map: it's not as portable and needs hash<string>.
#include <map>                          // for map, map<>::mapped_type, etc
#include <memory>
#include <ostream>
#include <string>                       // for string, basic_string, etc
#include <utility>                      // for pair, make_pair
#include <vector>                       // for vector, vector<>::iterator

#include "iwyu_path_util.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "iwyu_verrs.h"
#include "port.h"  // for CHECK_

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/system_error.h"

using std::find;
using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::unique_ptr;
using std::vector;

using llvm::MemoryBuffer;
using llvm::SourceMgr;
using llvm::error_code;
using llvm::errs;
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
  // equal.  The visibility on the symbol-name is ignored; by convension
  // we always set it to kPrivate.
  { "blksize_t", kPrivate, "<sys/types.h>", kPublic },
  { "blkcnt_t", kPrivate, "<sys/stat.h>", kPublic },
  { "blkcnt_t", kPrivate, "<sys/types.h>", kPublic },
  { "blksize_t", kPrivate, "<sys/stat.h>", kPublic },
  { "daddr_t", kPrivate, "<sys/types.h>", kPublic },
  { "daddr_t", kPrivate, "<rpc/types.h>", kPublic },
  { "dev_t", kPrivate, "<sys/types.h>", kPublic },
  { "dev_t", kPrivate, "<sys/stat.h>", kPublic },
  { "error_t", kPrivate, "<errno.h>", kPublic },
  { "error_t", kPrivate, "<argp.h>", kPublic },
  { "error_t", kPrivate, "<argz.h>", kPublic },
  { "fsblkcnt_t", kPrivate, "<sys/types.h>", kPublic },
  { "fsblkcnt_t", kPrivate, "<sys/statvfs.h>", kPublic },
  { "fsfilcnt_t", kPrivate, "<sys/types.h>", kPublic },
  { "fsfilcnt_t", kPrivate, "<sys/statvfs.h>", kPublic },
  { "gid_t", kPrivate, "<sys/types.h>", kPublic },
  { "gid_t", kPrivate, "<grp.h>", kPublic },
  { "gid_t", kPrivate, "<pwd.h>", kPublic },
  { "gid_t", kPrivate, "<stropts.h>", kPublic },
  { "gid_t", kPrivate, "<sys/ipc.h>", kPublic },
  { "gid_t", kPrivate, "<sys/stat.h>", kPublic },
  { "gid_t", kPrivate, "<unistd.h>", kPublic },
  { "id_t", kPrivate, "<sys/types.h>", kPublic },
  { "id_t", kPrivate, "<sys/resource.h>", kPublic },
  { "ino64_t", kPrivate, "<sys/types.h>", kPublic },
  { "ino64_t", kPrivate, "<dirent.h>", kPublic },
  { "ino_t", kPrivate, "<sys/types.h>", kPublic },
  { "ino_t", kPrivate, "<dirent.h>", kPublic },
  { "ino_t", kPrivate, "<sys/stat.h>", kPublic },
  { "int8_t", kPrivate, "<sys/types.h>", kPublic },
  { "int8_t", kPrivate, "<stdint.h>", kPublic },
  { "intptr_t", kPrivate, "<stdint.h>", kPublic },
  { "intptr_t", kPrivate, "<unistd.h>", kPublic },
  { "key_t", kPrivate, "<sys/types.h>", kPublic },
  { "key_t", kPrivate, "<sys/ipc.h>", kPublic },
  { "mode_t", kPrivate, "<sys/types.h>", kPublic },
  { "mode_t", kPrivate, "<sys/stat.h>", kPublic },
  { "mode_t", kPrivate, "<sys/ipc.h>", kPublic },
  { "mode_t", kPrivate, "<sys/mman.h>", kPublic },
  { "nlink_t", kPrivate, "<sys/types.h>", kPublic },
  { "nlink_t", kPrivate, "<sys/stat.h>", kPublic },
  { "off64_t", kPrivate, "<sys/types.h>", kPublic },
  { "off64_t", kPrivate, "<unistd.h>", kPublic },
  { "off_t", kPrivate, "<sys/types.h>", kPublic },
  { "off_t", kPrivate, "<unistd.h>", kPublic },
  { "off_t", kPrivate, "<sys/stat.h>", kPublic },
  { "off_t", kPrivate, "<sys/mman.h>", kPublic },
  { "pid_t", kPrivate, "<sys/types.h>", kPublic },
  { "pid_t", kPrivate, "<unistd.h>", kPublic },
  { "pid_t", kPrivate, "<signal.h>", kPublic },
  { "pid_t", kPrivate, "<sys/msg.h>", kPublic },
  { "pid_t", kPrivate, "<sys/shm.h>", kPublic },
  { "pid_t", kPrivate, "<termios.h>", kPublic },
  { "pid_t", kPrivate, "<time.h>", kPublic },
  { "pid_t", kPrivate, "<utmpx.h>", kPublic },
  { "sigset_t", kPrivate, "<signal.h>", kPublic },
  { "sigset_t", kPrivate, "<sys/epoll.h>", kPublic },
  { "sigset_t", kPrivate, "<sys/select.h>", kPublic },
  { "socklen_t", kPrivate, "<bits/socket.h>", kPrivate },
  { "socklen_t", kPrivate, "<unistd.h>", kPublic },
  { "socklen_t", kPrivate, "<arpa/inet.h>", kPublic },
  { "ssize_t", kPrivate, "<sys/types.h>", kPublic },
  { "ssize_t", kPrivate, "<unistd.h>", kPublic },
  { "ssize_t", kPrivate, "<monetary.h>", kPublic },
  { "ssize_t", kPrivate, "<sys/msg.h>", kPublic },
  { "suseconds_t", kPrivate, "<sys/types.h>", kPublic },
  { "suseconds_t", kPrivate, "<sys/time.h>", kPublic },
  { "suseconds_t", kPrivate, "<sys/select.h>", kPublic },
  { "u_char", kPrivate, "<sys/types.h>", kPublic },
  { "u_char", kPrivate, "<rpc/types.h>", kPublic },
  { "uid_t", kPrivate, "<sys/types.h>", kPublic },
  { "uid_t", kPrivate, "<unistd.h>", kPublic },
  { "uid_t", kPrivate, "<pwd.h>", kPublic },
  { "uid_t", kPrivate, "<signal.h>", kPublic },
  { "uid_t", kPrivate, "<stropts.h>", kPublic },
  { "uid_t", kPrivate, "<sys/ipc.h>", kPublic },
  { "uid_t", kPrivate, "<sys/stat.h>", kPublic },
  { "useconds_t", kPrivate, "<sys/types.h>", kPublic },
  { "useconds_t", kPrivate, "<unistd.h>", kPublic },
  // glob.h seems to define size_t if necessary, but it should come from stddef.
  { "size_t", kPrivate, "<stddef.h>", kPublic },
  // Macros that can be defined in more than one file, don't have the
  // same __foo_defined guard that other types do, so the grep above
  // doesn't discover them.  Until I figure out a better way, I just
  // add them in by hand as I discover them.
  { "EOF", kPrivate, "<stdio.h>", kPublic },
  { "EOF", kPrivate, "<libio.h>", kPublic },
  { "va_list", kPrivate, "<stdarg.h>", kPublic },
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
};

// Private -> public include mappings for GNU libc
const IncludeMapEntry libc_include_map[] = {
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
  { "<bits/termios.h>", kPrivate, "<termios.h>", kPublic },
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
  // ( cd /usr/include && grep -R '^ *# *error "Never use' * | perl -nle 'm/<([^>]+).*<([^>]+)/ && print qq@    { "<$1>", kPrivate, "<$2>", kPublic },@' | sort )
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

// Private -> public include mappings for GNU libstdc++
const IncludeMapEntry libstdcpp_include_map[] = {
  // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && grep '^ *# *include' {ext/,tr1/,}* | perl -nle 'm/^([^:]+).*<([^>]+)>/ && print qq@    { "<$2>", kPrivate, "<$1>", kPublic },@' | grep -e bits/ -e tr1_impl/ | sort -u)
  // I removed a lot of 'meaningless' dependencies -- for instance,
  // <functional> #includes <bits/stringfwd.h>, but if someone is
  // using strings, <functional> isn't enough to satisfy iwyu.
  // We may need to add other dirs in future versions of gcc.
  { "<bits/algorithmfwd.h>", kPrivate, "<algorithm>", kPublic },
  { "<bits/allocator.h>", kPrivate, "<memory>", kPublic },
  { "<bits/atomic_word.h>", kPrivate, "<ext/atomicity.h>", kPublic },
  { "<bits/basic_file.h>", kPrivate, "<fstream>", kPublic },
  { "<bits/basic_ios.h>", kPrivate, "<ios>", kPublic },
  { "<bits/basic_string.h>", kPrivate, "<string>", kPublic },
  { "<bits/basic_string.tcc>", kPrivate, "<string>", kPublic },
  { "<bits/boost_sp_shared_count.h>", kPrivate, "<memory>", kPublic },
  { "<bits/c++io.h>", kPrivate, "<ext/stdio_sync_filebuf.h>", kPublic },
  { "<bits/c++config.h>", kPrivate, "<cstddef>", kPublic },
  { "<bits/char_traits.h>", kPrivate, "<string>", kPublic },
  { "<bits/cmath.tcc>", kPrivate, "<cmath>", kPublic },
  { "<bits/codecvt.h>", kPrivate, "<fstream>", kPublic },
  { "<bits/cxxabi_tweaks.h>", kPrivate, "<cxxabi.h>", kPublic },
  { "<bits/deque.tcc>", kPrivate, "<deque>", kPublic },
  { "<bits/fstream.tcc>", kPrivate, "<fstream>", kPublic },
  { "<bits/functional_hash.h>", kPrivate, "<unordered_map>", kPublic },
  { "<bits/gslice.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/gslice_array.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/hashtable.h>", kPrivate, "<unordered_map>", kPublic },
  { "<bits/hashtable.h>", kPrivate, "<unordered_set>", kPublic },
  { "<bits/indirect_array.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/ios_base.h>", kPrivate, "<iostream>", kPublic },
  { "<bits/ios_base.h>", kPrivate, "<ios>", kPublic },
  { "<bits/ios_base.h>", kPrivate, "<iomanip>", kPublic },
  { "<bits/locale_classes.h>", kPrivate, "<locale>", kPublic },
  { "<bits/locale_facets.h>", kPrivate, "<locale>", kPublic },
  { "<bits/locale_facets_nonio.h>", kPrivate, "<locale>", kPublic },
  { "<bits/localefwd.h>", kPrivate, "<locale>", kPublic },
  { "<bits/mask_array.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/ostream.tcc>", kPrivate, "<ostream>", kPublic },
  { "<bits/ostream_insert.h>", kPrivate, "<ostream>", kPublic },
  { "<bits/postypes.h>", kPrivate, "<iostream>", kPublic },
  { "<bits/slice_array.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/stl_algo.h>", kPrivate, "<algorithm>", kPublic },
  { "<bits/stl_algobase.h>", kPrivate, "<algorithm>", kPublic },
  { "<bits/stl_bvector.h>", kPrivate, "<vector>", kPublic },
  { "<bits/stl_construct.h>", kPrivate, "<memory>", kPublic },
  { "<bits/stl_deque.h>", kPrivate, "<deque>", kPublic },
  { "<bits/stl_function.h>", kPrivate, "<functional>", kPublic },
  { "<bits/stl_heap.h>", kPrivate, "<queue>", kPublic },
  { "<bits/stl_iterator.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/stl_iterator_base_funcs.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/stl_iterator_base_types.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/stl_list.h>", kPrivate, "<list>", kPublic },
  { "<bits/stl_map.h>", kPrivate, "<map>", kPublic },
  { "<bits/stl_multimap.h>", kPrivate, "<map>", kPublic },
  { "<bits/stl_multiset.h>", kPrivate, "<set>", kPublic },
  { "<bits/stl_numeric.h>", kPrivate, "<numeric>", kPublic },
  { "<bits/stl_pair.h>", kPrivate, "<utility>", kPublic },
  { "<bits/stl_pair.h>", kPrivate, "<tr1/utility>", kPublic },
  { "<bits/stl_queue.h>", kPrivate, "<queue>", kPublic },
  { "<bits/stl_raw_storage_iter.h>", kPrivate, "<memory>", kPublic },
  { "<bits/stl_relops.h>", kPrivate, "<utility>", kPublic },
  { "<bits/stl_set.h>", kPrivate, "<set>", kPublic },
  { "<bits/stl_stack.h>", kPrivate, "<stack>", kPublic },
  { "<bits/stl_tempbuf.h>", kPrivate, "<memory>", kPublic },
  { "<bits/stl_tree.h>", kPrivate, "<map>", kPublic },
  { "<bits/stl_tree.h>", kPrivate, "<set>", kPublic },
  { "<bits/stl_uninitialized.h>", kPrivate, "<memory>", kPublic },
  { "<bits/stl_vector.h>", kPrivate, "<vector>", kPublic },
  { "<bits/stream_iterator.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/streambuf.tcc>", kPrivate, "<streambuf>", kPublic },
  { "<bits/streambuf_iterator.h>", kPrivate, "<iterator>", kPublic },
  { "<bits/stringfwd.h>", kPrivate, "<string>", kPublic },
  { "<bits/valarray_after.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/valarray_array.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/valarray_before.h>", kPrivate, "<valarray>", kPublic },
  { "<bits/vector.tcc>", kPrivate, "<vector>", kPublic },
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
  // This didn't come from the grep, but seems to be where swap()
  // is defined?
  { "<bits/move.h>", kPrivate, "<algorithm>", kPublic },   // for swap<>()
  // Hash and hashtable-based containers.
  { "<tr1_impl/functional_hash.h>", kPrivate, "<tr1/functional>", kPublic },
  { "<tr1_impl/functional_hash.h>", kPrivate, "<tr1/unordered_map>", kPublic },
  { "<tr1_impl/functional_hash.h>", kPrivate, "<tr1/unordered_set>", kPublic },
  { "<tr1/functional_hash.h>", kPrivate, "<tr1/functional>", kPublic },
  { "<tr1/functional_hash.h>", kPrivate, "<tr1/unordered_map>", kPublic },
  { "<tr1/functional_hash.h>", kPrivate, "<tr1/unordered_set>", kPublic },
  { "<tr1_impl/hashtable>", kPrivate, "<tr1/unordered_map>", kPublic },
  { "<tr1_impl/hashtable>", kPrivate, "<tr1/unordered_set>", kPublic },
  { "<tr1/hashtable.h>", kPrivate, "<tr1/unordered_map>", kPublic },
  { "<tr1/hashtable.h>", kPrivate, "<tr1/unordered_set>", kPublic },
  // All .tcc files are gcc internal-include files.  We get them from
  // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && grep -R '^ *# *include.*tcc' * | perl -nle 'm/^([^:]+).*[<"]([^>"]+)[>"]/ && print qq@    { "<$2>", kPrivate, "<$1>", kPublic },@' | sort )
  // I had to manually edit some of the entries to say the map-to is private.
  { "<bits/basic_ios.tcc>", kPrivate, "<bits/basic_ios.h>", kPrivate },
  { "<bits/basic_string.tcc>", kPrivate, "<string>", kPublic },
  { "<bits/cmath.tcc>", kPrivate, "<cmath>", kPublic },
  { "<bits/deque.tcc>", kPrivate, "<deque>", kPublic },
  { "<bits/fstream.tcc>", kPrivate, "<fstream>", kPublic },
  { "<bits/istream.tcc>", kPrivate, "<istream>", kPublic },
  { "<bits/list.tcc>", kPrivate, "<list>", kPublic },
  { "<bits/locale_classes.tcc>", kPrivate, 
    "<bits/locale_classes.h>", kPrivate },
  { "<bits/locale_facets.tcc>", kPrivate, "<bits/locale_facets.h>", kPrivate },
  { "<bits/locale_facets_nonio.tcc>", kPrivate,
    "<bits/locale_facets_nonio.h>", kPrivate },
  { "<bits/ostream.tcc>", kPrivate, "<ostream>", kPublic },
  { "<bits/sstream.tcc>", kPrivate, "<sstream>", kPublic },
  { "<bits/streambuf.tcc>", kPrivate, "<streambuf>", kPublic },
  { "<bits/valarray_array.tcc>", kPrivate, 
    "<bits/valarray_array.h>", kPrivate },
  { "<bits/vector.tcc>", kPrivate, "<vector>", kPublic },
  { "<debug/safe_iterator.tcc>", kPrivate, "<debug/safe_iterator.h>", kPublic },
  { "<tr1/bessel_function.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/beta_function.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/ell_integral.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/exp_integral.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/gamma.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/hypergeometric.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/legendre_function.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/modified_bessel_func.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/poly_hermite.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/poly_laguerre.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1/riemann_zeta.tcc>", kPrivate, "<tr1/cmath>", kPublic },
  { "<tr1_impl/random.tcc>", kPrivate, "<tr1_impl/random>", kPrivate },
  // Some bits->bits #includes: A few files in bits re-export
  // symbols from other files in bits.
  // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && grep '^ *# *include.*bits/' bits/* | perl -nle 'm/^([^:]+).*<([^>]+)>/ && print qq@  { "<$2>", kPrivate, "<$1>", kPrivate },@' | grep bits/ | sort -u)
  // and carefully picked reasonable-looking results (algorithm
  // *uses* pair but doesn't *re-export* pair, for instance).
  { "<bits/boost_concept_check.h>", kPrivate,
    "<bits/concept_check.h>", kPrivate },
  { "<bits/c++allocator.h>", kPrivate, "<bits/allocator.h>", kPrivate },
  { "<bits/codecvt.h>", kPrivate, "<bits/locale_facets_nonio.h>", kPrivate },
  { "<bits/ctype_base.h>", kPrivate, "<bits/locale_facets.h>", kPrivate },
  { "<bits/ctype_inline.h>", kPrivate, "<bits/locale_facets.h>", kPrivate },
  { "<bits/functexcept.h>", kPrivate, "<bits/stl_algobase.h>", kPrivate },
  { "<bits/locale_classes.h>", kPrivate, "<bits/basic_ios.h>", kPrivate },
  { "<bits/locale_facets.h>", kPrivate, "<bits/basic_ios.h>", kPrivate },
  { "<bits/messages_members.h>", kPrivate,
    "<bits/locale_facets_nonio.h>", kPrivate },
  { "<bits/postypes.h>", kPrivate, "<bits/char_traits.h>", kPrivate },
  { "<bits/slice_array.h>", kPrivate, "<bits/valarray_before.h>", kPrivate },
  { "<bits/stl_construct.h>", kPrivate, "<bits/stl_tempbuf.h>", kPrivate },
  { "<bits/stl_move.h>", kPrivate, "<bits/stl_algobase.h>", kPrivate },
  { "<bits/stl_uninitialized.h>", kPrivate, "<bits/stl_tempbuf.h>", kPrivate },
  { "<bits/stl_vector.h>", kPrivate, "<bits/stl_bvector.h>", kPrivate },
  { "<bits/streambuf_iterator.h>", kPrivate, "<bits/basic_ios.h>", kPrivate },
  // I don't think we want to be having people move to 'backward/'
  // yet.  (These hold deprecated STL classes that we still use
  // actively.)  These are the ones that turned up in an analysis of
  { "<backward/auto_ptr.h>", kPrivate, "<memory>", kPublic },
  { "<backward/binders.h>", kPrivate, "<functional>", kPublic },
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
  { "<bits/exception_defines.h>", kPrivate, "<exception>", kPublic },
  { "<exception_defines.h>", kPrivate, "<exception>", kPublic },
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
void ExpandOnce(const IncludePicker::IncludeMap& m, vector<string>* nodes) {
  vector<string> nodes_and_children;
  set<string> seen_nodes_and_children;
  for (Each<string> node(nodes); !node.AtEnd(); ++node) {
    // First insert the node itself, then all its kids.
    if (!ContainsKey(seen_nodes_and_children, *node)) {
      nodes_and_children.push_back(*node);
      seen_nodes_and_children.insert(*node);
    }
    if (const vector<string>* children = FindInMap(&m, *node)) {
      for (Each<string> child(children); !child.AtEnd(); ++child) {
        if (!ContainsKey(seen_nodes_and_children, *child)) {
          nodes_and_children.push_back(*child);
          seen_nodes_and_children.insert(*child);
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
    // third-party code sometimes has #include cycles (*cough* boost
    // *cough*).  Because we add many implicit third-party mappings,
    // we may add a cycle without meaning to.  The best we can do is
    // to ignore the mapping that causes the cycle.  Same with code
    // in /internal/.  We could CHECK-fail in such a case, but it's
    // probably better to just keep going.
    if (StartsWith(key, "\"third_party/") ||
        key.find("internal/") != string::npos) {
      VERRS(4) << "Ignoring a cyclical mapping involving " << key << "\n";
      return;
    }
  }
  if (status == kCalculating) {
    VERRS(0) << "Cycle in include-mapping:\n";
    for (size_t i = 0; i < node_stack->size(); ++i)
      VERRS(0) << "  " << (*node_stack)[i] << " ->\n";
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
  for (Each<string> child(&node->second); !child.AtEnd(); ++child) {
    node_stack->push_back(*child);
    MakeNodeTransitive(filename_map, seen_nodes, node_stack, *child);
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
  for (Each<string, vector<string> > it(filename_map); !it.AtEnd(); ++it)
    MakeNodeTransitive(filename_map, &seen_nodes, &node_stack, it->first);
}

// Get a scalar value from a YAML node.
// Returns empty string if it's not of type ScalarNode.
string GetScalarValue(Node* node) {
  ScalarNode* scalar = llvm::dyn_cast<ScalarNode>(node);
  if (scalar == NULL)
    return string();

  llvm::SmallString<8> storage;
  return scalar->getValue(storage).str();
}

// Get a sequence value from a YAML node.
// Returns empty vector if it's not of type SequenceNode.
vector<string> GetSequenceValue(Node* node) {
  vector<string> result;

  SequenceNode* sequence = llvm::dyn_cast<SequenceNode>(node);
  if (sequence != NULL) {
    for (SequenceNode::iterator it = sequence->begin();
         it != sequence->end(); ++it) {
      result.push_back(GetScalarValue(&*it));
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
    for (Each<string> it(&search_path); !it.AtEnd(); ++it) {
      string candidate = MakeAbsolutePath(*it, filename);
      if (llvm::sys::fs::exists(candidate)) {
        return candidate;
      }
    }
  }

  // This is proven not to exist, so handle the error when
  // we attempt to open it.
  return filename;
}

}  // namespace

IncludePicker::IncludePicker(bool no_default_mappings)
    : symbol_include_map_(),
      filepath_include_map_(),
      filepath_visibility_map_(),
      quoted_includes_to_quoted_includers_(),
      has_called_finalize_added_include_lines_(false) {
  if (!no_default_mappings) {
    AddDefaultMappings();
  }
}

void IncludePicker::AddDefaultMappings() {
  AddSymbolMappings(libc_symbol_map, IWYU_ARRAYSIZE(libc_symbol_map));
  AddSymbolMappings(libstdcpp_symbol_map, IWYU_ARRAYSIZE(libstdcpp_symbol_map));

  AddIncludeMappings(libc_include_map,
      IWYU_ARRAYSIZE(libc_include_map));
  AddIncludeMappings(libstdcpp_include_map,
      IWYU_ARRAYSIZE(libstdcpp_include_map));
}

void IncludePicker::MarkVisibility(
    const string& quoted_filepath_pattern,
    IncludeVisibility vis) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");

  // insert() leaves any old value alone, and only inserts if the key is new.
  filepath_visibility_map_.insert(make_pair(quoted_filepath_pattern, vis));
  CHECK_(filepath_visibility_map_[quoted_filepath_pattern] == vis)
      << " Same file seen with two different visibilities: "
      << quoted_filepath_pattern
      << " Old vis: "
      << filepath_visibility_map_[quoted_filepath_pattern]
      << " New vis: "
      << vis;
}

// AddDirectInclude lets us use some hard-coded rules to add filepath
// mappings at runtime.  It includes, for instance, mappings from
// 'project/internal/foo.h' to 'project/public/foo_public.h' in google
// code (Google hides private headers in /internal/, much like glibc
// hides them in /bits/.)
void IncludePicker::AddDirectInclude(const string& includer_filepath,
                                     const string& includee_filepath,
                                     const string& quoted_include_as_typed) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");

  // Note: the includer may be a .cc file, which is unnecessary to add
  // to our map, but harmless.
  const string quoted_includer = ConvertToQuotedInclude(includer_filepath);
  const string quoted_includee = ConvertToQuotedInclude(includee_filepath);

  quoted_includes_to_quoted_includers_[quoted_includee].insert(quoted_includer);
  const pair<string, string> key(includer_filepath, includee_filepath);
  includer_and_includee_to_include_as_typed_[key] = quoted_include_as_typed;

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
    AddMapping(quoted_includee, quoted_includer);
  }

  // Automatically mark <asm-FOO/bar.h> as private, and map to <asm/bar.h>.
  if (StartsWith(quoted_includee, "<asm-")) {
    MarkIncludeAsPrivate(quoted_includee);
    string public_header = quoted_includee;
    StripPast(&public_header, "/");   // read past "asm-whatever/"
    public_header = "<asm/" + public_header;   // now it's <asm/something.h>
    AddMapping(quoted_includee, public_header);
  }
}

void IncludePicker::AddMapping(const string& map_from, const string& map_to) {
  VERRS(4) << "Adding mapping from " << map_from << " to " << map_to << "\n";
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  CHECK_(IsQuotedFilepathPattern(map_from)
         && "All map keys must be quoted filepaths or @ followed by regex");
  CHECK_(IsQuotedInclude(map_to) && "All map values must be quoted includes");
  filepath_include_map_[map_from].push_back(map_to);
}

void IncludePicker::AddIncludeMapping(const string& map_from,
                                      IncludeVisibility from_visibility,
                                      const string& map_to,
                                      IncludeVisibility to_visibility) {
  AddMapping(map_from, map_to);
  MarkVisibility(map_from, from_visibility);
  MarkVisibility(map_to, to_visibility);
}

void IncludePicker::AddSymbolMapping(const string& map_from,
                                     const string& map_to,
                                     IncludeVisibility to_visibility) {
  CHECK_(IsQuotedInclude(map_to) && "Map values must be quoted includes");
  symbol_include_map_[map_from].push_back(map_to);

  // Symbol-names are always marked as private (or GetPublicValues()
  // will self-map them, below).
  MarkVisibility(map_from, kPrivate);
  MarkVisibility(map_to, to_visibility);
}

void IncludePicker::AddIncludeMappings(const IncludeMapEntry* entries,
                                       size_t count) {
  for (size_t i = 0; i < count; ++i) {
    const IncludeMapEntry& e = entries[i];
    AddIncludeMapping(e.map_from, e.from_visibility, e.map_to, e.to_visibility);
  }
}

void IncludePicker::AddSymbolMappings(const IncludeMapEntry* entries,
                                      size_t count) {
  for (size_t i = 0; i < count; ++i) {
    const IncludeMapEntry& e = entries[i];
    AddSymbolMapping(e.map_from, e.map_to, e.to_visibility);
  }
}

void IncludePicker::MarkIncludeAsPrivate(
    const string& quoted_filepath_pattern) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  CHECK_(IsQuotedFilepathPattern(quoted_filepath_pattern)
         && "MIAP takes a quoted filepath pattern");
  MarkVisibility(quoted_filepath_pattern, kPrivate);
}

void IncludePicker::AddFriendRegex(const string& includee,
                                   const string& friend_regex) {
  friend_to_headers_map_["@" + friend_regex].insert(includee);
}

namespace {
// Given a map keyed by quoted filepath patterns, return a vector
// containing the @-regexes among the keys.
template <typename MapType>
vector<string> ExtractKeysMarkedAsRegexes(const MapType& m) {
  vector<string> regex_keys;
  for (Each<typename MapType::value_type> it(&m); !it.AtEnd(); ++it) {
    if (StartsWith(it->first, "@"))
      regex_keys.push_back(it->first);
  }
  return regex_keys;
}
}  // namespace

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
  for (Each<string, set<string> > incmap(&quoted_includes_to_quoted_includers_);
       !incmap.AtEnd(); ++incmap) {
    const string& hdr = incmap->first;
    for (Each<string> it(&filepath_include_map_regex_keys); !it.AtEnd(); ++it) {
      const string& regex_key = *it;
      const vector<string>& map_to = filepath_include_map_[regex_key];
      // Enclose the regex in ^(...)$ for full match.
      llvm::Regex regex(std::string("^(" + regex_key.substr(1) + ")$"));
      if (regex.match(hdr.c_str(), NULL) && !ContainsValue(map_to, hdr)) {
        Extend(&filepath_include_map_[hdr], filepath_include_map_[regex_key]);
        MarkVisibility(hdr, filepath_visibility_map_[regex_key]);
      }
    }
    for (Each<string> it(&friend_to_headers_map_regex_keys);
         !it.AtEnd(); ++it) {
      const string& regex_key = *it;
      llvm::Regex regex(std::string("^(" + regex_key.substr(1) + ")$"));
      if (regex.match(hdr.c_str(), NULL)) {
        InsertAllInto(friend_to_headers_map_[regex_key],
                      &friend_to_headers_map_[hdr]);
      }
    }
  }
}

// We treat third-party code specially, since it's difficult to add
// iwyu pragmas to code we don't own.  Basically, what we do is trust
// the code authors when it comes to third-party code: if they
// #include x.h to get symbols from y.h, then assume that's how the
// third-party authors wanted it.  This boils down to the following
// rules:
// 1) If there's already a mapping for third_party/y.h, do not
//    add any implicit maps for it.
// 2) if not_third_party/x.{h,cc} #includes third_party/y.h,
//    assume y.h is supposed to be included directly, and do not
//    add any implicit maps for it.
// 3) Otherwise, if third_party/x.h #includes third_party/y.h,
//    add a mapping from y.h to x.h.  Unless y.h already has
//    a hard-coded visibility set, make y.h private.  This
//    means iwyu will never suggest adding y.h.
void IncludePicker::AddImplicitThirdPartyMappings() {
  set<string> third_party_headers_with_explicit_mappings;
  for (Each<IncludeMap::value_type>
           it(&filepath_include_map_); !it.AtEnd(); ++it) {
    if (IsThirdPartyFile(it->first))
      third_party_headers_with_explicit_mappings.insert(it->first);
  }

  set<string> headers_included_from_non_third_party;
  for (Each<string, set<string> >
           it(&quoted_includes_to_quoted_includers_); !it.AtEnd(); ++it) {
    for (Each<string> includer(&it->second); !includer.AtEnd(); ++includer) {
      if (!IsThirdPartyFile(*includer)) {
        headers_included_from_non_third_party.insert(it->first);
        break;
      }
    }
  }

  for (Each<string, set<string> >
           it(&quoted_includes_to_quoted_includers_); !it.AtEnd(); ++it) {
    const string& includee = it->first;
    if (!IsThirdPartyFile(includee) ||
        ContainsKey(third_party_headers_with_explicit_mappings, includee) ||
        ContainsKey(headers_included_from_non_third_party, includee)) {
      continue;
    }
    for (Each<string> includer(&it->second); !includer.AtEnd(); ++includer) {
      // From the 'if' statement above, we already know that includee
      // is not included from non-third-party code.
      CHECK_(IsThirdPartyFile(*includer) && "Why not nixed!");
      CHECK_(IsThirdPartyFile(includee) && "Why not nixed!");
      AddMapping(includee, *includer);
      if (GetVisibility(includee) == kUnusedVisibility) {
        MarkIncludeAsPrivate(includee);
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

  // We treat third-party code specially, since it's difficult to add
  // iwyu pragmas to code we don't own.
  AddImplicitThirdPartyMappings();

  // If a.h maps to b.h maps to c.h, we'd like an entry from a.h to c.h too.
  MakeMapTransitive(&filepath_include_map_);
  // Now that filepath_include_map_ is transitively closed, it's an
  // easy task to get the values of symbol_include_map_ closed too.
  // We can't use Each<>() because we need a non-const iterator.
  for (IncludePicker::IncludeMap::iterator it = symbol_include_map_.begin();
       it != symbol_include_map_.end(); ++it) {
    ExpandOnce(filepath_include_map_, &it->second);
  }

  has_called_finalize_added_include_lines_ = true;
}

// For the given key, return the vector of values associated with that
// key, or an empty vector if the key does not exist in the map.
// *However*, we filter out all values that have private visibility
// before returning the vector.  *Also*, if the key is public in
// the map, we insert the key as the first of the returned values,
// this is an implicit "self-map."
vector<string> IncludePicker::GetPublicValues(
    const IncludePicker::IncludeMap& m, const string& key) const {
  CHECK_(!StartsWith(key, "@"));
  vector<string> retval;
  const vector<string>* values = FindInMap(&m, key);
  if (!values || values->empty())
    return retval;

  if (GetOrDefault(filepath_visibility_map_, key, kPublic) == kPublic)
    retval.push_back(key);                // we can map to ourself!
  for (Each<string> it(values); !it.AtEnd(); ++it) {
    CHECK_(!StartsWith(*it, "@"));
    if (GetOrDefault(filepath_visibility_map_, *it, kPublic) == kPublic)
      retval.push_back(*it);
  }
  return retval;
}

string IncludePicker::MaybeGetIncludeNameAsWritten(
    const string& includer_filepath, const string& includee_filepath) const {
  const pair<string, string> key(includer_filepath, includee_filepath);
  // I want to use GetOrDefault here, but it has trouble deducing tpl args.
  const string* value = FindInMap(&includer_and_includee_to_include_as_typed_,
                                  key);
  return value ? *value : "";
}

vector<string> IncludePicker::GetCandidateHeadersForSymbol(
    const string& symbol) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  return GetPublicValues(symbol_include_map_, symbol);
}

vector<string> IncludePicker::GetCandidateHeadersForFilepath(
    const string& filepath) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  const string quoted_header = ConvertToQuotedInclude(filepath);
  vector<string> retval = GetPublicValues(filepath_include_map_, quoted_header);
  if (retval.empty()) {
    // the filepath isn't in include_map, so just quote and return it.
    retval.push_back(quoted_header);
  }
  return retval;
}

// Except for the case that the includer is a 'friend' of the includee
// (via an '// IWYU pragma: friend XXX'), the same as
// GetCandidateHeadersForFilepath.
vector<string> IncludePicker::GetCandidateHeadersForFilepathIncludedFrom(
    const string& included_filepath, const string& including_filepath) const {
  vector<string> retval;
  const string quoted_includer = ConvertToQuotedInclude(including_filepath);
  const string quoted_includee = ConvertToQuotedInclude(included_filepath);
  const set<string>* headers_with_includer_as_friend =
      FindInMap(&friend_to_headers_map_, quoted_includer);
  if (headers_with_includer_as_friend != NULL &&
      ContainsKey(*headers_with_includer_as_friend, included_filepath)) {
    retval.push_back(quoted_includee);
  } else {
    retval = GetCandidateHeadersForFilepath(included_filepath);
    if (retval.size() == 1) {
      const string& quoted_header = retval[0];
      if (GetVisibility(quoted_header) == kPrivate) {
        VERRS(0) << "Warning: "
                 << "No public header found to replace the private header "
                 << quoted_header << "\n";
      }
    }
  }

  // We'll have called ConvertToQuotedInclude on members of retval,
  // but sometimes we can do better -- if included_filepath is in
  // retval, the iwyu-preprocessor may have stored the quoted-include
  // as typed in including_filepath.  This is better to use than
  // ConvertToQuotedInclude because it avoids trouble when the same
  // file is accessible via different include search-paths, or is
  // accessed via a symlink.
  const string& quoted_include_as_typed
      = MaybeGetIncludeNameAsWritten(including_filepath, included_filepath);
  if (!quoted_include_as_typed.empty()) {
    vector<string>::iterator it = std::find(retval.begin(), retval.end(),
                                            quoted_includee);
    if (it != retval.end())
      *it = quoted_include_as_typed;
  }
  return retval;
}

bool IncludePicker::HasMapping(const string& map_from_filepath,
                               const string& map_to_filepath) const {
  CHECK_(has_called_finalize_added_include_lines_ && "Must finalize includes");
  const string quoted_from = ConvertToQuotedInclude(map_from_filepath);
  const string quoted_to = ConvertToQuotedInclude(map_to_filepath);
  // We can't use GetCandidateHeadersForFilepath since includer might be private
  const vector<string>* all_mappers = FindInMap(&filepath_include_map_,
                                                quoted_from);
  if (all_mappers) {
    for (Each<string> it(all_mappers); !it.AtEnd(); ++it) {
      if (*it == quoted_to)
        return true;
    }
  }
  return quoted_to == quoted_from;   // indentity mapping, why not?
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

  unique_ptr<MemoryBuffer> buffer;
  error_code error = MemoryBuffer::getFile(absolute_path, buffer);
  if (error) {
    errs() << "Cannot open mapping file '" << absolute_path << "': "
           << error.message() << ".\n";
    return;
  }

  VERRS(5) << "Adding mappings from file '" << absolute_path << "'.\n";

  SourceMgr source_manager;
  Stream json_stream(buffer.release(), source_manager);

  document_iterator stream_begin = json_stream.begin();
  if (stream_begin == json_stream.end())
    return;

  // Get root sequence.
  Node* root = stream_begin->getRoot();
  SequenceNode *array = llvm::dyn_cast<SequenceNode>(root);
  if (array == NULL) {
    json_stream.printError(root, "Root element must be an array.");
    return;
  }

  for (SequenceNode::iterator it = array->begin(); it != array->end(); ++it) {
    Node* current_node = &(*it);

    // Every item must be a JSON object ("mapping" in YAML terms.)
    MappingNode* mapping = llvm::dyn_cast<MappingNode>(current_node);
    if (mapping == NULL) {
      json_stream.printError(current_node,
          "Mapping directives must be objects.");
      return;
    }

    for (MappingNode::iterator it = mapping->begin();
        it != mapping->end(); ++it) {
      // General form is { directive: <data> }.
      const string directive = GetScalarValue(it->getKey());

      if (directive == "symbol") {
        // Symbol mapping.
        vector<string> mapping = GetSequenceValue(it->getValue());
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

        AddSymbolMapping(mapping[0], mapping[2], to_visibility);
      } else if (directive == "include") {
        // Include mapping.
        vector<string> mapping = GetSequenceValue(it->getValue());
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

        AddIncludeMapping(mapping[0],
            from_visibility,
            mapping[2],
            to_visibility);
      } else if (directive == "ref") {
        // Mapping ref.
        string ref_file = GetScalarValue(it->getValue());
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
    const string& quoted_include) const {
  return GetOrDefault(
      filepath_visibility_map_, quoted_include, kUnusedVisibility);
}

}  // namespace include_what_you_use
