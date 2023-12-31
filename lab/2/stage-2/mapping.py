import os
from typing import Generator

ENTITY_PREFIX = "<http://rdf.freebase.com/ns/"

ROOT_DIR = os.path.dirname(os.path.realpath(__file__))
DATA_DIR = os.path.join(ROOT_DIR, "data")
OUTPUT_DIR = os.path.join(DATA_DIR, "Douban")

# extracted knowledge graph in stage 1
RAW_KG_PATH = os.path.join(DATA_DIR, "kg_extracted.txt")
# movie ID -> knowledge graph entity
LINK_INFO_PATH = os.path.join(DATA_DIR, "douban2fb.txt")
# movie ID -> index
ID_MAP_PATH = os.path.join(DATA_DIR, "movie_id_map.txt")
# entity -> index
ENTITY_MAP_PATH = os.path.join(OUTPUT_DIR, "entity_map.txt")
# relation -> index
RELATION_MAP_PATH = os.path.join(OUTPUT_DIR, "relation_map.txt")
# Final KG
FINAL_KG_PATH = os.path.join(OUTPUT_DIR, "kg_final.txt")

NUM_MOVIES = 578

kg_t = set[tuple[str, str, str]]


def load_raw_kg() -> kg_t:
    with open(RAW_KG_PATH, "r") as f:
        kg = {tuple(line.strip().split()) for line in f}
    return kg  # type: ignore


def load_link_info() -> dict[str, str]:
    """movie id to entity"""
    id_to_entity = {}
    with open(LINK_INFO_PATH, "r") as f:
        for line in f:
            id, entity = line.strip().split("\t")
            entity = ENTITY_PREFIX + entity + ">"
            id_to_entity[id] = entity
    return id_to_entity


def load_id_map() -> dict[str, int]:
    """movie id to index"""
    id_to_index = {}
    with open(ID_MAP_PATH, "r") as f:
        for line in f:
            id, index = line.strip().split("\t")
            id_to_index[id] = int(index)
    return id_to_index


def map_kg(
    kg: kg_t, entity_to_index: dict[str, int], relation_to_index: dict[str, int]
) -> Generator[tuple[str, str, str], None, None]:
    for start, relation, end in kg:
        yield (
            str(entity_to_index[start]),
            str(relation_to_index[relation]),
            str(entity_to_index[end]),
        )


if __name__ == "__main__":
    raw_kg = load_raw_kg()
    id_to_entity = load_link_info()
    id_to_index = load_id_map()

    # map entity to index and relation to index
    entity_to_index = {}
    relation_to_index = {}
    for id, entity in id_to_entity.items():
        entity_to_index[entity] = id_to_index[id]
    entity_index = max(id_to_index.values()) + 1
    relation_index = 0
    for start, relation, end in raw_kg:
        if start not in entity_to_index:
            entity_to_index[start] = entity_index
            entity_index += 1
        if relation not in relation_to_index:
            relation_to_index[relation] = relation_index
            relation_index += 1
        if end not in entity_to_index:
            entity_to_index[end] = entity_index
            entity_index += 1

    # save mappings
    with open(ENTITY_MAP_PATH, "w") as f:
        for entity, index in entity_to_index.items():
            f.write(entity + " " + str(index) + "\n")
    with open(RELATION_MAP_PATH, "w") as f:
        for relation, index in relation_to_index.items():
            f.write(relation + " " + str(index) + "\n")

    # map knowledge graph and save
    with open(FINAL_KG_PATH, "w") as f:
        for triplet in map_kg(raw_kg, entity_to_index, relation_to_index):
            f.write(" ".join(triplet) + "\n")
