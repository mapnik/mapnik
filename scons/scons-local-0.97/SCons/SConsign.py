"""SCons.SConsign

Writing and reading information to the .sconsign file or files.

"""

#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007 The SCons Foundation
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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/SConsign.py 0.97.D001 2007/05/17 11:35:19 knight"

import cPickle
import os
import os.path

import SCons.dblite
import SCons.Sig
import SCons.Warnings

def corrupt_dblite_warning(filename):
    SCons.Warnings.warn(SCons.Warnings.CorruptSConsignWarning,
                        "Ignoring corrupt .sconsign file: %s"%filename)

SCons.dblite.ignore_corrupt_dbfiles = 1
SCons.dblite.corruption_warning = corrupt_dblite_warning

#XXX Get rid of the global array so this becomes re-entrant.
sig_files = []

# Info for the database SConsign implementation (now the default):
# "DataBase" is a dictionary that maps top-level SConstruct directories
# to open database handles.
# "DB_Module" is the Python database module to create the handles.
# "DB_Name" is the base name of the database file (minus any
# extension the underlying DB module will add).
DataBase = {}
DB_Module = SCons.dblite
DB_Name = ".sconsign"
DB_sync_list = []

def Get_DataBase(dir):
    global DataBase, DB_Module, DB_Name
    top = dir.fs.Top
    if not os.path.isabs(DB_Name) and top.repositories:
        mode = "c"
        for d in [top] + top.repositories:
            if dir.is_under(d):
                try:
                    return DataBase[d], mode
                except KeyError:
                    path = d.entry_abspath(DB_Name)
                    try: db = DataBase[d] = DB_Module.open(path, mode)
                    except (IOError, OSError): pass
                    else:
                        if mode != "r":
                            DB_sync_list.append(db)
                        return db, mode
            mode = "r"
    try:
        return DataBase[top], "c"
    except KeyError:
        db = DataBase[top] = DB_Module.open(DB_Name, "c")
        DB_sync_list.append(db)
        return db, "c"
    except TypeError:
        print "DataBase =", DataBase
        raise

def Reset():
    """Reset global state.  Used by unit tests that end up using
    SConsign multiple times to get a clean slate for each test."""
    global sig_files, DB_sync_list
    sig_files = []
    DB_sync_list = []

normcase = os.path.normcase

def write():
    global sig_files
    for sig_file in sig_files:
        sig_file.write(sync=0)
    for db in DB_sync_list:
        try:
            syncmethod = db.sync
        except AttributeError:
            pass # Not all anydbm modules have sync() methods.
        else:
            syncmethod()

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

    def do_not_set_entry(self, filename, obj):
        pass

class DB(Base):
    """
    A Base subclass that reads and writes signature information
    from a global .sconsign.db* file--the actual file suffix is
    determined by the specified database module.
    """
    def __init__(self, dir, module=None):
        Base.__init__(self, module)

        self.dir = dir

        db, mode = Get_DataBase(dir)

        # Read using the path relative to the top of the Repository
        # (self.dir.tpath) from which we're fetching the signature
        # information.
        path = normcase(dir.tpath)
        try:
            rawentries = db[path]
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
            except Exception, e:
                SCons.Warnings.warn(SCons.Warnings.CorruptSConsignWarning,
                                    "Ignoring corrupt sconsign entry : %s (%s)\n"%(self.dir.tpath, e))
            for key, entry in self.entries.items():
                entry.convert_from_sconsign(dir, key)

        if mode == "r":
            # This directory is actually under a repository, which means
            # likely they're reaching in directly for a dependency on
            # a file there.  Don't actually set any entry info, so we
            # won't try to write to that .sconsign.dblite file.
            self.set_entry = self.do_not_set_entry

        global sig_files
        sig_files.append(self)

    def write(self, sync=1):
        if not self.dirty:
            return

        db, mode = Get_DataBase(self.dir)

        # Write using the path relative to the top of the SConstruct
        # directory (self.dir.path), not relative to the top of
        # the Repository; we only write to our own .sconsign file,
        # not to .sconsign files in Repositories.
        path = normcase(self.dir.path)
        for key, entry in self.entries.items():
            entry.convert_to_sconsign()
        db[path] = cPickle.dumps(self.entries, 1)

        if sync:
            try:
                syncmethod = db.sync
            except AttributeError:
                # Not all anydbm modules have sync() methods.
                pass
            else:
                syncmethod()

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

    def get_entry(self, filename):
        """
        Fetch the specified entry attribute, converting from .sconsign
        format to in-memory format.
        """
        entry = Dir.get_entry(self, filename)
        entry.convert_from_sconsign(self.dir, filename)
        return entry

    def write(self, sync=1):
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
            for key, entry in self.entries.items():
                entry.convert_to_sconsign()
            cPickle.dump(self.entries, file, 1)
            file.close()
            if fname != self.sconsign:
                try:
                    mode = os.stat(self.sconsign)[0]
                    os.chmod(self.sconsign, 0666)
                    os.unlink(self.sconsign)
                except (IOError, OSError):
                    # Try to carry on in the face of either OSError
                    # (things like permission issues) or IOError (disk
                    # or network issues).  If there's a really dangerous
                    # issue, it should get re-raised by the calls below.
                    pass
                try:
                    os.rename(fname, self.sconsign)
                except OSError:
                    # An OSError failure to rename may indicate something
                    # like the directory has no write permission, but
                    # the .sconsign file itself might still be writable,
                    # so try writing on top of it directly.  An IOError
                    # here, or in any of the following calls, would get
                    # raised, indicating something like a potentially
                    # serious disk or network issue.
                    open(self.sconsign, 'wb').write(open(fname, 'rb').read())
                    os.chmod(self.sconsign, mode)
            try:
                os.unlink(temp)
            except (IOError, OSError):
                pass

ForDirectory = DB

def File(name, dbm_module=None):
    """
    Arrange for all signatures to be stored in a global .sconsign.db*
    file.
    """
    global ForDirectory, DB_Name, DB_Module
    if name is None:
        ForDirectory = DirFile
        DB_Module = None
    else:
        ForDirectory = DB
        DB_Name = name
        if not dbm_module is None:
            DB_Module = dbm_module
