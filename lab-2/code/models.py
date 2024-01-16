import torch
from logger import get_logger
from transformers import (BertForSequenceClassification, BertTokenizerFast,
                          PreTrainedTokenizerBase)

LOGGER = get_logger(__name__)


def new_model(
    num_labels: int, device: torch.device
) -> tuple[BertForSequenceClassification, PreTrainedTokenizerBase]:
    model = BertForSequenceClassification.from_pretrained(
        "bert-base-chinese", num_labels=num_labels
    ).to(device)
    tokenizer = BertTokenizerFast.from_pretrained("bert-base-chinese")
    return model, tokenizer


def load_model(
    model_path: str, device: torch.device
) -> tuple[BertForSequenceClassification, PreTrainedTokenizerBase]:
    model = torch.load(model_path)
    if not isinstance(
        model,
    ):
        raise TypeError(f"Expect BertClassifyModel, got {type(model)}")
    tokenizer = BertTokenizerFast.from_pretrained("bert-base-chinese")
    return model.to(device), tokenizer
