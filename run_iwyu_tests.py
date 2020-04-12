#!/usr/bin/env python

##===--- run_iwyu_tests.py - include-what-you-use test framework driver ---===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

"""A test harness for IWYU testing."""

__author__ = 'dsturtevant@google.com (Dean Sturtevant)'

import glob
import os
import re
import sys
import unittest
import logging
logging.basicConfig(level=logging.INFO)
import posixpath
from fnmatch import fnmatch
import iwyu_test_util


def PosixPath(path):
    """Normalize Windows path separators to POSIX path separators."""
    return path.replace('\\', '/')


def Partition(l, delimiter):
  try:
    delim_index = l.index(delimiter)
  except ValueError:
    return l, []

  return l[:delim_index], l[delim_index+1:]


class OneIwyuTest(unittest.TestCase):
  """Superclass for tests.  A subclass per test-file is created at runtime."""

  def CheckAlsoExtension(self, extension):
    """Return a suitable iwyu flag for checking files with the given extension.
    """
    return '--check_also="%s"' % posixpath.join(self.rootdir, '*' + extension)

  def MappingFile(self, filename):
    """Return a suitable iwyu flag for adding the given mapping file."""
    return '--mapping_file=%s' % posixpath.join(self.rootdir, filename)

  def Include(self, filename):
    """Return a -include switch for clang to force include of file."""
    return '-include %s' % posixpath.join(self.rootdir, filename)

  def setUp(self):
    # Iwyu flags for specific tests.
    # Map from filename to flag list. If any test requires special
    # iwyu flags to run properly, add an entry to the map with
    # key=cc-filename (relative to self.rootdir), value=list of flags.
    flags_map = {
      'backwards_includes.cc': [self.CheckAlsoExtension('-d*.h')],
      'badinc.cc': [self.MappingFile('badinc.imp')],
      'builtins_with_mapping.cc': [self.MappingFile('builtins_with_mapping.imp')],
      'check_also.cc': [self.CheckAlsoExtension('-d1.h')],
      'implicit_ctor.cc': [self.CheckAlsoExtension('-d1.h')],
      'iwyu_stricter_than_cpp.cc': [self.CheckAlsoExtension('-autocast.h'),
                                    self.CheckAlsoExtension('-fnreturn.h'),
                                    self.CheckAlsoExtension('-typedefs.h'),
                                    self.CheckAlsoExtension('-d2.h')],
      'keep_includes.c': ['--keep=tests/c/keep_includes*.h'],
      'keep_mapping.cc': [self.CheckAlsoExtension('-public.h'),
                          self.MappingFile('keep_mapping.imp')],
      'keep_moc.cc': [self.CheckAlsoExtension('-i1.h')],
      'macro_location.cc': [self.CheckAlsoExtension('-d2.h')],
      'mapping_to_self.cc': [self.MappingFile('mapping_to_self.imp')],
      'non_transitive_include.cc': [self.CheckAlsoExtension('-d*.h'),
                                    '--transitive_includes_only'],
      'no_h_includes_cc.cc': [self.CheckAlsoExtension('.c')],
      'no_comments.cc': ['--no_comments'],
      'no_fwd_decls.cc': ['--no_fwd_decls'],
      'overloaded_class.cc': [self.CheckAlsoExtension('-i1.h')],
      'pch_in_code.cc': ['--pch_in_code', '--prefix_header_includes=remove'],
      'prefix_header_attribution.cc': ['--prefix_header_includes=remove'],
      'prefix_header_includes_add.cc': ['--prefix_header_includes=add'],
      'prefix_header_includes_keep.cc': ['--prefix_header_includes=keep'],
      'prefix_header_includes_remove.cc': ['--prefix_header_includes=remove'],
      'prefix_header_operator_new.cc': ['--prefix_header_includes=remove'],
      'quoted_includes_first.cc': ['--pch_in_code', '--quoted_includes_first'],
      'relative_exported_mapped_include.cc':
          [self.MappingFile('relative_exported_mapped_include.imp')],
      'cxx17ns.cc': ['--cxx17ns'],
    }
    prefix_headers = [self.Include('prefix_header_includes-d1.h'),
                      self.Include('prefix_header_includes-d2.h'),
                      self.Include('prefix_header_includes-d3.h'),
                      self.Include('prefix_header_includes-d4.h')]
    clang_flags_map = {
      'alias_template.cc': ['-std=c++11'],
      'auto_type_within_template.cc': ['-std=c++11'],
      # MSVC targets need to explicitly enable exceptions, so we do it for all.
      'catch.cc': ['-fcxx-exceptions', '-fexceptions'],
      'clmode.cc': ['--driver-mode=cl', '/GF', '/Os', '/W2'],
      'conversion_ctor.cc': ['-std=c++11'],
      'deleted_implicit.cc' : ['-std=c++11'],
      'funcptrs.cc': ['-Wno-unused'],
      'lambda_fwd_decl.cc': ['-std=c++11'],
      'lateparsed_template.cc': ['-fdelayed-template-parsing'],
      'macro_defined_by_includer.cc': [
          '-std=c++11', '-DCOMMAND_LINE_TYPE=double',
          self.Include('macro_defined_by_includer-prefix.h')],
      'macro_location.cc': ['-Wno-sizeof-pointer-div'],
      'ms_inline_asm.cc': ['-fms-extensions'],
      'operator_new.cc': ['-std=c++17'],
      'placement_new.cc': ['-std=c++17'],
      'prefix_header_attribution.cc': [self.Include('prefix_header_attribution-d1.h')],
      'prefix_header_includes_add.cc': prefix_headers,
      'prefix_header_includes_keep.cc': prefix_headers,
      'prefix_header_includes_remove.cc': prefix_headers,
      'range_for.cc': ['-std=c++11'],
      'typedef_in_template.cc': ['-std=c++11'],
      'inheriting_ctor.cc': ['-std=c++11'],
      'cxx17ns.cc': ['-std=c++17'],
    }
    include_map = {
      'alias_template.cc': ['.'],
      'array.cc': ['.'],
      'associated_h_file_heuristic.cc': ['.'],
      'associated_include.cc': ['.'],
      'associated_skipped.cc': ['.'],
      'backwards_includes.cc': ['.'],
      'badinc.cc': ['.'],
      'badinc-extradef.cc': ['.'],
      'binary_type_trait.cc': ['.'],
      'builtins_with_mapping.cc': ['.'],
      'funcptrs.cc': ['.'],
      'casts.cc': ['.'],
      'catch.cc': ['.'],
      'check_also.cc': ['.'],
      'clmode.cc': ['.'],
      'comment_pragmas.cc': ['.'],
      'computed_include.cc': ['.'],
      'conversion_ctor.cc': ['.'],
      'cvr.cc': ['.'],
      'default_template_arg_other_file.cc': ['.'],
      'depopulated_h_file.cc': ['.'],
      'derived_function_tpl_args.cc': ['.'],
      'dotdot.cc': ['.'],
      'double_include.cc': ['.'],
      'elaborated_struct.c': ['.'],
      'elaborated_type.cc': ['.'],
      'enum_base.cc': ['.'],
      'export_nesting.cc': ['.'],
      'external_including_internal.cc': ['.'],
      'forward_declare_in_macro.cc': ['.'],
      'fullinfo_for_templates.cc': ['.'],
      'fwd_decl_class_template.cc': ['.'],
      'fwd_decl_final.cc': ['.'],
      'fwd_decl_static_member.cc': ['.'],
      'fwd_decl_with_instantiation.cc': ['.'],
      'header_in_subdir.cc': ['.'],
      'implicit_ctor.cc': ['.'],
      'include_cycle.cc': ['.'],
      'include_with_using.cc': ['.'],
      'inline_namespace.cc': ['.'],
      'internal/internal_files.cc': ['.'],
      'iwyu_stricter_than_cpp.cc': ['.'],
      'keep_includes.c': ['.'],
      'keep_mapping.cc': ['.'],
      'keep_moc.cc': ['.'],
      'lateparsed_template.cc': ['.'],
      'macro_defined_by_includer.cc': ['.'],
      'macro_location.cc': ['.'],
      'mapping_to_self.cc': ['.'],
      'member_expr.cc': ['.'],
      'multiple_include_paths.cc': ['.'],
      'new_header_path_provided.cc': ['.'],
      'no_comments.cc': ['.'],
      'no_fwd_decl_nested_class.cc': ['.'],
      'no_fwd_decls.cc': ['.'],
      'no_h_includes_cc.cc': ['.'],
      'non_transitive_include.cc': ['.'],
      'operator_new.cc': ['.'],
      'overloaded_class.cc': ['.'],
      'pch_in_code.cc': ['.'],
      'pointer_arith.cc': ['.'],
      'placement_new.cc': ['.'],
      'pragma_associated.cc': ['.'],
      'precomputed_tpl_args.cc': ['.'],
      'prefix_header_attribution.cc': ['.'],
      'prefix_header_includes_add.cc': ['.'],
      'prefix_header_includes_keep.cc': ['.'],
      'prefix_header_includes_remove.cc': ['.'],
      'quoted_includes_first.cc' : ['.'],
      'range_for.cc': ['.'],
      're_fwd_decl.cc': ['.'],
      'redecls.cc': ['.'],
      'relative_exported_mapped_include.cc': ['tests/cxx/subdir'],
      'remove_fwd_decl_when_including.cc': ['.'],
      'self_include.cc': ['.'],
      'sizeof_reference.cc': ['.'],
      'specialization_needs_decl.cc': ['.'],
      'system_namespaces.cc': ['.'],
      'template_args.cc': ['.'],
      'templated_constructor.cc': ['.'],
      'template_specialization.cc': ['.'],
      'typedef_chain_in_template.cc': ['.'],
      'typedef_chain_no_follow.cc': ['.'],
      'typedef_in_template.cc': ['.'],
      'typedefs_and_resugaring.cc': ['.'],
      'unused_class_template_ctor.cc': ['.'],
      'uses_printf.cc': ['.'],
      'using_aliased_symbol.cc': ['.'],
      'using_aliased_symbol_unused.cc': ['.'],
      'varargs_and_references.cc': ['.'],
      'virtual_tpl_method.cc': ['.'],
      'cxx17ns.cc': ['.'],
    }
    # Internally, we like it when the paths start with rootdir.
    self._iwyu_flags_map = dict((posixpath.join(self.rootdir, k), v)
                                for (k,v) in flags_map.items())
    self._clang_flags_map = dict((posixpath.join(self.rootdir, k), v)
                                 for (k,v) in clang_flags_map.items())
    self._include_map = dict((posixpath.join(self.rootdir, k), ['-I ' + include for include in v])
                                 for (k,v) in include_map.items())

  def RunOneTest(self, filename):
    logging.info('Testing iwyu on %s', filename)
    # Split full/path/to/foo.cc into full/path/to/foo and .cc.
    (all_but_extension, _) = os.path.splitext(filename)
    (dirname, basename) = os.path.split(all_but_extension)
    # Generate diagnostics on all foo-* files (well, not other
    # foo-*.cc files, which is not kosher but is legal), in addition
    # to foo.h (if present) and foo.cc.
    all_files = (glob.glob('%s-*' % all_but_extension) +
                 glob.glob('%s/*/%s-*' % (dirname, basename)) +
                 glob.glob('%s.h' % all_but_extension) +
                 glob.glob('%s/*/%s.h' % (dirname, basename)))
    files_to_check = [f for f in all_files if not fnmatch(f, self.pattern)]
    files_to_check.append(filename)

    # IWYU emits summaries with canonicalized filepaths, where all the
    # directory separators are set to '/'. In order for the testsuite to
    # correctly match up file summaries, we must canonicalize the filepaths
    # in the same way here.
    files_to_check = [PosixPath(f) for f in files_to_check]

    iwyu_flags = self._iwyu_flags_map.get(filename, None)
    clang_flags = self._clang_flags_map.get(filename, [])
    clang_flags.extend(self._include_map.get(filename, []))
    iwyu_test_util.TestIwyuOnRelativeFile(self, filename, files_to_check,
                                          iwyu_flags, clang_flags, verbose=True)


def RegisterFilesForTesting(rootdir, pattern):
  """Create a test-class for every file in rootdir matching pattern."""
  filenames = []
  for (dirpath, dirs, files) in os.walk(rootdir):
    dirpath = PosixPath(dirpath)  # Normalize path separators.
    filenames.extend(posixpath.join(dirpath, f) for f in files
                     if fnmatch(f, pattern))
  if not filenames:
    print('No tests found in %s!' % os.path.abspath(rootdir))
    return

  module = sys.modules[__name__]

  for filename in filenames:
    all_but_extension = os.path.splitext(filename)[0]
    basename = os.path.basename(all_but_extension)
    class_name = re.sub('[^0-9a-zA-Z_]', '_', basename)  # python-clean
    if class_name[0].isdigit():            # classes can't start with a number
      class_name = '_' + class_name
    while class_name in module.__dict__:   # already have a class with that name
      class_name += '2'                    # just append a suffix :-)

    logging.info('Registering %s to test %s', class_name, filename)
    test_class = type(class_name,          # class name
                      (OneIwyuTest,),      # superclass
                      # and attrs. f=filename is required for proper scoping
                      {'runTest': lambda self, f=filename: self.RunOneTest(f),
                       'rootdir': rootdir,
                       'pattern': pattern})
    setattr(module, test_class.__name__, test_class)


if __name__ == '__main__':
  unittest_args, additional_args = Partition(sys.argv, '--')
  if additional_args:
    iwyu_test_util.SetIwyuPath(additional_args[0])

  RegisterFilesForTesting('tests/cxx', '*.cc')
  RegisterFilesForTesting('tests/c', '*.c')
  unittest.main(argv=unittest_args)
