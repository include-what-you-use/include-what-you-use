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
SYNOPSIS = re.compile(
    r'''Header\ \\tcode\{<(?P<headername>\w+)>\}
    \\?                     # An extra '\' is present in <iterator> synopsis.
    \ synopsis
    .*?                     # Search non-greedily.
    \\begin\{codeblock(digitsep)?\}  # codeblockdigitsep is used for <ratio>.
    (?P<code>.*?)
    \\end\{codeblock''',
    re.DOTALL | re.VERBOSE)

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
    'std::hash',
    'std::tm',
    'std::tuple',
}

ERRONEOUS_SYMBOLS = dict((
    ('std::formatter<std::chrono::hh_mm_ss<duration<:0, :1>>, :2>',
     'std::formatter<std::chrono::hh_mm_ss<std::chrono::duration<:0, :1>>, :2>'),
))

# For some symbols defined in different headers, a "canonical" header should be
# placed before others in the mapping so that IWYU "prefers" it.
CANONICAL_HEADERS = {
    'std::mbstate_t': 'cwchar',
    'std::tuple_element': 'tuple',
    'std::tuple_size': 'tuple',
    'std::size_t': 'cstddef',
}

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
            (OVERLOADED_FUNCS | DUPLICATED_CLASSES)]

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
            self.add_identifier_from_ns(id)
            return self.skip_function()
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

        # Ignore operators for a while like other overloaded functions.

        #end = t.find('(', self.__cur_pos)
        #op = t[self.__cur_pos : end].strip()
        #id = OPERATOR_KWD + ((' ' + op) if op[0].isalpha() else op)
        #self.add_identifier_from_ns(id)

        self.skip_till_semicolon()

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

            # Replace template parameters with ':N' (for partial
            # specializations).
            for i, param in enumerate(self.__tpl_params):
                tpl_args = re.sub(rf'\b{param}\b', f':{i}', tpl_args)

            # Replace multiple space symbols with probably intervening comments
            # with a single one.
            tpl_args = re.sub(r'\s+(//.*\n\s*)*', ' ', tpl_args)

            # Look up names and qualify them with missing namespaces. Exclude
            # names that are preceded by '::' (they are already qualified).
            p = re.compile(r'(?<!::)\b\w+\b')
            m = p.search(tpl_args)
            while m:
                arg_id = self.look_up(m.group(0))
                tpl_args = tpl_args[:m.start()] + arg_id + tpl_args[m.end():]
                m = p.search(tpl_args, m.start() + len(arg_id))

            # Format pointer, reference, and function types as IWYU expects
            # (i.e., add a space).
            tpl_args = re.sub(r'(\w)([\*&(])', r'\1 \2', tpl_args)

            id += tpl_args
            self.skip_spaces()
        return id

    def look_up(self, name):
        l = len(self.__namespace_stack)
        # Look up from the nearest enclosing namespace.
        for i in range(l):
            id = '::'.join(self.__namespace_stack[: l-i]) + f'::{name}'
            if self.found_names.count(id):
                return id
        return name

    def skip_function(self):
        t = self.__text
        assert t[self.__cur_pos] == '('
        self.skip_braced_text('(', ')')
        self.skip_spaces()
        while t[self.__cur_pos] not in '{;':
            self.__cur_pos += 1
        if t[self.__cur_pos] == ';':
            self.__cur_pos += 1
        elif t[self.__cur_pos] == '{':
            self.skip_braced_text('{', '}')

    def add_identifier_from_ns(self, id):
        if self.__namespace_stack:
            id = '::'.join(self.__namespace_stack) + f'::{id}'
        if '\\' in id:
            # Avoid names containing TeX commands.
            return
        if id not in self.found_names:
            # Work around a bug in the standard text.
            id = ERRONEOUS_SYMBOLS.get(id, id)
            self.found_names.append(id)

def process_synopsis(syn):
    p = Parser(syn)
    p.parse()
    return p.found_names

def process_tex_file(tex_file):
    res = []
    for m in SYNOPSIS.finditer(tex_file):
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
        if path.endswith('iostreams.tex'):
            # Skip until <iosfwd> can be handled correctly.
            continue
        with open(path, 'r') as f:
            for symbol, headername in process_tex_file(f.read()):
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
