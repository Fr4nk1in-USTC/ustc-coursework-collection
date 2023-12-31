"""
A bool query class that supports AND, OR, and NOT operations.

Grammar:
    Query  -> Term (OR Term)*
    Term   -> Factor (AND Factor)*
    Factor -> NOT* LPAREN Query RPAREN | NOT* KEYWORD
"""

import re
from enum import Enum
from typing import Optional

from inverted_index import InvertedIndex


class _FactorType(Enum):
    QUERY = 0
    KEYWORD = 1


class _Factor:
    """Factor -> NOT* LPAREN Query RPAREN | NOT* KEYWORD"""

    def __init__(
        self,
        type: _FactorType,
        inverse: bool = False,
        query: Optional["Query"] = None,
        keyword: Optional[str] = None,
    ) -> None:
        self._type = type
        self._query: Optional["Query"] = None
        self._keyword: Optional[str] = None
        self._inverse: bool = inverse
        match type:
            case _FactorType.QUERY:
                assert query is not None
                self._query = query
            case _FactorType.KEYWORD:
                assert keyword is not None
                self._keyword = keyword
            case _:
                raise ValueError("Invalid factor type")

    def __str__(self) -> str:
        match self._type:
            case _FactorType.QUERY:
                assert self._query is not None, "query is None"
                if self._inverse:
                    return f"NOT({str(self._query)})"
                return str(self._query)
            case _FactorType.KEYWORD:
                assert self._keyword is not None, "keyword is None"
                if self._inverse:
                    return f"NOT({self._keyword})"
                return self._keyword
            case _:
                raise ValueError("Invalid factor type")

    def query(self, index: InvertedIndex) -> set[int]:
        match self._type:
            case _FactorType.QUERY:
                assert self._query is not None, "query is None"
                if self._inverse:
                    return index.get_all() - self._query.query(index)
                return self._query.query(index)
            case _FactorType.KEYWORD:
                assert self._keyword is not None, "keyword is None"
                if self._inverse:
                    return index.get_all() - index.get(self._keyword)
                return index.get(self._keyword)
            case _:
                raise ValueError("Invalid factor type")

    def estimate_query_size(self, index: InvertedIndex) -> int:
        match self._type:
            case _FactorType.QUERY:
                assert self._query is not None, "query is None"
                if self._inverse:
                    return index.get_all_size() - self._query.estimate_query_size(index)
                return self._query.estimate_query_size(index)
            case _FactorType.KEYWORD:
                assert self._keyword is not None, "keyword is None"
                if self._inverse:
                    return index.get_all_size() - index.get_size(self._keyword)
                return index.get_size(self._keyword)
            case _:
                raise ValueError("Invalid factor type")


class _Term:
    """Term -> Factor (AND Factor)*"""

    def __init__(
        self,
        factors: list[_Factor],
    ) -> None:
        assert len(factors) > 0, "factors is empty"
        self._factors = factors

    def __str__(self) -> str:
        if len(self._factors) == 1:
            return str(self._factors[0])
        repr = "AND(" + str(self._factors[0])
        for factor in self._factors[1:]:
            repr += ", " + str(factor)
        return repr + ")"

    def query(self, index: InvertedIndex) -> set[int]:
        estimated_size = [
            (factor, factor.estimate_query_size(index)) for factor in self._factors
        ]
        estimated_size.sort(key=lambda x: x[1])
        doc_ids = estimated_size[0][0].query(index)
        for factor, _ in estimated_size[1:]:
            doc_ids &= factor.query(index)
        return doc_ids

    def estimate_query_size(self, index: InvertedIndex) -> int:
        results = [factor.estimate_query_size(index) for factor in self._factors]
        return min(results)


class Query:
    """Query -> Term (OR Term)*"""

    def __init__(
        self,
        terms: Optional[list["_Term"]] = None,
    ) -> None:
        assert terms is not None, "terms is None"
        self._terms = terms

    def __str__(self) -> str:
        if len(self._terms) == 1:
            return str(self._terms[0])
        repr = "OR(" + str(self._terms[0])
        for term in self._terms[1:]:
            repr += ", " + str(term)
        return repr + ")"

    def query(self, index: InvertedIndex) -> set[int]:
        doc_ids = set()
        for term in self._terms:
            doc_ids |= term.query(index)
        return doc_ids

    def estimate_query_size(self, index: InvertedIndex) -> int:
        upper_bound = index.get_all_size()
        results = [term.estimate_query_size(index) for term in self._terms]
        return min(sum(results), upper_bound)

    @classmethod
    def parse(cls, query: str) -> "Query":
        raw_tokens = query.split()
        tokens = []
        # parentheses: 'A(B' -> 'A' '(' 'B'
        for raw_token in raw_tokens:
            if raw_token != "" and "(" not in raw_token and ")" not in raw_token:
                tokens.append(raw_token)
                continue
            res = re.split(r"(\(|\))", raw_token)
            tokens.extend([token for token in res if token != ""])
        # token to node
        nodes: list[_Node] = []
        for token in tokens:
            match token:
                case "AND":
                    nodes.append(_AndNode())
                case "OR":
                    nodes.append(_OrNode())
                case "NOT":
                    nodes.append(_NotNode())
                case "(":
                    nodes.append(_LParenNode())
                case ")":
                    subquery = []
                    while not isinstance(nodes[-1], _LParenNode):
                        subquery.append(nodes.pop())
                    nodes.pop()
                    nodes.append(_SubQueryNode(subquery[::-1]))
                case _:
                    nodes.append(_KeywordNode(token))

        return _nodes_to_query(nodes)


class _NodeType(Enum):
    SUBQUERY = 0
    AND = 1
    OR = 2
    NOT = 3
    KEYWORD = 4
    LPAREN = 5


class _Node:
    def __init__(self, type: _NodeType) -> None:
        self._type = type


class _AndNode(_Node):
    def __init__(self) -> None:
        super().__init__(_NodeType.AND)


class _OrNode(_Node):
    def __init__(self) -> None:
        super().__init__(_NodeType.OR)


class _NotNode(_Node):
    def __init__(self) -> None:
        super().__init__(_NodeType.NOT)


class _SubQueryNode(_Node):
    def __init__(self, subquery: list[_Node]) -> None:
        super().__init__(_NodeType.SUBQUERY)
        self._subquery = subquery


class _KeywordNode(_Node):
    def __init__(self, keyword: str) -> None:
        super().__init__(_NodeType.KEYWORD)
        self._keyword = keyword


class _LParenNode(_Node):
    """Temporary node for parentheses. To be removed after parsing."""

    def __init__(self) -> None:
        super().__init__(_NodeType.LPAREN)


def _nodes_to_factor(nodes: list[_Node]) -> _Factor:
    if all(isinstance(node, _NotNode) for node in nodes[:-1]):
        inverse = len(nodes) % 2 == 0
        if isinstance(nodes[-1], _KeywordNode):
            return _Factor(
                _FactorType.KEYWORD, inverse=inverse, keyword=nodes[-1]._keyword
            )
        if isinstance(nodes[-1], _SubQueryNode):
            return _Factor(
                _FactorType.QUERY,
                inverse=inverse,
                query=_nodes_to_query(nodes[-1]._subquery),
            )
    raise ValueError("Invalid factor")


def _nodes_to_term(nodes: list[_Node]) -> _Term:
    # split by AND
    factors: list[_Factor] = []
    and_index = -1
    for i, node in enumerate(nodes):
        if isinstance(node, _AndNode):
            factors.append(_nodes_to_factor(nodes[and_index + 1 : i]))
            and_index = i
    if and_index == -1:
        return _Term([_nodes_to_factor(nodes)])
    return _Term(factors + [_nodes_to_factor(nodes[and_index + 1 :])])


def _nodes_to_query(nodes: list[_Node]) -> Query:
    terms: list[_Term] = []
    or_index = -1
    for i, node in enumerate(nodes):
        if isinstance(node, _OrNode):
            terms.append(_nodes_to_term(nodes[or_index + 1 : i]))
            or_index = i
    if or_index == -1:
        return Query([_nodes_to_term(nodes)])
    return Query(terms + [_nodes_to_term(nodes[or_index + 1 :])])


if __name__ == "__main__":
    while True:
        query = input("Please input your query: ")
        try:
            q = Query.parse(query)
            print(q)
        except Exception as e:
            print(e)
        stop_str = input("Continue? (Y/n) ").strip()
        if stop_str == "n":
            break
