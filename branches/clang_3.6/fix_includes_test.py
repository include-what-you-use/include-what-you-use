#!/usr/bin/python

##===--- fix_includes_test.py - test for fix_includes.py ------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

"""Test for fix_includes.py

Test test test!
"""

__author__ = 'csilvers@google.com (Craig Silverstein)'

import cStringIO
import re
import sys
# I use unittest instead of googletest to ease opensourcing.  Luckily,
# the only googletest function I have to re-implement is assertListEqual.
import unittest
import fix_includes


class FakeFlags(object):
  def __init__(self):
    self.blank_lines = False
    self.comments = True
    self.dry_run = False
    self.ignore_re = None
    self.checkout_command = None
    self.safe_headers = False
    self.separate_project_includes = None
    self.create_cl_if_possible = True
    self.invoking_command_line = 'iwyu.py my_targets'
    self.find_affected_targets = True
    self.keep_iwyu_namespace_format = False

class FakeRunCommand(object):
  def __init__(self):
    self.command = None
    self.args = None

  def RunCommand(self, command, args):
    self.command = command
    self.args = args


class FakeGetCommandOutputWithInput(object):
  def __init__(self, stdout):
    self.stdout = stdout
    self.command = None
    self.stdin = None

  def GetCommandOutputWithInput(self, command, stdin):
    print 'GetCommandOutputWithInput(%s, ...)' % command
    self.command = command
    self.stdin = stdin
    return self.stdout


class FakeGetCommandOutputLines(object):
  def __init__(self):
    self.stdout_lines_map = {}
    self.command = None
    self.args = None

  def GetCommandOutputLines(self, command, args):
    print 'GetCommandOutputLines(%s, %s)' % (command, args)
    self.command = command
    self.args = args
    return self.stdout_lines_map[command]

  def SetCommandOutputLines(self, command, lines):
    lines = [line + '\n' for line in lines]
    self.stdout_lines_map[command] = lines

class FixIncludesBase(unittest.TestCase):
  """Does setup that every test will want."""

  def _ReadFile(self, filename):
    assert filename in self.before_map, filename
    return self.before_map[filename]

  def setUp(self):
    self.flags = FakeFlags()

    # Map from filename to its contents (a list of lines) before fixing.
    self.before_map = {}
    # Map from filename to the 'correct' contents it should have after fixing.
    self.expected_after_map = {}

    # INPUT: fix_includes._ReadFile takes a filename
    # and returns the contents of filename (as a list).
    fix_includes._ReadFile = self._ReadFile

    # OUTPUT: Instead of writing to file, save full output.
    self.actual_after_contents = []
    fix_includes._WriteFileContents = \
        lambda filename, contents: self.actual_after_contents.extend(contents)

    # Stub out OS writeability check to say all files are writeable.
    fix_includes.os.access = \
        lambda filename, flags: True

    # Stub out the checkout command; keep a list of all files checked out.
    self.fake_checkout_command = FakeRunCommand()
    fix_includes._RunCommand = self.fake_checkout_command.RunCommand

    # Stub out the CL creation command.
    self.fake_cl_creation_command = (
        FakeGetCommandOutputWithInput('Change 1234 created.\n'))
    fix_includes._GetCommandOutputWithInput = (
        self.fake_cl_creation_command.GetCommandOutputWithInput)

    # Create a generic 'stubber'.
    self.fake_get_command_output_lines = FakeGetCommandOutputLines()
    fix_includes._GetCommandOutputLines = (
        self.fake_get_command_output_lines.GetCommandOutputLines)

    # Stub out the CL template command.
    self.fake_get_command_output_lines.SetCommandOutputLines(
        'g4', ['Description:',
               '\t<enter description here>',
               '',
               'Files:',
               '\tthis_file_should_not_appear_in_the_cl'])

    # Stub out the find_clients_of_files command.
    self.find_clients_of_files_command = (
        '/google/data/ro/projects/cymbal/tools/find_clients_of_files.par')
    self.fake_get_command_output_lines.SetCommandOutputLines(
        self.find_clients_of_files_command,
        ['==== 1 targets from 1 packages are affected',
         '==== targets',
         '//foo/bar:a'])

  def SetFindClientsOfFilesOutput(self, output_lines):
    self.fake_get_command_output_lines.SetCommandOutputLines(
        self.find_clients_of_files_command, output_lines)

  def assertListEqual(self, a, b):
    """If the two lists aren't equal, raise an error and print the diffs."""
    self.assert_(a == b, (a, b))

  def RegisterFileContents(self, file_contents_map):
    """Parses and stores the given map from filename to file-contents.

    The values of the map are file 'contents', written in a simple
    markup language that allows us to encode both the 'before' and
    expected 'after' contents of a file.  Every line is taken
    literally to be in both the before and after, with the following
    exceptions:
       1) Lines that look like '///+foo' are removed from 'before',
          and replaced by 'foo' in 'after'.  (This is an 'add'
          instruction.)
       2) Lines that end in '///-' are removed from both 'after'
          and the '\s*///-' suffix is removed from 'before'.
          (This is a 'remove' instruction.)

    This function processes the input map to produce self.before_map
    and self.expected_after_map.

    Arguments:
      file_contents_map: a map from filename to 'contents'.  Contents
         is a string, having the format mentioned above.
    """
    remove_re = re.compile('\s*///-$')
    for (filename, contents) in file_contents_map.iteritems():
      before_contents = []
      expected_after_contents = []
      for line in contents.splitlines():
        m = remove_re.search(line)
        if m:
          before_contents.append(line[:m.start()])
        elif line.startswith('///+'):
          expected_after_contents.append(line[len('///+'):])
        else:
          before_contents.append(line)
          expected_after_contents.append(line)
      self.before_map[filename] = before_contents
      self.expected_after_map[filename] = expected_after_contents

  def ProcessAndTest(self, iwyu_output, cmdline_files=None, unedited_files=[],
                     expected_num_modified_files=None):
    """For all files mentioned in iwyu_output, compare expected and actual.

    Arguments:
       iwyu_output: the output the iwyu script gave when run over the
          set of input files.
       cmdline_files: files to pass in to ProcessIWYUOutput (that, in
          an actual fix_includes run, would come from the commandline).
          These limit what files fix_includes chooses to edit.
       unedited_files: the list of files that are listed in iwyu_output,
          but fix_files has chosen not to edit for some reason.
       expected_num_modified_files: what we expect ProcessIWYUOutput to
          return.  If None, suppress this check.
    """
    filenames = re.findall('^(\S+) should add these lines:', iwyu_output, re.M)
    if not filenames:    # This is the other possible starting-line
      filenames = re.findall('^\((\S+) has correct #includes/fwd-decls\)',
                             iwyu_output, re.M)
    expected_after = []
    for filename in set(filenames):   # uniquify
      if filename not in unedited_files:
        expected_after.extend(self.expected_after_map[filename])

    iwyu_output_as_file = cStringIO.StringIO(iwyu_output)
    num_modified_files = fix_includes.ProcessIWYUOutput(iwyu_output_as_file,
                                                        cmdline_files,
                                                        self.flags)

    if expected_after != self.actual_after_contents:
      print "=== Expected:"
      for line in expected_after:
        print line
      print "=== Got:"
      for line in self.actual_after_contents:
        print line
      print "==="
    self.assertListEqual(expected_after, self.actual_after_contents)
    if expected_num_modified_files is not None:
      self.assertEqual(expected_num_modified_files, num_modified_files)

  def MakeFilesUnwriteable(self):
    """Stub out OS writeability check to say all files are NOT writeable."""
    fix_includes.os.access = \
        lambda filename, flags: False


class FixIncludesTest(FixIncludesBase):
  """End-to-end tests from input file to output file."""

  def testSimple(self):
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
simple should add these lines:
#include <stdio.h>
#include "used2.h"

simple should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for simple:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'simple': infile})
    self.ProcessAndTest(iwyu_output, expected_num_modified_files=1)

  def testNodiffOutput(self):
    """Tests handling of the '(<file> has correct #includes)' iwyu output."""
    infile = """\
// Copyright 2010

#include <stdio.h>
#include <ctype.h>  // iwyu will not reorder, even though non-alphabetical

namespace Foo;

namespace Bar;

int main() { return 0; }
"""
    iwyu_output = "(nodiffs.h has correct #includes/fwd-decls)\n"
    self.RegisterFileContents({'nodiffs.h': infile})
    # fix_includes gives special output when there are no changes, so
    # we can't use the normal ProcessAndTest.
    iwyu_output_as_file = cStringIO.StringIO(iwyu_output)
    num_modified_files = fix_includes.ProcessIWYUOutput(iwyu_output_as_file,
                                                        None, self.flags)
    self.assertListEqual([], self.actual_after_contents)  # 'no diffs'
    self.assertEqual(0, num_modified_files)

  def testNodiffOutputWithNoSorting(self):
    """Tests 'correct #includes' iwyu output, but does not need reordering."""
    infile = """\
// Copyright 2010

#include <ctype.h>
#include <stdio.h>

namespace Foo;

namespace Bar;

int main() { return 0; }
"""
    iwyu_output = "(nodiffs_nosorting.h has correct #includes/fwd-decls)\n"
    self.RegisterFileContents({'nodiffs_nosorting.h': infile})
    # fix_includes gives special output when there are no changes, so
    # we can't use the normal ProcessAndTest.
    iwyu_output_as_file = cStringIO.StringIO(iwyu_output)
    num_modified_files = fix_includes.ProcessIWYUOutput(iwyu_output_as_file,
                                                        None, self.flags)
    self.assertListEqual([], self.actual_after_contents)  # 'no diffs'
    self.assertEqual(0, num_modified_files)

  def testRemoveEmptyNamespace(self):
    """Tests we remove a namespace if we remove all fwd-decls inside it."""
    infile = """\
// Copyright 2010

#include <stdio.h>

namespace ns {   ///-
class Foo;       ///-
namespace ns2 {  ///-
namespace ns3 {  ///-
class Bar;       ///-
} }              ///-
class Baz;       ///-
}                ///-
                 ///-
int main() { return 0; }
"""
    iwyu_output = """\
empty_namespace should add these lines:

empty_namespace should remove these lines:
- class Foo;  // lines 6-6
- namespace ns { namespace ns2 { namespace ns3 { class Bar; } } }  // lines 9-9
- namespace ns { class Baz; } }  // lines 11-11

The full include-list for empty_namespace:
#include <stdio.h>
---
"""
    self.RegisterFileContents({'empty_namespace': infile})
    self.ProcessAndTest(iwyu_output)

  def testRemovePartOfEmptyNamespace(self):
    """Tests we remove a namespace if empty, but not enclosing namespaces."""
    infile = """\
// Copyright 2010

namespace maps_transit_realtime {
namespace service_alerts {
class StaticServiceAlertStore;
namespace trigger {                  ///-
class Trigger;                       ///-
}  // namespace trigger              ///-
namespace ui {                       ///-
class Alert;                         ///-
}  // namespace ui                   ///-
}  // namespace service_alerts
}  // namespace maps_transit_realtime

int main() { return 0; }
"""
    iwyu_output = """\
empty_internal_namespace should add these lines:

empty_internal_namespace should remove these lines:
- namespace maps_transit_realtime { namespace service_alerts { namespace trigger { class Trigger; } } }  // lines 7-7
- namespace maps_transit_realtime { namespace service_alerts { namespace ui { class Alert; } } }  // lines 10-10

The full include-list for empty_internal_namespace:
namespace maps_transit_realtime { namespace service_alerts { class StaticServiceAlertStore; } }   // lines 5-5
---
"""
    self.RegisterFileContents({'empty_internal_namespace': infile})
    self.ProcessAndTest(iwyu_output)

  def testRemoveEmptyIfdef(self):
    """Tests we remove an #ifdef if we remove all #includes inside it."""
    # Also makes sure we reorder properly around the removed ifdef.
    infile = """\
// Copyright 2010

#include <stdio.h>
///+#include <stdlib.h>
// Only on windows.   ///-
#ifdef OS_WINDOWS     ///-
#include <notused.h>  ///-
#include <notused2.h> ///-
#endif                ///-
#include "used.h"
#include <stdlib.h>   ///-
///+#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
empty_ifdef should add these lines:
#include "used2.h"

empty_ifdef should remove these lines:
- #include <notused.h>  // lines 6-6
- #include <notused2.h> // lines 7-7

The full include-list for empty_ifdef:
#include <stdio.h>
#include <stdlib.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'empty_ifdef': infile})
    self.ProcessAndTest(iwyu_output)

  def testRemoveEmptyNestedIfdef(self):
    """Tests we remove an empty #ifdef inside a non-empty #ifdef."""
    infile = """\
// Copyright 2010

#include <stdio.h>
#ifdef NDEBUG
  // Only on windows.     ///-
# ifdef OS_WINDOWS        ///-
#   include <notused.h>   ///-
#   include <notused2.h>  ///-
# endif                   ///-
# undef VERBOSE_LOGGING
#endif
///+#include <stdlib.h>
#include "used.h"
#include <stdlib.h>   ///-
///+#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
empty_nested_ifdef should add these lines:
#include "used2.h"

empty_nested_ifdef should remove these lines:
- #include <notused.h>  // lines 7-7
- #include <notused2.h> // lines 8-8

The full include-list for empty_nested_ifdef:
#include <stdlib.h>
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'empty_nested_ifdef': infile})
    self.ProcessAndTest(iwyu_output)

  def testNonEmptyIfdef(self):
    """Tests we keep an #ifdef if we don't remove all #includes inside it."""
    infile = """\
// Copyright 2010

#include <stdio.h>
#ifdef OS_WINDOWS
#include <notused.h>  ///-
#include <used_win.h>
#endif
#include "used.h"
///+#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
nonempty_ifdef should add these lines:
#include "used2.h"

nonempty_ifdef should remove these lines:
- #include <notused.h>  // lines 5-5

The full include-list for nonempty_ifdef:
#include <used_win.h>
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'nonempty_ifdef': infile})
    self.ProcessAndTest(iwyu_output)

  def testKeepIfdefsWithNonIncludes(self):
    """Tests we keep an #ifdef if we have a non-#include inside it."""
    infile = """\
// Copyright 2010

#include <stdio.h>
#ifdef OS_WINDOWS
#define IN_WINDOWS
#include <notused.h>  ///-
#endif
#include "used.h"
///+#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
nonempty_ifdef should add these lines:
#include "used2.h"

nonempty_ifdef should remove these lines:
- #include <notused.h>  // lines 6-6

The full include-list for nonempty_ifdef:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'nonempty_ifdef': infile})
    self.ProcessAndTest(iwyu_output)

  def testRemoveComments(self):
    """Tests we remove comments above #includes."""
    infile = """\
// Copyright 2010

#include <stdio.h>
// This file is not used.   ///-
#include <notused.h>        ///-
                            ///-
// This file is not used either.  ///-
// It's not used.           ///-
// Not used at all.         ///-
#include <notused2.h>       ///-
                            ///-
#include "notused3.h"       ///-

// This comment should stay, it's not before an #include.
const int kInt = 5;
// This file is used.
// It's definitedly used.
#include "used.h"
///+#include "used2.h"

const int kInt2 = 6;
                                                              ///-
// This forward-declare is in a reorder_span all by itself.   ///-
class NotUsed;                                                ///-

// This comment should stay, it's not before an #include.
int main() { return 0; }
"""
    iwyu_output = """\
remove_comments should add these lines:
#include "used2.h"

remove_comments should remove these lines:
- #include <notused.h>  // lines 5-5
- #include <notused2.h>  // lines 10-10
- #include "notused3.h"  // lines 12-12
- class NotUsed;  // lines 23-23

The full include-list for remove_comments:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'remove_comments': infile})
    self.ProcessAndTest(iwyu_output)

  def testNoBlankLineAfterTopOfFileCxxComments(self):
    """Tests we don't remove top-of-file c++ comments right before #includes."""
    infile = """\
// Copyright 2010
#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
top_of_file_comments.cc should add these lines:
#include <stdio.h>
#include "used2.h"

top_of_file_comments.cc should remove these lines:
- #include <notused.h>  // lines 2-2

The full include-list for top_of_file_comments.cc:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'top_of_file_comments.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testNoBlankLineAfterTopOfFileCComments(self):
    """Tests we don't remove top-of-file c comments right before #includes."""
    infile = """\
/*
 * Copyright 2010
 */
#include <notused.h>  ///-
/* This is a one-line c-style comment. */  ///-
#include <notused2.h>   /* this is a c-style comment after a line */ ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
top_of_file_comments.c should add these lines:
#include <stdio.h>
#include "used2.h"

top_of_file_comments.c should remove these lines:
- #include <notused.h>  // lines 4-4
- #include <notused2.h>  // lines 6-6

The full include-list for top_of_file_comments.c:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'top_of_file_comments.c': infile})
    self.ProcessAndTest(iwyu_output)

  def testNotFullLineCComments(self):
    """Tests that we treat lines with c comments then code as code-lines."""
    infile = """\
// Copyright 2010
///+#include <stdio.h>
///+
/* code here */  x = 4;

int main() { return 0; }
"""
    iwyu_output = """\
not_full_line_c_comments.c should add these lines:
#include <stdio.h>

not_full_line_c_comments.c should remove these lines:

The full include-list for not_full_line_c_comments.c:
#include <stdio.h>
---
"""
    self.RegisterFileContents({'not_full_line_c_comments.c': infile})
    self.ProcessAndTest(iwyu_output)

  def testUnusualHFileNames(self):
    """Tests we treat .pb.h files as header files."""
    infile = """\
/*
 * Copyright 2010
 */
#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.pb.h"
///+#include "used2.pb.h"

int main() { return 0; }
"""
    iwyu_output = """\
pb.h.cc should add these lines:
#include <stdio.h>
#include "used2.pb.h"

pb.h.cc should remove these lines:
- #include <notused.h>  // lines 4-4

The full include-list for pb.h.cc:
#include <stdio.h>
#include "used.pb.h"
#include "used2.pb.h"
---
"""
    self.RegisterFileContents({'pb.h.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testFwdDeclLines(self):
    """Tests that we keep or remove forward declares based on iwyu output."""
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

struct KeepStruct;
class NoKeepClass;  ///-
template<typename Foo> class KeepTplClass;

int main() { return 0; }
"""
    iwyu_output = """\
fwd_decl should add these lines:
#include <stdio.h>
#include "used2.h"

fwd_decl should remove these lines:
- #include <notused.h>  // lines 3-3
- class NoKeepClass;  // lines 7-7

The full include-list for fwd_decl:
#include <stdio.h>
#include "used.h"
#include "used2.h"
struct KeepStruct; // lines 6-6
template<typename Foo> class KeepTplClass;  // lines 8-8
---
"""
    self.RegisterFileContents({'fwd_decl': infile})
    self.ProcessAndTest(iwyu_output)

  def testMultiLineFwdDecls(self):
    """Tests we keep forward-decls that span more than one line."""
    infile = """\
// Copyright 2010

struct KeepStruct;
class NoKeepClass;  ///-
template<typename Foo, typename Bar = Baz>
class Keep2LineTplClass;
template<typename Foo, typename Bar = Baz>     ///-
class NoKeep2LineTplClass;                     ///-
template<typename Foo,
         typename Bar = Baz>
class Keep3LineTplClass;
template<typename Foo,                         ///-
         typename Bar = Baz>                   ///-
class NoKeep3LineTplClass;                     ///-

int main() { return 0; }
"""
    iwyu_output = """\
multiline_fwd_decl should add these lines:

multiline_fwd_decl should remove these lines:
- class NoKeepClass;  // lines 4-4
- template<typename Foo, typename Bar = Baz> class NoKeep2LineTplClass;  // lines 7-8
- template<typename Foo, typename Bar = Baz> class NoKeep3LineTplClass;  // lines 12-14

The full include-list for multiline_fwd_decl:
struct KeepStruct; // lines 3-3
template<typename Foo, typename Bar=Baz> class Keep2LineTplClass;  // lines 5-6
template<typename Foo, typename Bar=Baz> class Keep3LineTplClass;  // lines 9-11
---
"""
    self.RegisterFileContents({'multiline_fwd_decl': infile})
    self.ProcessAndTest(iwyu_output)

  def testKeepExplicitSpecializations(self):
    """Tests we don't interpret an explicit spec. as a forward-declare."""
    infile = """\
// Copyright 2010

struct KeepStruct;
class NoKeepClass;  ///-
template<typename Foo> class KeepTplClass;
///+
template<> class KeepTplClass<int>;
template<typename T> void TplFn<T>();

int main() { return 0; }
"""
    iwyu_output = """\
explicit_specialization should add these lines:

explicit_specialization should remove these lines:
- class NoKeepClass;  // lines 4-4

The full include-list for explicit_specialization:
struct KeepStruct; // lines 3-3
template<typename Foo> class KeepTplClass;  // lines 5-5
---
"""
    self.RegisterFileContents({'explicit_specialization': infile})
    self.ProcessAndTest(iwyu_output)

  def testKeepNestedForwardDeclares(self):
    """Tests that we don't remove forward-declares inside classes/structs."""
    infile = """\
// Copyright 2010

class Keep;
class NoKeep;    ///-
///+
class Nest {
  class NestedClass;
///+
  class NestedClass {
  };
  class NestedClass2 { };   // looks just like a fwd declare, except for the {}
  template<typename T>
    class NestedTplClass;   // test multi-line nested classes as well
///+
  friend class NoKeep;
  template<typename T> friend class NoKeepTpl;
};

int main() { return 0; }
"""
    iwyu_output = """\
nested_fwd_decl should add these lines:

nested_fwd_decl should remove these lines:
- class NoKeep;    // lines 4-4

The full include-list for nested_fwd_decl:
class Keep;  // lines 3-3
class Nest::NestedClass;  // lines 6-6
template<typename T> class Nest::NestedTplClass;  // lines 11-11
---
"""
    self.RegisterFileContents({'nested_fwd_decl': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddForwardDeclareBeforeUsingStatement(self):
    """Tests we never add a forward-declare after a contentful line."""
    infile = """\
// Copyright 2010

#include "foo.h"

///+namespace Bar {
///+class Baz;
///+}  // namespace Bar
///+
using Bar::baz;

namespace Foo { class Bang; }  ///-
///+namespace Foo {
///+class Bang;
///+}  // namespace Foo

int main() { return 0; }
"""
    iwyu_output = """\
add_fwd_decl_before_using should add these lines:
namespace Bar { class Baz; }

add_fwd_decl_before_using should remove these lines:

The full include-list for add_fwd_decl_before_using:
#include "foo.h"
namespace Bar { class Baz; }
namespace Foo { class Bang; }  // lines 7-7
---
"""
    self.RegisterFileContents({'add_fwd_decl_before_using': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddForwardDeclareInNamespace(self):
    """Make sure we normalize namespaces properly."""
    infile = """\
// Copyright 2010

#include "foo.h"

///+namespace ns {
///+class Foo;
///+namespace ns2 {
///+namespace ns3 {
///+class Bar;
///+template <typename T> class Bang;
///+}  // namespace ns3
///+}  // namespace ns2
///+namespace ns4 {
///+class Baz;
///+}  // namespace ns4
///+}  // namespace ns
///+

int main() { return 0; }
"""
    iwyu_output = """\
add_fwd_decl_inside_namespace should add these lines:
namespace ns { class Foo; }
namespace ns { namespace ns2 { namespace ns3 { class Bar; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class Bang; } } }
namespace ns { namespace ns4 { class Baz; } }

add_fwd_decl_inside_namespace should remove these lines:

The full include-list for add_fwd_decl_inside_namespace:
#include "foo.h"  // lines 3-3
namespace ns { class Foo; }
namespace ns { namespace ns2 { namespace ns3 { class Bar; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class Bang; } } }
namespace ns { namespace ns4 { class Baz; } }
---
"""
    self.RegisterFileContents({'add_fwd_decl_inside_namespace': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddForwardDeclareInsideNamespaceSometimes(self):
    """Tests that in special situations, we will put fwd-decls inside a ns."""
    infile = """\
// Copyright 2010

#include "foo.h"

class Bar;
template <typename T> class Baz;

namespace ns {

  namespace  ns2   {   // we sure do love nesting our namespaces!

class NsFoo;
///+namespace ns3 {
///+class NsBang;
///+template <typename T> class NsBaz;
///+}  // namespace ns3
template <typename T> class NsBar;

}
}

int main() { return 0; }
"""
    iwyu_output = """\
add_fwd_decl_inside_namespace should add these lines:
namespace ns { namespace ns2 { namespace ns3 { class NsBang; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class NsBaz; } } }

add_fwd_decl_inside_namespace should remove these lines:

The full include-list for add_fwd_decl_inside_namespace:
#include "foo.h"  // lines 3-3
class Bar;  // lines 5-5
namespace ns { namespace ns2 { class NsFoo; } }  // lines 12-12
namespace ns { namespace ns2 { namespace ns3 { class NsBang; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class NsBaz; } } }
namespace ns { namespace ns2 { template <typename T> class NsBar; } }  // lines 13-13
template <typename T> class Baz;  // lines 6-6
---
"""
    self.RegisterFileContents({'add_fwd_decl_inside_namespace': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddForwardDeclareInsideNamespaceWithHeaderGuard(self):
    """Tests that the header guard doesn't confuse our in-ns algorithm."""
    infile = """\
// Copyright 2010

#ifndef HDR_GUARD
#define HDR_GUARD

#include "foo.h"

class Bar;
template <typename T> class Baz;

namespace ns {

  namespace  ns2   {   // we sure do love nesting our namespaces!

class NsFoo;
///+namespace ns3 {
///+class NsBang;
///+template <typename T> class NsBaz;
///+}  // namespace ns3
template <typename T> class NsBar;

}
}

#endif // HDR_GUARD
"""
    iwyu_output = """\
add_fwd_decl_with_hdr_guard should add these lines:
namespace ns { namespace ns2 { namespace ns3 { class NsBang; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class NsBaz; } } }

add_fwd_decl_with_hdr_guard should remove these lines:

The full include-list for add_fwd_decl_with_hdr_guard:
#include "foo.h"  // lines 6-6
class Bar;  // lines 8-8
namespace ns { namespace ns2 { class NsFoo; } }  // lines 15-15
namespace ns { namespace ns2 { namespace ns3 { class NsBang; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class NsBaz; } } }
namespace ns { namespace ns2 { template <typename T> class NsBar; } }  // lines 16-16
template <typename T> class Baz;  // lines 9-9
---
"""
    self.RegisterFileContents({'add_fwd_decl_with_hdr_guard': infile})
    self.ProcessAndTest(iwyu_output)

  def testDoNotAddForwardDeclareInsideNamespaceWithContentfulLine(self):
    """Tests that for 'confusing' code, we keep fwd-decl at the top."""
    infile = """\
// Copyright 2010

#include "foo.h"

class Bar;
///+namespace ns {
///+namespace ns2 {
///+class NsBang;
///+template <typename T> class NsBaz;
///+}  // namespace ns2
///+}  // namespace ns
template <typename T> class Baz;

#ifdef THIS_IS_A_CONTENTFUL_LINE
#include "bar.h"
#endif

namespace ns {

namespace ns2 {

class NsFoo;
template <typename T> class NsBar;

}

}

int main() { return 0; }
"""
    iwyu_output = """\
do_not_add_fwd_decl_inside_namespace should add these lines:
namespace ns { namespace ns2 { class NsBang; } }
namespace ns { namespace ns2 { template <typename T> class NsBaz; } }

do_not_add_fwd_decl_inside_namespace should remove these lines:

The full include-list for do_not_add_fwd_decl_inside_namespace:
#include "foo.h"  // lines 3-3
class Bar;  // lines 5-5
namespace ns { namespace ns2 { class NsBang; } }
namespace ns { namespace ns2 { class NsFoo; } }  // lines 16-16
namespace ns { namespace ns2 { template <typename T> class NsBar; } }  // lines 17-17
namespace ns { namespace ns2 { template <typename T> class NsBaz; } }
template <typename T> class Baz;  // lines 6-6
---
"""
    self.RegisterFileContents({'do_not_add_fwd_decl_inside_namespace': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddForwardDeclareInsideNamespaceWithoutForwardDeclaresAlready(self):
    """Tests we put fwd-decls inside an ns even if the ns has no fwd-decl."""
    infile = """\
// Copyright 2010

#include "foo.h"

class Bar;
template <typename T> class Baz;

namespace ns {

  namespace  ns2   {   // we sure do love nesting our namespaces!

///+namespace ns3 {
///+class NsBang;
///+template <typename T> class NsBaz;
///+}  // namespace ns3
///+
int MyFunction() { }

}
}

int main() { return 0; }
"""
    iwyu_output = """\
add_fwd_decl_inside_namespace_without_fwd_decl should add these lines:
namespace ns { namespace ns2 { namespace ns3 { class NsBang; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class NsBaz; } } }

add_fwd_decl_inside_namespace_without_fwd_decl should remove these lines:

The full include-list for add_fwd_decl_inside_namespace_without_fwd_decl:
#include "foo.h"  // lines 3-3
class Bar;  // lines 5-5
namespace ns { namespace ns2 { namespace ns3 { class NsBang; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class NsBaz; } } }
template <typename T> class Baz;  // lines 6-6
---
"""
    self.RegisterFileContents({'add_fwd_decl_inside_namespace_without_fwd_decl':
                                 infile})
    self.ProcessAndTest(iwyu_output)

  def testAddForwardDeclareInsideNamespaceWithUnnamedNamespace(self):
    """Tests that unnamed namespaces do not mess up our in-ns calculation."""
    infile = """\
// Copyright 2010

#include "foo.h"

class Bar;

namespace ns {
///+
///+class NsBang;
///+template <typename T> class NsBaz;

namespace   {
class NsFoo;
template <typename T> class NsBar;
}
}

int main() { return 0; }
"""
    iwyu_output = """\
add_fwd_decl_inside_namespace_unnamed_ns should add these lines:
namespace ns { class NsBang; }
namespace ns { template <typename T> class NsBaz; }

add_fwd_decl_inside_namespace_unnamed_ns should remove these lines:

The full include-list for add_fwd_decl_inside_namespace_unnamed_ns:
#include "foo.h"  // lines 3-3
class Bar;  // lines 5-5
namespace ns { namespace { class NsFoo; } }  // lines 12-12
namespace ns { class NsBang; }
namespace ns { template <typename T> class NsBaz; }
namespace ns { namespace { template <typename T> class NsBar; } }  // lines 13-13
---
"""
    self.RegisterFileContents({'add_fwd_decl_inside_namespace_unnamed_ns':
                                 infile})
    self.ProcessAndTest(iwyu_output)

  def testRemoveNamespaces(self):
    """Tests that we keep or remove ns's based on fwd decl content."""
    infile = """\
// Copyright 2010

namespace ns1 {
struct KeepStruct;
class NoKeepClass;  ///-
template<typename Foo> class KeepTplClass;
}
                                                ///-
namespace ns1 {                                 ///-
namespace ns2 {                                 ///-
// This should all go away.                     ///-
// Even with the multi-line comment here.       ///-
template<typename Foo> class NoKeepTplClass;    ///-
}                                               ///-
}                                               ///-

int main() { return 0; }
"""
    iwyu_output = """\
ns_fwd_decl should add these lines:

ns_fwd_decl should remove these lines:
- class NoKeepClass;  // lines 5-5
- namespace ns1 { namespace ns2 { template<typename Foo> class NoKeepTplClass; } }   // lines 13-13

The full include-list for ns_fwd_decl:
namespace ns1 { struct KeepStruct; }  // lines 4-4
namespace ns1 { template<typename Foo> class KeepTplClass; }  // lines 6-6
---
"""
    self.RegisterFileContents({'ns_fwd_decl': infile})
    self.ProcessAndTest(iwyu_output)

  def testKeepNamespacesWithNonForwardDecls(self):
    """Tests we never remove a ns with 'real' content in it."""
    infile = """\
// Copyright 2010

namespace ns1 {
int ns_int = 5;
class NoKeepClass;  ///-
}

int main() { return 0; }
"""
    iwyu_output = """\
keep_ns_fwd_decl should add these lines:

keep_ns_fwd_decl should remove these lines:
- class NoKeepClass;  // lines 5-5

The full include-list for keep_ns_fwd_decl:
---
"""
    self.RegisterFileContents({'keep_ns_fwd_decl': infile})
    self.ProcessAndTest(iwyu_output)

  def testICUNamespaces(self):
    """Tests we treat the icu namespace macros as namespaces."""
    infile = """\
// Copyright 2010

U_NAMESPACE_BEGIN   // macro from icu
struct KeepStruct;
class NoKeepClass;  ///-
template<typename Foo> class KeepTplClass;
U_NAMESPACE_END
                                                  ///-
  U_NAMESPACE_BEGIN                               ///-
  template<typename Foo> class NoKeepTplClass;    ///-
  U_NAMESPACE_END                                 ///-

int main() { return 0; }
"""
    iwyu_output = """\
icu_namespace should add these lines:

icu_namespace should remove these lines:
- class NoKeepClass;  // lines 5-5
- template<typename Foo> class NoKeepTplClass;    // lines 10-10

The full include-list for icu_namespace:
namespace ns1 { struct KeepStruct; }  // lines 4-4
namespace ns1 { template<typename Foo> class KeepTplClass; }  // lines 6-6
---
"""
    self.RegisterFileContents({'icu_namespace': infile})
    self.ProcessAndTest(iwyu_output)

  def testHashNamespaces(self):
    """Tests we treat the hash namespace macros as namespaces."""
    infile = """\
// Copyright 2010

HASH_NAMESPACE_DECLARATION_START   // macro from hash.h
struct KeepStruct;
class NoKeepClass;  ///-
template<typename Foo> class KeepTplClass;
HASH_NAMESPACE_DECLARATION_END
                                                  ///-
  HASH_NAMESPACE_DECLARATION_START                ///-
  template<typename Foo> class NoKeepTplClass;    ///-
  HASH_NAMESPACE_DECLARATION_END                  ///-

int main() { return 0; }
"""
    iwyu_output = """\
hash_namespace should add these lines:

hash_namespace should remove these lines:
- class NoKeepClass;  // lines 5-5
- template<typename Foo> class NoKeepTplClass;    // lines 10-10

The full include-list for hash_namespace:
namespace ns1 { struct KeepStruct; }  // lines 4-4
namespace ns1 { template<typename Foo> class KeepTplClass; }  // lines 6-6
---
"""
    self.RegisterFileContents({'hash_namespace': infile})
    self.ProcessAndTest(iwyu_output)

  def testElaboratedClasses(self):
    """Tests we don't remove lines like 'class Foo* fooptr'."""
    infile = """\
// Copyright 2010

struct KeepStruct;
class NoKeepClass;  ///-
///+
struct NoKeepStruct* s;
struct NoKeepStruct& t;

int main() { return 0; }
"""
    iwyu_output = """\
elaborated_class should add these lines:

elaborated_class should remove these lines:
- class NoKeepClass;  // lines 4-4

The full include-list for elaborated_class:
struct KeepStruct; // lines 3-3
---
"""
    self.RegisterFileContents({'elaborated_class': infile})
    self.ProcessAndTest(iwyu_output)

  def testBFlag(self):
    """Tests that --b properly separates sections."""
    self.flags.blank_lines = True
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include "subdir/bflag.h"
///+
///+#include <stdio.h>
///+
#include "used.h"
///+#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
subdir/bflag.cc should add these lines:
#include "subdir/bflag.h"
#include <stdio.h>
#include "used2.h"

subdir/bflag.cc should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for subdir/bflag.cc:
#include "subdir/bflag.h"
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'subdir/bflag.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testSafeHeadersFlag(self):
    """Tests that --safe_headers causes us to not delete lines."""
    self.flags.safe_headers = True
    infile = """\
// Copyright 2010

#include <notused.h>
#include <notused2.h>  // Hello!
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

class Foo;
template<typename T>
class Bar;

int main() { return 0; }
"""
    iwyu_output = """\
safe_flag.h should add these lines:
#include <stdio.h>
#include "used2.h"

safe_flag.h should remove these lines:
- #include <notused.h>  // lines 3-3
- #include <notused.h>  // lines 4-4
- class Foo             // lines 7-7
- class Bar             // lines 8-9

The full include-list for safe_flag.h:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'safe_flag.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testSafeHeadersFlagTwice(self):
    """Tests running --safe_headers 2ce in a row doesn't duplicate comments."""
    self.flags.safe_headers = True
    infile = """\
// Copyright 2010

#include <notused.h>  // iwyu says this can be removed
#include <notused2.h>  // Hello!; iwyu says this can be removed
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

class Foo;  // iwyu says this can be removed
template<typename T>  // iwyu says this can be removed
class Bar;  // iwyu says this can be removed

int main() { return 0; }
"""
    iwyu_output = """\
safe_flag_twice.h should add these lines:
#include <stdio.h>
#include "used2.h"

safe_flag_twice.h should remove these lines:
- #include <notused.h>  // lines 3-3
- #include <notused.h>  // lines 4-4
- class Foo             // lines 7-7
- class Bar             // lines 8-9

The full include-list for safe_flag_twice.h:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'safe_flag_twice.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testSafeHeadersFlagOnCcFiles(self):
    """Tests that we delete even in --safe_headers mode, on .cc files."""
    self.flags.safe_headers = True
    infile = """\
// Copyright 2010

#include <notused.h>              ///-
#include <notused2.h>  // Hello!  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

class Foo;            ///-
template<typename T>  ///-
class Bar;            ///-
                      ///-
int main() { return 0; }
"""
    iwyu_output = """\
safe_flag.cc should add these lines:
#include <stdio.h>
#include "used2.h"

safe_flag.cc should remove these lines:
- #include <notused.h>  // lines 3-3
- #include <notused.h>  // lines 4-4
- class Foo             // lines 7-7
- class Bar             // lines 8-9

The full include-list for safe_flag.cc:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'safe_flag.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testIncludeComments(self):
    """Tests that we properly include comments on #include lines."""
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include "subdir/include_comments.h"
///+#include <stdio.h>   // for printf(), etc.
#include "used.h"    ///-
///+#include "used.h"    // for Used
///+#include "used2.h"   // for Used2, Used2::Used2, Used2::~Used2, Used2::Used_Enum, operator==()

int main() { return 0; }
"""
    iwyu_output = """\
subdir/include_comments.cc should add these lines:
#include "subdir/include_comments.h"
#include <stdio.h>   // for printf(), etc.
#include "used2.h"   // for Used2, Used2::Used2, Used2::~Used2, Used2::Used_Enum, operator==()

subdir/include_comments.cc should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for subdir/include_comments.cc:
#include "subdir/include_comments.h"
#include <stdio.h>   // for printf(), etc.
#include "used.h"    // for Used
#include "used2.h"   // for Used2, Used2::Used2, Used2::~Used2, Used2::Used_Enum, operator==()
---
"""
    self.RegisterFileContents({'subdir/include_comments.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testNocommentsFlag(self):
    """Tests we properly don't include/modify comments with --nocomments."""
    self.flags.comments = False
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include "subdir/include_comments.h"
///+#include <stdio.h>
#include "used.h"  // my favorite #include!
///+#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
subdir/include_comments.cc should add these lines:
#include "subdir/include_comments.h"
#include <stdio.h>   // for printf(), etc.
#include "used2.h"   // for Used2, Used2::Used2, Used2::~Used2, Used2::Used_Enum, operator==()

subdir/include_comments.cc should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for subdir/include_comments.cc:
#include "subdir/include_comments.h"
#include <stdio.h>   // for printf(), etc.
#include "used.h"    // for Used
#include "used2.h"   // for Used2, Used2::Used2, Used2::~Used2, Used2::Used_Enum, operator==()
---
"""
    self.RegisterFileContents({'subdir/include_comments.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testFixingTwoFiles(self):
    """Make sure data for one fix doesn't overlap with a second."""
    file_a = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"
#include "used_only_in_file_a.h"
#include "used_only_in_file_b.h"  ///-

class FileAClass;   // kept for file A, not for file B
class FileBClass;   // kept for file B, not for file A   ///-
"""
    file_b = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"
#include "used_only_in_file_a.h"  ///-
#include "used_only_in_file_b.h"

class FileAClass;   // kept for file A, not for file B   ///-
class FileBClass;   // kept for file B, not for file A
"""
    iwyu_output = """\
file_a.cc should add these lines:
#include <stdio.h>
#include "used2.h"

file_a.cc should remove these lines:
- #include <notused.h>  // lines 3-3
- #include "used_only_in_file_b.h"  // lines 6-6
- class FileBClass;  // lines 9-9

The full include-list for file_a.cc:
#include <stdio.h>
#include "used.h"
#include "used2.h"
#include "used_only_in_file_a.h"
class FileAClass;  // lines 8-8
---

file_b.cc should add these lines:
#include <stdio.h>
#include "used2.h"

file_b.cc should remove these lines:
- #include <notused.h>  // lines 3-3
- #include "used_only_in_file_a.h"  // lines 5-5
- class FileAClass;  // lines 8-8

The full include-list for file_b.cc:
#include <stdio.h>
#include "used.h"
#include "used2.h"
#include "used_only_in_file_b.h"
class FileBClass;  // lines 9-9
---
"""
    self.RegisterFileContents({'file_a.cc': file_a, 'file_b.cc': file_b})
    self.ProcessAndTest(iwyu_output)

  def testListingTheSameFileTwice(self):
    """Test when foo.cc is specified twice.  It should fix conservatively."""
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
///+#include "include_this_file"  // for reason 2
#include "used.h"
///+#include "used2.h"
#include "used_only_in_file_a.h"
#include "used_only_in_file_b.h"  ///-

class FileAClass;   // kept for file A, not for file B
class FileBClass;   // kept for file B, not for file A   ///-
///+namespace foo {
///+template <typename Arg1> ClassTemplate;
///+}  // namespace foo
///+template <typename Arg1> ClassTemplate;
"""
    iwyu_output = """\
twice.cc should add these lines:
#include <stdio.h>
#include "include_this_file"  // for reason 1
namespace foo { template <typename Arg1> ClassTemplate; }
template <typename Arg1> ClassTemplate;

twice.cc should remove these lines:
- #include <notused.h>  // lines 3-3
- #include "used_only_in_file_a.h"  // lines 5-5
- #include "used_only_in_file_b.h"  // lines 6-6
- class FileAClass;  // lines 8-8
- class FileBClass;  // lines 9-9

The full include-list for twice.cc:
#include <stdio.h>
#include "include_this_file"  // for reason 1
#include "used.h"
namespace foo { template <typename Arg1> ClassTemplate; }
template <typename Arg1> ClassTemplate;
---

twice.cc should add these lines:
#include "used2.h"
#include "include_this_file"  // for reason 2
namespace foo { template <typename Arg2> ClassTemplate; }
template <typename Arg2> ClassTemplate;

twice.cc should remove these lines:
- #include <notused.h>  // lines 3-3
- #include "used_only_in_file_b.h"  // lines 6-6
- class FileBClass;  // lines 9-9

The full include-list for twice.cc:
#include "include_this_file"  // for reason 2
#include "used.h"
#include "used2.h"
#include "used_only_in_file_a.h"
class FileAClass;  // lines 8-8
namespace foo { template <typename Arg2> ClassTemplate; }
template <typename Arg2> ClassTemplate;
---
"""
    self.RegisterFileContents({'twice.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testListingTheSameFileTwiceAndOnceIsANoop(self):
    """Test when foo.cc is specified twice, once with 'all correct'."""
    infile = """\
// Copyright 2010

#include <notused.h>
///+#include <stdio.h>
///+#include "include_this_file"  // for reason 1
#include "used.h"
#include "used_only_in_file_a.h"

class FileAClass;
///+namespace foo {
///+template <typename Arg1> ClassTemplate;
///+}  // namespace foo
///+template <typename Arg1> ClassTemplate;
"""
    iwyu_output = """\
twice.cc should add these lines:
#include <stdio.h>
#include "include_this_file"  // for reason 1
namespace foo { template <typename Arg1> ClassTemplate; }
template <typename Arg1> ClassTemplate;

twice.cc should remove these lines:
- #include <notused.h>  // lines 3-3
- #include "used_only_in_file_a.h"  // lines 5-5
- class FileAClass;  // lines 7-7

The full include-list for twice.cc:
#include <stdio.h>
#include "include_this_file"  // for reason 1
#include "used.h"
namespace foo { template <typename Arg1> ClassTemplate; }
template <typename Arg1> ClassTemplate;
---

(twice.cc has correct #includes/fwd-decls)
"""
    self.RegisterFileContents({'twice.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddForwardDeclare(self):
    """Test adding a forward-declare, rather than keeping one."""
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"
///+
///+struct NotUsed;

int main() { return 0; }
"""
    iwyu_output = """\
new_fwd_decl should add these lines:
#include <stdio.h>
#include "used2.h"
struct NotUsed;

new_fwd_decl should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for new_fwd_decl:
#include <stdio.h>
#include "used.h"
#include "used2.h"
struct NotUsed;
---
"""
    self.RegisterFileContents({'new_fwd_decl': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddAndKeepForwardDeclare(self):
    """Test adding a forward-declare in addition to keeping one."""
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

class ForwardDeclClass;
///+struct NotUsed;

int main() { return 0; }
"""
    iwyu_output = """\
new_and_keep_fwd_decl should add these lines:
#include <stdio.h>
#include "used2.h"
struct NotUsed;

new_and_keep_fwd_decl should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for new_and_keep_fwd_decl:
#include <stdio.h>
#include "used.h"
#include "used2.h"
class ForwardDeclareClass;  // lines 6-6
struct NotUsed;
---
"""
    self.RegisterFileContents({'new_and_keep_fwd_decl': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeToFileThatHasOnlyForwardDeclarations(self):
    """Tests we add an #include in an appropriate place if none exist."""
    infile = """\
// Copyright 2010

///+#include <stdio.h>
///+#include "used.h"
///+
const int kFoo = 5;  // we should insert before the contentful line.

class Foo;

int main() { return 0; }
"""
    iwyu_output = """\
no_include_fwd_decl should add these lines:
#include <stdio.h>
#include "used.h"

no_include_fwd_decl should remove these lines:

The full include-list for no_include_fwd_decl:
#include <stdio.h>
#include "used.h"
class Foo;  // lines 5-5
---
"""
    self.RegisterFileContents({'no_include_fwd_decl': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddForwardDeclarationToFileThatHasOnlyIncludes(self):
    """Tests we add a forward-declare in an appropriate place if none exist."""
    infile = """\
// Copyright 2010

const int kFoo = 5;  // make sure we don't just insert at the beginning

#include <stdio.h>
#include "used.h"
///+
///+class Foo;

int main() { return 0; }
"""
    iwyu_output = """\
no_fwd_decl_include should add these lines:
class Foo;

no_fwd_decl_include should remove these lines:

The full include-list for no_fwd_decl_include:
#include <stdio.h>
#include "used.h"
class Foo;
---
"""
    self.RegisterFileContents({'no_fwd_decl_include': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeToContentlessFile(self):
    """Tests we add an #include ok to a basically empty file.."""
    infile = """\
// Copyright 2010
///+#include <stdio.h>
///+#include "used.h"
///+
///+class Foo;
"""
    iwyu_output = """\
no_include should add these lines:
#include <stdio.h>
#include "used.h"
class Foo;

no_include should remove these lines:

The full include-list for no_include:
#include <stdio.h>
#include "used.h"
class Foo;
---
"""
    self.RegisterFileContents({'no_include': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeToEmptyFile(self):
    """Tests we add an #include ok to an empty file.."""
    infile = ''
    iwyu_output = """\
empty_file should add these lines:
#include <stdio.h>
#include "used.h"
class Foo;

empty_file should remove these lines:

The full include-list for empty_file:
#include <stdio.h>
#include "used.h"
class Foo;
---
"""
    self.RegisterFileContents({'empty_file': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeToOnlyOneContentfulLineFile(self):
    """Prevent regression when the only contentful line was the last."""
    infile = """\
// Copyright 2010
///+
///+#include <stdio.h>

int main() { return 0; }
"""
    iwyu_output = """\
only_one_contentful_line.c should add these lines:
#include <stdio.h>

only_one_contentful_line.c should remove these lines:

The full include-list for only_one_contentful_line.c:
#include <stdio.h>
---
"""
    self.RegisterFileContents({'only_one_contentful_line.c': infile})
    self.ProcessAndTest(iwyu_output)

  def testCommentsAtEndOfFile(self):
    """Tests we don't crash if a file ends with #includs and then a comment."""
    infile = """\
// Copyright 2010

const int kFoo = 5;  // make sure we don't just insert at the beginning

#include <stdio.h>
#include "used.h"
///+
///+class Foo;
// Comments, and then...nothing
"""
    iwyu_output = """\
comments_at_end_of_file should add these lines:
class Foo;

comments_at_end_of_file should remove these lines:

The full include-list for comments_at_end_of_file:
#include <stdio.h>
#include "used.h"
class Foo;
---
"""
    self.RegisterFileContents({'comments_at_end_of_file': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddSystemIncludeToFileWithoutAny(self):
    """Tests we add a system #include to a non-sys location when needed."""
    infile = """\
// Copyright 2010

#ifdef HAVE_TYPE_TRAITS_H
#include <type_traits.h>
#endif
///+#include <stdio.h>
#include "used.h"

int main() { return 0; }
"""
    iwyu_output = """\
system_include should add these lines:
#include <stdio.h>

system_include should remove these lines:

The full include-list for system_include:
#include <stdio.h>
#include "used.h"
---
"""
    self.RegisterFileContents({'system_include': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddNonSystemHeaderUnderMainCUHeader(self):
    """Tests we distinguish main-cu headers from other non-system headers."""
    infile = """\
// Copyright 2010

///+#include "main_cu.h"
#include "main_cu-inl.h"
                        ///-
#include <stdio.h>
///+#include <stdlib.h>
#ifdef WINDOWS
#include <windows.h>
#endif

#include "used.h"
///+#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
main_cu_test.cc should add these lines:
#include <stdlib.h>
#include "main_cu.h"
#include "used2.h"

main_cu_test.cc should remove these lines:

The full include-list for main_cu_test.cc:
#include "main_cu.h"
#include "main_cu-inl.h"
#include <stdio.h>
#include <stdlib.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'main_cu_test.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddWithNearestIncludes(self):
    """Tests we add "includes" with <includes> when there's a choice."""
    infile = """\
// Copyright 2010

#include "nearest_include.h"

static int x = 6;
#include <stdio.h>
///+#include "used.h"

static int y = 7;
class Foo;

int main() { return 0; }
"""
    iwyu_output = """\
nearest_include.cc should add these lines:
#include "used.h"

nearest_include.cc should remove these lines:

The full include-list for nearest_include.cc:
#include "nearest_include.h"
#include <stdio.h>
#include "used.h"
class Foo;  // lines 9-9
---
"""
    self.RegisterFileContents({'nearest_include.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testFalseAlarmHeaderGuard(self):
    """Tests we calculate top-level-ness even in face of a fake header-guard."""
    infile = """\
// Copyright 2010

#include "nearest_toplevel_include.h"

static int x = 6;
#include <stdio.h>
///+#include "used.h"

#ifndef MAP_ANONYMOUS  // This is the fake header guard!
# define MAP_ANONYMOUS MAP_ANON
#endif

#ifdef FOO
#include <foo.h>
#endif
#if defined(BAR)
#include <bar.h>
#endif

static int y = 7;
class Foo;

int main() { return 0; }
"""
    iwyu_output = """\
nearest_toplevel_include.cc should add these lines:
#include "used.h"

nearest_toplevel_include.cc should remove these lines:

The full include-list for nearest_toplevel_include.cc:
#include "nearest_toplevel_include.h"
#include <bar.h>
#include <foo.h>
#include <stdio.h>
#include "used.h"
class Foo;  // lines 9-9
---
"""
    self.RegisterFileContents({'nearest_toplevel_include.cc': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeAfterHeaderGuard(self):
    """Test that we are willing to insert .h's inside a header guard."""
    infile = """\
// Copyright 2010

#ifndef SIMPLE_H_
#define SIMPLE_H_

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

#endif
"""
    iwyu_output = """\
simple.h should add these lines:
#include <stdio.h>
#include "used2.h"

simple.h should remove these lines:
- #include <notused.h>  // lines 6-6

The full include-list for simple.h:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'simple.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeAfterSoloPragmaOnce(self):
    """Test that we are willing to insert .h's after #pragma once."""
    infile = """\
// Copyright 2010

#pragma once

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

"""
    iwyu_output = """\
pragma_once.h should add these lines:
#include <stdio.h>
#include "used2.h"

pragma_once.h should remove these lines:
- #include <notused.h>  // lines 5-5

The full include-list for pragma_once.h:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'pragma_once.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeAfterPragmaOnceWithHeaderGuard(self):
    """Test that we are willing to insert .h's after #pragma once and header
    guard."""
    infile = """\
// Copyright 2010

#pragma once
#ifndef PRAGMA_ONCE_H_
#define PRAGMA_ONCE_H_

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

#endif
"""
    iwyu_output = """\
pragma_once_with_guard.h should add these lines:
#include <stdio.h>
#include "used2.h"

pragma_once_with_guard.h should remove these lines:
- #include <notused.h>  // lines 7-7

The full include-list for pragma_once_with_guard.h:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'pragma_once_with_guard.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeAfterEarlyPragmaOnce(self):
    """Test that we are willing to insert .h's after early #pragma once."""
    infile = """\
#pragma once
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

"""
    iwyu_output = """\
early_pragma_once.h should add these lines:
#include <stdio.h>
#include "used2.h"

early_pragma_once.h should remove these lines:
- #include <notused.h>  // lines 4-4

The full include-list for early_pragma_once.h:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'early_pragma_once.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeAfterEarlyPragmaOnceWithHeaderGuard(self):
    """Test that we are willing to insert .h's after early #pragma once and
    header guard."""
    infile = """\
#pragma once
// Copyright 2010

#ifndef PRAGMA_ONCE_H_
#define PRAGMA_ONCE_H_

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

#endif
"""
    iwyu_output = """\
early_pragma_once_with_guard.h should add these lines:
#include <stdio.h>
#include "used2.h"

early_pragma_once_with_guard.h should remove these lines:
- #include <notused.h>  // lines 7-7

The full include-list for early_pragma_once_with_guard.h:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'early_pragma_once_with_guard.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeAfterWeirdPragmaOnce(self):
    """Test that we are willing to insert .h's after creatively formatted
    #pragma once."""
    infile = """\
  # pragma    once

#include <notused.h>  ///-
///+#include <stdio.h>
"""
    iwyu_output = """\
weird_pragma_once.h should add these lines:
#include <stdio.h>

weird_pragma_once.h should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for weird_pragma_once.h:
#include <stdio.h>
---
"""
    self.RegisterFileContents({'weird_pragma_once.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeBeforePragmaMessage(self):
    """Test that non-once #pragmas are pushed after the #includes."""
    infile = """\
///+#include <stdio.h>
///+
#pragma message "Hello world!"

#include <notused.h>  ///-
"""
    iwyu_output = """\
weird_pragma_once.h should add these lines:
#include <stdio.h>

weird_pragma_once.h should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for weird_pragma_once.h:
#include <stdio.h>
---
"""
    self.RegisterFileContents({'weird_pragma_once.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeAfterWeirdHeaderGuard(self):
    """Test that we are willing to insert .h's inside a non-standard h-guard."""
    infile = """\
// Copyright 2010

#if  !    defined (SIMPLE_H_)
#define SIMPLE_H_

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

#endif
"""
    iwyu_output = """\
simple.h should add these lines:
#include <stdio.h>
#include "used2.h"

simple.h should remove these lines:
- #include <notused.h>  // lines 6-6

The full include-list for simple.h:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'simple.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeAfterHeaderGuardLikeIfdef(self):
    """Test that we are willing to insert .h's inside a h-guard-*like* line."""
    infile = """\
// Copyright 2010

#ifdef __linux   // serves the same role as a header guard

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

#endif

// Comments are allowed after the header guard.
"""
    iwyu_output = """\
os_header_guard.h should add these lines:
#include <stdio.h>
#include "used2.h"

os_header_guard.h should remove these lines:
- #include <notused.h>  // lines 5-5

The full include-list for os_header_guard.h:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'os_header_guard.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddIncludeAfterHeaderGuardButBeforeComments(self):
    """Test that we introduce new #includes right after a header guard."""
    infile = """\
// Copyright 2010

#ifndef SIMPLE_WITH_COMMENT_H_
#define SIMPLE_WITH_COMMENT_H_

///+#include <stdio.h>
///+#include "used.h"
///+
// This is a comment
void ForThisFunction();

#endif
"""
    iwyu_output = """\
simple_with_comment.h should add these lines:
#include <stdio.h>
#include "used.h"

simple_with_comment.h should remove these lines:

The full include-list for simple_with_comment.h:
#include <stdio.h>
#include "used.h"
---
"""
    self.RegisterFileContents({'simple_with_comment.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testIdentifyingHeaderGuardLines(self):
    """Test that not all #defines look like header guards."""
    infile = """\
// Copyright 2010

#ifndef IDENTIFYING_HEADER_GUARD_LINES_H_
#define IDENTIFYING_HEADER_GUARD_LINES_H_

namespace foo {
// The namespace decl should come before this #define, not after.
// It will, unless we wrongly say the #define is a header-guard define.
///+namespace bar {
///+class Baz;
///+}  // namespace bar
///+
#define NOT_A_HEADER_GUARD_LINE 1
}

#endif
"""
    iwyu_output = """\
identifying_header_guard_lines.h should add these lines:
namespace foo { namespace bar { class Baz; } }

identifying_header_guard_lines.h should remove these lines:

The full include-list for identifying_header_guard_lines.h:
namespace foo { namespace bar { class Baz; } }
---
"""
    self.RegisterFileContents({'identifying_header_guard_lines.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testIncludeOfCcFile(self):
    """Test that iwyu leaves .cc #includes alone."""
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

int kFoo = 5;

#include "not_mentioned.cc"

int main() { return 0; }
"""
    iwyu_output = """\
cc_include should add these lines:
#include <stdio.h>
#include "used2.h"

cc_include should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for cc_include:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'cc_include': infile})
    self.ProcessAndTest(iwyu_output)

  def testCommentsBeforeIncludeLines(self):
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+// This is the first include.
///+// Or it will be, after we reorder.
///+#include <stdio.h>
// This is the second include.
// Or *it* will be, after we reorder.
#include "used.h"
// This is the first include.       ///-
// Or it will be, after we reorder. ///-
#include <stdio.h>                  ///-

int main() { return 0; }
"""
    iwyu_output = """\
comments_with_includes should add these lines:

comments_with_includes should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for comments_with_includes:
#include <stdio.h>
#include "used.h"
---
"""
    self.RegisterFileContents({'comments_with_includes': infile})
    self.ProcessAndTest(iwyu_output)

  def testRemoveDuplicates(self):
    """Tests we uniquify if an #include is in there twice."""
    infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
#include "used2.h"
#include "used.h"  // same line even though it has a comment ///-
// Even though these two comment-lines are the same, they won't get de-duped.
// Even though these two comment-lines are the same, they won't get de-duped.
#ifdef _WINDOWS
// But keep this one because it's in an #ifdef.
#include "used.h"
#endif

int main() { return 0; }
"""
    iwyu_output = """\
remove_duplicates should add these lines:
#include <stdio.h>

remove_duplicates should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for remove_duplicates:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'remove_duplicates': infile})
    self.ProcessAndTest(iwyu_output)

  def testNestedNamespaces(self):
    infile = """\
// Copyright 2010

///+#include <stdio.h>
///+
namespace X {
class OneA
///+
namespace Y {
///+class TwoA;
class TwoB;
class TwoA;    ///-
}}
class Toplevel;
                               ///-
namespace A {                  ///-
namespace B { namespace C {    ///-
class Delete1;                 ///-
}}}                            ///-
                                                ///-
namespace A { namespace B { class Delete2; } }  ///-
                         ///-
namespace A {            ///-
   namespace B {         ///-
     class Delete3;      ///-
   }                     ///-
}  // namespace A        ///-

int main() { return 0; }
"""
    iwyu_output = """\
many_namespaces should add these lines:
#include <stdio.h>

many_namespaces should remove these lines:
- class Delete1;  // lines 13-13
- class Delete2;  // lines 16-16
- class Delete3;  // lines 20-20

The full include-list for many_namespaces:
#include <stdio.h>
class Toplevel;  // lines 9-9
class TwoA;  // lines 7-7
class TwoB;  // lines 6-6
class OneA;  // lines 4-4
---
"""
    self.RegisterFileContents({'many_namespaces': infile})
    self.ProcessAndTest(iwyu_output)

  def testDoNotInsertIncludeIntoAClass(self):
    infile = """\
// Copyright 2010

///+#include <stdio.h>
///+
class Foo {
};

class Bar {
  class FwdDecl;

  FwdDecl* f;
}
"""
    iwyu_output = """\
include_not_in_class should add these lines:
#include <stdio.h>

include_not_in_class should remove these lines:

The full include-list for include_not_in_class:
#include <stdio.h>
class FwdDecl;  // lines 7-7
---
"""
    self.RegisterFileContents({'include_not_in_class': infile})
    self.ProcessAndTest(iwyu_output)

  def testIdenticalForwardDeclaredNamesInDifferentNamespaces(self):
    infile = """\
// Copyright 2010

///+namespace ns1 {
///+class ForwardDeclared;
///+}  // namespace ns1
///+namespace ns2 {
///+class ForwardDeclared;
///+}  // namespace ns2
"""
    iwyu_output = """\
identical_names should add these lines:
namespace ns1 { class ForwardDeclared; }
namespace ns2 { class ForwardDeclared; }

identical_names should remove these lines:

The full include-list for identical_names:
namespace ns1 { class ForwardDeclared; }
namespace ns2 { class ForwardDeclared; }
---
"""
    self.RegisterFileContents({'identical_names': infile})
    self.ProcessAndTest(iwyu_output)

  def testIterativeNamespaceDelete(self):
    """Tests deleting a namespace with an emptied #ifdef inside it."""
    infile = """\
// Copyright 2010
                 ///-
namespace foo {  ///-
#ifdef FWD_DECL  ///-
class Bar;       ///-
#endif           ///-
}                ///-

int main() { return 0; }
"""
    iwyu_output = """\
iterative_namespace should add these lines:

iterative_namespace should remove these lines:
- class Bar;  // lines 5-5

The full include-list for iterative_namespace:
---
"""
    self.RegisterFileContents({'iterative_namespace': infile})
    self.ProcessAndTest(iwyu_output)

  def testIterativeIfdefDelete(self):
    """Tests deleting an ifdef with an emptied namespace inside it."""
    infile = """\
// Copyright 2010
                 ///-
#ifdef FWD_DECL  ///-
namespace foo {  ///-
class Bar;       ///-
}                ///-
#endif           ///-

int main() { return 0; }
"""
    iwyu_output = """\
iterative_ifdef should add these lines:

iterative_ifdef should remove these lines:
- class Bar;  // lines 5-5

The full include-list for iterative_ifdef:
---
"""
    self.RegisterFileContents({'iterative_ifdef': infile})
    self.ProcessAndTest(iwyu_output)

  def testOutOfRangeLineNumber(self):
    """Test we skip editing completely if iwyu has a really big line number."""
    # fix_includes skips the file-editing if it detects a problem, as
    # in this test case.  The way that skipping is evidenced in the
    # test, is the output is empty.
    infile = """\
// Copyright 2010         ///-
                          ///-
#include <notused.h>      ///-
#include "used.h"         ///-
                          ///-
int main() { return 0; }  ///-
"""
    iwyu_output = """\
out_of_range should add these lines:
#include <stdio.h>
#include "used2.h"

out_of_range should remove these lines:
- #include <notused.h>  // lines 3-3
- #include <bignumber.h>  // lines 3000-3000

The full include-list for out_of_range:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'out_of_range': infile})
    self.ProcessAndTest(iwyu_output)

  def testDeleteExtraneousBlankLines(self):
    """Test we delete blank lines around deleted spans correctly."""
    infile = """\
// Copyright 2010

class Foo { };
            ///-
class Bar;  ///-


class Baz { };


class Bang;   ///-
              ///-
class Qux { };

int main() { return 0; }
"""
    iwyu_output = """\
extraneous_blank_lines should add these lines:

extraneous_blank_lines should remove these lines:
- class Bar;  // lines 5-5
- class Bang;  // lines 11-11

The full include-list for extraneous_blank_lines:
---
"""
    self.RegisterFileContents({'extraneous_blank_lines': infile})
    self.ProcessAndTest(iwyu_output)

  def testKeepNolintComment(self):
    """Test we keep a nolint comment."""
    infile = """\
// Copyright 2010

#include "bar.h"  // NOLINT(iwyu)
#include "baz.h"  // NOLINT(iwyu): blah blah

int main() { return 0; }
"""
    iwyu_output = """\
keep_nolint should add these lines:

keep_nolint should remove these lines:

The full include-list for keep_nolint:
#include "bar.h"  // lines 3-3
#include "baz.h"  // lines 4-4
---
"""

    self.RegisterFileContents({'keep_nolint': infile})
    # No files are written, because there are no changes.
    self.ProcessAndTest(iwyu_output, unedited_files=['keep_nolint'])

  def testKeepNolintCommentInNocommentMode(self):
    """Test we keep a nolint comment even with --nocomments."""
    self.flags.comments = False
    self.testKeepNolintComment()

  # Test the IWYUOutputParser method _MatchSectionHeading.

  def testIWYUOutputParserMatchSectionHeadingSuccess(self):
    parser = fix_includes.IWYUOutputParser()
    self.assertEqual(None, parser.current_section)
    self.assertEqual('<unknown file>', parser.filename)

    self.assert_(parser._ProcessOneLine(''))
    self.assertEqual(None, parser.current_section)
    self.assertEqual('<unknown file>', parser.filename)

    self.assert_(parser._ProcessOneLine(
        'myfile.cc should add these lines:'))
    self.assertEqual(parser._ADD_SECTION_RE, parser.current_section)
    self.assertEqual('add', parser._RE_TO_NAME[parser.current_section])
    self.assertEqual('myfile.cc', parser.filename)

    self.assert_(parser._ProcessOneLine(
        'myfile.cc should remove these lines:'))
    self.assertEqual(parser._REMOVE_SECTION_RE, parser.current_section)
    self.assertEqual('remove', parser._RE_TO_NAME[parser.current_section])
    self.assertEqual('myfile.cc', parser.filename)

    self.assert_(parser._ProcessOneLine(
        'The full include-list for myfile.cc:'))
    self.assertEqual(parser._TOTAL_SECTION_RE, parser.current_section)
    self.assertEqual('total', parser._RE_TO_NAME[parser.current_section])
    self.assertEqual('myfile.cc', parser.filename)

    self.assert_(not parser._ProcessOneLine('---'))
    self.assertEqual(parser._SECTION_END_RE, parser.current_section)
    self.assertEqual('end', parser._RE_TO_NAME[parser.current_section])
    self.assertEqual('myfile.cc', parser.filename)

  def testIWYUOutputParserProcessOneLineProcessNoEditsHeader(self):
    parser = fix_includes.IWYUOutputParser()
    line = '(myfile.cc has correct #includes/fwd-decls)'
    self.assert_(not parser._ProcessOneLine(line))
    self.assertEqual(parser._NO_EDITS_RE, parser.current_section)
    self.assertEqual('no_edits', parser._RE_TO_NAME[parser.current_section])
    self.assertEqual('myfile.cc', parser.filename)

  def testIWYUOutputParserProcessOneLineAddNotSeenFirst(self):
    parser = fix_includes.IWYUOutputParser()
    self.assertRaises(fix_includes.FixIncludesError,
                      parser._ProcessOneLine,
                      'myfile.cc should remove these lines:')

  def testIWYUOutputParserProcessOneLineOutOfOrder(self):
    parser = fix_includes.IWYUOutputParser()
    self.assert_(parser._ProcessOneLine(
        'myfile.cc should add these lines:'))
    self.assertRaises(fix_includes.FixIncludesError,
                      parser._ProcessOneLine,
                      'The full include-list for myfile.cc:')

  def testIWYUOutputParserProcessOneLineIncorrectFilename(self):
    parser = fix_includes.IWYUOutputParser()
    self.assert_(parser._ProcessOneLine(
        'myfile.cc should add these lines:'))
    self.assertRaises(fix_includes.FixIncludesError,
                      parser._ProcessOneLine,
                      'not_myfile.cc should remove these lines:')

  def testIWYUOutputParserProcessOneLineNoMatcher(self):
    parser = fix_includes.IWYUOutputParser()
    # We successfully process this not-in-any-section line, but update no data.
    self.assert_(parser._ProcessOneLine('#include <foo>'))
    self.assertEqual(None, parser.current_section)
    self.assertEqual('<unknown file>', parser.filename)

  def testIWYUOutputParserSuccess(self):
    """Tests the IWYUOutputParser method ParseOneRecord."""
    iwyu_output = """\
simple should add these lines:
#include <stdio.h>
#include "used2.h"
namespace ns {class ForwardDeclared;}

simple should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for simple:
#include <stdio.h>
#include "used.h"
#include "used2.h"
namespace ns {class ForwardDeclared;}
---
"""
    parser = fix_includes.IWYUOutputParser()
    record = parser.ParseOneRecord(iwyu_output.splitlines(), self.flags)

    self.assertEqual('simple', record.filename)
    self.assertEqual(set([3]), record.lines_to_delete)
    self.assertEqual(set(('#include <stdio.h>',
                          '#include "used2.h"',
                          'namespace ns {class ForwardDeclared;}')),
                     record.includes_and_forward_declares_to_add)

  def testIWYUOutputParserRemoveLineNoComment(self):
    iwyu_output = """\
no_comment should add these lines:

no_comment should remove these lines:
- #include <notused.h>

The full include-list for no_key:
---
"""
    parser = fix_includes.IWYUOutputParser()
    self.assertRaises(fix_includes.FixIncludesError,
                      parser.ParseOneRecord,
                      iwyu_output.splitlines(),
                      self.flags)

  def testNotWriteable(self):
    """Test that files don't get rewritten if they are not writeable."""
    infile = """\
// Copyright 2010

#include <notused.h>
#include "used.h"

int main() { return 0; }
"""
    iwyu_output = """\
unwritable should add these lines:
#include <stdio.h>
#include "used2.h"

unwritable should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for unwritable:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'unwritable': infile})
    self.MakeFilesUnwriteable()
    # No files are written, because they are not writeable.
    self.ProcessAndTest(iwyu_output, unedited_files=['unwritable'])


  def testFileSpecifiedOnCommandline(self):
    """Test we limit editing to files specified on the commandline."""
    changed_infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

int main() { return 0; }
"""
    unchanged_infile = """\
// Copyright 2010

#include <notused.h>
#include "used.h"

int main() { return 0; }
"""
    iwyu_output = """\
changed should add these lines:
#include <stdio.h>
#include "used2.h"

changed should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for changed:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    # Have the exact same iwyu output for 'unchanged' as for 'changed'.
    iwyu_output += iwyu_output.replace('changed', 'unchanged')

    self.RegisterFileContents({'changed': changed_infile,
                               'unchanged': unchanged_infile})
    # unchanged should not be edited, since it is not listed on the 'cmdline'.
    self.ProcessAndTest(iwyu_output, cmdline_files=['changed'],
                        unedited_files=['unchanged'])

  def testIgnoreRe(self):
    """Test the behavior of the --ignore_re flag."""
    changed_infile = """\
// Copyright 2010

#include <notused.h>  ///-
///+#include <stdio.h>
#include "used.h"
///+#include "used2.h"

int main() { return 0; }
"""
    unchanged_infile = """\
// Copyright 2010

#include <notused.h>
#include "used.h"

int main() { return 0; }
"""
    iwyu_output = """\
changed should add these lines:
#include <stdio.h>
#include "used2.h"

changed should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for changed:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    # Have the exact same iwyu output for 'unchanged' as for 'changed'.
    iwyu_output += iwyu_output.replace('changed', 'unchanged')

    self.RegisterFileContents({'changed': changed_infile,
                               'unchanged': unchanged_infile})
    # unchanged should not be edited, since it matches ignore_re.
    self.flags.ignore_re = 'nch'
    self.ProcessAndTest(iwyu_output, unedited_files=['unchanged'])

  def testSortIncludes(self):
    """Test sorting includes only -- like running fix_includes.py -s."""
    infile = """\
// Copyright 2010

#include <stdio.h>
// This file is not used.
#include <notused.h>

// This file is not used either.
// It's not used.
// Not used at all.
#include <notused2.h>

#include "notused3.h"

// This comment should stay, it's not before an #include.
const int kInt = 5;
// This file is used.
// It's definitedly used.
#include "used.h"
#include <stdlib.h>

const int kInt2 = 6;

#include "foo.cc"

// This comment should stay, it's not before an #include.
int main() { return 0; }
"""
    expected_output = """\
// Copyright 2010

// This file is not used.
#include <notused.h>
// This file is not used either.
// It's not used.
// Not used at all.
#include <notused2.h>
#include <stdio.h>
#include "notused3.h"

// This comment should stay, it's not before an #include.
const int kInt = 5;
#include <stdlib.h>
// This file is used.
// It's definitedly used.
#include "used.h"

const int kInt2 = 6;

#include "foo.cc"

// This comment should stay, it's not before an #include.
int main() { return 0; }
"""
    self.RegisterFileContents({'sort': infile})
    num_files_modified = fix_includes.SortIncludesInFiles(['sort'], self.flags)
    self.assertListEqual(expected_output.strip().split('\n'),
                         self.actual_after_contents)
    self.assertEqual(1, num_files_modified)

  def testSortingMultipleFiles(self):
    """Tests passing more than one argument to SortIncludesInFiles()."""
    infile1 = """\
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
"""
    infile2 = """\
#include "z.h"
#include "y.h"
#include "x.y"
"""

    expected_output = """\
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "x.y"
#include "y.h"
#include "z.h"
"""
    self.RegisterFileContents({'f1': infile1, 'f2': infile2})
    num_files_modified = fix_includes.SortIncludesInFiles(['f1', 'f2'],
                                                          self.flags)
    self.assertListEqual(expected_output.strip().split('\n'),
                         self.actual_after_contents)
    self.assertEqual(2, num_files_modified)


  def testSortingIncludesAlreadySorted(self):
    """Tests sorting includes only, when includes are already sorted."""
    infile = """\
// Copyright 2010

#include <ctype.h>
#include <stdio.h>

namespace Foo;   // fwd-decls are out of order, but sorter ignores them

namespace Bar;

int main() { return 0; }
"""
    self.RegisterFileContents({'sort_nosorting.h': infile})
    num_files_modified = fix_includes.SortIncludesInFiles(['sort_nosorting.h'],
                                                           self.flags)
    self.assertListEqual([], self.actual_after_contents)
    self.assertEqual(0, num_files_modified)

  def testBarrierIncludes(self):
    """Tests that we correctly sort 'around' _BARRIER_INCLUDES."""
    infile = """\
// Copyright 2010

#include <linux/a_stay_top.h>
#include <stdlib.h>   ///-
#include <linux/can_sort_around_this_deleted_include>  ///-
#include <stdio.h>
///+#include <stdlib.h>
#include "user/include.h"
///+#include "user/new_include.h"
#include <linux/c_stay_second.h>
#include <linux/b_stay_third.h>
#include <ctype.h>
#include <cpp_include>
///+#include <new_cpp_include>
#include <linux/d_stay_fourth.h>

int main() { return 0; }
"""
    iwyu_output = """\
barrier_includes.h should add these lines:
#include "user/new_include.h"
#include <new_cpp_include>

barrier_includes.h should remove these lines:
- #include <linux/can_sort_around_this_deleted_include>  // lines 5-5

The full include-list for barrier_includes.h:
#include "user/include.h"
#include "user/new_include.h"
#include <cpp_include>
#include <ctype.h>
#include <linux/a_stay_top.h>
#include <linux/b_stay_third.h>
#include <linux/c_stay_second.h>
#include <linux/d_stay_fourth.h>
#include <new_cpp_include>
#include <stdio.h>
#include <stdlib.h>
---
"""
    self.RegisterFileContents({'barrier_includes.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testSortingMainCUIncludeInSameDirectory(self):
    """Check that we identify when first .h file is a main-cu #include."""
    infile = """\
#include <stdio.h>
#include "me/subdir0/foo.h"
#include "other/baz.h"
"""
    expected_output = """\
#include "me/subdir0/foo.h"
#include <stdio.h>
#include "other/baz.h"
"""
    self.RegisterFileContents({'me/subdir0/foo.cc': infile})
    num_files_modified = fix_includes.SortIncludesInFiles(
        ['me/subdir0/foo.cc'], self.flags)
    self.assertListEqual(expected_output.strip().split('\n'),
                         self.actual_after_contents)
    self.assertEqual(1, num_files_modified)

  def testSortingMainCUIncludeInSameDirectoryWithInl(self):
    """Check that we identify when first -inl.h file is a main-cu #include."""
    infile = """\
#include <stdio.h>
#include "me/subdir0/foo-inl.h"
#include "other/baz.h"
"""
    expected_output = """\
#include "me/subdir0/foo-inl.h"
#include <stdio.h>
#include "other/baz.h"
"""
    self.RegisterFileContents({'me/subdir0/foo.cc': infile})
    num_files_modified = fix_includes.SortIncludesInFiles(
        ['me/subdir0/foo.cc'], self.flags)
    self.assertListEqual(expected_output.strip().split('\n'),
                         self.actual_after_contents)
    self.assertEqual(1, num_files_modified)

  def testSortingMainCUIncludeInDifferentDirectory(self):
    """Check that we identify when first .h file is a main-cu #include."""
    infile = """\
#include "me/subdir0/foo.h"
#include <stdio.h>
#include "other/baz.h"
"""
    self.RegisterFileContents({'me/other_subdir/foo.cc': infile})
    num_files_modified = fix_includes.SortIncludesInFiles(
        ['me/other_subdir/foo.cc'], self.flags)
    self.assertListEqual([], self.actual_after_contents)
    self.assertEqual(0, num_files_modified)

  def testSortingMainCUIncludeInDifferentDirectoryWhenNotFirst(self):
    """Check that we don't let second .h be a main-cu #include."""
    infile = """\
#include <stdio.h>
#include "me/subdir0/foo.h"
#include "other/baz.h"
"""
    self.RegisterFileContents({'me/other_subdir/foo.cc': infile})
    num_files_modified = fix_includes.SortIncludesInFiles(
        ['me/other_subdir/foo.cc'], self.flags)
    self.assertListEqual([], self.actual_after_contents)
    self.assertEqual(0, num_files_modified)

  def testSortingProjectIncludesAuto(self):
    """Check that project includes can be sorted separately."""
    infile = """\
#include "me/subdir0/foo.h"
#include <stdio.h>
#include "me/subdir2/bar.h"
#include "me/subdir1/bar.h"
#include "me/subdir0/bar.h"
#include "other/baz.h"
"""
    expected_output = """\
#include "me/subdir0/foo.h"
#include <stdio.h>
#include "other/baz.h"
#include "me/subdir0/bar.h"
#include "me/subdir1/bar.h"
#include "me/subdir2/bar.h"
"""
    self.RegisterFileContents({'me/subdir0/foo.cc': infile})
    self.flags.separate_project_includes = '<tld>'
    num_files_modified = fix_includes.SortIncludesInFiles(['me/subdir0/foo.cc'],
                                                          self.flags)
    self.assertListEqual(expected_output.strip().split('\n'),
                         self.actual_after_contents)
    self.assertEqual(1, num_files_modified)

  def testSortingProjectIncludesUserSpecified(self):
    """Test user-specified project directory name."""
    infile = """\
#include "me/subdir0/foo.h"
#include <stdio.h>
#include "me/subdir2/bar.h"
#include "me/subdir1/bar.h"
#include "me/subdir0/bar.h"
#include "other/baz.h"
"""
    expected_output = """\
#include "me/subdir0/foo.h"
#include <stdio.h>
#include "me/subdir1/bar.h"
#include "me/subdir2/bar.h"
#include "other/baz.h"
#include "me/subdir0/bar.h"
"""
    self.RegisterFileContents({'me/subdir0/foo.cc': infile})
    self.flags.separate_project_includes = 'me/subdir0'
    num_files_modified = fix_includes.SortIncludesInFiles(['me/subdir0/foo.cc'],
                                                          self.flags)
    self.assertListEqual(expected_output.strip().split('\n'),
                         self.actual_after_contents)
    self.assertEqual(1, num_files_modified)

  def testAddingNewIncludesAfterRemovingOldOnes(self):
    infile = """\
// Copyright 2008 Google Inc. All Rights Reserved.
// Author: zhifengc@google.com (Zhifeng Chen)

#ifndef STRUCTUREDSEARCH_COMMON_INTERNAL_DFS_H_
#define STRUCTUREDSEARCH_COMMON_INTERNAL_DFS_H_

#include "util/task/status.h"     ///-
#include "strings/stringpiece.h"  ///-
///+#include <string>                       // for string
///+#include "base/macros.h"                // for DISALLOW_COPY_AND_ASSIGN
///+#include "base/scoped_ptr.h"            // for scoped_ptr

class Query;
///+namespace util {
///+class Status;
///+}  // namespace util

namespace structuredsearch {

///+class FieldSpecification;
class FieldTokenizer;
class FieldSpecification;  ///-
class TokenizationSpec;

class QueryXlator { ... };

#endif  // #define STRUCTUREDSEARCH_COMMON_INTERNAL_DFS_H_
"""
    iwyu_output = """\
structuredsearch/common/internal/query_field_xlate.h should add these lines:
#include <string>                       // for string
#include "base/macros.h"                // for DISALLOW_COPY_AND_ASSIGN
#include "base/scoped_ptr.h"            // for scoped_ptr
namespace util { class Status; }

structuredsearch/common/internal/query_field_xlate.h should remove these lines:
- #include "strings/stringpiece.h"  // lines 8-8
- #include "util/task/status.h"  // lines 7-7

The full include-list for structuredsearch/common/internal/query_field_xlate.h:
#include <string>                       // for string
#include "base/macros.h"                // for DISALLOW_COPY_AND_ASSIGN
#include "base/scoped_ptr.h"            // for scoped_ptr
class Query;  // lines 10-10
namespace structuredsearch { class FieldSpecification; }  // lines 15-15
namespace structuredsearch { class FieldTokenizer; }  // lines 14-14
namespace structuredsearch { class TokenizationSpec; }  // lines 16-16
namespace util { class Status; }
---
"""
    self.RegisterFileContents(
        {'structuredsearch/common/internal/query_field_xlate.h': infile})
    self.ProcessAndTest(iwyu_output)

  def testDryRun(self):
    """Tests that --dry_run mode does not modify files."""
    self.flags.dry_run = True
    infile = """\
// Copyright 2010

#include <notused.h>
#include <stdio.h>
#include "used.h"
#include "used2.h"

int main() { return 0; }
"""
    iwyu_output = """\
dry_run should add these lines:
#include <stdio.h>
#include "used2.h"

dry_run should remove these lines:
- #include <notused.h>  // lines 3-3

The full include-list for dry_run:
#include <stdio.h>
#include "used.h"
#include "used2.h"
---
"""
    self.RegisterFileContents({'dry_run': infile})
    num_modified_files = fix_includes.ProcessIWYUOutput(
        cStringIO.StringIO(iwyu_output), ['dry_run'], self.flags)
    self.assertListEqual([], self.actual_after_contents)
    self.assertEqual(0, num_modified_files)

  def testAddForwardDeclareAndKeepIwyuNamespaceFormat(self):
    """Tests that --keep_iwyu_namespace_format writes namespace lines
    using the IWYU one-line format.
    Input code similar to case testAddForwardDeclareInNamespace."""
    self.flags.keep_iwyu_namespace_format = True
    infile = """\
// Copyright 2010

#include "foo.h"

///+namespace ns { class Foo; }
///+namespace ns { namespace ns2 { namespace ns3 { class Bar; } } }
///+namespace ns { namespace ns2 { namespace ns3 { template <typename T> class Bang; } } }
///+namespace ns { namespace ns4 { class Baz; } }
///+

int main() { return 0; }
"""
    iwyu_output = """\
add_fwd_declare_keep_iwyu_namespace should add these lines:
namespace ns { class Foo; }
namespace ns { namespace ns2 { namespace ns3 { class Bar; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class Bang; } } }
namespace ns { namespace ns4 { class Baz; } }

add_fwd_declare_keep_iwyu_namespace should remove these lines:

The full include-list for add_fwd_declare_keep_iwyu_namespace:
#include "foo.h"  // lines 3-3
namespace ns { class Foo; }
namespace ns { namespace ns2 { namespace ns3 { class Bar; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class Bang; } } }
namespace ns { namespace ns4 { class Baz; } }
---
"""
    self.RegisterFileContents({'add_fwd_declare_keep_iwyu_namespace': infile})
    self.ProcessAndTest(iwyu_output, expected_num_modified_files=1)

  def testAddNestedForwardDeclaresWithKeepIwyuNamespaceFormat(self):
    """Tests that --keep_iwyu_namespace_format writes namespace lines
    using the IWYU one-line format.
    Input code similar to case
    testAddForwardDeclareInsideNamespaceWithoutForwardDeclaresAlready."""
    self.flags.keep_iwyu_namespace_format = True
    infile = """\
// Copyright 2010

#include "foo.h"

class Bar;
///+class Foo;
///+namespace ns1 { class NsFoo; }
///+namespace ns1 { namespace ns2 { namespace ns3 { class NsBaz; } } }
///+namespace ns1 { namespace ns2 { namespace ns3 { template <typename T> class NsBang; } } }
template <typename T> class Baz;


namespace ns {

///+class NsFoo;
///+namespace ns2 { namespace ns3 { class NsBaz; } }
///+namespace ns2 { namespace ns3 { template <typename T> class NsBang; } }
///+
class NsBar;

  namespace  ns2   {   // we sure do love nesting our namespaces!

int MyFunction() { }

}
}

int main() { return 0; }
"""
    iwyu_output = """\
add_fwd_decl_with_keep_iwyu_format should add these lines:
class Foo;
namespace ns { class NsFoo; }
namespace ns { namespace ns2 { namespace ns3 { class NsBaz; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class NsBang; } } }
namespace ns1 { class NsFoo; }
namespace ns1 { namespace ns2 { namespace ns3 { class NsBaz; } } }
namespace ns1 { namespace ns2 { namespace ns3 { template <typename T> class NsBang; } } }

add_fwd_decl_with_keep_iwyu_format should remove these lines:

The full include-list for add_fwd_decl_with_keep_iwyu_format:
#include "foo.h"  // lines 3-3
class Bar;  // lines 5-5
class Foo;
namespace ns { class NsFoo; }
namespace ns { namespace ns2 { class NsBar; } }
namespace ns { namespace ns2 { namespace ns3 { class NsBaz; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class NsBang; } } }
namespace ns1 { class NsFoo; }
namespace ns1 { namespace ns2 { namespace ns3 { class NsBaz; } } }
namespace ns1 { namespace ns2 { namespace ns3 { template <typename T> class NsBang; } } }
template <typename T> class Baz;  // lines 6-6
---
"""
    self.RegisterFileContents({'add_fwd_decl_with_keep_iwyu_format': infile})
    self.ProcessAndTest(iwyu_output)

  def testAddForwardDeclareInNamespaceWithKeepIwyuNamespaceFormat(self):
    """Tests that --keep_iwyu_namespace_format writes namespace lines
    using the IWYU one-line format.
    Input code similar to case testAddForwardDeclareInNamespace."""
    self.flags.keep_iwyu_namespace_format = True
    infile = """\
// Copyright 2010

#include "foo.h"

///+namespace ns { class Foo; }
///+namespace ns { namespace ns2 { namespace ns3 { class Bar; } } }
///+namespace ns { namespace ns2 { namespace ns3 { template <typename T> class Bang; } } }
///+namespace ns { namespace ns4 { class Baz; } }
///+

int main() { return 0; }
"""
    iwyu_output = """\
add_fwd_declare_keep_iwyu_namespace should add these lines:
namespace ns { class Foo; }
namespace ns { namespace ns2 { namespace ns3 { class Bar; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class Bang; } } }
namespace ns { namespace ns4 { class Baz; } }

add_fwd_declare_keep_iwyu_namespace should remove these lines:

The full include-list for add_fwd_declare_keep_iwyu_namespace:
#include "foo.h"  // lines 3-3
namespace ns { class Foo; }
namespace ns { namespace ns2 { namespace ns3 { class Bar; } } }
namespace ns { namespace ns2 { namespace ns3 { template <typename T> class Bang; } } }
namespace ns { namespace ns4 { class Baz; } }
---
"""
    self.RegisterFileContents({'add_fwd_declare_keep_iwyu_namespace': infile})
    self.ProcessAndTest(iwyu_output, expected_num_modified_files=1)

  def testMain(self):
    """Make sure calling main doesn't crash.  Inspired by a syntax-error bug."""
    # Give an empty stdin so we don't actually try to parse anything.
    old_stdin = sys.stdin
    try:
      sys.stdin = cStringIO.StringIO()
      fix_includes.main(['fix_includes.py'])    # argv[0] doesn't really matter
    finally:
      sys.stdin = old_stdin

  def testFilenamesForSortingInMain(self):
    """Make sure if we use s, we have a filename specified, in main()."""
    # -s without any files to sort.
    self.assertRaises(SystemExit, fix_includes.main,
                      ['fix_includes.py', '-s'])


if __name__ == '__main__':
  unittest.main()
