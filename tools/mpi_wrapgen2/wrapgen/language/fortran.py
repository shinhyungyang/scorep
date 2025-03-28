#
# This file is part of the Score-P software (http://www.score-p.org)
#
# Copyright (c) 2025,
# Forschungszentrum Juelich GmbH, Germany
#
# This software may be modified and distributed under the terms of
# a BSD-style license. See the COPYING file in the package base
# directory for details.
#
"""
Classes to represent Fortran types and function arguments.

Helper functions to parse pieces of Fortran code and construct those classes.
This module does not attempt to implement a full Fortran parser.
It is enough to parse the pieces of Fortran code that the pympistandard outputs from its parameter classes.

This is necessary because the pympistandard does not expose an interface to individual parts of an argument declaration
(e.g. dimension, attributes, ...).
It exposes only the entire argument descriptor (everything before '::') and the entire argument declarator
(everything after '::').
"""

from enum import Enum, auto
from dataclasses import dataclass, field
from typing import Optional, List, Dict, Tuple

from wrapgen.language import ParameterBase, VariableBase


@dataclass
class FortranType:
    name: str = ''
    subtype: str = ''
    parameters: Dict = field(default_factory=dict)

    def __str__(self) -> str:
        s = self.name
        ps = []
        if self.subtype is not None and self.subtype != '':
            ps.append(self.subtype)
        for param_name, param_value in self.parameters.items():
            ps.append(f'{param_name}={param_value}')
        if len(ps) > 0:
            s += '('
            s += ', '.join(ps)
            s += ')'
        return s


@dataclass
class FortranArgumentDescriptor:
    type: FortranType = field(default_factory=FortranType)
    dimension: Optional[List[str]] = None
    intent: Optional[str] = None
    attributes: List[str] = field(default_factory=list)

    def __str__(self) -> str:
        s = self.type.__str__()
        if self.dimension is not None and len(self.dimension) > 0:
            s += f', DIMENSION({",".join(self.dimension)})'
        if self.intent is not None:
            s += f', INTENT({self.intent})'
        for a in self.attributes:
            s += f', {a}'
        return s

    @staticmethod
    def from_string(descriptor_str: str, declarator_str: Optional[str] = None) -> 'FortranArgumentDescriptor':
        descriptor = parse_descriptor(descriptor_str)

        if declarator_str is not None:
            decl_name, decl_dim = parse_entity_decl(declarator_str)
            assert descriptor.dimension is None or decl_dim is None
            if decl_dim is not None:
                descriptor.dimension = decl_dim
        return descriptor


@dataclass
class FortranArgument(ParameterBase):
    descriptor: FortranArgumentDescriptor = field(default_factory=FortranArgumentDescriptor)

    def __str__(self) -> str:
        return f'{self.descriptor} :: {self.name}'

    @staticmethod
    def from_string(descriptor_str: str, declarator_str: str) -> 'FortranArgument':
        decl_name, decl_dim = parse_entity_decl(declarator_str)
        return FortranArgument(name=decl_name,
                               descriptor=FortranArgumentDescriptor.from_string(descriptor_str, declarator_str))

    @staticmethod
    def new(name, type_name, type_subtype='', type_parameters=None, dimension=None, intent=None, attributes=None):
        """
        This is a convenience method to construct a FortranArgument from the fields of all of its components directly.
        Equivalent to the nested constructor
        argument = FortranArgument(descriptor=FortranArgumentDescriptor(type=FortranType(...), ...), ...).

        :param name: name component of FortranArgument (argument)
        :param type_name: name component of FortranType (argument.descriptor.type)
        :param type_subtype: subtype component of FortranType (argument.descriptor.type)
        :param type_parameters: parameters component of FortranType (argument.descriptor.type)
        :param dimension: dimension component of FortranArgumentDescriptor (argument.descriptor)
        :param intent: intent component of FortranArgumentDescriptor (argument.descriptor)
        :param attributes: attributes component of FortranArguemenDescriptor (argument.descriptor)
        :return:
        """
        if type_parameters is None:
            type_parameters = {}
        if attributes is None:
            attributes = []
        return FortranArgument(
            name=name,
            descriptor=FortranArgumentDescriptor(
                type=FortranType(
                    name=type_name,
                    subtype=type_subtype,
                    parameters=type_parameters
                ),
                dimension=dimension,
                intent=intent,
                attributes=attributes
            )
        )


@dataclass
class FortranVariable(VariableBase):
    parameter: FortranArgument = field(default_factory=FortranArgument)

    def initialization(self):
        if self.initial_value != '':
            return f"{self.name} = {self.initial_value}"
        else:
            return ''

    def __str__(self):
        return self.parameter.__str__()


class TokenClass(Enum):
    WORD = auto()
    STRING_LITERAL = auto()
    CONTROL_CHAR = auto()


@dataclass
class Token:
    type: Optional[TokenClass] = None
    value: Optional[str] = None


@dataclass
class TokenUnit:
    """
    Word optionally followed by tokens in parentheses, i.e, word ['(' tokens... ')']
    """
    first: Optional[Token] = None
    parenthesis: Optional[List[Token]] = None


def split_token_units(tokens: List[Token]) -> List[TokenUnit]:
    unit_buffer = TokenUnit()
    units: List[TokenUnit] = []

    def flush() -> None:
        nonlocal unit_buffer
        if unit_buffer.first is not None:
            units.append(unit_buffer)
            unit_buffer = TokenUnit()

    paren_level = 0
    for current_token in tokens:
        if paren_level == 0:
            if current_token in (Token(type=TokenClass.CONTROL_CHAR, value=','),
                                 Token(type=TokenClass.WORD, value='::')):
                flush()
                unit_buffer.first = current_token
                flush()
                continue
            elif current_token.type is TokenClass.WORD:
                flush()
                unit_buffer.first = current_token
                continue

        if current_token == Token(type=TokenClass.CONTROL_CHAR, value='('):
            paren_level += 1
            if paren_level == 1:
                assert unit_buffer.first is not None, "Opening parenthesis not preceded by a word"
                unit_buffer.parenthesis = []
                continue

        if current_token == Token(type=TokenClass.CONTROL_CHAR, value=')'):
            paren_level -= 1
            assert paren_level >= 0, "Closing parenthesis without a matching opening parenthesis"
            if paren_level == 0:
                flush()
                continue

        assert paren_level > 0, "Unexpected token outside parenthesis"
        unit_buffer.parenthesis.append(current_token)

    flush()
    return units


def group_by_comma(tokens: List[Token]) -> List[List[Token]]:
    """
    Regroup a sequence of tokens into sub-lists delimited by the commas that are not enclosed in parentheses.
    The commas are not included.

    :param: tokens: List of tokens
    :return: List of lists of tokens.
    """
    grouped_tokens: List[List[Token]] = [[]]
    paren_level = 0
    for token in tokens:
        if token == Token(type=TokenClass.CONTROL_CHAR, value='('):
            paren_level = paren_level + 1
        elif token == Token(type=TokenClass.CONTROL_CHAR, value=')'):
            paren_level = paren_level - 1

        if token == Token(type=TokenClass.CONTROL_CHAR, value=',') and paren_level == 0:
            grouped_tokens.append([])
        else:
            grouped_tokens[-1].append(token)
    return grouped_tokens


def parse_descriptor(descriptor_string: str) -> FortranArgumentDescriptor:
    """Fortran Standard 2008: R501:
    type-declaration-stmt is declaration-type-spec [ [ , attr-spec ] ... :: ] entity-decl-list

    The descriptor as used by mpistandard is everything before the ::

    This function is by no means designed to parse every valid piece of Fortran code correctly.
    It only has to work for the descriptor strings output by mpistandard.
    """
    descriptor = FortranArgumentDescriptor()

    tokens = tokenize(descriptor_string)
    units = split_token_units(tokens)
    descriptor.type = parse_type(units[0])
    for unit in units[1:]:
        if unit.first == Token(type=TokenClass.CONTROL_CHAR, value=','):
            pass
        elif unit.first == Token(type=TokenClass.WORD, value='DIMENSION'):
            dimension_str, descriptor.dimension = parse_dimension(unit)
        elif unit.first == Token(type=TokenClass.WORD, value='INTENT'):
            descriptor.intent = parse_intent(unit)
        else:
            descriptor.attributes.append(parse_attribute(unit))
    return descriptor


def parse_entity_decl(entity_decl_string):
    """
    entity-decl is name[(dim1, ...)]
    """
    tokens = tokenize(entity_decl_string)
    units = split_token_units(tokens)
    assert len(units) == 1
    name, dimensions = parse_dimension(units[0])
    return name, dimensions


def parse_type(unit: TokenUnit) -> FortranType:
    """
    type_name ([param0,] param_name_1 = param_val_1, ... )
    """
    assert unit.first is not None and unit.first.type is TokenClass.WORD
    fortran_type = FortranType(name=unit.first.value, parameters={})

    is_intrinsic = True
    if unit.first.value in ('CLASS', 'TYPE', 'PROCEDURE'):
        is_intrinsic = False
        assert unit.parenthesis is not None and len(unit.parenthesis) > 0

    if unit.parenthesis is not None and len(unit.parenthesis) > 0:
        grouped_tokens = group_by_comma(unit.parenthesis)
        if not is_intrinsic:
            assert len(grouped_tokens[0]) == 1
            type_token = grouped_tokens[0][0]
            assert type_token.type is TokenClass.WORD
            fortran_type.subtype = type_token.value
            grouped_tokens = grouped_tokens[1:]
        for ts in grouped_tokens:
            param_name, param_value = parse_type_parameter(ts)
            if param_name is None:
                if fortran_type.name in ('CHARACTER', 'CHARACTER*'):
                    param_name = 'LEN'
                else:
                    param_name = 'KIND'
            fortran_type.parameters[param_name] = param_value
    return fortran_type


def parse_type_parameter(tokens: List[Token]) -> Tuple[Optional[str], str]:
    """
    param_name = param_val
    or
    param_val
    """
    if len(tokens) == 3:
        assert tokens[0].type is TokenClass.WORD
        assert tokens[1] == Token(type=TokenClass.CONTROL_CHAR, value='=')
        assert tokens[2].type is TokenClass.WORD
        return tokens[0].value, tokens[2].value
    elif len(tokens) == 1:
        assert tokens[0].type is TokenClass.WORD
        return None, tokens[0].value
    else:
        assert False


def parse_dimension(unit: TokenUnit) -> Tuple[Optional[str], Optional[List[str]]]:
    """
    As part of the descriptor:
    'dimension' ( dim1 , ... )
    As part of the parameter name:
    parameter_name ( dim1, ... )
    """
    assert unit.first.type is TokenClass.WORD

    dimensions = None
    if unit.parenthesis is not None:
        dimensions = []
        for ts in group_by_comma(unit.parenthesis):
            assert len(ts) == 1
            assert ts[0].type is TokenClass.WORD
            dimensions.append(ts[0].value)
    return unit.first.value, dimensions


def parse_intent(unit: TokenUnit) -> str:
    """
    intent ( in|out|inout )
    """
    assert unit.first == Token(type=TokenClass.WORD, value='INTENT')

    assert unit.parenthesis is not None and len(unit.parenthesis) == 1
    assert unit.parenthesis[0].type is TokenClass.WORD
    assert unit.parenthesis[0].value in ('IN', 'OUT', 'INOUT')
    return unit.parenthesis[0].value


def parse_attribute(unit: TokenUnit) -> str:
    """
    See Fortran Standard 2008: R502 attr-spec
    Not including
    'access-spec',
    'CODIMENSION lbracket coarray-spec rbracket',
    'language-binding-spec',
    which do not appear in MPI bindings.

    Also, not including 'DIMENSION ( array-spec )' and 'INTENT ( intent-spec )' attributes,
    which are treated separately by the parser.
    """
    attribute_keywords = ('ALLOCATABLE',
                          'ASYNCHRONOUS',
                          'CONTIGUOUS',
                          'EXTERNAL',
                          'INTRINSIC',
                          'OPTIONAL',
                          'PARAMETER',
                          'POINTER',
                          'PROTECTED',
                          'SAVE',
                          'TARGET',
                          'VALUE',
                          'VOLATILE'
                          )
    assert unit.first is not None
    assert unit.parenthesis is None
    assert unit.first.type is TokenClass.WORD
    assert unit.first.value in attribute_keywords
    return unit.first.value


def tokenize(fortran_code: str) -> List[Token]:
    """
    Splits a piece of fortran code into a sequence of tokens.
    Tokens can be either:
        - control characters: single character with syntactical meaning
        - string literals: Strings enclosed by either single or double quotes
        - words: Any sequence of characters delimited by one of the other token classes or whitespace
    :param fortran_code:
    :return: List of tokens. Each token is a tuple (<token_class>, <value>)
    """
    space_chars = ' \t\n'
    control_chars = ',()='
    quote_chars = '\'"'
    tokens: List[Token] = []
    token_buffer = ''
    inside_quote = False
    quote_char: Optional[str] = None
    previous_char: Optional[str] = None

    def flush_token_buffer(token_class: TokenClass) -> None:
        nonlocal token_buffer
        if token_class is TokenClass.STRING_LITERAL:
            tokens.append(Token(type=token_class, value=token_buffer))
            token_buffer = ''
        else:
            if token_buffer != '':
                tokens.append(Token(type=token_class, value=token_buffer.upper()))
                token_buffer = ''

    for char in fortran_code:
        if inside_quote:
            if char is quote_char:
                flush_token_buffer(TokenClass.STRING_LITERAL)
                inside_quote = False
            else:
                token_buffer = token_buffer + char
            previous_char = char
            continue

        if char in space_chars:
            flush_token_buffer(TokenClass.WORD)
        elif char in control_chars:
            flush_token_buffer(TokenClass.WORD)
            tokens.append(Token(type=TokenClass.CONTROL_CHAR, value=char))
        elif char in quote_chars:
            if previous_char == char:
                token_buffer = tokens.pop().value
                token_buffer += char
            else:
                flush_token_buffer(TokenClass.WORD)
            inside_quote = True
            quote_char = char
        else:
            token_buffer = token_buffer + char
        previous_char = char
    assert not inside_quote
    flush_token_buffer(TokenClass.WORD)
    return tokens
