# MIT License
#
# Copyright The SCons Foundation
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

"""
MS Compilers: Visual C/C++ detection and configuration.

# TODO:
#   * gather all the information from a single vswhere call instead
#     of calling repeatedly (use json format?)
#   * support passing/setting location for vswhere in env.
#   * supported arch for versions: for old versions of batch file without
#     argument, giving bogus argument cannot be detected, so we have to hardcode
#     this here
#   * print warning when msvc version specified but not found
#   * find out why warning do not print
#   * test on 64 bits XP +  VS 2005 (and VS 6 if possible)
#   * SDK
#   * Assembly
"""

import SCons.compat

import subprocess
import os
import platform
from pathlib import Path
from string import digits as string_digits
from subprocess import PIPE
import re
from collections import (
    namedtuple,
    OrderedDict,
)

import SCons.Util
import SCons.Warnings
from SCons.Tool import find_program_path

from . import common
from .common import CONFIG_CACHE, debug
from .sdk import get_installed_sdks

from . import MSVC

from .MSVC.Exceptions import (
    VisualCException,
    MSVCUserError,
    MSVCArgumentError,
    MSVCToolsetVersionNotFound,
)

class UnsupportedVersion(VisualCException):
    pass

class MSVCUnsupportedHostArch(VisualCException):
    pass

class MSVCUnsupportedTargetArch(VisualCException):
    pass

class MissingConfiguration(VisualCException):
    pass

class NoVersionFound(VisualCException):
    pass

class BatchFileExecutionError(VisualCException):
    pass

class MSVCScriptNotFound(MSVCUserError):
    pass

class MSVCUseSettingsError(MSVCUserError):
    pass


# Dict to 'canonalize' the arch
_ARCH_TO_CANONICAL = {
    "amd64"     : "amd64",
    "emt64"     : "amd64",
    "i386"      : "x86",
    "i486"      : "x86",
    "i586"      : "x86",
    "i686"      : "x86",
    "ia64"      : "ia64",      # deprecated
    "itanium"   : "ia64",      # deprecated
    "x86"       : "x86",
    "x86_64"    : "amd64",
    "arm"       : "arm",
    "arm64"     : "arm64",
    "aarch64"   : "arm64",
}

# The msvc batch files report errors via stdout.  The following
# regular expression attempts to match known msvc error messages
# written to stdout.
re_script_output_error = re.compile(
    r'^(' + r'|'.join([
        r'VSINSTALLDIR variable is not set',             # 2002-2003
        r'The specified configuration type is missing',  # 2005+
        r'Error in script usage',                        # 2005+
        r'ERROR\:',                                      # 2005+
        r'\!ERROR\!',                                    # 2015-2015
        r'\[ERROR\:',                                    # 2017+
        r'\[ERROR\]',                                    # 2017+
        r'Syntax\:',                                     # 2017+
    ]) + r')'
)

# Lists of compatible host/target combinations are derived from a set of defined
# constant data structures for each host architecture. The derived data structures
# implicitly handle the differences in full versions and express versions of visual
# studio. The host/target combination search lists are contructed in order of
# preference. The construction of the derived data structures is independent of actual
# visual studio installations.  The host/target configurations are used in both the
# initial msvc detection and when finding a valid batch file for a given host/target
# combination.
#
# HostTargetConfig description:
#
#     label:
#         Name used for identification.
#
#     host_all_hosts:
#         Defined list of compatible architectures for each host architecture.
#
#     host_all_targets:
#         Defined list of target architectures for each host architecture.
#
#     host_def_targets:
#         Defined list of default target architectures for each host architecture.
#
#     all_pairs:
#         Derived list of all host/target combination tuples.
#
#     host_target_map:
#         Derived list of all compatible host/target combinations for each
#         supported host/target combination.
#
#     host_all_targets_map:
#         Derived list of all compatible host/target combinations for each
#         supported host.  This is used in the initial check that cl.exe exists
#         in the requisite visual studio vc host/target directory for a given host.
#
#     host_def_targets_map:
#         Derived list of default compatible host/target combinations for each
#         supported host.  This is used for a given host when the user does not
#         request a target archicture.
#
#     target_host_map:
#         Derived list of compatible host/target combinations for each supported
#         target/host combination.  This is used for a given host and target when
#         the user requests a target architecture.

_HOST_TARGET_CONFIG_NT = namedtuple("HostTargetConfig", [
    # defined
    "label",                # name for debugging/output
    "host_all_hosts",       # host_all_hosts[host] -> host_list
    "host_all_targets",     # host_all_targets[host] -> target_list
    "host_def_targets",     # host_def_targets[host] -> target_list
    # derived
    "all_pairs",            # host_target_list
    "host_target_map",      # host_target_map[host][target] -> host_target_list
    "host_all_targets_map", # host_all_targets_map[host][target] -> host_target_list
    "host_def_targets_map", # host_def_targets_map[host][target] -> host_target_list
    "target_host_map",      # target_host_map[target][host] -> host_target_list
])

def _host_target_config_factory(*, label, host_all_hosts, host_all_targets, host_def_targets):

    def _make_host_target_map(all_hosts, all_targets):
        # host_target_map[host][target] -> host_target_list
        host_target_map = {}
        for host, host_list in all_hosts.items():
            host_target_map[host] = {}
            for host_platform in host_list:
                for target_platform in all_targets[host_platform]:
                    if target_platform not in host_target_map[host]:
                        host_target_map[host][target_platform] = []
                    host_target_map[host][target_platform].append((host_platform, target_platform))
        return host_target_map

    def _make_host_all_targets_map(all_hosts, host_target_map, all_targets):
        # host_all_target_map[host] -> host_target_list
        # special host key '_all_' contains all (host,target) combinations
        all = '_all_'
        host_all_targets_map = {}
        host_all_targets_map[all] = []
        for host, host_list in all_hosts.items():
            host_all_targets_map[host] = []
            for host_platform in host_list:
                # all_targets[host_platform]: all targets for compatible host
                for target in all_targets[host_platform]:
                    for host_target in host_target_map[host_platform][target]:
                        for host_key in (host, all):
                            if host_target not in host_all_targets_map[host_key]:
                                host_all_targets_map[host_key].append(host_target)
        return host_all_targets_map

    def _make_host_def_targets_map(all_hosts, host_target_map, def_targets):
        # host_def_targets_map[host] -> host_target_list
        host_def_targets_map = {}
        for host, host_list in all_hosts.items():
            host_def_targets_map[host] = []
            for host_platform in host_list:
                # def_targets[host]: default targets for true host
                for target in def_targets[host]:
                    for host_target in host_target_map[host_platform][target]:
                        if host_target not in host_def_targets_map[host]:
                            host_def_targets_map[host].append(host_target)
        return host_def_targets_map

    def _make_target_host_map(all_hosts, host_all_targets_map):
        # target_host_map[target][host] -> host_target_list
        target_host_map = {}
        for host_platform in all_hosts.keys():
            for host_target in host_all_targets_map[host_platform]:
                _, target = host_target
                if target not in target_host_map:
                    target_host_map[target] = {}
                if host_platform not in target_host_map[target]:
                    target_host_map[target][host_platform] = []
                if host_target not in target_host_map[target][host_platform]:
                    target_host_map[target][host_platform].append(host_target)
        return target_host_map

    host_target_map = _make_host_target_map(host_all_hosts, host_all_targets)
    host_all_targets_map = _make_host_all_targets_map(host_all_hosts, host_target_map, host_all_targets)
    host_def_targets_map = _make_host_def_targets_map(host_all_hosts, host_target_map, host_def_targets)
    target_host_map = _make_target_host_map(host_all_hosts, host_all_targets_map)

    all_pairs = host_all_targets_map['_all_']
    del host_all_targets_map['_all_']

    host_target_cfg = _HOST_TARGET_CONFIG_NT(
        label = label,
        host_all_hosts = dict(host_all_hosts),
        host_all_targets = host_all_targets,
        host_def_targets = host_def_targets,
        all_pairs = all_pairs,
        host_target_map = host_target_map,
        host_all_targets_map = host_all_targets_map,
        host_def_targets_map = host_def_targets_map,
        target_host_map = target_host_map,
    )

    return host_target_cfg

# 14.1 (VS2017) and later

# Given a (host, target) tuple, return a tuple containing the batch file to
# look for and a tuple of path components to find cl.exe. We can't rely on returning
# an arg to use for vcvarsall.bat, because that script will run even if given
# a host/target pair that isn't installed.
#
# Starting with 14.1 (VS2017), the batch files are located in directory
# <VSROOT>/VC/Auxiliary/Build.  The batch file name is the first value of the
# stored tuple.
#
# The build tools are organized by host and target subdirectories under each toolset
# version directory.  For example,  <VSROOT>/VC/Tools/MSVC/14.31.31103/bin/Hostx64/x64.
# The cl path fragment under the toolset version folder is the second value of
# the stored tuple.

_GE2017_HOST_TARGET_BATCHFILE_CLPATHCOMPS = {

    ('amd64', 'amd64') : ('vcvars64.bat',          ('bin', 'Hostx64', 'x64')),
    ('amd64', 'x86')   : ('vcvarsamd64_x86.bat',   ('bin', 'Hostx64', 'x86')),
    ('amd64', 'arm')   : ('vcvarsamd64_arm.bat',   ('bin', 'Hostx64', 'arm')),
    ('amd64', 'arm64') : ('vcvarsamd64_arm64.bat', ('bin', 'Hostx64', 'arm64')),

    ('x86',   'amd64') : ('vcvarsx86_amd64.bat',   ('bin', 'Hostx86', 'x64')),
    ('x86',   'x86')   : ('vcvars32.bat',          ('bin', 'Hostx86', 'x86')),
    ('x86',   'arm')   : ('vcvarsx86_arm.bat',     ('bin', 'Hostx86', 'arm')),
    ('x86',   'arm64') : ('vcvarsx86_arm64.bat',   ('bin', 'Hostx86', 'arm64')),

}

_GE2017_HOST_TARGET_CFG = _host_target_config_factory(

    label = 'GE2017',

    host_all_hosts = OrderedDict([
        ('amd64', ['amd64', 'x86']),
        ('x86',   ['x86']),
        ('arm64', ['amd64', 'x86']),
        ('arm',   ['x86']),
    ]),

    host_all_targets = {
        'amd64': ['amd64', 'x86', 'arm64', 'arm'],
        'x86':   ['x86', 'amd64', 'arm', 'arm64'],
        'arm64': [],
        'arm':   [],
    },

    host_def_targets = {
        'amd64': ['amd64', 'x86'],
        'x86':   ['x86'],
        'arm64': ['arm64', 'arm'],
        'arm':   ['arm'],
    },

)

# debug("_GE2017_HOST_TARGET_CFG: %s", _GE2017_HOST_TARGET_CFG)

# 14.0 (VS2015) to 8.0 (VS2005)

# Given a (host, target) tuple, return a tuple containing the argument for
# the batch file and a tuple of the path components to find cl.exe.
#
# In 14.0 (VS2015) and earlier, the original x86 tools are in the tools
# bin directory (i.e., <VSROOT>/VC/bin).  Any other tools are in subdirectory
# named for the the host/target pair or a single name if the host==target.

_LE2015_HOST_TARGET_BATCHARG_CLPATHCOMPS = {

    ('amd64', 'amd64') : ('amd64',     ('bin', 'amd64')),
    ('amd64', 'x86')   : ('amd64_x86', ('bin', 'amd64_x86')),
    ('amd64', 'arm')   : ('amd64_arm', ('bin', 'amd64_arm')),

    ('x86',   'amd64') : ('x86_amd64', ('bin', 'x86_amd64')),
    ('x86',   'x86')   : ('x86',       ('bin', )),
    ('x86',   'arm')   : ('x86_arm',   ('bin', 'x86_arm')),
    ('x86',   'ia64')  : ('x86_ia64',  ('bin', 'x86_ia64')),

    ('arm',   'arm')   : ('arm',       ('bin', 'arm')),
    ('ia64',  'ia64')  : ('ia64',      ('bin', 'ia64')),

}

_LE2015_HOST_TARGET_CFG = _host_target_config_factory(

    label = 'LE2015',

    host_all_hosts = OrderedDict([
        ('amd64', ['amd64', 'x86']),
        ('x86',   ['x86']),
        ('arm',   ['arm']),
        ('ia64',  ['ia64']),
    ]),

    host_all_targets = {
        'amd64': ['amd64', 'x86', 'arm'],
        'x86':   ['x86', 'amd64', 'arm', 'ia64'],
        'arm':   ['arm'],
        'ia64':  ['ia64'],
    },

    host_def_targets = {
        'amd64': ['amd64', 'x86'],
        'x86':   ['x86'],
        'arm':   ['arm'],
        'ia64':  ['ia64'],
    },

)

# debug("_LE2015_HOST_TARGET_CFG: %s", _LE2015_HOST_TARGET_CFG)

# 7.1 (VS2003) and earlier

# For 7.1 (VS2003) and earlier, there are only x86 targets and the batch files
# take no arguments.

_LE2003_HOST_TARGET_CFG = _host_target_config_factory(

    label = 'LE2003',

    host_all_hosts = OrderedDict([
        ('amd64', ['x86']),
        ('x86',   ['x86']),
    ]),

    host_all_targets = {
        'amd64': ['x86'],
        'x86':   ['x86'],
    },

    host_def_targets = {
        'amd64': ['x86'],
        'x86':   ['x86'],
    },

)

# debug("_LE2003_HOST_TARGET_CFG: %s", _LE2003_HOST_TARGET_CFG)

_CL_EXE_NAME = 'cl.exe'

def get_msvc_version_numeric(msvc_version):
    """Get the raw version numbers from a MSVC_VERSION string, so it
    could be cast to float or other numeric values. For example, '14.0Exp'
    would get converted to '14.0'.

    Args:
        msvc_version: str
            string representing the version number, could contain non
            digit characters

    Returns:
        str: the value converted to a numeric only string

    """
    return ''.join([x for x in msvc_version if x in string_digits + '.'])

def get_host_platform(host_platform):

    host_platform = host_platform.lower()

    # Solaris returns i86pc for both 32 and 64 bit architectures
    if host_platform == 'i86pc':
        if platform.architecture()[0] == "64bit":
            host_platform = "amd64"
        else:
            host_platform = "x86"

    try:
        host =_ARCH_TO_CANONICAL[host_platform]
    except KeyError:
        msg = "Unrecognized host architecture %s"
        raise MSVCUnsupportedHostArch(msg % repr(host_platform)) from None

    return host

_native_host_platform = None

def get_native_host_platform():
    global _native_host_platform

    if _native_host_platform is None:

        _native_host_platform = get_host_platform(platform.machine())

    return _native_host_platform

def get_host_target(env, msvc_version, all_host_targets=False):

    vernum = float(get_msvc_version_numeric(msvc_version))

    if vernum > 14:
        # 14.1 (VS2017) and later
        host_target_cfg = _GE2017_HOST_TARGET_CFG
    elif 14 >= vernum >= 8:
        # 14.0 (VS2015) to 8.0 (VS2005)
        host_target_cfg = _LE2015_HOST_TARGET_CFG
    else:
        # 7.1 (VS2003) and earlier
        host_target_cfg = _LE2003_HOST_TARGET_CFG

    host_arch = env.get('HOST_ARCH') if env else None
    debug("HOST_ARCH:%s", str(host_arch))

    if host_arch:
        host_platform = get_host_platform(host_arch)
    else:
        host_platform = get_native_host_platform()

    target_arch = env.get('TARGET_ARCH') if env else None
    debug("TARGET_ARCH:%s", str(target_arch))

    if target_arch:

        try:
            target_platform = _ARCH_TO_CANONICAL[target_arch.lower()]
        except KeyError:
            all_archs = str(list(_ARCH_TO_CANONICAL.keys()))
            raise MSVCUnsupportedTargetArch(
                "Unrecognized target architecture %s\n\tValid architectures: %s"
                % (repr(target_arch), all_archs)
            ) from None

        target_host_map = host_target_cfg.target_host_map

        try:
            host_target_list = target_host_map[target_platform][host_platform]
        except KeyError:
            host_target_list = []
            warn_msg = "unsupported host, target combination ({}, {}) for MSVC version {}".format(
                repr(host_platform), repr(target_platform), msvc_version
            )
            SCons.Warnings.warn(SCons.Warnings.VisualCMissingWarning, warn_msg)
            debug(warn_msg)

    else:

        target_platform = None

        if all_host_targets:
            host_targets_map = host_target_cfg.host_all_targets_map
        else:
            host_targets_map = host_target_cfg.host_def_targets_map

        try:
            host_target_list = host_targets_map[host_platform]
        except KeyError:
            msg = "Unrecognized host architecture %s for version %s"
            raise MSVCUnsupportedHostArch(msg % (repr(host_platform), msvc_version)) from None

    return host_platform, target_platform, host_target_list

# If you update this, update SupportedVSList in Tool/MSCommon/vs.py, and the
# MSVC_VERSION documentation in Tool/msvc.xml.
_VCVER = [
    "14.3",
    "14.2",
    "14.1", "14.1Exp",
    "14.0", "14.0Exp",
    "12.0", "12.0Exp",
    "11.0", "11.0Exp",
    "10.0", "10.0Exp",
    "9.0", "9.0Exp",
    "8.0", "8.0Exp",
    "7.1",
    "7.0",
    "6.0"]

# if using vswhere, configure command line arguments to probe for installed VC editions
_VCVER_TO_VSWHERE_VER = {
    '14.3': [
        ["-version", "[17.0, 18.0)"],  # default: Enterprise, Professional, Community  (order unpredictable?)
        ["-version", "[17.0, 18.0)", "-products", "Microsoft.VisualStudio.Product.BuildTools"],  # BuildTools
    ],
    '14.2': [
        ["-version", "[16.0, 17.0)"],  # default: Enterprise, Professional, Community  (order unpredictable?)
        ["-version", "[16.0, 17.0)", "-products", "Microsoft.VisualStudio.Product.BuildTools"],  # BuildTools
    ],
    '14.1': [
        ["-version", "[15.0, 16.0)"],  # default: Enterprise, Professional, Community (order unpredictable?)
        ["-version", "[15.0, 16.0)", "-products", "Microsoft.VisualStudio.Product.BuildTools"],  # BuildTools
    ],
    '14.1Exp': [
        ["-version", "[15.0, 16.0)", "-products", "Microsoft.VisualStudio.Product.WDExpress"],  # Express
    ],
}

_VCVER_TO_PRODUCT_DIR = {
    '14.3': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'')],  # not set by this version
    '14.2': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'')],  # not set by this version
    '14.1': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'')],  # not set by this version
    '14.1Exp': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'')],  # not set by this version
    '14.0': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VisualStudio\14.0\Setup\VC\ProductDir')],
    '14.0Exp': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VCExpress\14.0\Setup\VC\ProductDir')],
    '12.0': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VisualStudio\12.0\Setup\VC\ProductDir'),
    ],
    '12.0Exp': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VCExpress\12.0\Setup\VC\ProductDir'),
    ],
    '11.0': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VisualStudio\11.0\Setup\VC\ProductDir'),
    ],
    '11.0Exp': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VCExpress\11.0\Setup\VC\ProductDir'),
    ],
    '10.0': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VisualStudio\10.0\Setup\VC\ProductDir'),
    ],
    '10.0Exp': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VCExpress\10.0\Setup\VC\ProductDir'),
    ],
    '9.0': [
        (SCons.Util.HKEY_CURRENT_USER, r'Microsoft\DevDiv\VCForPython\9.0\installdir',),
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VisualStudio\9.0\Setup\VC\ProductDir',),
    ],
    '9.0Exp': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VCExpress\9.0\Setup\VC\ProductDir'),
    ],
    '8.0': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VisualStudio\8.0\Setup\VC\ProductDir'),
    ],
    '8.0Exp': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VCExpress\8.0\Setup\VC\ProductDir'),
    ],
    '7.1': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VisualStudio\7.1\Setup\VC\ProductDir'),
    ],
    '7.0': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VisualStudio\7.0\Setup\VC\ProductDir'),
    ],
    '6.0': [
        (SCons.Util.HKEY_LOCAL_MACHINE, r'Microsoft\VisualStudio\6.0\Setup\Microsoft Visual C++\ProductDir'),
    ]
}


def msvc_version_to_maj_min(msvc_version):
    msvc_version_numeric = get_msvc_version_numeric(msvc_version)

    t = msvc_version_numeric.split(".")
    if not len(t) == 2:
        raise ValueError("Unrecognized version %s (%s)" % (msvc_version,msvc_version_numeric))
    try:
        maj = int(t[0])
        min = int(t[1])
        return maj, min
    except ValueError:
        raise ValueError("Unrecognized version %s (%s)" % (msvc_version,msvc_version_numeric)) from None


VSWHERE_PATHS = [os.path.join(p,'vswhere.exe') for p in [
    os.path.expandvars(r"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"),
    os.path.expandvars(r"%ProgramFiles%\Microsoft Visual Studio\Installer"),
    os.path.expandvars(r"%ChocolateyInstall%\bin"),
]]

def msvc_find_vswhere():
    """ Find the location of vswhere """
    # For bug 3333: support default location of vswhere for both
    # 64 and 32 bit windows installs.
    # For bug 3542: also accommodate not being on C: drive.
    # NB: this gets called from testsuite on non-Windows platforms.
    # Whether that makes sense or not, don't break it for those.
    vswhere_path = None
    for pf in VSWHERE_PATHS:
        if os.path.exists(pf):
            vswhere_path = pf
            break

    return vswhere_path

def find_vc_pdir_vswhere(msvc_version, env=None):
    """ Find the MSVC product directory using the vswhere program.

    Args:
        msvc_version: MSVC version to search for
        env: optional to look up VSWHERE variable

    Returns:
        MSVC install dir or None

    Raises:
        UnsupportedVersion: if the version is not known by this file

    """
    try:
        vswhere_version = _VCVER_TO_VSWHERE_VER[msvc_version]
    except KeyError:
        debug("Unknown version of MSVC: %s", msvc_version)
        raise UnsupportedVersion("Unknown version %s" % msvc_version) from None

    if env is None or not env.get('VSWHERE'):
        vswhere_path = msvc_find_vswhere()
    else:
        vswhere_path = env.subst('$VSWHERE')

    if vswhere_path is None:
        return None

    debug('VSWHERE: %s', vswhere_path)
    for vswhere_version_args in vswhere_version:

        vswhere_cmd = [vswhere_path] + vswhere_version_args + ["-property", "installationPath"]

        debug("running: %s", vswhere_cmd)

        # TODO: Python 3.7
        #  cp = subprocess.run(vswhere_cmd, capture_output=True, check=True)  # 3.7+ only
        cp = subprocess.run(vswhere_cmd, stdout=PIPE, stderr=PIPE, check=True)

        if cp.stdout:
            # vswhere could return multiple lines, e.g. if Build Tools
            # and {Community,Professional,Enterprise} are both installed.
            # We could define a way to pick the one we prefer, but since
            # this data is currently only used to make a check for existence,
            # returning the first hit should be good enough.
            lines = cp.stdout.decode("mbcs").splitlines()
            return os.path.join(lines[0], 'VC')
        else:
            # We found vswhere, but no install info available for this version
            pass

    return None


def find_vc_pdir(env, msvc_version):
    """Find the MSVC product directory for the given version.

    Tries to look up the path using a registry key from the table
    _VCVER_TO_PRODUCT_DIR; if there is no key, calls find_vc_pdir_wshere
    for help instead.

    Args:
        msvc_version: str
            msvc version (major.minor, e.g. 10.0)

    Returns:
        str: Path found in registry, or None

    Raises:
        UnsupportedVersion: if the version is not known by this file.
        MissingConfiguration: found version but the directory is missing.

        Both exceptions inherit from VisualCException.

    """
    root = 'Software\\'
    try:
        hkeys = _VCVER_TO_PRODUCT_DIR[msvc_version]
    except KeyError:
        debug("Unknown version of MSVC: %s", msvc_version)
        raise UnsupportedVersion("Unknown version %s" % msvc_version) from None

    for hkroot, key in hkeys:
        try:
            comps = None
            if not key:
                comps = find_vc_pdir_vswhere(msvc_version, env)
                if not comps:
                    debug('no VC found for version %s', repr(msvc_version))
                    raise OSError
                debug('VC found: %s', repr(msvc_version))
                return comps
            else:
                if common.is_win64():
                    try:
                        # ordinarily at win64, try Wow6432Node first.
                        comps = common.read_reg(root + 'Wow6432Node\\' + key, hkroot)
                    except OSError:
                        # at Microsoft Visual Studio for Python 2.7, value is not in Wow6432Node
                        pass
                if not comps:
                    # not Win64, or Microsoft Visual Studio for Python 2.7
                    comps = common.read_reg(root + key, hkroot)
        except OSError:
            debug('no VC registry key %s', repr(key))
        else:
            if msvc_version == '9.0' and key.lower().endswith('\\vcforpython\\9.0\\installdir'):
                # Visual C++ for Python registry key is installdir (root) not productdir (vc)
                comps = os.path.join(comps, 'VC')
            debug('found VC in registry: %s', comps)
            if os.path.exists(comps):
                return comps
            else:
                debug('reg says dir is %s, but it does not exist. (ignoring)', comps)
                raise MissingConfiguration("registry dir {} not found on the filesystem".format(comps))
    return None

def find_batch_file(env, msvc_version, host_arch, target_arch):
    """
    Find the location of the batch script which should set up the compiler
    for any TARGET_ARCH whose compilers were installed by Visual Studio/VCExpress

    In newer (2017+) compilers, make use of the fact there are vcvars
    scripts named with a host_target pair that calls vcvarsall.bat properly,
    so use that and return an empty argument.
    """
    pdir = find_vc_pdir(env, msvc_version)
    if pdir is None:
        raise NoVersionFound("No version of Visual Studio found")
    debug('looking in %s', pdir)

    # filter out e.g. "Exp" from the version name
    msvc_ver_numeric = get_msvc_version_numeric(msvc_version)
    vernum = float(msvc_ver_numeric)

    arg = ''
    vcdir = None

    if vernum > 14:
        # 14.1 (VS2017) and later
        batfiledir = os.path.join(pdir, "Auxiliary", "Build")
        batfile, _ = _GE2017_HOST_TARGET_BATCHFILE_CLPATHCOMPS[(host_arch, target_arch)]
        batfilename = os.path.join(batfiledir, batfile)
        vcdir = pdir
    elif 14 >= vernum >= 8:
        # 14.0 (VS2015) to 8.0 (VS2005)
        arg, _ = _LE2015_HOST_TARGET_BATCHARG_CLPATHCOMPS[(host_arch, target_arch)]
        batfilename = os.path.join(pdir, "vcvarsall.bat")
        if msvc_version == '9.0' and not os.path.exists(batfilename):
            # Visual C++ for Python batch file is in installdir (root) not productdir (vc)
            batfilename = os.path.normpath(os.path.join(pdir, os.pardir, "vcvarsall.bat"))
    else:
        # 7.1 (VS2003) and earlier
        pdir = os.path.join(pdir, "Bin")
        batfilename = os.path.join(pdir, "vcvars32.bat")

    if not os.path.exists(batfilename):
        debug("Not found: %s", batfilename)
        batfilename = None

    installed_sdks = get_installed_sdks()
    for _sdk in installed_sdks:
        sdk_bat_file = _sdk.get_sdk_vc_script(host_arch, target_arch)
        if not sdk_bat_file:
            debug("batch file not found:%s", _sdk)
        else:
            sdk_bat_file_path = os.path.join(pdir, sdk_bat_file)
            if os.path.exists(sdk_bat_file_path):
                debug('sdk_bat_file_path:%s', sdk_bat_file_path)
                return batfilename, arg, vcdir, sdk_bat_file_path

    return batfilename, arg, vcdir, None

__INSTALLED_VCS_RUN = None
_VC_TOOLS_VERSION_FILE_PATH = ['Auxiliary', 'Build', 'Microsoft.VCToolsVersion.default.txt']
_VC_TOOLS_VERSION_FILE = os.sep.join(_VC_TOOLS_VERSION_FILE_PATH)

def _check_cl_exists_in_vc_dir(env, vc_dir, msvc_version):
    """Return status of finding a cl.exe to use.

    Locates cl in the vc_dir depending on TARGET_ARCH, HOST_ARCH and the
    msvc version. TARGET_ARCH and HOST_ARCH can be extracted from the
    passed env, unless the env is None, in which case the native platform is
    assumed for the host and all associated targets.

    Args:
        env: Environment
            a construction environment, usually if this is passed its
            because there is a desired TARGET_ARCH to be used when searching
            for a cl.exe
        vc_dir: str
            the path to the VC dir in the MSVC installation
        msvc_version: str
            msvc version (major.minor, e.g. 10.0)

    Returns:
        bool:

    """

    # Find the host, target, and all candidate (host, target) platform combinations:
    platforms = get_host_target(env, msvc_version, all_host_targets=True)
    debug("host_platform %s, target_platform %s host_target_list %s", *platforms)
    host_platform, target_platform, host_target_list = platforms

    vernum = float(get_msvc_version_numeric(msvc_version))

    # make sure the cl.exe exists meaning the tool is installed
    if vernum > 14:
        # 14.1 (VS2017) and later
        # 2017 and newer allowed multiple versions of the VC toolset to be
        # installed at the same time. This changes the layout.
        # Just get the default tool version for now
        # TODO: support setting a specific minor VC version
        default_toolset_file = os.path.join(vc_dir, _VC_TOOLS_VERSION_FILE)
        try:
            with open(default_toolset_file) as f:
                vc_specific_version = f.readlines()[0].strip()
        except IOError:
            debug('failed to read %s', default_toolset_file)
            return False
        except IndexError:
            debug('failed to find MSVC version in %s', default_toolset_file)
            return False

        for host_platform, target_platform in host_target_list:

            debug('host platform %s, target platform %s for version %s', host_platform, target_platform, msvc_version)

            batchfile_clpathcomps = _GE2017_HOST_TARGET_BATCHFILE_CLPATHCOMPS.get((host_platform, target_platform), None)
            if batchfile_clpathcomps is None:
                debug('unsupported host/target platform combo: (%s,%s)', host_platform, target_platform)
                continue

            _, cl_path_comps = batchfile_clpathcomps
            cl_path = os.path.join(vc_dir, 'Tools', 'MSVC', vc_specific_version, *cl_path_comps, _CL_EXE_NAME)
            debug('checking for %s at %s', _CL_EXE_NAME, cl_path)

            if os.path.exists(cl_path):
                debug('found %s!', _CL_EXE_NAME)
                return True

    elif 14 >= vernum >= 8:
        # 14.0 (VS2015) to 8.0 (VS2005)

        for host_platform, target_platform in host_target_list:

            debug('host platform %s, target platform %s for version %s', host_platform, target_platform, msvc_version)

            batcharg_clpathcomps = _LE2015_HOST_TARGET_BATCHARG_CLPATHCOMPS.get((host_platform, target_platform), None)
            if batcharg_clpathcomps is None:
                debug('unsupported host/target platform combo: (%s,%s)', host_platform, target_platform)
                continue

            _, cl_path_comps = batcharg_clpathcomps
            cl_path = os.path.join(vc_dir, *cl_path_comps, _CL_EXE_NAME)
            debug('checking for %s at %s', _CL_EXE_NAME, cl_path)

            if os.path.exists(cl_path):
                debug('found %s', _CL_EXE_NAME)
                return True

    elif 8 > vernum >= 6:
        # 7.1 (VS2003) to 6.0 (VS6)

        # quick check for vc_dir/bin and vc_dir/ before walk
        # need to check root as the walk only considers subdirectories
        for cl_dir in ('bin', ''):
            cl_path = os.path.join(vc_dir, cl_dir, _CL_EXE_NAME)
            if os.path.exists(cl_path):
                debug('%s found %s', _CL_EXE_NAME, cl_path)
                return True
        # not in bin or root: must be in a subdirectory
        for cl_root, cl_dirs, _ in os.walk(vc_dir):
            for cl_dir in cl_dirs:
                cl_path = os.path.join(cl_root, cl_dir, _CL_EXE_NAME)
                if os.path.exists(cl_path):
                    debug('%s found %s', _CL_EXE_NAME, cl_path)
                    return True
        return False

    else:
        # version not support return false
        debug('unsupported MSVC version: %s', str(vernum))

    return False

def get_installed_vcs(env=None):
    global __INSTALLED_VCS_RUN

    if __INSTALLED_VCS_RUN is not None:
        return __INSTALLED_VCS_RUN

    installed_versions = []

    for ver in _VCVER:
        debug('trying to find VC %s', ver)
        try:
            VC_DIR = find_vc_pdir(env, ver)
            if VC_DIR:
                debug('found VC %s', ver)
                if _check_cl_exists_in_vc_dir(env, VC_DIR, ver):
                    installed_versions.append(ver)
                else:
                    debug('no compiler found %s', ver)
            else:
                debug('return None for ver %s', ver)
        except (MSVCUnsupportedTargetArch, MSVCUnsupportedHostArch):
            # Allow this exception to propagate further as it should cause
            # SCons to exit with an error code
            raise
        except VisualCException as e:
            debug('did not find VC %s: caught exception %s', ver, str(e))

    __INSTALLED_VCS_RUN = installed_versions
    return __INSTALLED_VCS_RUN

def reset_installed_vcs():
    """Make it try again to find VC.  This is just for the tests."""
    global __INSTALLED_VCS_RUN
    __INSTALLED_VCS_RUN = None
    MSVC._reset()

def msvc_default_version(env=None):
    """Get default msvc version."""
    vcs = get_installed_vcs(env)
    msvc_version = vcs[0] if vcs else None
    debug('msvc_version=%s', repr(msvc_version))
    return msvc_version

def get_installed_vcs_components(env=None):
    """Test suite convenience function: return list of installed msvc version component tuples"""
    vcs = get_installed_vcs(env)
    msvc_version_component_defs = [MSVC.Util.msvc_version_components(vcver) for vcver in vcs]
    return msvc_version_component_defs

# Running these batch files isn't cheap: most of the time spent in
# msvs.generate() is due to vcvars*.bat.  In a build that uses "tools='msvs'"
# in multiple environments, for example:
#    env1 = Environment(tools='msvs')
#    env2 = Environment(tools='msvs')
# we can greatly improve the speed of the second and subsequent Environment
# (or Clone) calls by memoizing the environment variables set by vcvars*.bat.
#
# Updated: by 2018, vcvarsall.bat had gotten so expensive (vs2017 era)
# it was breaking CI builds because the test suite starts scons so many
# times and the existing memo logic only helped with repeated calls
# within the same scons run. Windows builds on the CI system were split
# into chunks to get around single-build time limits.
# With VS2019 it got even slower and an optional persistent cache file
# was introduced. The cache now also stores only the parsed vars,
# not the entire output of running the batch file - saves a bit
# of time not parsing every time.

script_env_cache = None

def script_env(env, script, args=None):
    global script_env_cache

    if script_env_cache is None:
        script_env_cache = common.read_script_env_cache()
    cache_key = (script, args if args else None)
    cache_data = script_env_cache.get(cache_key, None)

    # Brief sanity check: if we got a value for the key,
    # see if it has a VCToolsInstallDir entry that is not empty.
    # If so, and that path does not exist, invalidate the entry.
    # If empty, this is an old compiler, just leave it alone.
    if cache_data is not None:
        try:
            toolsdir = cache_data["VCToolsInstallDir"]
        except KeyError:
            # we write this value, so should not happen
            pass
        else:
            if toolsdir:
                toolpath = Path(toolsdir[0])
                if not toolpath.exists():
                    cache_data = None

    if cache_data is None:
        stdout = common.get_output(script, args)
        cache_data = common.parse_output(stdout)

        # debug(stdout)
        olines = stdout.splitlines()

        # process stdout: batch file errors (not necessarily first line)
        script_errlog = []
        for line in olines:
            if re_script_output_error.match(line):
                if not script_errlog:
                    script_errlog.append('vc script errors detected:')
                script_errlog.append(line)

        if script_errlog:
            script_errmsg = '\n'.join(script_errlog)

            have_cl = False
            if cache_data and 'PATH' in cache_data:
                for p in cache_data['PATH']:
                    if os.path.exists(os.path.join(p, _CL_EXE_NAME)):
                        have_cl = True
                        break

            debug(
                'script=%s args=%s have_cl=%s, errors=%s',
                repr(script), repr(args), repr(have_cl), script_errmsg
            )
            MSVC.Policy.msvc_scripterror_handler(env, script_errmsg)

            if not have_cl:
                # detected errors, cl.exe not on path
                raise BatchFileExecutionError(script_errmsg)

        # once we updated cache, give a chance to write out if user wanted
        script_env_cache[cache_key] = cache_data
        common.write_script_env_cache(script_env_cache)

    return cache_data

def get_default_version(env):
    msvc_version = env.get('MSVC_VERSION')
    msvs_version = env.get('MSVS_VERSION')
    debug('msvc_version:%s msvs_version:%s', msvc_version, msvs_version)

    if msvs_version and not msvc_version:
        SCons.Warnings.warn(
                SCons.Warnings.DeprecatedWarning,
                "MSVS_VERSION is deprecated: please use MSVC_VERSION instead ")
        return msvs_version
    elif msvc_version and msvs_version:
        if not msvc_version == msvs_version:
            SCons.Warnings.warn(
                    SCons.Warnings.VisualVersionMismatch,
                    "Requested msvc version (%s) and msvs version (%s) do "
                    "not match: please use MSVC_VERSION only to request a "
                    "visual studio version, MSVS_VERSION is deprecated"
                    % (msvc_version, msvs_version))
        return msvs_version

    if not msvc_version:
        msvc_version = msvc_default_version(env)
        if not msvc_version:
            #SCons.Warnings.warn(SCons.Warnings.VisualCMissingWarning, msg)
            debug('No installed VCs')
            return None
        debug('using default installed MSVC version %s', repr(msvc_version))
    else:
        debug('using specified MSVC version %s', repr(msvc_version))

    return msvc_version

def msvc_setup_env_once(env, tool=None):
    try:
        has_run = env["MSVC_SETUP_RUN"]
    except KeyError:
        has_run = False

    if not has_run:
        MSVC.SetupEnvDefault.register_setup(env, msvc_exists)
        msvc_setup_env(env)
        env["MSVC_SETUP_RUN"] = True

    req_tools = MSVC.SetupEnvDefault.register_iserror(env, tool, msvc_exists)
    if req_tools:
        msg = "No versions of the MSVC compiler were found.\n" \
              "  Visual Studio C/C++ compilers may not be set correctly.\n" \
              "  Requested tool(s) are: {}".format(req_tools)
        MSVC.Policy.msvc_notfound_handler(env, msg)

def msvc_find_valid_batch_script(env, version):
    """Find and execute appropriate batch script to set up build env.

    The MSVC build environment depends heavily on having the shell
    environment set.  SCons does not inherit that, and does not count
    on that being set up correctly anyway, so it tries to find the right
    MSVC batch script, or the right arguments to the generic batch script
    vcvarsall.bat, and run that, so we have a valid environment to build in.
    There are dragons here: the batch scripts don't fail (see comments
    elsewhere), they just leave you with a bad setup, so try hard to
    get it right.
    """

    # Find the host, target, and all candidate (host, target) platform combinations:
    platforms = get_host_target(env, version)
    debug("host_platform %s, target_platform %s host_target_list %s", *platforms)
    host_platform, target_platform, host_target_list = platforms

    d = None
    version_installed = False
    for host_arch, target_arch, in host_target_list:
        # Set to current arch.
        env['TARGET_ARCH'] = target_arch
        arg = ''

        # Try to locate a batch file for this host/target platform combo
        try:
            (vc_script, arg, vc_dir, sdk_script) = find_batch_file(env, version, host_arch, target_arch)
            debug('vc_script:%s vc_script_arg:%s sdk_script:%s', vc_script, arg, sdk_script)
            version_installed = True
        except VisualCException as e:
            msg = str(e)
            debug('Caught exception while looking for batch file (%s)', msg)
            version_installed = False
            continue

        # Try to use the located batch file for this host/target platform combo
        debug('use_script 2 %s, args:%s', repr(vc_script), arg)
        found = None
        if vc_script:
            arg = MSVC.ScriptArguments.msvc_script_arguments(env, version, vc_dir, arg)
            try:
                d = script_env(env, vc_script, args=arg)
                found = vc_script
            except BatchFileExecutionError as e:
                debug('use_script 3: failed running VC script %s: %s: Error:%s', repr(vc_script), arg, e)
                vc_script=None
                continue
        if not vc_script and sdk_script:
            debug('use_script 4: trying sdk script: %s', sdk_script)
            try:
                d = script_env(env, sdk_script)
                found = sdk_script
            except BatchFileExecutionError as e:
                debug('use_script 5: failed running SDK script %s: Error:%s', repr(sdk_script), e)
                continue
        elif not vc_script and not sdk_script:
            debug('use_script 6: Neither VC script nor SDK script found')
            continue

        debug("Found a working script/target: %s/%s", repr(found), arg)
        break # We've found a working target_platform, so stop looking

    # If we cannot find a viable installed compiler, reset the TARGET_ARCH
    # To it's initial value
    if not d:
        env['TARGET_ARCH'] = target_platform

        if version_installed:
            msg = "MSVC version '{}' working host/target script was not found.\n" \
                  "  Host = '{}', Target = '{}'\n" \
                  "  Visual Studio C/C++ compilers may not be set correctly".format(
                     version, host_platform, target_platform
                  )
        else:
            installed_vcs = get_installed_vcs(env)
            if installed_vcs:
                msg = "MSVC version '{}' was not found.\n" \
                      "  Visual Studio C/C++ compilers may not be set correctly.\n" \
                      "  Installed versions are: {}".format(version, installed_vcs)
            else:
                msg = "MSVC version '{}' was not found.\n" \
                      "  No versions of the MSVC compiler were found.\n" \
                      "  Visual Studio C/C++ compilers may not be set correctly".format(version)

        MSVC.Policy.msvc_notfound_handler(env, msg)

    return d

_UNDEFINED = object()

def get_use_script_use_settings(env):

    #   use_script  use_settings   return values   action
    #     value       ignored      (value, None)   use script or bypass detection
    #   undefined  value not None  (False, value)  use dictionary
    #   undefined  undefined/None  (True,  None)   msvc detection

    # None (documentation) or evaluates False (code): bypass detection
    # need to distinguish between undefined and None
    use_script = env.get('MSVC_USE_SCRIPT', _UNDEFINED)

    if use_script != _UNDEFINED:
        # use_script defined, use_settings ignored (not type checked)
        return use_script, None

    # undefined or None: use_settings ignored
    use_settings = env.get('MSVC_USE_SETTINGS', None)

    if use_settings is not None:
        # use script undefined, use_settings defined and not None (type checked)
        return False, use_settings

    # use script undefined, use_settings undefined or None
    return True, None

def msvc_setup_env(env):
    debug('called')
    version = get_default_version(env)
    if version is None:
        if not msvc_setup_env_user(env):
            MSVC.SetupEnvDefault.set_nodefault()
        return None

    # XXX: we set-up both MSVS version for backward
    # compatibility with the msvs tool
    env['MSVC_VERSION'] = version
    env['MSVS_VERSION'] = version
    env['MSVS'] = {}

    use_script, use_settings = get_use_script_use_settings(env)
    if SCons.Util.is_String(use_script):
        use_script = use_script.strip()
        if not os.path.exists(use_script):
            raise MSVCScriptNotFound('Script specified by MSVC_USE_SCRIPT not found: "{}"'.format(use_script))
        args = env.subst('$MSVC_USE_SCRIPT_ARGS')
        debug('use_script 1 %s %s', repr(use_script), repr(args))
        d = script_env(env, use_script, args)
    elif use_script:
        d = msvc_find_valid_batch_script(env,version)
        debug('use_script 2 %s', d)
        if not d:
            return d
    elif use_settings is not None:
        if not SCons.Util.is_Dict(use_settings):
            error_msg = 'MSVC_USE_SETTINGS type error: expected a dictionary, found {}'.format(type(use_settings).__name__)
            raise MSVCUseSettingsError(error_msg)
        d = use_settings
        debug('use_settings %s', d)
    else:
        debug('MSVC_USE_SCRIPT set to False')
        warn_msg = "MSVC_USE_SCRIPT set to False, assuming environment " \
                   "set correctly."
        SCons.Warnings.warn(SCons.Warnings.VisualCMissingWarning, warn_msg)
        return None

    for k, v in d.items():
        env.PrependENVPath(k, v, delete_existing=True)
        debug("env['ENV']['%s'] = %s", k, env['ENV'][k])

    # final check to issue a warning if the compiler is not present
    if not find_program_path(env, 'cl'):
        debug("did not find %s", _CL_EXE_NAME)
        if CONFIG_CACHE:
            propose = "SCONS_CACHE_MSVC_CONFIG caching enabled, remove cache file {} if out of date.".format(CONFIG_CACHE)
        else:
            propose = "It may need to be installed separately with Visual Studio."
        warn_msg = "Could not find MSVC compiler 'cl'. {}".format(propose)
        SCons.Warnings.warn(SCons.Warnings.VisualCMissingWarning, warn_msg)

def msvc_exists(env=None, version=None):
    vcs = get_installed_vcs(env)
    if version is None:
        rval = len(vcs) > 0
    else:
        rval = version in vcs
    if not rval:
        debug('version=%s, return=%s', repr(version), rval)
    return rval

def msvc_setup_env_user(env=None):
    rval = False
    if env:

        # Intent is to use msvc tools:
        #     MSVC_VERSION:         defined and evaluates True
        #     MSVS_VERSION:         defined and evaluates True
        #     MSVC_USE_SCRIPT:      defined and (is string or evaluates False)
        #     MSVC_USE_SETTINGS:    defined and is not None

        # defined and is True
        for key in ['MSVC_VERSION', 'MSVS_VERSION']:
            if key in env and env[key]:
                rval = True
                debug('key=%s, return=%s', repr(key), rval)
                return rval

        # defined and (is string or is False)
        for key in ['MSVC_USE_SCRIPT']:
            if key in env and (SCons.Util.is_String(env[key]) or not env[key]):
                rval = True
                debug('key=%s, return=%s', repr(key), rval)
                return rval

        # defined and is not None
        for key in ['MSVC_USE_SETTINGS']:
            if key in env and env[key] is not None:
                rval = True
                debug('key=%s, return=%s', repr(key), rval)
                return rval

    debug('return=%s', rval)
    return rval

def msvc_setup_env_tool(env=None, version=None, tool=None):
    MSVC.SetupEnvDefault.register_tool(env, tool, msvc_exists)
    rval = False
    if not rval and msvc_exists(env, version):
        rval = True
    if not rval and msvc_setup_env_user(env):
        rval = True
    return rval

def msvc_sdk_versions(version=None, msvc_uwp_app=False):
    debug('version=%s, msvc_uwp_app=%s', repr(version), repr(msvc_uwp_app))

    rval = []

    if not version:
        version = msvc_default_version()

    if not version:
        debug('no msvc versions detected')
        return rval

    version_def = MSVC.Util.msvc_extended_version_components(version)
    if not version_def:
        msg = 'Unsupported version {}'.format(repr(version))
        raise MSVCArgumentError(msg)

    rval = MSVC.WinSDK.get_msvc_sdk_version_list(version, msvc_uwp_app)
    return rval

def msvc_toolset_versions(msvc_version=None, full=True, sxs=False):
    debug('msvc_version=%s, full=%s, sxs=%s', repr(msvc_version), repr(full), repr(sxs))

    env = None
    rval = []

    if not msvc_version:
        msvc_version = msvc_default_version()

    if not msvc_version:
        debug('no msvc versions detected')
        return rval

    if msvc_version not in _VCVER:
        msg = 'Unsupported msvc version {}'.format(repr(msvc_version))
        raise MSVCArgumentError(msg)

    vc_dir = find_vc_pdir(env, msvc_version)
    if not vc_dir:
        debug('VC folder not found for version %s', repr(msvc_version))
        return rval

    rval = MSVC.ScriptArguments._msvc_toolset_versions_internal(msvc_version, vc_dir, full=full, sxs=sxs)
    return rval

def msvc_toolset_versions_spectre(msvc_version=None):
    debug('msvc_version=%s', repr(msvc_version))

    env = None
    rval = []

    if not msvc_version:
        msvc_version = msvc_default_version()

    if not msvc_version:
        debug('no msvc versions detected')
        return rval

    if msvc_version not in _VCVER:
        msg = 'Unsupported msvc version {}'.format(repr(msvc_version))
        raise MSVCArgumentError(msg)

    vc_dir = find_vc_pdir(env, msvc_version)
    if not vc_dir:
        debug('VC folder not found for version %s', repr(msvc_version))
        return rval

    rval = MSVC.ScriptArguments._msvc_toolset_versions_spectre_internal(msvc_version, vc_dir)
    return rval

def msvc_query_version_toolset(version=None, prefer_newest=True):
    """
    Returns an msvc version and a toolset version given a version
    specification.

    This is an EXPERIMENTAL proxy for using a toolset version to perform
    msvc instance selection.  This function will be removed when
    toolset version is taken into account during msvc instance selection.

    Search for an installed Visual Studio instance that supports the
    specified version.

    When the specified version contains a component suffix (e.g., Exp),
    the msvc version is returned and the toolset version is None. No
    search if performed.

    When the specified version does not contain a component suffix, the
    version is treated as a toolset version specification. A search is
    performed for the first msvc instance that contains the toolset
    version.

    Only Visual Studio 2017 and later support toolset arguments.  For
    Visual Studio 2015 and earlier, the msvc version is returned and
    the toolset version is None.

    Args:

        version: str
            The version specification may be an msvc version or a toolset
            version.

        prefer_newest: bool
            True:  prefer newer Visual Studio instances.
            False: prefer the "native" Visual Studio instance first. If
                   the native Visual Studio instance is not detected, prefer
                   newer Visual Studio instances.

    Returns:
        tuple: A tuple containing the msvc version and the msvc toolset version.
               The msvc toolset version may be None.

    Raises:
        MSVCToolsetVersionNotFound: when the specified version is not found.
        MSVCArgumentError: when argument validation fails.
    """
    debug('version=%s, prefer_newest=%s', repr(version), repr(prefer_newest))

    env = None
    msvc_version = None
    msvc_toolset_version = None

    if not version:
        version = msvc_default_version()

    if not version:
        debug('no msvc versions detected')
        return msvc_version, msvc_toolset_version

    version_def = MSVC.Util.msvc_extended_version_components(version)

    if not version_def:
        msg = 'Unsupported msvc version {}'.format(repr(version))
        raise MSVCArgumentError(msg)

    if version_def.msvc_suffix:
        if version_def.msvc_verstr != version_def.msvc_toolset_version:
            # toolset version with component suffix
            msg = 'Unsupported toolset version {}'.format(repr(version))
            raise MSVCArgumentError(msg)

    if version_def.msvc_vernum > 14.0:
        # VS2017 and later
        force_toolset_msvc_version = False
    else:
        # VS2015 and earlier
        force_toolset_msvc_version = True
        extended_version = version_def.msvc_verstr + '0.00000'
        if not extended_version.startswith(version_def.msvc_toolset_version):
            # toolset not equivalent to msvc version
            msg = 'Unsupported toolset version {} (expected {})'.format(
                repr(version), repr(extended_version)
            )
            raise MSVCArgumentError(msg)

    msvc_version = version_def.msvc_version

    if msvc_version not in MSVC.Config.MSVC_VERSION_TOOLSET_SEARCH_MAP:
        # VS2013 and earlier
        debug(
            'ignore: msvc_version=%s, msvc_toolset_version=%s',
            repr(msvc_version), repr(msvc_toolset_version)
        )
        return msvc_version, msvc_toolset_version

    if force_toolset_msvc_version:
        query_msvc_toolset_version = version_def.msvc_verstr
    else:
        query_msvc_toolset_version = version_def.msvc_toolset_version

    if prefer_newest:
        query_version_list = MSVC.Config.MSVC_VERSION_TOOLSET_SEARCH_MAP[msvc_version]
    else:
        query_version_list = MSVC.Config.MSVC_VERSION_TOOLSET_DEFAULTS_MAP[msvc_version] + \
                             MSVC.Config.MSVC_VERSION_TOOLSET_SEARCH_MAP[msvc_version]

    seen_msvc_version = set()
    for query_msvc_version in query_version_list:

        if query_msvc_version in seen_msvc_version:
            continue
        seen_msvc_version.add(query_msvc_version)

        vc_dir = find_vc_pdir(env, query_msvc_version)
        if not vc_dir:
            continue

        if query_msvc_version.startswith('14.0'):
            # VS2015 does not support toolset version argument
            msvc_toolset_version = None
            debug(
                'found: msvc_version=%s, msvc_toolset_version=%s',
                repr(query_msvc_version), repr(msvc_toolset_version)
            )
            return query_msvc_version, msvc_toolset_version

        try:
            toolset_vcvars = MSVC.ScriptArguments._msvc_toolset_internal(query_msvc_version, query_msvc_toolset_version, vc_dir)
            if toolset_vcvars:
                msvc_toolset_version = toolset_vcvars
                debug(
                    'found: msvc_version=%s, msvc_toolset_version=%s',
                    repr(query_msvc_version), repr(msvc_toolset_version)
                )
                return query_msvc_version, msvc_toolset_version

        except MSVCToolsetVersionNotFound:
            pass

    msvc_toolset_version = query_msvc_toolset_version

    debug(
        'not found: msvc_version=%s, msvc_toolset_version=%s',
        repr(msvc_version), repr(msvc_toolset_version)
    )

    if version_def.msvc_verstr == msvc_toolset_version:
        msg = 'MSVC version {} was not found'.format(repr(version))
        MSVC.Policy.msvc_notfound_handler(None, msg)
        return msvc_version, msvc_toolset_version

    msg = 'MSVC toolset version {} not found'.format(repr(version))
    raise MSVCToolsetVersionNotFound(msg)


# internal consistency check (should be last)
MSVC._verify()

