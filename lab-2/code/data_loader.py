import os
from dataclasses import dataclass

import pandas as pd
import torch
from datasets import Dataset
from logger import get_logger
from torch.utils.data import DataLoader, random_split
from transformers import PreTrainedTokenizerBase

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(FILE_DIR)
DATA_DIR = os.path.relpath(os.path.join(ROOT_DIR, "data"))

TRAIN_XLSX_PATH = os.path.join(DATA_DIR, "train.xlsx")
TEST_XLSX_PATH = os.path.join(DATA_DIR, "test.xlsx")

LOGGER = get_logger(__file__)


@dataclass
class TrainDataLoaders:
    train: DataLoader
    val: DataLoader


def get_train_data_loaders(
    tokenizer: PreTrainedTokenizerBase,
    batch_size: int = 32,
    val_ratio: float = 0.2,
    max_length: int = 30,
) -> TrainDataLoaders:
    """
    Get train data loaders
    Each batch is a dict with the following keys:
    - input_ids
    - token_type_ids
    - attention_mask
    - label
    """
    if not 0 <= val_ratio <= 1:
        raise ValueError(f"Invalid validation set ratio: {val_ratio}")
    LOGGER.info("Generating training data loaders...")
    LOGGER.debug(f"Batch size: {batch_size}")
    # load from xlsx
    if not os.path.exists(TRAIN_XLSX_PATH):
        raise FileNotFoundError(f"Cannot find {TRAIN_XLSX_PATH}")
    df = pd.read_excel(TRAIN_XLSX_PATH)
    LOGGER.debug(f"Found {len(df)} training samples")
    # tokenize inputs
    encoded_inputs = tokenizer(
        df["数据"].tolist(),
        padding=True,
        truncation=True,
        max_length=max_length,
        return_attention_mask=True,
        return_tensors="pt",
    )
    encoded_inputs["label"] = torch.tensor(df["标签"].tolist())
    # generate datasets
    dataset = Dataset.from_dict(encoded_inputs).with_format("torch")
    LOGGER.debug("Training samples tokenized")
    train_set, val_set = random_split(dataset, [(1 - val_ratio), val_ratio])
    LOGGER.debug(f"Training set size: {len(train_set)}")
    LOGGER.debug(f"Validation set size: {len(val_set)}")

    # data loaders
    data_loaders = TrainDataLoaders(
        DataLoader(train_set, batch_size=batch_size, shuffle=True),
        DataLoader(val_set, batch_size=batch_size, shuffle=False),
    )
    LOGGER.info("Training data loaders generated")
    return data_loaders


def get_test_data_loader(
    tokenizer: PreTrainedTokenizerBase,
    batch_size: int = 32,
    max_length: int = 30,
) -> DataLoader:
    """
    Each batch is a dict with the following keys:
    - input_ids
    - token_type_ids
    - attention_mask
    - label
    """
    LOGGER.info("Generating testing data loaders...")
    LOGGER.debug(f"Batch size: {batch_size}")
    # load from xlsx
    if not os.path.exists(TEST_XLSX_PATH):
        raise FileNotFoundError(f"Cannot find {TEST_XLSX_PATH}")
    df = pd.read_excel(TEST_XLSX_PATH)
    LOGGER.debug(f"Found {len(df)} testing samples")
    # tokenize inputs
    encoded_inputs = tokenizer(
        df["数据"].tolist(),
        padding=True,
        truncation=True,
        max_length=max_length,
        return_attention_mask=True,
        return_tensors="pt",
    )
    encoded_inputs["label"] = torch.tensor(df["标签"].tolist())
    # generate datasets
    dataset = Dataset.from_dict(encoded_inputs).with_format("torch")
    LOGGER.debug("Testing samples tokenized")
    # data loaders
    data_loader = DataLoader(dataset, batch_size=batch_size, shuffle=False)
    LOGGER.info("Testing data loaders generated")
    return data_loader


if __name__ == "__main__":
    from transformers import BertTokenizer

    tokenizer = BertTokenizer.from_pretrained("bert-base-chinese")
    train_data_loaders = get_train_data_loaders(tokenizer)
    test_data_loaders = get_test_data_loader()
