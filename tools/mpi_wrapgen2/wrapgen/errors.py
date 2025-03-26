from logging import Logger

_permissive: bool = False


def be_permissive():
    global _permissive
    _permissive = True


def be_strict():
    global _permissive
    _permissive = False


def permissive() -> bool:
    return _permissive


def log_and_raise(ex: Exception, msg: str | None = None, logger: Logger | None = None):
    if msg is not None:
        logger.critical(msg)
    if logger is not None:
        logger.critical(ex)
    raise ex


def error(msg: str, logger: Logger | None = None):
    log_and_raise(RuntimeError(msg), logger=logger)


def error_if_strict(msg: str, logger: Logger | None = None):
    if permissive():
        logger.warning(msg)
    else:
        log_and_raise(RuntimeError(msg), logger=logger)
