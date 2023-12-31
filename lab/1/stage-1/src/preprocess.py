import json
import os
from typing import Any

import jieba

from inverted_index import InvertedIndex

SRC_DIR = os.path.dirname(__file__)
DATA_DIR = os.path.join(SRC_DIR, "..", "data")
DATABASE_DIR = os.path.join(DATA_DIR, "database")

BOOK_DATABASE_DIR = os.path.join(DATABASE_DIR, "book")
MOVIE_DATABASE_DIR = os.path.join(DATABASE_DIR, "movie")

BOOK_DATA_PATH = os.path.join(DATA_DIR, "book.json")
MOVIE_DATA_PATH = os.path.join(DATA_DIR, "movie.json")
STOPWORDS_PATH = os.path.join(DATA_DIR, "stopwords.txt")

USEFUL_MOVIE_ENTRIES = ["title", "type", "country", "language", "movie_intro", "tags"]
USEFUL_BOOK_ENTRIES = ["title", "author", "publisher", "series", "intro", "tags"]


def load_stopwords():
    stopwords: set[str] = set()
    with open(STOPWORDS_PATH, "r") as file:
        for line in file:
            stopwords.add(line.strip())
    return stopwords


def load_movie_data() -> list[dict[str, Any]]:
    with open(MOVIE_DATA_PATH, "r") as file:
        return json.load(file)


def load_book_data() -> list[dict[str, Any]]:
    with open(BOOK_DATA_PATH, "r") as file:
        return json.load(file)


def movie_keyword_generator():
    movies = load_movie_data()
    stopwords = load_stopwords()
    for data in movies:
        id = int(data["id"])
        keywords = set()
        for entry in USEFUL_MOVIE_ENTRIES:
            for string in data[entry]:
                tokens = [token.strip() for token in jieba.cut(string, cut_all=False)]
                keywords |= set(tokens)
        keywords -= stopwords
        yield id, keywords


def book_keyword_generator():
    books = load_book_data()
    stopwords = load_stopwords()
    for data in books:
        id = int(data["id"])
        keywords = set()
        for entry in USEFUL_BOOK_ENTRIES:
            for string in data[entry]:
                tokens = [token.strip() for token in jieba.cut(string, cut_all=False)]
                keywords |= set(tokens)
        keywords -= stopwords
        yield id, keywords


if __name__ == "__main__":
    movie_keywords_to_id = {}
    for movie_id, keywords in movie_keyword_generator():
        for keyword in keywords:
            if keyword not in movie_keywords_to_id:
                movie_keywords_to_id[keyword] = set()
            movie_keywords_to_id[keyword].add(movie_id)

    movie_index = InvertedIndex(MOVIE_DATABASE_DIR)
    for keyword, movie_ids in movie_keywords_to_id.items():
        movie_index.set(keyword, movie_ids)
    movie_index.save()

    book_keywords_to_id = {}
    for book_id, keywords in book_keyword_generator():
        for keyword in keywords:
            if keyword not in book_keywords_to_id:
                book_keywords_to_id[keyword] = set()
            book_keywords_to_id[keyword].add(book_id)

    book_index = InvertedIndex(BOOK_DATABASE_DIR)
    for keyword, book_ids in book_keywords_to_id.items():
        book_index.set(keyword, book_ids)
    book_index.save()
