import json
import os
import signal
import sys

from bool_query import Query
from inverted_index import InvertedIndex

BASE_BOOK_URL = "https://book.douban.com/subject/"
BASE_MOVIE_URL = "https://movie.douban.com/subject/"


SRC_DIR = os.path.dirname(__file__)
DATA_DIR = os.path.join(SRC_DIR, "..", "data")
DATABASE_DIR = os.path.join(DATA_DIR, "database")

BOOK_DATABASE_DIR = os.path.join(DATABASE_DIR, "book")
MOVIE_DATABASE_DIR = os.path.join(DATABASE_DIR, "movie")

BOOK_DATA_PATH = os.path.join(DATA_DIR, "book.json")
MOVIE_DATA_PATH = os.path.join(DATA_DIR, "movie.json")


with open(MOVIE_DATA_PATH, "r") as file:
    movie_data = json.load(file)
with open(BOOK_DATA_PATH, "r") as file:
    book_data = json.load(file)

movie_database = InvertedIndex.load(MOVIE_DATABASE_DIR)
book_database = InvertedIndex.load(BOOK_DATABASE_DIR)


def signal_handler(sig, frame):
    print("\nexiting...")
    sys.exit(0)


def display_results(books, movies):
    if len(books) == 0:
        print("No book found")
    else:
        print("Found books:")
        book_infos = []
        count = 0
        for book_info in book_data:
            if book_info["id"] in books:
                count += 1
                book_infos.append(book_info)
            if count == len(books):
                break
        for book_info in book_infos:
            id = book_info["id"]
            author = book_info["author"]
            title = book_info["title"]
            if len(title) == 0:
                title = "Unknown"
            else:
                title = title[0]
            if len(author) == 0:
                author = "Unknown"
            else:
                author = author[0].replace(" ", "").replace("\n", "")
            print(f"- 《{title}》 by {author}: {BASE_BOOK_URL}{id}")

    if len(movies) == 0:
        print("No movie found")
    else:
        print("Found movies:")
        movie_infos = []
        count = 0
        for movie_info in movie_data:
            if movie_info["id"] in movies:
                count += 1
                movie_infos.append(movie_info)
            if count == len(movies):
                break
        for movie_info in movie_infos:
            id = movie_info["id"]
            title = movie_info["title"]
            type = ", ".join(movie_info["type"])
            if len(title) == 0:
                title = "Unknown"
            else:
                title = title[0]
            print(f"- 《{title}》({type}): {BASE_MOVIE_URL}{id}")


if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    print("Example query: A AND B AND NOT C / (A OR B) AND C")
    while True:
        query_str = input("Query (Press CTRL+C to exit): ")
        try:
            query = Query.parse(query_str)
        except Exception as e:
            print("Invalid query:", e)
            continue
        query_books = query.query(book_database)
        query_movies = query.query(movie_database)
        display_results(query_books, query_movies)
        print("--------------------------------------------")
