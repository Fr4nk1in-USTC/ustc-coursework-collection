import os
import pickle
from dataclasses import dataclass
from io import BufferedReader, BufferedWriter
from typing import Optional


class InvertedRecord:
    def __init__(self, doc_ids: set[int]) -> None:
        self.doc_ids = doc_ids

    @classmethod
    def load(
        cls, filepath: Optional[str] = None, file: Optional[BufferedReader] = None
    ) -> "InvertedRecord":
        if filepath is not None:
            return cls(pickle.load(open(filepath, "rb")))
        elif file is not None:
            return cls(pickle.load(file))
        else:
            raise ValueError("filepath and file are both None")

    def dump(self, filepath: Optional[str], file: Optional[BufferedWriter]) -> None:
        if filepath is not None:
            pickle.dump(self.doc_ids, open(filepath, "wb"))
        elif file is not None:
            pickle.dump(self.doc_ids, file)
        else:
            raise ValueError("filepath and file are both None")


@dataclass
class InvertedIndexEntry:
    size: int
    record: Optional[InvertedRecord] = None


class InvertedIndex:
    def __init__(self, dir: str) -> None:
        self.indices: dict[str, InvertedIndexEntry] = {}
        self.dir = dir
        self.all_ids: Optional[set] = set()  # lazy load
        if not os.path.exists(f"{dir}/index"):
            os.makedirs(f"{dir}/index", exist_ok=True)

    def __len__(self) -> int:
        return len(self.indices)

    def __contains__(self, keyword: str) -> bool:
        return keyword in self.indices

    def get_all(self) -> set[int]:
        if self.all_ids is None:
            with open(f"{self.dir}/all_ids", "rb") as file:
                self.all_ids = pickle.load(file)
        return self.all_ids

    def get_all_size(self) -> int:
        return len(self.get_all())

    def get(self, keyword: str) -> set[int]:
        if keyword not in self.indices:
            return set()
        record = self.indices[keyword].record
        if record is None:
            return InvertedRecord.load(f"{self.dir}/index/{keyword}.idx").doc_ids
        return record.doc_ids

    def get_size(self, keyword: str) -> int:
        if keyword not in self.indices:
            return 0
        return self.indices[keyword].size

    def create_entry(self, keyword: str) -> InvertedIndexEntry:
        if keyword in self.indices:
            raise ValueError(f"keyword {keyword} already exists")
        entry = InvertedIndexEntry(size=0)
        self.indices[keyword] = entry
        return entry

    def add(self, keyword: str, doc_id: int):
        if self.all_ids is None:
            with open(f"{self.dir}/all_ids", "rb") as file:
                self.all_ids = pickle.load(file)
        self.all_ids.add(doc_id)
        if keyword not in self.indices:
            entry = self.create_entry(keyword)
            with open(f"{self.dir}/index/{keyword}.idx", "wb") as file:
                InvertedRecord(set([doc_id])).dump(None, file)
            entry.size = 1
            return

        record = self.indices[keyword].record
        if record is None:
            # Load and update the record, very inefficient
            with open(f"{self.dir}/index/{keyword}.idx", "rb+") as file:
                record = InvertedRecord.load(file=file)
                record.doc_ids.add(doc_id)
                self.indices[keyword].size = len(record.doc_ids)
                record.dump(None, file)
            return
        record.doc_ids.add(doc_id)
        self.indices[keyword].size = len(record.doc_ids)

    def set(self, keyword: str, doc_ids: set[int]):
        if self.all_ids is None:
            with open(f"{self.dir}/all_ids", "rb") as file:
                self.all_ids = pickle.load(file)
        self.all_ids |= doc_ids

        if keyword not in self.indices:
            entry = self.create_entry(keyword)
            with open(f"{self.dir}/index/{keyword}.idx", "wb") as file:
                InvertedRecord(doc_ids).dump(None, file)
            entry.size = len(doc_ids)
            return

        record = self.indices[keyword].record
        if record is None:
            with open(f"{self.dir}/index/{keyword}.idx", "rb+") as file:
                record = InvertedRecord.load(file=file)
                record.doc_ids |= doc_ids
                self.indices[keyword].size = len(record.doc_ids)
                record.dump(None, file)
            return

        record.doc_ids |= doc_ids
        self.indices[keyword].size = len(record.doc_ids)

    def load_record(self, keyword: str):
        if keyword not in self.indices:
            raise ValueError(f"keyword {keyword} does not exist")
        entry = self.indices[keyword]
        entry.record = InvertedRecord.load(f"{self.dir}/index/{keyword}.idx")

    def save_record(self, keyword: str):
        entry = self.indices[keyword]
        if entry.record is None:
            raise ValueError("record is None")
        entry.record.dump(f"{self.dir}/index/{keyword}.idx", None)

    @classmethod
    def load(cls, dir: str, blocking: Optional[int] = None) -> "InvertedIndex":
        index = cls(dir)
        index.all_ids = None

        keyword_to_size: dict[str, int] = {}

        if blocking is None:
            with open(f"{dir}/indices", "rb") as file:
                keyword_to_size = pickle.load(file)
        else:
            keywords: str = ""
            with open(f"{dir}/keywords", "rb") as file:
                keywords = pickle.load(file)
            compressed_indices: dict[int, list[int]] = {}
            with open(f"{dir}/indices_compressed_blocking_{blocking}", "rb") as file:
                compressed_indices = pickle.load(file)
            for block_offset, sizes in compressed_indices.items():
                in_block_offset = 0
                for size in sizes:
                    offset = block_offset + in_block_offset
                    keyword_length = ord(keywords[offset])
                    keyword = keywords[offset + 1 : offset + 1 + keyword_length]
                    keyword_to_size[keyword] = size
                    in_block_offset += keyword_length + 1
        for keyword, size in keyword_to_size.items():
            index.indices[keyword] = InvertedIndexEntry(size=size, record=None)
        return index

    def save(self, blocking: Optional[int] = None):
        keyword_to_size = {}
        for keyword, entry in self.indices.items():
            keyword_to_size[keyword] = entry.size

        if blocking is None:
            with open(f"{self.dir}/indices", "wb") as file:
                pickle.dump(keyword_to_size, file)
        else:
            # transform indices into compressed blocks
            block_offset = 0
            block_size = 0
            keywords = ""
            compressed_indices: dict[int, list[int]] = {}
            index = 0
            for keyword, size in keyword_to_size.items():
                if index % blocking == 0:
                    block_offset += block_size
                    compressed_indices[block_offset] = []
                    block_size = 0
                keywords += chr(len(keyword)) + keyword
                block_size += len(keyword) + 1
                compressed_indices[block_offset].append(size)
                index += 1
            with open(f"{self.dir}/keywords", "wb") as file:
                pickle.dump(keywords, file)
            with open(
                f"{self.dir}/indices_compressed_blocking_{blocking}", "wb"
            ) as file:
                pickle.dump(compressed_indices, file)

        if self.all_ids is not None:
            with open(f"{self.dir}/all_ids", "wb") as file:
                pickle.dump(self.all_ids, file)
