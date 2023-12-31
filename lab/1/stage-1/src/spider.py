import json
import os
import random
import time
from typing import Any, Optional

import numpy as np
import pandas
import requests
from lxml import html
from rich.progress import (BarColumn, MofNCompleteColumn, Progress,
                           TaskProgressColumn, TextColumn, TimeElapsedColumn,
                           TimeRemainingColumn)
from wasabi import msg

SHOW_SUCCESS = False

BASE_BOOK_URL = "https://book.douban.com/subject/"
BASE_MOVIE_URL = "https://movie.douban.com/subject/"
HEADERS = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
    "AppleWebKit/537.36 (KHTML, like Gecko) "
    "Chrome/118.0.0.0 Safari/537.36 Edg/118.0.2088.61"
}

SRC_DIR = os.path.dirname(__file__)
DATA_DIR = os.path.join(SRC_DIR, "..", "data")

COOKIES_PATH = os.path.join(DATA_DIR, "cookies.json")
BOOK_TAG_PATH = os.path.join(DATA_DIR, "book_tag.csv")
MOVIE_TAG_PATH = os.path.join(DATA_DIR, "movie_tag.csv")

BOOK_DATA_PATH = os.path.join(DATA_DIR, "book.json")
MOVIE_DATA_PATH = os.path.join(DATA_DIR, "movie.json")

BOOK_ERROR_PATH = os.path.join(DATA_DIR, "book_error.json")
MOVIE_ERROR_PATH = os.path.join(DATA_DIR, "movie_error.json")

BOOK_XPATHS = {
    "title": '//span[@property="v:itemreviewed"]/text()',
    "rate": '//strong[@property="v:average"]/text()',
    "author": '//span[contains(text(), "作者")]/following-sibling::*[1]/text()',
    "publisher": '//span[contains(text(), "出版社")]/following-sibling::*[1]/text()',
    "original_name": '//span[contains(text(), "原作名")]/following-sibling::text()[1]',
    "translator": '//span[contains(text(), "译者")]/following-sibling::a[1]/text()',
    "publishYear": '//span[contains(text(), "出版年")]/following-sibling::text()[1]',
    "pages": '//span[contains(text(), "页数")]/following-sibling::text()[1]',
    "price": '//span[contains(text(), "定价")]/following-sibling::text()[1]',
    "binding": '//span[contains(text(), "装帧")]/following-sibling::text()[1]',
    "series": '//span[contains(text(), "丛书")]/following-sibling::*[1]/text()',
    "isbn": '//span[contains(text(), "ISBN")]/following-sibling::text()[1]',
    "intro": [
        '//*[@id="link-report"]/span[2]/div/div//p/text()',
        '//*[@id="link-report"]/div/div//p/text()',
    ],
    "author_intro": [
        '//*[@id="content"]/div/div[1]/div[3]/div[3]/div/div//p/text()',
        '//*[@id="content"]/div/div[1]/div[3]/div[4]/div/div//p/text()',
        '//*[@id="content"]/div/div[1]/div[3]/div[3]/span[2]/div//p/text()',
        '//*[@id="content"]/div/div[1]/div[3]/div[4]/span[2]/div//p/text()',
    ],
}

MOVIE_XPATHS = {
    "title": '//span[@property="v:itemreviewed"]/text()',
    "rate": '//strong[@property="v:average"]/text()',
    "director": '//*[@id="info"]/span[1]/span[2]//a/text()',
    "screenwriter": '//*[@id="info"]/span[2]/span[2]//a/text()',
    "starring": '//*[@id="info"]/span[3]/span[2]//a/text()',
    "type": '//span[@property="v:genre"]/text()',
    "country": '//span[contains(text(), "制片国家/地区")]/following-sibling::text()[1]',
    "language": '//span[contains(text(), "语言")]/following-sibling::text()[1]',
    "date": '//span[@property="v:initialReleaseDate"]/text()',
    "runtime": '//span[@property="v:runtime"]/text()',
    "aka": '//span[contains(text(), "又名")]/following-sibling::text()[1]',
    "imdb": '//span[contains(text(), "IMDb")]/following-sibling::text()[1]',
    "movie_intro": [
        '//span[@property = "v:summary"]/text()',
        '//*[@id="link-report-intra"]/span[1]/text()',
    ],
    "celebrity_intro": '//*[@id="celebrities"]/ul//a/text()',
}

PROGRESS_BAR = Progress(
    TextColumn("[progress.description]{task.description}"),
    BarColumn(),
    MofNCompleteColumn(),
    TextColumn("•"),
    TaskProgressColumn(),
    TextColumn("•"),
    TimeElapsedColumn(),
    TextColumn("<"),
    TimeRemainingColumn(),
)


def load_cookies() -> dict[str, str]:
    with open(COOKIES_PATH, "r") as f:
        cookies_raw = json.load(f)
    cookies = {}
    for cookie in cookies_raw:
        cookies[cookie["name"]] = cookie["value"]
    return cookies


def sleep() -> None:
    """Random sleep for 2 - 5 seconds to avoid being blocked."""
    time.sleep(random.randint(2, 5))


def get_book_info(
    book_id: int, cookies: Optional[dict[str, str]] = None
) -> Optional[dict[str, Any]]:
    # send request and get response
    url = BASE_BOOK_URL + str(book_id)

    try:
        response = requests.get(url, headers=HEADERS, cookies=cookies)
    except requests.exceptions.ConnectionError:
        msg.fail(f"(Connection Error) {url}")
        return None

    if response.status_code != 200:
        msg.fail(f"({response.status_code}) {url}")
        return None
    else:
        msg.good(f"({response.status_code}) {url}", show=SHOW_SUCCESS)

    # parse html
    tree = html.fromstring(response.text)
    info: dict[str, Any] = {"id": book_id}
    for k, v in BOOK_XPATHS.items():
        if isinstance(v, list):
            for xpath in v:
                info[k] = [
                    info.strip() for info in tree.xpath(xpath) if info.strip() != ""
                ]
                if len(info[k]) != 0:
                    break
        else:
            info[k] = [info.strip() for info in tree.xpath(v) if info.strip() != ""]
    return info


def get_movie_info(
    movie_id: int, cookies: Optional[dict[str, str]] = None
) -> Optional[dict[str, Any]]:
    # send request and get response
    url = BASE_MOVIE_URL + str(movie_id)

    try:
        response = requests.get(url, headers=HEADERS, cookies=cookies)
    except requests.exceptions.ConnectionError:
        msg.fail(f"(Connection Error) {url}")
        return None

    if response.status_code != 200:
        msg.fail(f"({response.status_code}) {url}")
        return None
    else:
        msg.good(f"({response.status_code}) {url}", show=SHOW_SUCCESS)

    # parse html
    tree = html.fromstring(response.text)
    info: dict[str, Any] = {"id": movie_id}
    for k, v in MOVIE_XPATHS.items():
        if isinstance(v, list):
            for xpath in v:
                info[k] = [
                    info.strip() for info in tree.xpath(xpath) if info.strip() != ""
                ]
                if len(info[k]) != 0:
                    break
        else:
            info[k] = [info.strip() for info in tree.xpath(v) if info.strip() != ""]
    return info


def scrape_books(cookies: Optional[dict[str, str]] = None) -> list[str]:
    book_ids = pandas.read_csv(BOOK_TAG_PATH)
    book_datas = []
    num_books = book_ids.shape[0]
    errors = []
    with PROGRESS_BAR as p:
        for _, book_id in p.track(
            book_ids.iterrows(),
            description="Scraping books...",
            total=num_books,
        ):
            id = int(book_id["Id"])
            info = get_book_info(id, cookies)
            if info is not None:
                tag = book_id["Tag"]
                info["tags"] = str(tag).split(",") if tag == np.nan else []
                book_datas.append(info)
            else:
                errors.append(id)
            sleep()

    with open(BOOK_DATA_PATH, "w") as f:
        json.dump(book_datas, f, ensure_ascii=False, indent=2)

    return errors


def scrape_movies(cookies: Optional[dict[str, str]] = None) -> list[str]:
    movie_ids = pandas.read_csv(MOVIE_TAG_PATH)
    movie_datas = []
    num_movies = movie_ids.shape[0]
    errors = []
    with PROGRESS_BAR as p:
        for _, movie_id in p.track(
            movie_ids.iterrows(), description="Scraping movies...", total=num_movies
        ):
            id = int(movie_id["Id"])
            info = get_movie_info(id, cookies)
            if info is not None:
                tag = movie_id["Tag"]
                info["tags"] = str(tag).split(",") if tag == np.nan else []
                movie_datas.append(info)
            else:
                errors.append(id)
            sleep()

    with open(MOVIE_DATA_PATH, "w") as f:
        json.dump(movie_datas, f, ensure_ascii=False, indent=2)

    return errors


if __name__ == "__main__":
    cookies = load_cookies()

    error_books = scrape_books(cookies)
    if len(error_books) != 0:
        msg.warn(f"Failed to scrape {len(error_books)} books.")
        with open(BOOK_ERROR_PATH, "w") as f:
            json.dump(error_books, f, ensure_ascii=False, indent=2)

    error_movies = scrape_movies(cookies)
    if len(error_movies) != 0:
        msg.warn(f"Failed to scrape {len(error_movies)} movies.")
        with open(MOVIE_ERROR_PATH, "w") as f:
            json.dump(error_movies, f, ensure_ascii=False, indent=2)
