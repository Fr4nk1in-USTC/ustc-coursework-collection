import logging

_FORMAT = "%(asctime)s %(levelname)-7s| %(message)s"

_root_logger_set = False


def setup_root_logger(level: int = logging.DEBUG):
    logging.basicConfig(level=level, format=_FORMAT)
    global _root_logger_set
    _root_logger_set = True


def get_logger(name: str) -> logging.Logger:
    if not _root_logger_set:
        setup_root_logger()
    return logging.getLogger(name)
