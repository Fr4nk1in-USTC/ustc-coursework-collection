import argparse


def _get_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command")

    # global arguments
    def add_global_arguments(parser: argparse.ArgumentParser):
        parser.add_argument(
            "--device",
            type=str,
            default="auto",
            choices=["auto", "cpu", "cuda"],
            help="Device to use, auto for cuda if available",
        )
        parser.add_argument("--seed", type=int, help="Random seed")
        parser.add_argument(
            "--debug", action="store_true", help="Log debug messages"
        )
        parser.add_argument(
            "--split-ratio",
            nargs=3,
            type=float,
            default=[0.8, 0.1, 0.1],
            help="Train/val/test ratio",
        )
        parser.add_argument(
            "--model-path", type=str, default="model.pkl", help="Model path"
        )
        parser.add_argument(
            "-is", "--image-size", type=int, default=224, help="Image size"
        )
        parser.add_argument(
            "-bs", "--batch-size", type=int, default=4, help="Batch size"
        )

    # subparser for training
    train_parser = subparsers.add_parser(
        "train", description="Training and testing"
    )
    add_global_arguments(train_parser)
    train_parser.add_argument(
        "-m",
        "--model",
        type=str,
        default="alexnet",
        choices=["alexnet", "resnet18", "resnet34", "resnet50"],
        help="Model name",
    )
    train_parser.add_argument(
        "--pretrained", action="store_true", help="Use pretrained model weights"
    )
    train_parser.add_argument(
        "-lr",
        "--learning-rate",
        type=float,
        default=0.001,
        help="Learning rate",
    )
    train_parser.add_argument(
        "--momentum", type=float, default=0.9, help="Momentum"
    )
    train_parser.add_argument(
        "-e", "--epochs", type=int, default=25, help="Max number of epochs"
    )
    train_parser.add_argument(
        "--early-stopping", action="store_true", help="Enable early stopping"
    )
    train_parser.add_argument(
        "--patience", type=int, default=7, help="Patience for early stopping"
    )
    train_parser.add_argument(
        "--delta", type=float, default=0, help="Delta for early stopping"
    )
    train_parser.add_argument(
        "--save-model", action="store_true", default=True, help="Save model"
    )
    train_parser.add_argument(
        "--checkpoint-dir", type=str, default="checkpoints"
    )
    train_parser.add_argument(
        "--save-loss", action="store_true", default=True, help="Save loss"
    )
    train_parser.add_argument(
        "--loss-path", type=str, default="loss.pkl", help="Loss path"
    )
    # subparser for testing
    test_parser = subparsers.add_parser("test", description="Testing only")
    add_global_arguments(test_parser)

    return parser


def parse_args() -> argparse.Namespace:
    """
    Parse command line arguments
    Args:
    - model: str, model name
    - batch_size: int, batch size
    - epochs: int, max number of epochs
    - early_stopping: bool, enable early stopping
    - patience: int, patience for early stopping
    - delta: float, delta for early stopping
    - seed: int, random seed
    - save_model: bool, save model
    - checkpoint_dir: str, checkpoint directory
    - log_level: str, log level
    """
    return _get_parser().parse_args()
