from __future__ import annotations

from collections import OrderedDict
from typing import TypeVar, Iterable

KeyType = TypeVar('KeyType')
ValueType = TypeVar('ValueType')


class OrderedAttrDict(OrderedDict[KeyType, ValueType]):
    def __getattr__(self, name: KeyType) -> ValueType:
        return self[name]

    @staticmethod
    def from_named_items(*args) -> 'OrderedAttrDict[KeyType, ValueType]':
        if len(args) == 1 and isinstance(args[0], Iterable):
            values = args[0]
        else:
            values = args
        d: OrderedAttrDict[KeyType, ValueType] = OrderedAttrDict()
        d.update(((item.name, item) for item in values))
        return d


Scope = OrderedAttrDict[str, ValueType]
