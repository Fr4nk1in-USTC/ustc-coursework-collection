import gzip
import os
from typing import Callable, Iterable

try:
    import tqdm
except ImportError:
    tqdm = None


def process(iters: Iterable, total: int = 0):
    if tqdm is None:
        return iters
    return tqdm.tqdm(iters, total=total)


ENTITY_PREFIX = "<http://rdf.freebase.com/ns/"

ROOT_DIR = os.path.dirname(os.path.realpath(__file__))
DATA_DIR = os.path.join(ROOT_DIR, "data")

# provided knowledge graph
KG_PATH = os.path.join(DATA_DIR, "freebase_douban.gz")
KG_SIZE = 395577070
# map douban movie ID to knowledge graph entity
LINK_INFO_PATH = os.path.join(DATA_DIR, "douban2fb.txt")
NUM_LINKS = 578
# movie ID & tag
MOVIE_ID_PATH = os.path.join(DATA_DIR, "Movie_id.csv")
MOVIE_TAG_PATH = os.path.join(DATA_DIR, "Movie_tag.csv")
# knowledge graph
OUTPUT_KG_PATH = os.path.join(DATA_DIR, "kg_extracted.txt")


kg_entry_t = tuple[str, str, str]
kg_t = set[kg_entry_t]


def load_movie_entities() -> set[str]:
    # load movie entities
    entities = set()
    with open(LINK_INFO_PATH, "r") as f:
        for line in process(f, NUM_LINKS):
            line = line.strip()
            entities.add(f"{ENTITY_PREFIX}{line.split()[-1]}>")
    return entities


def extract_subgraph(entities: set[str]) -> kg_t:
    # load knowledge graph with only provided entities
    kg = set()
    with gzip.open(KG_PATH, "rb") as f:
        for line in process(f, KG_SIZE):
            line = line.strip()
            triplet = tuple(line.decode().split()[:3])
            if triplet[0] in entities and triplet[2].startswith(ENTITY_PREFIX):
                kg.add(triplet)
    return kg


def hop(kg: kg_t):
    entities = {triplet[0] for triplet in kg}
    entities.update({triplet[2] for triplet in kg})
    return extract_subgraph(entities)


def kg_info(kg: kg_t):
    """get the entity and relation count of a knowledge graph"""
    entity_count: dict[str, int] = {}
    relation_count: dict[str, int] = {}
    for start, relation, end in kg:
        entity_count[start] = entity_count.get(start, 0) + 1
        entity_count[end] = entity_count.get(end, 0) + 1
        relation_count[relation] = relation_count.get(relation, 0) + 1
    return entity_count, relation_count


def filter(
    kg: kg_t,
    filter_fn: Callable[[kg_entry_t], bool],
):
    filtered = set()
    for triplet in process(kg, len(kg)):
        if filter_fn(triplet):
            filtered.add(triplet)
    return filtered


if __name__ == "__main__":
    print("Loading movie entities...")
    movie_entities = load_movie_entities()
    print("Done, size:", len(movie_entities))

    # first hop
    print("\nFirst hop...")
    first_hop = extract_subgraph(movie_entities)
    print("Done, size:", len(first_hop))
    print("Filtering first hop...")
    entity_count, relation_count = kg_info(first_hop)
    first_hop = filter(
        first_hop,
        lambda triplet: entity_count[triplet[0]] > 20
        and entity_count[triplet[2]] > 20
        and relation_count[triplet[1]] > 50,
    )
    entity_count, relation_count = kg_info(first_hop)
    print(
        f"Done, {len(entity_count.keys())} entities, "
        f"{len(relation_count.keys())} relations, size: {len(first_hop)}"
    )

    # second hop
    print("\nSecond hop...")
    second_hop = hop(first_hop)
    print("Done, size:", len(second_hop))
    print("Filtering second hop...")
    entity_count, relation_count = kg_info(second_hop)
    second_hop = filter(
        second_hop,
        lambda triplet: entity_count[triplet[0]] < 20000
        and entity_count[triplet[2]] < 20000
        and relation_count[triplet[1]] > 50,
    )
    entity_count, relation_count = kg_info(second_hop)
    second_hop = filter(
        second_hop,
        lambda triplet: entity_count[triplet[0]] > 15
        and entity_count[triplet[2]] > 15
        and relation_count[triplet[1]] > 50,
    )
    entity_count, relation_count = kg_info(second_hop)
    print(
        f"Done, {len(entity_count.keys())} entities, "
        f"{len(relation_count.keys())} relations, size: {len(second_hop)}"
    )

    # save extracted knowledge graph
    print("\nSaving extracted knowledge graph...")
    with open(OUTPUT_KG_PATH, "w") as f:
        for triplet in process(second_hop):
            f.write(" ".join(triplet) + "\n")
