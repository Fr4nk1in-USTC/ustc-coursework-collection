import os
from dataclasses import dataclass

from logger import get_logger
from torch.utils.data import DataLoader, Subset, random_split
from torchvision import datasets, transforms

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(FILE_DIR)
DATA_DIR = os.path.relpath(os.path.join(ROOT_DIR, "data"))

LOGGER = get_logger(__file__)


@dataclass
class DataLoaders:
    train: DataLoader
    val: DataLoader
    test: DataLoader


def __get_image_folder(size: int) -> datasets.ImageFolder:
    LOGGER.info(f"Loading images from {DATA_DIR} ...")
    LOGGER.debug(f"Target image size: {size}x{size}")

    image_folder = datasets.ImageFolder(
        DATA_DIR,
        transform=transforms.Compose(
            [
                transforms.RandomResizedCrop(size),
                transforms.RandomHorizontalFlip(),
                transforms.ToTensor(),
                transforms.Normalize(
                    mean=[0.485, 0.456, 0.406],
                    std=[0.229, 0.224, 0.225],
                ),
            ]
        ),
    )
    LOGGER.info(
        f"ImageFolder loaded with {len(image_folder)} images "
        f"and target classes {image_folder.classes}"
    )
    return image_folder


def get_data_loaders(
    image_size,
    train_val_test_ratio: list[float],
    batch_size: int,
) -> tuple[DataLoaders, list[str]]:
    """
    Creates data loaders for train, test and validation data sets.
    Returns data loaders and a list of target classes.
    """
    LOGGER.info("Generating data loaders...")
    LOGGER.debug(f"Batch size: {batch_size}")
    image_folder = __get_image_folder(image_size)
    target_classes = image_folder.classes

    train_set, val_set, test_set = random_split(
        image_folder, train_val_test_ratio
    )
    LOGGER.debug(
        f"Dataset split into training set ({len(train_set)}), "
        f"validation set ({len(val_set)}), and testing set ({len(val_set)})."
    )

    def subset_to_loader(subset: Subset):
        return DataLoader(
            subset, batch_size=batch_size, shuffle=True, num_workers=4
        )

    data_loaders = DataLoaders(
        train=subset_to_loader(train_set),
        val=subset_to_loader(val_set),
        test=subset_to_loader(test_set),
    )
    LOGGER.info("Data loaders generated")
    return data_loaders, target_classes


if __name__ == "__main__":
    _, _ = get_data_loaders(227, [0.8, 0.1, 0.1], 4)
