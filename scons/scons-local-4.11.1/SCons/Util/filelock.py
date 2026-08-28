# MIT License
#
# Copyright The SCons Foundation

"""SCons file locking functions.

Simple-minded filesystem-based locking. Provides a context manager
which acquires a lock (or at least, permission) on entry and
releases it on exit.

The model is to prevent concurrency problems if there are multiple
SCons processes running which may try to read/write the same
externally shared resource, such as a cache file. They don't
have to be the same machine in the case of a network share,
so we can't use OS-specific locking schemes like fcntl/msvcrt.

Usage::

    from SCons.Util.filelock import FileLock

    with FileLock("myfile.txt", writer=True) as lock:
        print(f"Lock on {lock.file} acquired.")
        # work with the file as it is now locked
"""

# TODO: things to consider.
#   Is raising an exception the right thing for failing to get lock?
#   Is this scheme sufficient for our needs? Or do we need the extra
#     level of creating a "claim file" first and then trying to link
#     that to the lockfile? Bonus: a claim file can contain details, like
#     how long before another process is allowed to break the lock,
#     plus identifying the creator.
#   Is this safe enough? Or do we risk dangling lockfiles?
#   Permission issues in case of multi-user. This *should* be okay,
#     the cache usually goes in user's homedir, plus you already have
#     enough rights for the lockfile if the dir lets you create the cache,
#     but manual permission fiddling may be needed for network shares.
#   Need a forced break-lock method?
#   The lock attributes could probably be made opaque. Showed one visible
#     in the example above, but not sure the benefit of that.

from __future__ import annotations

import os
import time


class SConsLockFailure(Exception):
    """Generic lock failure exception."""

class AlreadyLockedError(SConsLockFailure):
    """Non-blocking lock attempt on already locked object."""

class LockTimeoutError(SConsLockFailure):
    """Blocking lock attempt timed out."""


class FileLock:
    """Lock a resource using a lockfile.

    Basic locking for when multiple processes may hit an externally shared
    resource (i.e. a file) that cannot depend on locking within a single
    SCons process. SCons does not have a lot of those, but caches come to
    mind.  Note that the resource named by *file* does not need to exist.

    Cross-platform safe, does not use any OS-specific features.  Is not
    safe from outside interference - a "locked" resource can be messed
    with by anyone who doesn't pay attention to this protocol.  Provides
    context manager support, or can be called with :meth:`acquire_lock`
    and :meth:`release_lock`.

    Lock can be a write lock, which is held until released, or a read
    lock, which releases immediately upon acquisition - we want to prevent
    reading a file while somebody else may be writing to it, but once we
    get that permission, we don't want to block others from reading it
    too, or asking for a write lock on it.

    TODO: Should default timeout be None (non-blocking), or 0 (block forever),
       or some arbitrary number? For now it's non-blocking.

    Arguments:
       file: name of file to lock. Only used to build the lockfile name.
       timeout: optional time (sec) after which to give up trying.
          If ``None``, the lock attempt will be non-blocking (the default).
          If 0, block forever (well, a long time).
       delay: optional delay between tries [default 0.05s]
       writer: if true, obtain and hold the lock for safe writing.
          If false (the default), release the lock right away.

    Raises:
        SConsLockFailure: if the operation "timed out", including the
          non-blocking mode.
    """

    def __init__(
        self,
        file: str,
        timeout: int | None = None,
        delay: float | None = 0.05,
        writer: bool = False,
    ) -> None:
        if timeout is not None and delay is None:
            raise ValueError("delay cannot be None if timeout is None.")
        # It isn't completely obvious where to put the lockfile.
        # This scheme depends on diffrent processes using the same path
        # to the lockfile, since the lockfile is the magic resource,
        # not the file itself. getcwd() is no good for testcases, each of
        # which run in a unique test directory. tempfile is no good,
        # as those are (intentionally) unique per process.
        # Our simple first guess is just put it where the file is.
        self.file = file
        self.lockfile = f"{file}.lock"
        self.lock: int | None = None
        self.timeout = 999999 if timeout == 0 else timeout
        self.delay = 0.0 if delay is None else delay
        self.writer = writer

    def acquire_lock(self) -> None:
        """Acquire the lock.

        Blocks until the lock is acquired, unless the lock object
        was initialized to not block.
        If the lock is in use, check again every *delay* seconds.
        Continue until lock acquired or *timeout* expires.
        """
        start_time = time.perf_counter()
        while True:
            try:
                self.lock = os.open(self.lockfile, os.O_CREAT|os.O_EXCL|os.O_RDWR)
            except (FileExistsError, PermissionError) as exc:
                if self.timeout is None:
                    raise AlreadyLockedError(
                        f"Could not acquire lock on {self.file!r}"
                    ) from exc
                if (time.perf_counter() - start_time) > self.timeout:
                    raise LockTimeoutError(
                        f"Timeout waiting for lock on {self.file!r} for {self.timeout} seconds."
                    ) from exc
                time.sleep(self.delay)
            else:
                if not self.writer:
                    # reader: waits to get lock, but doesn't hold it
                    self.release_lock()
                break

    def release_lock(self) -> None:
        """Release the lock by deleting the lockfile."""
        if self.lock:
            os.close(self.lock)
            os.unlink(self.lockfile)
            self.lock = None

    def __enter__(self) -> FileLock:
        """Context manager entry: acquire lock if not holding."""
        if self.lock is None:
            self.acquire_lock()
        return self

    def __exit__(self, exc_type, exc_value, exc_tb) -> None:
        """Context manager exit: release lock if holding."""
        if self.lock is not None:
            self.release_lock()

    def __repr__(self) -> str:
        """Nicer display if someone repr's the lock class."""
        return (
            f"<{self.__class__.__name__}("
            f"file={self.file!r}, "
            f"type={'writer, ' if self.writer else 'reader, '}"
            f"lock={'unlocked' if self.lock is None else 'locked'}, "
            f"timeout={self.timeout}, "
            f"delay={self.delay}, "
            f"pid={os.getpid()}) "
            f"at 0x{id(self):x}>"
        )
