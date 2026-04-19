#!/usr/bin/env python3

##===--- iwyu-mapgen-std-symbol.py ----------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

""" Generate mappings for C++ standard library symbols

This produces the symbol mapping to be included into iwyu_include_picker.cc
from the LaTeX sources of the C++ standard.
"""

import argparse
import glob
import os
import re
import sys
from itertools import repeat

NAMESPACE_KWD = 'namespace'
INLINE_KWD = 'inline'
REQUIRES_KWD = 'requires'
TEMPLATE_KWD = 'template'
OPERATOR_KWD = 'operator'
DECLTYPE_KWD = 'decltype'
USING_KWD = 'using'
TYPENAME_KWD = 'typename'
CONST_KWD = 'const'
VOLATILE_KWD = 'volatile'
COMBINABLE_TYPE_SPECIFIERS = (
    'signed',
    'unsigned',
    'char',
    'short',
    'long',
    'int',
    'double',
)
DEFINE = 'define'
TEXMACRO = re.compile(r'''@
                      \\\w+         # Macro name with a preceding backslash.
                      (\[[\w-]*\])? # Optional argument.
                      (\{[\w-]*\})* # Mandatory arguments.
                      @
                      ''', re.VERBOSE)
DEFNLIBXNAME = re.compile(r'@\\defnlibxname\{(\w*)\}')
LIBGLOBAL = re.compile(r'@\\libglobal\{(\w*)\}')
LIBMEMBER = re.compile(r'@\\libmember\{(\w*)\}')
ITCORR = r'@\itcorr@'
SYNOPSIS = re.compile(
    r'''Header\ \\tcode\{<(?P<headername>\w+)>\}
    \\?                     # An extra '\' is present in <iterator> synopsis.
    \ synopsis
    .*?                     # Search non-greedily.
    \\begin\{codeblock(digitsep)?\}  # codeblockdigitsep is used for <ratio>.
    (?P<code>.*?)
    \\end\{codeblock''',
    re.DOTALL | re.VERBOSE)
FP_PLACEHOLDER = r'@\placeholder{floating-point-type}@'
# Pattern for matching parameter declaration clauses of string literal
# operators. They should contain a pointer to some char type and std::size_t
# parameters.
STR_LIT_OP_PARAMS = re.compile('const w?char.*size_t');

OUTPUT_HEADER = \
'C++ standard library mapping, produced with mapgen/iwyu-mapgen-std-symbol.py.'

# Functions overloaded in different headers may cause problems because IWYU
# doesn't consider function parameters in mappings. Hence they are excluded
# for a while.
OVERLOADED_FUNCS = {
    'std::abs', 'std::acos', 'std::acosh', 'std::asin',
    'std::asinh', 'std::atan', 'std::atan2', 'std::atanh', 'std::begin',
    'std::cos', 'std::cosh', 'std::div', 'std::end', 'std::erase',
    'std::erase_if', 'std::exp', 'std::get', 'std::isalnum', 'std::isalpha',
    'std::isblank', 'std::iscntrl', 'std::isdigit', 'std::isgraph',
    'std::islower', 'std::isprint', 'std::ispunct', 'std::isspace',
    'std::isupper', 'std::isxdigit', 'std::log', 'std::log10',
    'std::make_error_code', 'std::make_error_condition', 'std::move',
    'std::pow', 'std::print', 'std::println', 'std::remove', 'std::sin',
    'std::sinh', 'std::sqrt', 'std::swap', 'std::tan', 'std::tanh',
    'std::to_string', 'std::tolower', 'std::toupper', 'std::vprint_nonunicode',
    'std::vprint_unicode',
}

# Some classes and template specializations are excluded as well.
# The specializations defined in different headers differ only in C++20
# constraints, and hence cannot be distinguished in the mapping. The classes are
# provided by different headers, and some of the headers provide only forward-
# declarations, therefore some additional consideration is required.
DUPLICATED_CLASSES = {
    'std::allocator',
    'std::atomic',
    'std::basic_common_reference<:0, :1, :2, :3>',
    'std::compare_three_way',
    'std::formatter<:0, :1>',
    'std::tm',
    'std::tuple',
}

# Type aliases whose underlying type is not specified in the standard cannot
# be used in a portable mapping.
UNSPEC_ALIASES = (
    'std::iter_difference_t',
)

QUAL_MAP = dict((
    ('duration', 'std::chrono::duration'),
    ('basic_ostream', 'std::basic_ostream'),
    ('basic_istream', 'std::basic_istream'),
    ('ostream','std::basic_ostream<char, std::char_traits<char>>'),
    ('iterator_traits', 'std::iterator_traits'),
    ('basic_string','std::basic_string'),
    ('nullptr_t','std::nullptr_t'),
))

# For some symbols defined in different headers, a "canonical" header should be
# placed before others in the mapping so that IWYU "prefers" it.
CANONICAL_HEADERS = {
    'std::mbstate_t': 'cwchar',
    'std::tuple_element': 'tuple',
    'std::tuple_size': 'tuple',
    'std::size_t': 'cstddef',
# AFAIU, <cstdlib> should be preferred over <cmath> for these:
    'std::abs(int)': 'cstdlib',
    'std::abs(long)': 'cstdlib',
    'std::abs(long long)': 'cstdlib',

    'std::hash': 'functional',
}

# Skip these until <iosfwd> can be handled correctly.
EXCLUDED_HEADERS = {
    'iosfwd',
    'ios',
    'streambuf',
    'istream',
    'ostream',
    'sstream',
    'spanstream',
    'fstream',
    'syncstream',
    'print',
}

# The underlying type of this is not specified. Moreover, the corresponding
# overloads of 'std::abs' and 'std::div' don't even exist if it coincides with
# one of the standard integer types.
ALIAS_TO_AVOID = 'intmax_t'

HANDWRITTEN_MAPPING = (
# Some namespace-scope symbols are not defined in the corresponding header
# synopses.
    ('std::hash', 'bitset'),
    ('std::hash<std::bitset<:0>>', 'bitset'),
    ('std::hash', 'thread'),
    ('std::hash<std::thread::id>', 'thread'),
)

def contains_unspec_alias(inp):
    for alias in UNSPEC_ALIASES:
        if re.search(f'{alias}\\b', inp):
            return True
    return False

class ValueSaver:
    def __init__(self, sequence):
        self.__ref = sequence
        self.__copy = sequence.copy()

    def __enter__(self):
        return self

    def __exit__(self, *t):
        self.__ref[:] = self.__copy

class Parser:
    """ Parses a standard library header synopsis from a LaTeX file and fills in
        found_names list. This stupid pseudo-C++ parser has been written with
        a lot of assumptions about the synopsis syntax. Use carefully and check
        the output!
    """
    def __init__(self, text):
        self.__text = text
        self.__cur_pos = 0
        self.__namespace_stack = []
        # __tpl_params holds parameters while parsing a template declaration.
        self.__tpl_params = []
        self.found_names = []

    def parse(self):
        while self.__cur_pos < len(self.__text):
            self.parse_top_level()
        self.found_names = [name for name in self.found_names if name not in \
            DUPLICATED_CLASSES]

    def parse_top_level(self):
        self.skip_spaces()
        t = self.__text
        cp = self.__cur_pos
        if cp >= len(t):
            return

        if t[cp] == '@':
            # Skip LaTeX commands at the beginning.
            self.skip_braced_text('@', '@')
        elif t.startswith('\\indexlibraryglobal', cp):
            self.skip_till_eol()
        elif t[cp] == '#':
            self.act_on_pp_directive()
        elif t.startswith(NAMESPACE_KWD, cp):
            self.act_on_namespace()
        elif t.startswith(INLINE_KWD, cp):
            self.__cur_pos += len(INLINE_KWD)
            self.skip_spaces()
            if t.startswith(NAMESPACE_KWD, self.__cur_pos):
                return self.act_on_inline_namespace()
            self.act_on_identifier_or_kwd()
        else:
            self.act_on_identifier_or_kwd()

    def skip_braced_text(self, opening, closing):
        t = self.__text
        cp = self.__cur_pos
        assert t[cp] == opening
        cp += 1
        num_opened_braces = 1
        while num_opened_braces:
            if t[cp] == closing:
                num_opened_braces -= 1
            elif t[cp] == opening:
                num_opened_braces += 1
            cp += 1
        self.__cur_pos = cp

    def skip_spaces(self):
        t = self.__text
        while self.__cur_pos < len(t) and t[self.__cur_pos].isspace():
            self.__cur_pos += 1
        if t.startswith('//', self.__cur_pos):
            # A comment. Skip it.
            self.skip_till_eol()
            self.skip_spaces()
        elif t.startswith(ITCORR, self.__cur_pos):
            # AFAIU, \itcorr macro just inserts some spacing.
            self.__cur_pos += len(ITCORR)
            self.skip_spaces()

    def skip_till_eol(self):
        self.__cur_pos = self.__text.find('\n', self.__cur_pos) + 1

    def skip_till_semicolon(self):
        t = self.__text
        while t[self.__cur_pos] != ';':
            if t[self.__cur_pos] == '{':
                # Skip possible class definitions.
                self.skip_braced_text('{', '}')
            else:
                self.__cur_pos += 1
        self.__cur_pos += 1     # Consume ';'.

    def skip_till_comma_or_right_bracket(self):
        t = self.__text
        while t[self.__cur_pos] != ',' and t[self.__cur_pos] != '>':
            if t[self.__cur_pos] == '<':
                # Skip internal template arguments.
                self.skip_braced_text('<', '>')
            else:
                self.__cur_pos += 1

    def act_on_pp_directive(self):
        assert self.__text[self.__cur_pos] == '#'
        self.__cur_pos += 1  # consume '#'
        self.skip_spaces()
        if self.__text.startswith(DEFINE, self.__cur_pos):
            self.act_on_define()
        else:
            self.skip_till_eol()

    def act_on_define(self):
        assert self.__text.startswith(DEFINE, self.__cur_pos)
        self.__cur_pos += len(DEFINE)  # consume 'define'

        # Ignore #define directives until IWYU has the C standard library symbol
        # mapping.

        #found_name = self.read_identifier()
        #if found_name and found_name.find('\\') == -1:
        #    self.found_names.append(found_name)

        self.skip_till_eol()

    def act_on_namespace(self):
        assert self.__text.startswith(NAMESPACE_KWD, self.__cur_pos)
        self.__cur_pos += len(NAMESPACE_KWD)
        with ValueSaver(self.__namespace_stack):
            id = self.read_identifier()
            self.add_identifier_from_ns(id)
            self.__namespace_stack.append(id)
            while self.__text.startswith('::', self.__cur_pos):
                self.__cur_pos += 2  # consume '::'
                id = self.read_identifier()
                if id == INLINE_KWD:
                    # Ignore inline namespaces.
                    self.read_identifier()
                    self.skip_spaces()
                    continue
                self.add_identifier_from_ns(id)
                self.__namespace_stack.append(id)
            self.skip_spaces()

            if self.__text[self.__cur_pos] == '=':  # Namespace alias.
                self.skip_till_semicolon()
                return

            assert self.__text[self.__cur_pos] == '{'
            self.__cur_pos += 1
            self.skip_spaces()
            while self.__text[self.__cur_pos] != '}':
                self.parse_top_level()
                self.skip_spaces()
            self.__cur_pos += 1  # Consume '}'.

    def act_on_inline_namespace(self):
        # IWYU skips names of inline namespaces. Hence, don't store it
        # in the stack.
        assert self.__text.startswith(NAMESPACE_KWD, self.__cur_pos)
        self.__cur_pos += len(NAMESPACE_KWD)
        self.read_identifier()
        self.skip_spaces()
        assert self.__text[self.__cur_pos] == '{'
        self.__cur_pos += 1
        self.skip_spaces()
        while self.__text[self.__cur_pos] != '}':
            self.parse_top_level()
            self.skip_spaces()
        self.__cur_pos += 1

    def read_identifier(self):
        self.skip_spaces()
        t = self.__text
        end_pos = self.__cur_pos
        while t[end_pos].isalnum() or t[end_pos] == '_' or t[end_pos] == '@':
            m = TEXMACRO.match(t, end_pos)
            if m:
                end_pos = m.end()
            else:
                end_pos += 1
        found_name = t[self.__cur_pos:end_pos]
        self.__cur_pos = end_pos

        # Check if the name is contained inside some LaTeX macro.
        m = DEFNLIBXNAME.match(found_name)
        if m:
            return f'__{m.group(1)}'
        m = LIBGLOBAL.match(found_name)
        if m:
            return m.group(1)
        m = LIBMEMBER.match(found_name)
        if m:
            return m.group(1)
        return found_name

    def act_on_identifier_or_kwd(self):
        self.skip_spaces()
        t = self.__text
        cp = self.__cur_pos
        if t.startswith('.\n', cp):
            # Occurs in <functional>, "placeholders" namespace.
            self.__cur_pos += 1     # Consume the dot.
            return self.act_on_identifier_or_kwd()
        if t.startswith(OPERATOR_KWD, cp):
            return self.act_on_operator()
        if t.startswith(TEMPLATE_KWD, cp):
            return self.act_on_template()
        if t.startswith(DECLTYPE_KWD, cp):
            self.skip_decltype()
            return self.act_on_identifier_or_kwd()
        if t.startswith(USING_KWD, cp):
            self.__cur_pos += len(USING_KWD)
            self.skip_spaces()
            if t.startswith(NAMESPACE_KWD, self.__cur_pos):
                # Skip using-directive.
                return self.skip_till_semicolon()

        id = self.read_qualified_id()
        cp = self.__cur_pos
        if t[cp] in '=:;{':         # Variable, class, enum, or type alias.
            self.add_identifier_from_ns(id)
            return self.skip_till_semicolon()
        if t[cp] == '(':            # Function.
            if self.get_identifier_with_ns(id) in OVERLOADED_FUNCS:
                id += self.parse_fn_parameters()
                assert '\\' not in id or FP_PLACEHOLDER in id
                self.skip_fn_body()
            else:
                self.skip_function()

            if FP_PLACEHOLDER in id:
                for t in ('float', 'double', 'long double'):
                    self.add_identifier_from_ns(id.replace(FP_PLACEHOLDER, t))
            elif ALIAS_TO_AVOID not in id:
                self.add_identifier_from_ns(id)
            return
        if t.startswith('[[', cp):      # Attribute.
            self.__cur_pos = t.find(']]', cp + 2) + 2
            return self.act_on_identifier_or_kwd()
        if t[cp].isalpha() or t[cp] == '_' or t[cp] == '@':
            # id was probably a keyword or a type specifier; go on with parsing.
            return self.act_on_identifier_or_kwd()
        if t[cp] == '*' or t[cp] == '&':
            self.__cur_pos += 1         # Consume ptr or ref.
            return self.act_on_identifier_or_kwd()
        if t[cp] == '"':                # 'extern "C"' or 'extern "C++"'.
            self.skip_braced_text('"', '"')
            return self.act_on_identifier_or_kwd()
        assert False

    def act_on_operator(self):
        t = self.__text
        assert t.startswith(OPERATOR_KWD, self.__cur_pos)
        self.__cur_pos += len(OPERATOR_KWD)

        end = t.find('(', self.__cur_pos)
        op = t[self.__cur_pos : end].strip()
        self.__cur_pos = end
        if 'new' in op or 'delete' in op:
            # These should not be mapped because users can override them.
            self.skip_function()
            return
        assert not op[0].isalpha()
        id = OPERATOR_KWD + op
        # All operators from 'chrono' namespace are in '<chrono>' header.
        if self.__namespace_stack[-1] != 'chrono':
            params = self.parse_fn_parameters()
            # Parameters of string literal operators cannot be used
            # in the mapping due to 'size_t' alias, but they don't have to: it
            # is sufficient to specify parameters for all other 'operator""s'
            # overloads. The remaining overloads for 'operator""s' are in
            # '<string>', and for 'operator""sv' are in '<string_view>'.
            if not r'operator""s' in id or not STR_LIT_OP_PARAMS.search(params):
                id += params
        assert '\\' not in id
        self.skip_fn_body()
        if not contains_unspec_alias(id):
            self.add_identifier_from_ns(id)

    def act_on_template(self):
        assert self.__text.startswith(TEMPLATE_KWD, self.__cur_pos)
        self.__cur_pos += len(TEMPLATE_KWD)
        self.skip_spaces()
        self.act_on_template_params()
        self.skip_spaces()
        if self.__text.startswith(REQUIRES_KWD, self.__cur_pos):
            self.skip_requires_clause()
        self.act_on_identifier_or_kwd()
        self.__tpl_params.clear()

    def act_on_template_params(self):
        assert self.__text[self.__cur_pos] == '<'
        self.__cur_pos += 1
        self.read_qualified_id()    # Skip 'typename' or NTTP type-specifier.
        self.act_on_tpl_param_id_or_kwd()

    def act_on_tpl_param_id_or_kwd(self):
        self.skip_spaces()
        t = self.__text
        if t[self.__cur_pos] == '>':
            self.__cur_pos += 1
            return
        id = self.read_identifier()
        self.skip_spaces()
        cp = self.__cur_pos
        if t[cp] == ',' or t[cp] == '=' or t[cp] == '>':
            if id:
                self.__tpl_params.append(id)
            self.skip_till_comma_or_right_bracket()
            if t[self.__cur_pos] == ',':
                self.__cur_pos += 1             # Consume it.
        elif t[cp] == '<':
            # A template template parameter or template-id in an NTTP type.
            self.skip_braced_text('<', '>')
        elif t[cp:cp+3] == '...':
            self.__cur_pos += 3
        elif t[cp:cp+2] == '::':
            self.__cur_pos += 2
        self.act_on_tpl_param_id_or_kwd()

    def skip_requires_clause(self):
        self.__cur_pos += len(REQUIRES_KWD)
        self.skip_primary_expr()
        while self.__text.startswith('&&', self.__cur_pos) \
            or self.__text.startswith('||', self.__cur_pos):
            self.__cur_pos += 2
            self.skip_primary_expr()

    def skip_primary_expr(self):
        self.skip_spaces()
        t = self.__text
        if t[self.__cur_pos] == '(':
            self.skip_braced_text('(', ')')
        elif t.startswith(REQUIRES_KWD, self.__cur_pos):
            self.skip_requires_expr()
        else:
            # Skip id-expression.
            self.read_qualified_id()
        self.skip_spaces()

    def skip_requires_expr(self):
        t = self.__text
        assert t.startswith(REQUIRES_KWD, self.__cur_pos)
        self.__cur_pos += len(REQUIRES_KWD)
        self.skip_spaces()
        if t[self.__cur_pos] == '(':
            self.skip_braced_text('(', ')')
        self.skip_spaces()
        assert t[self.__cur_pos] == '{'
        self.skip_braced_text('{', '}')

    def skip_decltype(self):
        t = self.__text
        assert t.startswith(DECLTYPE_KWD, self.__cur_pos)
        self.__cur_pos += len(DECLTYPE_KWD)
        self.skip_spaces()
        assert t[self.__cur_pos] == '('
        self.skip_braced_text('(', ')')

    def read_qualified_id(self):
        id = self.read_unqualified_id()
        while self.__text.startswith('::', self.__cur_pos):
            self.__cur_pos += 2
            id += '::' + self.read_unqualified_id()
        return id

    def read_unqualified_id(self):
        id = self.read_identifier()
        self.skip_spaces()
        assert id != OPERATOR_KWD
        if self.__text[self.__cur_pos] == '<':
            # Because it is not operator<, it should be template-id.
            start_tpl_args = self.__cur_pos
            self.skip_braced_text('<', '>')
            end_tpl_args = self.__cur_pos
            tpl_args = self.__text[start_tpl_args:end_tpl_args]

            # Replace multiple space symbols with probably intervening comments
            # with a single one.
            tpl_args = re.sub(r'\s+(//.*\n\s*)*', ' ', tpl_args)

            tpl_args = self.qualify_names(tpl_args)

            # Format pointer, reference, and function types as IWYU expects
            # (i.e., add a space).
            tpl_args = re.sub(r'(\w)([\*&(])', r'\1 \2', tpl_args)

            # Insert a whitespace after comma if there isn't any.
            tpl_args = re.sub(r',(?=\S)', ', ', tpl_args)

            tpl_args = tpl_args.replace(TYPENAME_KWD + ' ', '')

            id += tpl_args
            self.skip_spaces()

        # Replace template parameters with ':N' (for partial specialization
        # arguments and function parameters).
        for i, param in enumerate(self.__tpl_params):
            id = re.sub(rf'\b{param}\b', f':{i}', id)

        if id == 'coroutine_handle<>':
            id = 'coroutine_handle<void>'
        return id

    def qualify_names(self, inp):
        """ Looks up names and qualifies them with missing namespaces. Excludes
            names that are preceded by '::' (they are already qualified). """
        p = re.compile(r'(?<!::)\b\w+\b')
        m = p.search(inp)
        while m:
            arg_id = self.look_up(m.group(0))
            inp = inp[:m.start()] + arg_id + inp[m.end():]
            m = p.search(inp, m.start() + len(arg_id))
        # Replace some unqualified (without leading '::') names with desugared
        # qualified forms.
        for name, repl in QUAL_MAP.items():
            inp = re.sub(fr'(?<!::)\b{name}\b', repl, inp)
        return inp

    def look_up(self, name):
        l = len(self.__namespace_stack)
        # Look up from the nearest enclosing namespace.
        for i in range(l):
            id = '::'.join(self.__namespace_stack[: l-i]) + f'::{name}'
            if self.found_names.count(id):
                return id
        return name

    def parse_fn_parameters(self):
        t = self.__text
        assert t[self.__cur_pos] == '('
        res = '('
        self.__cur_pos += 1
        self.skip_spaces()
        while t[self.__cur_pos] != ')':
            res += self.parse_param_decl()
        res += ')'
        return self.qualify_names(res)

    def parse_param_decl(self):
        t = self.__text
        if t.startswith('...', self.__cur_pos):
            self.__cur_pos += 3
            self.skip_spaces()
            return '...'
        res = self.parse_decl_spec_seq()
        res += self.parse_declarator()
        if t[self.__cur_pos] == ',':
            self.__cur_pos += 1
            self.skip_spaces()
            res += ', '
        return res

    def parse_decl_spec_seq(self):
        res = self.read_cv_qualifiers()
        id = self.read_qualified_id()
        if id == TYPENAME_KWD:
            id = self.read_qualified_id()  # Skip 'typename' keyword.
        res += id
        prev = id
        while id in COMBINABLE_TYPE_SPECIFIERS:
            # It is acceptable if this consumes declarator-id: it is ignored.
            id = self.read_identifier()
            self.skip_spaces()
            # 'long int' should be spelled as just 'long'.
            if id in COMBINABLE_TYPE_SPECIFIERS and id != 'int':
                res += ' ' + id
                prev = id
        # 'unsigned' should be spelled as 'unsigned int'.
        if prev == 'unsigned':
            res += ' int'
        return res

    def read_cv_qualifiers(self):
        t = self.__text
        res = ''
        if t.startswith(CONST_KWD, self.__cur_pos):
            res += CONST_KWD + ' '
            self.__cur_pos += len(CONST_KWD)
            self.skip_spaces()
        if t.startswith(VOLATILE_KWD, self.__cur_pos):
            res += VOLATILE_KWD + ' '
            self.__cur_pos += len(VOLATILE_KWD)
            self.skip_spaces()
        # 'const' after 'volatile' is not supported yet.
        assert not t.startswith(CONST_KWD, self.__cur_pos)
        return res

    def parse_declarator(self):
        t = self.__text
        res = ''
        ptr_operator_present = False
        while t[self.__cur_pos] in '*&':
            if len(res) == 0:
                res += ' '
            res += t[self.__cur_pos]
            self.__cur_pos += 1
            self.skip_spaces()
            res += self.read_cv_qualifiers()
            ptr_operator_present = True
        return res + self.parse_noptr_declarator(ptr_operator_present)

    def parse_noptr_declarator(self, after_ptr):
        """ Parses noptr-declarator or noptr-abstract-declarator.
            'after_ptr' means "after ptr-operator" in the sense described
            in the standard, including references. """
        t = self.__text
        res = ''
        if t[self.__cur_pos] == '(':
            self.__cur_pos += 1
            self.skip_spaces()
            assert t[self.__cur_pos] in '*&'  # The only cases supported now.
            if not after_ptr:
                res += ' '
            res += '(' + self.parse_declarator()[1:]  # Strip the whitespace.
            assert t[self.__cur_pos] == ')'
            res += ')'
            self.__cur_pos += 1
        else:
            self.read_identifier()  # Ignore declarator-id.
        self.skip_spaces()

        while t[self.__cur_pos] not in ',)':
            if t[self.__cur_pos] == '[':
                res += '['
                self.__cur_pos += 1
                res += self.read_unqualified_id()  # Probably just an NTTP name.
                assert t[self.__cur_pos] == ']'
                res += ']'
                self.__cur_pos += 1
            else:
                assert False  # Other cases are not supported yet.
            self.skip_spaces()
        return res

    def skip_fn_body(self):
        t = self.__text
        while t[self.__cur_pos] not in '{;':
            self.__cur_pos += 1
        if t[self.__cur_pos] == ';':
            self.__cur_pos += 1
        elif t[self.__cur_pos] == '{':
            self.skip_braced_text('{', '}')

    def skip_function(self):
        assert self.__text[self.__cur_pos] == '('
        self.skip_braced_text('(', ')')
        self.skip_fn_body()

    def add_identifier_from_ns(self, id):
        id = self.get_identifier_with_ns(id)
        if '\\' in id:
            # Avoid names containing TeX commands.
            return
        if id not in self.found_names:
            self.found_names.append(id)

    def get_identifier_with_ns(self, id):
        if self.__namespace_stack:
            id = '::'.join(self.__namespace_stack) + f'::{id}'
        return id

def process_synopsis(syn):
    p = Parser(syn)
    p.parse()
    return p.found_names

def process_tex_file(tex_file):
    res = []
    for m in SYNOPSIS.finditer(tex_file):
        if m.group('headername') in EXCLUDED_HEADERS:
            continue
        symbols = process_synopsis(m.group('code'))
        res += zip(symbols, repeat(m.group('headername')))
    return res

def print_line(symbol, headername, lang):
    escaped = symbol.replace('"', r'\"')
    if lang == 'c++':
        print(f'{{ "{escaped}", kPrivate, "<{headername}>", kPublic }},')
    else:
        print(f'{{ "symbol": ["{escaped}", "private", '
                              f'"<{headername}>", "public"] }},')

def print_content(std_source_path, lang):
    headers_by_symbol = {}
    for path in sorted(glob.glob(os.path.join(std_source_path, '*.tex'))):
        with open(path, 'r') as f:
            for symbol, headername in process_tex_file(f.read()):
                headers_by_symbol.setdefault(symbol, []).append(headername)
    for symbol, headername in HANDWRITTEN_MAPPING:
        headers_by_symbol.setdefault(symbol, []).append(headername)

    for symbol, headernames in sorted(headers_by_symbol.items()):
        canonical_header = CANONICAL_HEADERS.get(symbol)
        if canonical_header:
            print_line(symbol, canonical_header, lang)
        for headername in sorted(headernames):
            if headername != canonical_header:
                print_line(symbol, headername, lang)

def main(std_source_path, lang):
    comment_prefix = '// ' if lang == 'c++' else '# '
    print(comment_prefix + OUTPUT_HEADER)

    if lang == 'imp':
        print('[')
        print_content(std_source_path, lang)
        print(']')
    else:
        print_content(std_source_path, lang)

if __name__ == '__main__':
    argparser = argparse.ArgumentParser(
        description='Generate IWYU mappings for the C++ standard library.')
    argparser.add_argument('std_source_path',
        help='Path to TeX sources of the standard ("source" subdirectory)')
    argparser.add_argument('--lang', choices=['c++', 'imp'], default='c++',
        help='Output language')
    args = argparser.parse_args()
    sys.exit(main(args.std_source_path, args.lang))
