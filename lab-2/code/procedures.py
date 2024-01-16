from typing import Optional

import torch
import torch.nn as nn
from torch.utils.data import DataLoader
from transformers import BertForSequenceClassification

# isort: off
from data_loader import TrainDataLoaders
from early_stopping import EarlyStopping
from logger import get_logger

LOGGER = get_logger(__file__)


def __train_one_epoch(
    model: BertForSequenceClassification,
    data_loader: DataLoader,
    optimizer: torch.optim.Optimizer,
    device: torch.device,
) -> tuple[float, float]:
    model.train()
    train_loss = 0.0
    corrects = 0
    for batch in data_loader:
        input_ids = batch["input_ids"].to(device)
        token_type_ids = batch["token_type_ids"].to(device)
        attention_mask = batch["attention_mask"].to(device)
        labels = batch["label"].to(device)
        optimizer.zero_grad()
        outputs = model(
            input_ids,
            token_type_ids=token_type_ids,
            attention_mask=attention_mask,
            labels=labels.unsqueeze(1),
        )
        outputs.loss.backward()
        optimizer.step()
        preds = torch.argmax(outputs.logits, dim=1)
        train_loss += outputs.loss.item() * input_ids.size(0)
        corrects += torch.sum(preds == labels)
    train_loss /= len(data_loader.dataset)
    train_acc = corrects.float() / len(data_loader.dataset)
    return train_loss, train_acc


def __val_one_epoch(
    model: BertForSequenceClassification,
    data_loader: DataLoader,
    device: torch.device,
) -> tuple[float, float]:
    model.eval()
    val_loss = 0.0
    corrects = 0
    for batch in data_loader:
        input_ids = batch["input_ids"].to(device)
        token_type_ids = batch["token_type_ids"].to(device)
        attention_mask = batch["attention_mask"].to(device)
        labels = batch["label"].to(device)
        outputs = model(
            input_ids,
            token_type_ids=token_type_ids,
            attention_mask=attention_mask,
            labels=labels.unsqueeze(1),
        )
        preds = torch.argmax(outputs.logits, dim=1)
        val_loss += outputs.loss.item() * input_ids.size(0)
        corrects += torch.sum(preds == labels)
    val_loss /= len(data_loader.dataset)
    val_acc = corrects.float() / len(data_loader.dataset)
    return val_loss, val_acc


def train(
    model: nn.Module,
    n_epochs: int,
    data_loaders: TrainDataLoaders,
    learning_rate: float,
    early_stopping: Optional[EarlyStopping],
) -> tuple[list[float], list[float]]:
    LOGGER.info("Start training...")
    epoch_len = len(str(n_epochs))

    LOGGER.debug(f"Using AdamW optimizer with lr={learning_rate}")
    params = model.named_parameters()
    no_decay = ["bias", "LayerNorm.weight", "LayerNorm.bias"]
    optimizer_grouped_parameters = [
        {
            "params": [
                p for n, p in params if not any(nd in n for nd in no_decay)
            ],
            "weight_decay": 0.01,
        },
        {
            "params": [p for n, p in params if any(nd in n for nd in no_decay)],
            "weight_decay": 0,
        },
    ]
    optimizer = torch.optim.AdamW(
        optimizer_grouped_parameters, lr=learning_rate
    )

    device = next(model.parameters()).device

    train_losses = []
    val_losses = []

    for epoch in range(n_epochs):
        train_loss, train_acc = __train_one_epoch(
            model, data_loaders.train, optimizer, device
        )
        train_losses.append(train_loss)

        val_loss, val_acc = __val_one_epoch(model, data_loaders.val, device)
        val_losses.append(val_loss)
        # logging
        LOGGER.info(
            f"[{epoch + 1:>{epoch_len}}/{n_epochs}] "
            f"training loss: {train_loss:.5f}, "
            f"training accuracy: {train_acc:2.2%}, "
            f"validation loss: {val_loss:.5f}, "
            f"validation accuracy: {val_acc:2.2%}"
        )
        # early stopping
        if early_stopping is None:
            continue
        early_stopping(val_loss, model)
        if early_stopping.early_stop:
            LOGGER.info("Early stopping")
            break
    return train_losses, val_losses


def test(model: nn.Module, data_loader: DataLoader) -> tuple[float, float]:
    LOGGER.info("Start testing...")
    device = next(model.parameters()).device
    test_loss, test_acc = __val_one_epoch(model, data_loader, device)
    LOGGER.info(f"Test loss: {test_loss:.5f}, test accuracy: {test_acc:2.2%}")
    return test_loss, test_acc
