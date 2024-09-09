# SPDX-License-Identifier: MIT
#
# Copyright The SCons Foundation

"""Various SCons type aliases.

For representing complex types across the entire repo without risking
circular dependencies, we take advantage of TYPE_CHECKING to import
modules in an tool-only environment. This allows us to introduce
hinting that resolves as expected in IDEs without clashing at runtime.

For consistency, it's recommended to ALWAYS use these aliases in a
type-hinting context, even if the type is actually expected to be
resolved in a given file.
"""

from typing import Union, TYPE_CHECKING

if TYPE_CHECKING:
    import SCons.Executor


# Because we don't have access to TypeAlias until 3.10, we have to utilize
# 'Union' for all aliases. As it expects at least two entries, anything that
# is only represented with a single type needs to list itself twice.
ExecutorType = Union["SCons.Executor.Executor", "SCons.Executor.Executor"]


# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
