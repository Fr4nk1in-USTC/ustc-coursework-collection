from typing import Optional

import torch
import torch.nn as nn
from torch.utils.data import DataLoader

# isort: off
from data_loader import DataLoaders
from early_stopping import EarlyStopping
from logger import get_logger

LOGGER = get_logger(__file__)


def __train_one_epoch(
    model: nn.Module,
    data_loader: DataLoader,
    loss_fn: nn.Module,
    optimizer: torch.optim.Optimizer,
    device: torch.device,
) -> tuple[float, float]:
    model.train()
    train_loss = 0.0
    corrects = 0
    for inputs, labels in data_loader:
        inputs, labels = inputs.to(device), labels.to(device)
        optimizer.zero_grad()
        outputs = model(inputs)
        loss = loss_fn(outputs, labels)
        preds = torch.argmax(outputs, dim=1)
        loss.backward()
        optimizer.step()
        train_loss += loss.item() * inputs.size(0)
        corrects += torch.sum(preds == labels.data)
    train_loss /= len(data_loader.dataset)
    train_acc = corrects.float() / len(data_loader.dataset)
    return train_loss, train_acc


def __val_one_epoch(
    model: nn.Module,
    data_loader: DataLoader,
    loss_fn: nn.Module,
    device: torch.device,
) -> tuple[float, float]:
    model.eval()
    val_loss = 0.0
    corrects = 0
    for inputs, labels in data_loader:
        inputs, labels = inputs.to(device), labels.to(device)
        outputs = model(inputs)
        loss = loss_fn(outputs, labels)
        preds = torch.argmax(outputs, dim=1)
        val_loss += loss.item() * inputs.size(0)
        corrects += torch.sum(preds == labels.data)
    val_loss /= len(data_loader.dataset)
    val_acc = corrects.float() / len(data_loader.dataset)
    return val_loss, val_acc


def train(
    model: nn.Module,
    n_epochs: int,
    data_loaders: DataLoaders,
    learning_rate: float,
    momentum: float,
    early_stopping: Optional[EarlyStopping],
) -> tuple[list[float], list[float]]:
    LOGGER.info("Start training...")
    epoch_len = len(str(n_epochs))

    LOGGER.debug(
        f"Using SGD optimizer with lr={learning_rate} "
        f"and momentum={momentum}"
    )
    optimizer = torch.optim.SGD(
        model.parameters(), lr=learning_rate, momentum=momentum
    )
    LOGGER.debug("Using CrossEntropyLoss as loss function")
    loss_fn = nn.CrossEntropyLoss()

    device = next(model.parameters()).device

    train_losses = []
    val_losses = []

    for epoch in range(n_epochs):
        train_loss, train_acc = __train_one_epoch(
            model, data_loaders.train, loss_fn, optimizer, device
        )
        train_losses.append(train_loss)

        val_loss, val_acc = __val_one_epoch(
            model, data_loaders.val, loss_fn, device
        )
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
    loss_fn = nn.CrossEntropyLoss()
    test_loss, test_acc = __val_one_epoch(model, data_loader, loss_fn, device)
    LOGGER.info(f"Test loss: {test_loss:.5f}, test accuracy: {test_acc:2.2%}")
    return test_loss, test_acc
