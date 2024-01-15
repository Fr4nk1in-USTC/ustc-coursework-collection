import torch
from logger import get_logger

LOGGER = get_logger(__name__)


class EarlyStopping:
    def __init__(
        self, patience=7, delta=0, path="./checkpoints/best_checkpoint.pt"
    ):
        LOGGER.debug(
            f"EarlyStopping with patience={patience} and delta={delta}"
        )
        self.patience = patience
        self.delta = delta
        self.path = path
        self.counter = 0
        self.min_val_loss = None
        self.early_stop = False

    def __call__(self, val_loss, model):
        if self.min_val_loss is None:
            self.min_val_loss = val_loss
            self.save_checkpoint(model)
        elif val_loss > self.min_val_loss + self.delta:
            self.counter += 1
            LOGGER.debug(
                f"EarlyStopping counter: {self.counter} out of {self.patience}"
            )
            if self.counter >= self.patience:
                self.early_stop = True
        else:
            LOGGER.debug(
                "Validation loss decreased "
                f"({self.min_val_loss:.6f} --> {val_loss:.6f}). "
            )
            self.save_checkpoint(model)
            self.min_val_loss = val_loss
            self.counter = 0

    def save_checkpoint(self, model):
        torch.save(model.state_dict(), self.path)

    def load_checkpoint(self, model):
        model.load_state_dict(torch.load(self.path))
