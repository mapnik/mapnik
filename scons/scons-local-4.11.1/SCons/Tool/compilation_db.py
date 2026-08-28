#
# MIT License
#
# Copyright 2020 MongoDB Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

"""Compilation Database

Implements the ability for SCons to emit a compilation database for a
project. See https://clang.llvm.org/docs/JSONCompilationDatabase.html
for details on what a compilation database is, and why you might want one.
The only user visible entry point here is ``env.CompilationDatabase``.
This method takes an optional *target* to name the file that should hold
the compilation database, otherwise, the file defaults to
``compile_commands.json``, the name that most clang tools search for by default.
"""

import fnmatch
import itertools
import json

from SCons.Action import Action
from SCons.Builder import Builder, ListEmitter
from SCons.Platform import TempFileMunge
from SCons.Tool import createObjBuilders
from SCons.Tool.asm import ASPPSuffixes, ASSuffixes
from SCons.Tool.cc import CSuffixes
from SCons.Tool.cxx import CXXSuffixes

DEFAULT_DB_NAME = "compile_commands.json"

# TODO: Is there a better way to do this than this global? Right now this exists so that the
# emitter we add can record all of the things it emits, so that the scanner for the top level
# compilation database can access the complete list, and also so that the writer has easy
# access to write all of the files. But it seems clunky. How can the emitter and the scanner
# communicate more gracefully?
__COMPILATION_DB_ENTRIES = []


class CompDBTEMPFILE(TempFileMunge):
    def __call__(self, target, source, env, for_signature):
        return self.cmd


def write_compilation_db(target, source, env) -> None:
    directory = env.Dir("#").get_abspath()
    overrides = {"TEMPFILE": CompDBTEMPFILE}
    use_abspath = env["COMPILATIONDB_USE_ABSPATH"] in [True, 1, "True", "true"]
    use_path_filter = env.subst("$COMPILATIONDB_PATH_FILTER")

    entries = []
    for db_target, db_source, db_env, db_action in __COMPILATION_DB_ENTRIES:
        # Parse command before filtering.
        command = db_action.strfunction(db_target, db_source, db_env, None, overrides)

        if not db_source.is_derived():
            db_source = db_source.srcnode()

        if use_abspath:
            file = db_source.get_abspath()
            output = db_target.get_abspath()
        else:
            file = db_source.get_path()
            output = db_target.get_path()

        if use_path_filter and not fnmatch.fnmatch(output, use_path_filter):
            continue

        entries.append(
            {
                "command": command,
                "directory": directory,
                "file": file,
                "output": output,
            }
        )

    with open(target[0].get_path(), "w", encoding="utf-8", newline="\n") as output_file:
        json.dump(entries, output_file, sort_keys=True, indent=4)
        output_file.write("\n")


def compilation_db_emitter(target, source, env):
    """Fix up the source/targets"""

    # Someone called env.CompilationDatabase('my_targetname.json')
    if not target and len(source) == 1:
        target = source

    if not target:
        target = [DEFAULT_DB_NAME]

    # No source should have been passed. Drop it.
    if source:
        source = []

    # TODO: Should eventually have a way to allow the entries themselves to
    #  function as dependencies.
    env.AlwaysBuild(target)
    env.NoCache(target)

    return target, source


def generate(env, **kwargs) -> None:
    def _generate_emitter(command):
        # Construct new action to bypass `COMSTR`.
        action = Action(command)

        def _compilation_db_entry_emitter(target, source, env):
            __COMPILATION_DB_ENTRIES.append((target[0], source[0], env, action))
            return target, source

        return _compilation_db_entry_emitter

    cc_emitter = _generate_emitter("$CCCOM")
    shcc_emitter = _generate_emitter("$SHCCCOM")
    cxx_emitter = _generate_emitter("$CXXCOM")
    shcxx_emitter = _generate_emitter("$SHCXXCOM")
    as_emitter = _generate_emitter("$ASCOM")
    aspp_emitter = _generate_emitter("$ASPPCOM")

    static_obj, shared_obj = createObjBuilders(env)

    for suffix, (builder, emitter) in itertools.chain(
        itertools.product(
            CSuffixes, [(static_obj, cc_emitter), (shared_obj, shcc_emitter)]
        ),
        itertools.product(
            CXXSuffixes, [(static_obj, cxx_emitter), (shared_obj, shcxx_emitter)]
        ),
        itertools.product(
            ASSuffixes, [(static_obj, as_emitter), (shared_obj, as_emitter)]
        ),
        itertools.product(
            ASPPSuffixes, [(static_obj, aspp_emitter), (shared_obj, aspp_emitter)]
        ),
    ):
        emitter_old = builder.emitter.get(suffix)
        # Only setup emitters for Tools supported by the environment.
        if emitter_old:
            builder.emitter[suffix] = ListEmitter(env.Flatten(emitter_old) + [emitter])

    env["BUILDERS"]["CompilationDatabase"] = Builder(
        action=Action(write_compilation_db, "$COMPILATIONDB_COMSTR"),
        emitter=compilation_db_emitter,
        suffix="json",
    )

    env["COMPILATIONDB_USE_ABSPATH"] = env.get("COMPILATIONDB_USE_ABSPATH", False)
    env["COMPILATIONDB_PATH_FILTER"] = env.get("COMPILATIONDB_PATH_FILTER", "")
    env["COMPILATIONDB_COMSTR"] = env.get(
        "COMPILATIONDB_COMSTR",
        kwargs.get("COMPILATIONDB_COMSTR", "Building compilation database $TARGET"),
    )


def exists(env) -> bool:
    return True
