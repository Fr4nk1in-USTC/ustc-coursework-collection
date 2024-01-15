import torch
import torch.nn as nn
from logger import get_logger
from torchvision.models import (AlexNet_Weights, ResNet18_Weights,
                                ResNet34_Weights, ResNet50_Weights, alexnet,
                                resnet18, resnet34, resnet50)

LOGGER = get_logger(__name__)


def __get_model(name: str, num_classes: int):
    LOGGER.debug(f"Using model {name}")
    if name == "alexnet":
        return alexnet(num_classes=num_classes)
    if name == "resnet18":
        return resnet18(num_classes=num_classes)
    if name == "resnet34":
        return resnet34(num_classes=num_classes)
    if name == "resnet50":
        return resnet50(num_classes=num_classes)
    raise ValueError(f"Unknown model: {name}")


def __get_pretrained_model(name: str, num_classes: int):
    LOGGER.debug(f"Using model {name} with pretrained weights")
    if name == "alexnet":
        model = alexnet(weights=AlexNet_Weights.DEFAULT)
        model.classifier[-1] = nn.Linear(
            model.classifier[-1].in_features, num_classes
        )
        return model
    if name == "resnet18":
        model = resnet18(weights=ResNet18_Weights.DEFAULT)
        model.fc = nn.Linear(model.fc.in_features, num_classes)
        return model
    if name == "resnet34":
        model = resnet34(weights=ResNet34_Weights.DEFAULT)
        model.fc = nn.Linear(model.fc.in_features, num_classes)
        return model
    if name == "resnet50":
        model = resnet50(weights=ResNet50_Weights.DEFAULT)
        model.fc = nn.Linear(model.fc.in_features, num_classes)
        return model
    raise ValueError(f"Unknown model: {name}")


def new_model(model_name: str, num_classes: int, pretrained: bool) -> nn.Module:
    if pretrained:
        return __get_pretrained_model(model_name, num_classes)
    return __get_model(model_name, num_classes)


def load_model(model_path: str) -> nn.Module:
    model = torch.load(model_path)
    if not isinstance(model, nn.Module):
        raise ValueError(f"Model {model_path} is not an instance of nn.Module")
    return model
