import logging
import os
import pickle
from argparse import ArgumentParser
from typing import Optional

import torch

# isort: off
from arg_parser import parse_args
from logger import get_logger, setup_root_logger
from early_stopping import EarlyStopping
from data_loader import get_train_data_loaders, get_test_data_loader
from models import new_model, load_model
from procedures import train, test

LOGGER: Optional[logging.Logger] = None
DEVICE: Optional[torch.device] = None

NUM_CLASSES = 2


def setup_from_args(args: ArgumentParser):
    # set logger
    global LOGGER
    level = logging.DEBUG if args.debug else logging.INFO
    setup_root_logger(level)
    LOGGER = get_logger(__file__)
    LOGGER.debug(f"Logging level: {'DEBUG' if args.debug else 'INFO'}")
    # set device
    global DEVICE
    device = args.device
    if device == "auto":
        device = "cuda" if torch.cuda.is_available() else "cpu"
    DEVICE = torch.device(device)
    LOGGER.debug(f"Device: {device}")

    # set seed
    if args.seed is not None:
        torch.manual_seed(args.seed)
        torch.cuda.manual_seed(args.seed)
        LOGGER.debug(f"Random seed: {args.seed}")


def get_early_stopping(args: ArgumentParser) -> Optional[EarlyStopping]:
    if not args.early_stopping:
        return None
    if not os.path.exists(args.checkpoint_dir):
        LOGGER.warning(
            f"Checkpoint directory {args.checkpoint_dir} does not exist, "
            "creating it..."
        )
        os.mkdir(args.checkpoint_dir)
    return EarlyStopping(
        patience=args.patience,
        delta=args.delta,
        path=os.path.join(args.checkpoint_dir, "checkpoint.pt"),
    )


if __name__ == "__main__":
    args = parse_args()
    setup_from_args(args)
    if args.command == "train":
        LOGGER.info("Training and testing")
        model, tokenizer = new_model(NUM_CLASSES, DEVICE)
        train_data_loaders = get_train_data_loaders(
            tokenizer, args.batch_size, args.val_ratio, args.max_length
        )
        early_stopping = get_early_stopping(args)
        losses = train(
            model,
            args.epochs,
            train_data_loaders,
            args.learning_rate,
            early_stopping,
        )
        test_data_loader = get_test_data_loader(tokenizer, args.batch_size)
        test(model, test_data_loader)
        if args.save_model:
            LOGGER.info(f"Saving model to {args.model_path}")
            torch.save(model, args.model_path)
        if args.save_loss:
            LOGGER.info(f"Saving loss to {args.loss_path}")
            with open(args.loss_path, "wb") as f:
                pickle.dump(losses, f)
    elif args.command == "test":
        LOGGER.info("Testing only")
        LOGGER.info(f"Loading model from {args.model_path}")
        model, tokenizer = load_model(args.model_path, DEVICE)
        data_loaders, _ = get_test_data_loader(
            tokenizer,
            args.batch_size,
            args.max_length,
        )
        test(model, data_loaders.test)
    else:
        raise ValueError(f"Unknown command: {args.command}")
