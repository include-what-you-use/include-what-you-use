//===--- iwyu_include_picker.cpp - map to canonical #includes for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "port.h"
#include "iwyu_include_picker.h"

#ifndef _MSC_VER      // _MSC_VER gets its own fnmatch from ./port.h
#include <fnmatch.h>
#endif
#include <algorithm>
#include <map>  // not hash_map: it's not as portable and needs hash<string>
#include <string>
#include <utility>
#include <vector>
#include "iwyu_globals.h"
#include "iwyu_path_util.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "llvm/Support/Path.h"

using std::find;
using std::map;
using std::pair;
using std::string;
using std::vector;

namespace include_what_you_use {

namespace {

// For ease of maintenance, we split up the hard-coded filepath mappings
// into sections: one for C++ mappings, one for C mappings, one for
// Google-specific mappings, one for third-party/ mappings (which is
// actually treated specially), etc.  They will all get inserted into
// the same data structure, though.
//
// The entries below support limited globbing: '*' will match any
// range of characters, *including slash* (unlike shell globbing).
// So "bits/*" means "every file under bits, including bits/a/foo.h".
// We support all the globbing metacharacters that fnmatch supports.
//
// Array entries below may be prefixed by a comment saying what shell
// command I ran to produce the data.  I often would manually sanitize
// the data afterwards, though.

// Shorter nicknames.
const IncludePicker::Visibility kPrivate = IncludePicker::kPrivate;
const IncludePicker::Visibility kPublic = IncludePicker::kPublic;

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
const IncludePicker::IncludeMapEntry symbol_include_map[] = {
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

  // These are c++ symbol maps that handle the forwarding headers
  // that define classes as typedefs.  Because gcc uses typedefs for
  // these, we are tricked into thinking the classes are defined
  // there, rather than just declared there.  This maps each symbol
  // to where it's defined (I had to fix up ios manually, and add in
  // iostream and string which are defined unusually in gcc headers):
  // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1; find . -name '*fwd*' | xargs grep -oh 'typedef basic_[^ <]*' | sort -u | sed "s/typedef basic_//" | while read class; do echo -n "$class "; grep -lR "^ *class basic_$class " *; echo | head -n1; done | grep . | perl -lane 'print qq@    { "std::$F[0]", kPrivate, "<$F[1]>", kPublic },@;' )
  { "std::filebuf", kPrivate, "<fstream>", kPublic },
  { "std::fstream", kPrivate, "<fstream>", kPublic },
  { "std::ifstream", kPrivate, "<fstream>", kPublic },
  { "std::ios", kPrivate, "<ios>", kPublic },
  { "std::iostream", kPrivate, "<iostream>", kPublic },
  { "std::istream", kPrivate, "<istream>", kPublic },
  { "std::istringstream", kPrivate, "<sstream>", kPublic },
  { "std::ofstream", kPrivate, "<fstream>", kPublic },
  { "std::ostream", kPrivate, "<ostream>", kPublic },
  { "std::ostringstream", kPrivate, "<sstream>", kPublic },
  { "std::streambuf", kPrivate, "<streambuf>", kPublic },
  { "std::string", kPrivate, "<string>", kPublic },
  { "std::stringbuf", kPrivate, "<sstream>", kPublic },
  { "std::stringstream", kPrivate, "<sstream>", kPublic },
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

const IncludePicker::IncludeMapEntry c_include_map[] = {
  // ( cd /usr/include && grep '^ *# *include' {sys/,net/,}* | perl -nle 'm/^([^:]+).*<([^>]+)>/ && print qq@    { "<$2>", kPrivate, "<$1>", kPublic },@' | grep bits/ | sort )
  // When I saw more than one mapping for these, I typically picked
  // what I thought was the "best" one.
  { "<bits/a.out.h>", kPrivate, "<a.out.h>", kPublic },
  { "<bits/byteswap.h>", kPrivate, "<byteswap.h>", kPublic },
  { "<bits/cmathcalls.h>", kPrivate, "<complex.h>", kPublic },
  { "<bits/confname.h>", kPrivate, "<unistd.h>", kPublic },
  { "<bits/dirent.h>", kPrivate, "<dirent.h>", kPublic },
  { "<bits/dlfcn.h>", kPrivate, "<dlfcn.h>", kPublic },
  { "<bits/elfclass.h>", kPrivate, "<link.h>", kPublic },
  { "<bits/endian.h>", kPrivate, "<endian.h>", kPublic },
  { "<bits/environments.h>", kPrivate, "<unistd.h>", kPublic },
  { "<bits/errno.h>", kPrivate, "<errno.h>", kPublic },
  { "<bits/error.h>", kPrivate, "<error.h>", kPublic },
  { "<bits/fcntl.h>", kPrivate, "<fcntl.h>", kPublic },
  { "<bits/fcntl2.h>", kPrivate, "<fcntl.h>", kPublic },
  { "<bits/fenv.h>", kPrivate, "<fenv.h>", kPublic },
  { "<bits/fenvinline.h>", kPrivate, "<fenv.h>", kPublic },
  { "<bits/huge_val.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/huge_valf.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/huge_vall.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/ioctl-types.h>", kPrivate, "<sys/ioctl.h>", kPublic },
  { "<bits/ioctls.h>", kPrivate, "<sys/ioctl.h>", kPublic },
  { "<bits/ipc.h>", kPrivate, "<sys/ipc.h>", kPublic },
  { "<bits/ipctypes.h>", kPrivate, "<sys/ipc.h>", kPublic },
  { "<bits/libio-ldbl.h>", kPrivate, "<libio.h>", kPublic },
  { "<bits/link.h>", kPrivate, "<link.h>", kPublic },
  { "<bits/locale.h>", kPrivate, "<locale.h>", kPublic },
  { "<bits/mathcalls.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mathdef.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mman.h>", kPrivate, "<sys/mman.h>", kPublic },
  { "<bits/monetary-ldbl.h>", kPrivate, "<monetary.h>", kPublic },
  { "<bits/mqueue.h>", kPrivate, "<mqueue.h>", kPublic },
  { "<bits/mqueue2.h>", kPrivate, "<mqueue.h>", kPublic },
  { "<bits/msq.h>", kPrivate, "<sys/msg.h>", kPublic },
  { "<bits/nan.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/netdb.h>", kPrivate, "<netdb.h>", kPublic },
  { "<bits/poll.h>", kPrivate, "<sys/poll.h>", kPrivate },
  { "<bits/posix1_lim.h>", kPrivate, "<limits.h>", kPublic },
  { "<bits/posix2_lim.h>", kPrivate, "<limits.h>", kPublic },
  { "<bits/posix_opt.h>", kPrivate, "<unistd.h>", kPublic },
  { "<bits/printf-ldbl.h>", kPrivate, "<printf.h>", kPublic },
  { "<bits/pthreadtypes.h>", kPrivate, "<pthread.h>", kPublic },
  { "<bits/resource.h>", kPrivate, "<sys/resource.h>", kPublic },
  { "<bits/sched.h>", kPrivate, "<sched.h>", kPublic },
  { "<bits/select.h>", kPrivate, "<sys/select.h>", kPublic },
  { "<bits/sem.h>", kPrivate, "<sys/sem.h>", kPublic },
  { "<bits/semaphore.h>", kPrivate, "<semaphore.h>", kPublic },
  { "<bits/setjmp.h>", kPrivate, "<setjmp.h>", kPublic },
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
  { "<bits/stab.def>", kPrivate, "<stab.h>", kPublic },
  { "<bits/stat.h>", kPrivate, "<sys/stat.h>", kPublic },
  { "<bits/statfs.h>", kPrivate, "<sys/statfs.h>", kPublic },
  { "<bits/statvfs.h>", kPrivate, "<sys/statvfs.h>", kPublic },
  { "<bits/stdio-ldbl.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/stdio-lock.h>", kPrivate, "<libio.h>", kPublic },
  { "<bits/stdio.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/stdio2.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/stdio_lim.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/stdlib-ldbl.h>", kPrivate, "<stdlib.h>", kPublic },
  { "<bits/stdlib.h>", kPrivate, "<stdlib.h>", kPublic },
  { "<bits/string.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/string2.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/string3.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/stropts.h>", kPrivate, "<stropts.h>", kPublic },
  { "<bits/sys_errlist.h>", kPrivate, "<stdio.h>", kPublic },
  { "<bits/syscall.h>", kPrivate, "<sys/syscall.h>", kPrivate },
  { "<bits/syslog-ldbl.h>", kPrivate, "<sys/syslog.h>", kPrivate },
  { "<bits/syslog-path.h>", kPrivate, "<sys/syslog.h>", kPrivate },
  { "<bits/syslog.h>", kPrivate, "<sys/syslog.h>", kPrivate },
  { "<bits/termios.h>", kPrivate, "<termios.h>", kPublic },
  { "<bits/time.h>", kPrivate, "<sys/time.h>", kPublic },
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
  { "<bits/xopen_lim.h>", kPrivate, "<limits.h>", kPublic },
  { "<bits/xtitypes.h>", kPrivate, "<stropts.h>", kPublic },
  // Sometimes libc tells you what mapping to do via an '#error':
  // # error "Never use <bits/dlfcn.h> directly; include <dlfcn.h> instead."
  // ( cd /usr/include && grep -R '^ *# *error "Never use' * | perl -nle 'm/<([^>]+).*<([^>]+)/ && print qq@    { "<$1>", kPrivate, "<$2>", kPublic },@' | sort )
  { "<bits/a.out.h>", kPrivate, "<a.out.h>", kPublic },
  { "<bits/byteswap.h>", kPrivate, "<byteswap.h>", kPublic },
  { "<bits/cmathcalls.h>", kPrivate, "<complex.h>", kPublic },
  { "<bits/confname.h>", kPrivate, "<unistd.h>", kPublic },
  { "<bits/dirent.h>", kPrivate, "<dirent.h>", kPublic },
  { "<bits/dlfcn.h>", kPrivate, "<dlfcn.h>", kPublic },
  { "<bits/elfclass.h>", kPrivate, "<link.h>", kPublic },
  { "<bits/endian.h>", kPrivate, "<endian.h>", kPublic },
  { "<bits/fcntl.h>", kPrivate, "<fcntl.h>", kPublic },
  { "<bits/fenv.h>", kPrivate, "<fenv.h>", kPublic },
  { "<bits/huge_val.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/huge_valf.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/huge_vall.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/in.h>", kPrivate, "<netinet/in.h>", kPublic },
  { "<bits/inf.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/ioctl-types.h>", kPrivate, "<sys/ioctl.h>", kPublic },
  { "<bits/ioctls.h>", kPrivate, "<sys/ioctl.h>", kPublic },
  { "<bits/ipc.h>", kPrivate, "<sys/ipc.h>", kPublic },
  { "<bits/locale.h>", kPrivate, "<locale.h>", kPublic },
  { "<bits/mathdef.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mathinline.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/mman.h>", kPrivate, "<sys/mman.h>", kPublic },
  { "<bits/mqueue.h>", kPrivate, "<mqueue.h>", kPublic },
  { "<bits/msq.h>", kPrivate, "<sys/msg.h>", kPublic },
  { "<bits/nan.h>", kPrivate, "<math.h>", kPublic },
  { "<bits/poll.h>", kPrivate, "<sys/poll.h>", kPrivate },
  { "<bits/predefs.h>", kPrivate, "<features.h>", kPublic },
  { "<bits/resource.h>", kPrivate, "<sys/resource.h>", kPublic },
  { "<bits/select.h>", kPrivate, "<sys/select.h>", kPublic },
  { "<bits/semaphore.h>", kPrivate, "<semaphore.h>", kPublic },
  { "<bits/sigcontext.h>", kPrivate, "<signal.h>", kPublic },
  { "<bits/string.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/string2.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/string3.h>", kPrivate, "<string.h>", kPublic },
  { "<bits/syscall.h>", kPrivate, "<sys/syscall.h>", kPrivate },
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
  { "<stdarg.h>", kPublic, "<cstdarg>", kPublic },
  { "<stdbool.h>", kPublic, "<cstdbool>", kPublic },
  { "<stddef.h>", kPublic, "<cstddef>", kPublic },
  { "<stdint.h>", kPublic, "<cstdint>", kPublic },
  { "<stdio.h>", kPublic, "<cstdio>", kPublic },
  { "<stdlib.h>", kPublic, "<cstdlib>", kPublic },
  { "<string.h>", kPublic, "<cstring>", kPublic },
  { "<tgmath.h>", kPublic, "<ctgmath>", kPublic },
  { "<time.h>", kPublic, "<ctime>", kPublic },
  { "<wchar.h>", kPublic, "<cwchar>", kPublic },
  { "<wctype.h>", kPublic, "<cwctype>", kPublic },
};

const IncludePicker::IncludeMapEntry cpp_include_map[] = {
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
  { "<tr1_impl/functional_hash.h>", kPrivate,
    "<tr1/functional_hash.h>", kPublic },
  { "<tr1_impl/hashtable>", kPrivate, "<tr1/hashtable.h>", kPublic },
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
  { "<bits/locale_classes.tcc>", kPrivate, "<bits/locale_classes.h>", kPrivate },
  { "<bits/locale_facets.tcc>", kPrivate, "<bits/locale_facets.h>", kPrivate },
  { "<bits/locale_facets_nonio.tcc>", kPrivate,
    "<bits/locale_facets_nonio.h>", kPrivate },
  { "<bits/ostream.tcc>", kPrivate, "<ostream>", kPublic },
  { "<bits/sstream.tcc>", kPrivate, "<sstream>", kPublic },
  { "<bits/streambuf.tcc>", kPrivate, "<streambuf>", kPublic },
  { "<bits/valarray_array.tcc>", kPrivate, "<bits/valarray_array.h>", kPrivate },
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
  { "<backward/binders.h>", kPrivate, "<functional>", kPublic },
  { "<backward/hash_fun.h>", kPrivate, "<hash_map>", kPublic },
  { "<backward/hash_fun.h>", kPrivate, "<hash_set>", kPublic },
  { "<backward/hashtable.h>", kPrivate, "<hash_map>", kPublic },
  { "<backward/hashtable.h>", kPrivate, "<hash_set>", kPublic },
  { "<backward/strstream>", kPrivate, "<strstream>", kPublic },
  // We have backward as part of the -I search path now, so have the
  // non-backwards-prefix version as well.
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
  // ( cd /usr/crosstool/v12/gcc-4.3.1-glibc-2.3.6-grte/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/include/c++/4.3.1 && egrep '^ *# *include <(istream|ostream|iostream|fstream|sstream|streambuf|ios)>' *stream* ios | perl -nle 'm/^([^:]+).*[<"]([^>"]+)[>"]/ and print qq@    { "<$2>", kPublic, "<$1>", kPublic },@' | sort -u )
  { "<ios>", kPublic, "<istream>", kPublic },
  { "<ios>", kPublic, "<ostream>", kPublic },
  { "<istream>", kPublic, "<fstream>", kPublic },
  { "<istream>", kPublic, "<iostream>", kPublic },
  { "<istream>", kPublic, "<sstream>", kPublic },
  { "<ostream>", kPublic, "<fstream>", kPublic },
  { "<ostream>", kPublic, "<iostream>", kPublic },
  { "<ostream>", kPublic, "<istream>", kPublic },
  { "<ostream>", kPublic, "<sstream>", kPublic },
  { "<streambuf>", kPublic, "<ios>", kPublic },
};

const IncludePicker::IncludeMapEntry google_include_map[] = {
  // These two are here just for unittesting.
  { "\"tests/badinc-private.h\"",
    kPrivate,
    "\"tests/badinc-inl.h\"",
    kPublic
  },
  { "\"tests/badinc-private2.h\"",
    kPrivate,
    "\"tests/badinc-inl.h\"",
    kPublic
  },
  { "\"tests/keep_mapping-private*\"",
    kPrivate,
    "\"tests/keep_mapping-public.h\"",
    kPublic
  },
  { "\"tests/keep_mapping-priv.h\"",
    kPrivate,
    "\"tests/keep_mapping-public.h\"",
    kPublic
  },
};


// It's very common for third-party libraries to just expose one
// header file.  So this map takes advantage of glob functionality.
const IncludePicker::IncludeMapEntry third_party_include_map[] = {
  { "\"third_party/dynamic_annotations/*\"", kPrivate,
    "\"base/dynamic_annotations.h\"", kPublic
  },
  { "\"third_party/gmock/include/gmock/*\"", kPrivate,
    "\"testing/base/public/gmock.h\"", kPublic
  },
  { "\"third_party/gtest/include/gtest/*\"", kPrivate,
    "\"testing/base/public/gunit.h\"", kPublic
  },
  { "\"third_party/python2_4_3/*\"", kPrivate, "<Python.h>", kPublic },
  { "\"third_party/icu/include/unicode/umachine.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/uversion.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/uconfig.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/udraft.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/udeprctd.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/uobslete.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/uintrnal.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/usystem.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/urename.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/platform.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/ptypes.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
  { "\"third_party/icu/include/unicode/uvernum.h\"", kPrivate,
    "\"third_party/icu/include/unicode/utypes.h\"", kPublic
  },
};

// Whew!  Done.

inline bool IsQuotedInclude(const string& s) {
  return ((StartsWith(s, "<") && EndsWith(s, ">")) ||
          (StartsWith(s, "\"") && EndsWith(s, "\"")));
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
                        const string& key) {
  // If we've already calculated this node's transitive closure, we're done.
  const TransitiveStatus status = (*seen_nodes)[key];
  if (status == kCalculating) {   // means there's a cycle in the mapping
    // third-party code sometimes has #include cycles (*cough* boost
    // *cough*).  Because we add many implicit third-party mappings,
    // we may add a cycle without meaning to.  The best we can do is
    // to ignore the mapping that causes the cycle.
    if (StartsWith(key, "\"third_party/"))
      return;
  }
  CHECK_(status != kCalculating && "Cycle in include-mapping");
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
    MakeNodeTransitive(filename_map, seen_nodes, *child);
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
  for (Each<string, vector<string> > it(filename_map); !it.AtEnd(); ++it)
    MakeNodeTransitive(filename_map, &seen_nodes, it->first);
}


}  // namespace

// Converts a file-path, such as /usr/include/stdio.h, to a
// quoted include, such as <stdio.h>.
string ConvertToQuotedInclude(const string& filepath) {
  // First, get rid of leading ./'s and the like.
  string path = NormalizeFilePath(filepath);

  // Case 1: Uses an explicit entry on the search path (-I) list.
  const vector<HeaderSearchPath>& search_paths = GlobalHeaderSearchPaths();
  // GlobalHeaderSearchPaths is sorted to be longest-first, so this
  // loop will prefer the longest prefix: /usr/include/c++/4.4/foo
  // will be mapped to <foo>, not <c++/4.4/foo>.
  for (Each<HeaderSearchPath> it(&search_paths); !it.AtEnd(); ++it) {
    if (StripLeft(&path, it->path)) {
      StripLeft(&path, "/");
      if (it->path_type == HeaderSearchPath::kSystemPath)
        return "<" + path + ">";
      else
        return "\"" + path + "\"";
    }
  }


  // Case 2: Uses the implicit "-I." entry on the search path.  Always local.
  return "\"" + path + "\"";
}

// Returns whether this is a system (as opposed to user) include file,
// based on where it lives.
bool IsSystemIncludeFile(const string& filepath) {
  return ConvertToQuotedInclude(filepath)[0] == '<';
}

#define IWYU_ARRAYSIZE(ar)  (sizeof(ar) / sizeof(*(ar)))

IncludePicker::IncludePicker()
    : symbol_include_map_(),
      filepath_include_map_(),
      filepath_visibility_map_(),
      quoted_includes_to_quoted_includers_(),
      has_called_finalize_added_include_lines_(false) {
  // Parse our hard-coded mappings into a data structure.
  for (size_t i = 0; i < IWYU_ARRAYSIZE(symbol_include_map); ++i) {
    const IncludePicker::IncludeMapEntry& e = symbol_include_map[i];
    CHECK_(IsQuotedInclude(e.map_to) && "Map values must be quoted includes");
    symbol_include_map_[e.map_from].push_back(e.map_to);
    // Symbol-names are always marked as private (or GetPublicValues()
    // will self-map them, below).
    MarkVisibility(e.map_from, kPrivate);
    MarkVisibility(e.map_to, e.to_visibility);
  }
  for (size_t i = 0; i < IWYU_ARRAYSIZE(c_include_map); ++i) {
    InsertIntoFilepathIncludeMap(c_include_map[i]);
  }
  for (size_t i = 0; i < IWYU_ARRAYSIZE(cpp_include_map); ++i) {
    InsertIntoFilepathIncludeMap(cpp_include_map[i]);
  }
  for (size_t i = 0; i < IWYU_ARRAYSIZE(google_include_map); ++i) {
    InsertIntoFilepathIncludeMap(google_include_map[i]);
  }
  for (size_t i = 0; i < IWYU_ARRAYSIZE(third_party_include_map); ++i) {
    InsertIntoFilepathIncludeMap(third_party_include_map[i]);
  }
}

void IncludePicker::MarkVisibility(const string& quoted_include,
                                   IncludePicker::Visibility vis) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");

  // insert() leaves any old value alone, and only inserts if the key is new.
  filepath_visibility_map_.insert(make_pair(quoted_include, vis));
  CHECK_(filepath_visibility_map_[quoted_include] == vis &&
         "Same file seen with two different visibilities");
}

void IncludePicker::InsertIntoFilepathIncludeMap(
    const IncludePicker::IncludeMapEntry& e) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  // Verify that the key/value starts with < or ".
  CHECK_(IsQuotedInclude(e.map_from) && "All map keys must be quoted includes");
  CHECK_(IsQuotedInclude(e.map_to) && "All map values must be quoted includes");
  filepath_include_map_[e.map_from].push_back(e.map_to);
  MarkVisibility(e.map_from, e.from_visibility);
  MarkVisibility(e.map_to, e.to_visibility);
}

// AddDirectInclude lets us use some hard-coded rules to add filepath
// mappings at runtime.  It includes, for instance, mappings from
// 'project/internal/foo.h' to 'project/public/foo_public.h' in google
// code (Google hides private headers in /internal/, much like glibc
// hides them in /bits/.)
void IncludePicker::AddDirectInclude(const string& includer_filepath,
                                     const string& includee_filepath) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");

  // Note: the includer may be a .cc file, which is unnecessary to add
  // to our map, but harmless.
  const string quoted_includer = ConvertToQuotedInclude(includer_filepath);
  const string quoted_includee = ConvertToQuotedInclude(includee_filepath);

  quoted_includes_to_quoted_includers_[quoted_includee].insert(quoted_includer);

  // Automatically mark files in foo/internal/bar as private, and map them.
  if (quoted_includee.find("/internal/") != string::npos) {
    MarkIncludeAsPrivate(quoted_includee);
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
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  CHECK_(IsQuotedInclude(map_from) && "All map keys must be quoted includes");
  CHECK_(IsQuotedInclude(map_to) && "All map values must be quoted includes");
  filepath_include_map_[map_from].push_back(map_to);
}

void IncludePicker::MarkIncludeAsPrivate(const string& quoted_include) {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't mutate anymore");
  CHECK_(IsQuotedInclude(quoted_include) && "MIAP takes a quoted_include");
  MarkVisibility(quoted_include, kPrivate);
}

// Given a map whose keys may have globs (* or [] or ?), expand the
// globs by matching them against all #includes seen by iwyu.  For
// each include that matches the glob, we add it to the map by copying
// the glob entry and replacing the key with the seen #include.
void IncludePicker::ExpandGlobs() {
  // First, get the glob entries.
  map<string, vector<string> > mappings_with_glob_keys;  // key to values-seen
  for (Each<IncludeMap::value_type>
           it(&filepath_include_map_); !it.AtEnd(); ++it) {
    if (it->first.find_first_of("*[?") != string::npos)
      mappings_with_glob_keys.insert(*it);
  }

  // Then, go through all #includes to see if they match the globs,
  // discarding the identity mappings.
  for (Each<string, set<string> > incmap(&quoted_includes_to_quoted_includers_);
       !incmap.AtEnd(); ++incmap) {
    const string& hdr = incmap->first;
    for (Each<string, vector<string> >
             it(&mappings_with_glob_keys); !it.AtEnd(); ++it) {
      const string& glob_key = it->first;
      const vector<string>& map_to = it->second;
      if (fnmatch(glob_key.c_str(), hdr.c_str(), 0) == 0 &&   // has a match
          !ContainsValue(map_to, hdr)) {
        Extend(&filepath_include_map_[hdr], filepath_include_map_[glob_key]);
        MarkVisibility(hdr, filepath_visibility_map_[glob_key]);
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
// 1) If there's already a mapping for third_party/x.h, do not
//    add any implicit maps for it.
// 2) if not_third_party/x.{h,cc} #includes third_party/y.h,
//    assume y.h is supposed to be included directly, and do not
//    add any implicit maps for it.
// 3) Otherwise, if third_party/x.h #includes third_party/y.h,
//    add a mapping from y.h to x.h, and make y.h private.  This
//    means iwyu will never suggest adding y.h.
void IncludePicker::AddImplicitThirdPartyMappings() {
  set<string> headers_with_explicit_mappings;
  for (Each<IncludeMap::value_type>
           it(&filepath_include_map_); !it.AtEnd(); ++it) {
    if (StartsWith(it->first, "\"third_party/"))
      headers_with_explicit_mappings.insert(it->first);
  }

  set<string> headers_included_from_non_third_party;
  for (Each<string, set<string> >
           it(&quoted_includes_to_quoted_includers_); !it.AtEnd(); ++it) {
    for (Each<string> includer(&it->second); !includer.AtEnd(); ++includer) {
      if (!StartsWith(*includer, "\"third_party/")) {
        headers_included_from_non_third_party.insert(it->first);
        break;
      }
    }
  }

  for (Each<string, set<string> >
           it(&quoted_includes_to_quoted_includers_); !it.AtEnd(); ++it) {
    if (ContainsKey(headers_with_explicit_mappings, it->first) ||
        ContainsKey(headers_included_from_non_third_party, it->first)) {
      continue;
    }
    for (Each<string> includer(&it->second); !includer.AtEnd(); ++includer) {
      CHECK_(StartsWith(*includer, "\"third_party/") && "Why not nixed!");
      AddMapping(it->first, *includer);
    }
  }
}

// Handle work that's best done after we've seen all the mappings
// (including dynamically-added ones) and all the include files.
// For instance, we can now expand all the globs we've seen in
// the mapping-keys, since we have the full list of #includes to
// match them again.  We also transitively-close the maps.
void IncludePicker::FinalizeAddedIncludes() {
  CHECK_(!has_called_finalize_added_include_lines_ && "Can't call FAI twice");

  // The map keys may have *'s in them.  Match those to seen #includes now.
  ExpandGlobs();

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
  vector<string> retval;
  const vector<string>* values = FindInMap(&m, key);
  if (!values || values->empty())
    return retval;

  if (GetOrDefault(filepath_visibility_map_, key, kPublic) == kPublic)
    retval.push_back(key);                // we can map to ourself!
  for (Each<string> it(values); !it.AtEnd(); ++it) {
    if (GetOrDefault(filepath_visibility_map_, *it, kPublic) == kPublic)
      retval.push_back(*it);
  }
  return retval;
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

// Except for the case of one /internal/ file including another, the
// same as GetCandidateHeadersForFilepath.
vector<string> IncludePicker::GetCandidateHeadersForFilepathIncludedFrom(
    const string& included_filepath, const string& including_filepath) const {
  const size_t internal_pos = included_filepath.find("/internal/");
  if (internal_pos != string::npos &&
      // Check if they're both part of the same 'internal' tree -- that
      // is, everything before the 'internal' string is the same.
      included_filepath.substr(0, internal_pos + 1) ==
      including_filepath.substr(0, internal_pos + 1)) {
    vector<string> retval;
    // The file we're including now is just fine.  No need to map it.
    retval.push_back(ConvertToQuotedInclude(included_filepath));
    return retval;
  }
  return GetCandidateHeadersForFilepath(included_filepath);
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

}  // namespace include_what_you_use
