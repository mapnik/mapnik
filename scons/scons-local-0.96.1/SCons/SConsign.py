"""SCons.SConsign

Writing and reading information to the .sconsign file or files.

"""

#
# Copyright (c) 2001, 2002, 2003, 2004 The SCons Foundation
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
#

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/SConsign.py 0.96.1.D001 2004/08/23 09:55:29 knight"

import cPickle
import os
import os.path
import time

import SCons.Node
import SCons.Sig
import SCons.Warnings

#XXX Get rid of the global array so this becomes re-entrant.
sig_files = []

database = None

def write():
    global sig_files
    for sig_file in sig_files:
        sig_file.write()

class Base:
    """
    This is the controlling class for the signatures for the collection of
    entries associated with a specific directory.  The actual directory
    association will be maintained by a subclass that is specific to
    the underlying storage method.  This class provides a common set of
    methods for fetching and storing the individual bits of information
    that make up signature entry.
    """
    def __init__(self, module=None):
        """
        module - the signature module being used
        """

        self.module = module or SCons.Sig.default_calc.module
        self.entries = {}
        self.dirty = 0

    def get_entry(self, filename):
        """
        Fetch the specified entry attribute.
        """
        return self.entries[filename]

    def set_entry(self, filename, obj):
        """
        Set the entry.
        """
        self.entries[filename] = obj
        self.dirty = 1

class DB(Base):
    """
    A Base subclass that reads and writes signature information
    from a global .sconsign.dbm file.
    """
    def __init__(self, dir, module=None):
        Base.__init__(self, module)

        self.dir = dir

        try:
            global database
            rawentries = database[self.dir.path]
        except KeyError:
            pass
        else:
            try:
                self.entries = cPickle.loads(rawentries)
                if type(self.entries) is not type({}):
                    self.entries = {}
                    raise TypeError
            except KeyboardInterrupt:
                raise
            except:
                SCons.Warnings.warn(SCons.Warnings.CorruptSConsignWarning,
                                    "Ignoring corrupt sconsign entry : %s"%self.dir.path)

        global sig_files
        sig_files.append(self)

    def write(self):
        if self.dirty:
            global database
            database[self.dir.path] = cPickle.dumps(self.entries, 1)
            try:
                database.sync()
            except AttributeError:
                # Not all anydbm modules have sync() methods.
                pass

class Dir(Base):
    def __init__(self, fp=None, module=None):
        """
        fp - file pointer to read entries from
        module - the signature module being used
        """
        Base.__init__(self, module)

        if fp:
            self.entries = cPickle.load(fp)
            if type(self.entries) is not type({}):
                self.entries = {}
                raise TypeError

class DirFile(Dir):
    """
    Encapsulates reading and writing a per-directory .sconsign file.
    """
    def __init__(self, dir, module=None):
        """
        dir - the directory for the file
        module - the signature module being used
        """

        self.dir = dir
        self.sconsign = os.path.join(dir.path, '.sconsign')

        try:
            fp = open(self.sconsign, 'rb')
        except IOError:
            fp = None

        try:
            Dir.__init__(self, fp, module)
        except KeyboardInterrupt:
            raise
        except:
            SCons.Warnings.warn(SCons.Warnings.CorruptSConsignWarning,
                                "Ignoring corrupt .sconsign file: %s"%self.sconsign)

        global sig_files
        sig_files.append(self)

    def write(self):
        """
        Write the .sconsign file to disk.

        Try to write to a temporary file first, and rename it if we
        succeed.  If we can't write to the temporary file, it's
        probably because the directory isn't writable (and if so,
        how did we build anything in this directory, anyway?), so
        try to write directly to the .sconsign file as a backup.
        If we can't rename, try to copy the temporary contents back
        to the .sconsign file.  Either way, always try to remove
        the temporary file at the end.
        """
        if self.dirty:
            temp = os.path.join(self.dir.path, '.scons%d' % os.getpid())
            try:
                file = open(temp, 'wb')
                fname = temp
            except IOError:
                try:
                    file = open(self.sconsign, 'wb')
                    fname = self.sconsign
                except IOError:
                    return
            cPickle.dump(self.entries, file, 1)
            file.close()
            if fname != self.sconsign:
                try:
                    mode = os.stat(self.sconsign)[0]
                    os.chmod(self.sconsign, 0666)
                    os.unlink(self.sconsign)
                except OSError:
                    pass
                try:
                    os.rename(fname, self.sconsign)
                except OSError:
                    open(self.sconsign, 'wb').write(open(fname, 'rb').read())
                    os.chmod(self.sconsign, mode)
            try:
                os.unlink(temp)
            except OSError:
                pass

ForDirectory = DirFile

def File(name, dbm_module=None):
    """
    Arrange for all signatures to be stored in a global .sconsign.dbm
    file.
    """
    global database
    if database is None:
        if dbm_module is None:
            import SCons.dblite
            dbm_module = SCons.dblite
        database = dbm_module.open(name, "c")

    global ForDirectory
    ForDirectory = DB
