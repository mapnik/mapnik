# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007 The SCons Foundation
# src/README.txt 0.97.D001 2007/05/17 11:35:19 knight


                 SCons - a software construction tool

                         Version 0.97


This is a beta release of SCons, a tool for building software (and other
files).  SCons is implemented in Python, and its "configuration files"
are actually Python scripts, allowing you to use the full power of a
real scripting language to solve build problems.  You do not, however,
need to know Python to use SCons effectively.

See the RELEASE.txt file for notes about this specific release,
including known problems.  See the CHANGES.txt file for a list of
changes since the previous release.


LATEST VERSION
==============

Before going further, you can check that this package you have is
the latest version by checking the SCons download page at:

        http://www.scons.org/download.html


EXECUTION REQUIREMENTS
======================

Running SCons requires Python version 1.5.2 or later.  There should be
no other dependencies or requirements to run SCons.  (There is, however,
an additional requirement to *install* SCons from this particular
package; see the next section.)

By default, SCons knows how to search for available programming tools
on various systems--see the SCons man page for details.  You may,
of course, override the default SCons choices made by appropriate
configuration of Environment construction variables.


INSTALLATION REQUIREMENTS
=========================

Installing SCons from this package requires the Python distutils
package.  The distutils package was not shipped as a standard part of
Python until Python version 1.6, so if your system is running Python
1.5.2, you may not have distutils installed.  If you are running
Python version 1.6 or later, you should be fine.

NOTE TO RED HAT USERS:  Red Hat shipped Python 1.5.2 as the default all
the way up to Red Hat Linux 7.3, so you probably do *not* have distutils
installed, unless you have already done so manually or are running Red
Hat 8.0 or later.

In this case, your options are:

    --  (Recommended.)  Install from a pre-packaged SCons package that
        does not require distutils:

            Red Hat Linux       scons-0.97-1.noarch.rpm

            Debian GNU/Linux    scons_0.97-1_all.deb
                                (or use apt-get)

            Windows             scons-0.97.win32.exe

    --  (Optional.)  Download the latest distutils package from the
        following URL:

            http://www.python.org/sigs/distutils-sig/download.html

        Install the distutils according to the instructions on the page.
        You can then proceed to the next section to install SCons from
        this package.


INSTALLATION
============

Assuming your system satisfies the installation requirements in the
previous section, install SCons from this package simply by running the
provided Python-standard setup script as follows:

        # python setup.py install

By default, the above command will do the following:

    --  Install the version-numbered "scons-0.97" and "sconsign-0.97"
        scripts in the default system script directory (/usr/bin or
        C:\Python*\Scripts, for example).  This can be disabled by
        specifying the "--no-version-script" option on the command
        line.

    --  Install scripts named "scons" and "sconsign" scripts in the
        default system script directory (/usr/bin or C:\Python*\Scripts,
        for example).  This can be disabled by specifying the
        "--no-scons-script" option on the command line, which is useful
        if you want to install and experiment with a new version before
        making it the default on your system.

        On UNIX or Linux systems, you can have the "scons" and "sconsign"
        scripts be hard links or symbolic links to the "scons-0.97" and
        "sconsign-0.97" scripts by specifying the "--hardlink-scons"
        or "--symlink-scons" options on the command line.

    --  Install "scons-0.97.bat" and "scons.bat" wrapper scripts in the
        Python prefix directory on Windows (C:\Python*, for example).
        This can be disabled by specifying the "--no-install-bat" option
        on the command line.

        On UNIX or Linux systems, the "--install-bat" option may be
        specified to have "scons-0.97.bat" and "scons.bat" files
        installed in the default system script directory, which is useful
        if you want to install SCons in a shared file system directory
        that can be used to execute SCons from both UNIX/Linux and
        Windows systems.

    --  Install the SCons build engine (a Python module) in an
        appropriate version-numbered SCons library directory
        (/usr/lib/scons-0.97 or C:\Python*\scons-0.97, for example).
        See below for more options related to installing the build
        engine library.

    --  Install the troff-format man pages in an appropriate directory
        on UNIX or Linux systems (/usr/share/man/man1 or /usr/man/man1,
        for example).  This can be disabled by specifying the
        "--no-install-man" option on the command line.  The man pages
        can be installed on Windows systems by specifying the
        "--install-man" option on the command line.

Note that, by default, SCons does not install its build engine library
in the standard Python library directories.  If you want to be able to
use the SCons library modules (the build engine) in other Python
scripts, specify the "--standard-lib" option on the command line, as
follows:

        # python setup.py install --standard-lib

This will install the build engine in the standard Python library
directory (/usr/lib/python*/site-packages or
C:\Python*\Lib\site-packages).

Alternatively, you can have SCons install its build engine library in a
hard-coded standalone library directory, instead of the default
version-numbered directory, by specifying the "--standalone-lib" option
on the command line, as follows:

        # python setup.py install --standalone-lib

This is usually not recommended, however.

Note that, to install SCons in any of the above system directories,
you should have system installation privileges (that is, "root" or
"Administrator") when running the setup.py script.  If you don't have
system installation privileges, you can use the --prefix option to
specify an alternate installation location, such as your home directory:

        $ python setup.py install --prefix=$HOME

This will install SCons in the appropriate locations relative to
$HOME--that is, the scons script itself $HOME/bin and the associated
library in $HOME/lib/scons, for example.


DOCUMENTATION
=============

See the RELEASE.txt file for notes about this specific release,
including known problems.  See the CHANGES.txt file for a list of
changes since the previous release.

The scons.1 man page is included in this package, and contains a section
of small examples for getting started using SCons.

Additional documentation for SCons is available at:

        http://www.scons.org/doc.html


LICENSING
=========

SCons is distributed under the MIT license, a full copy of which is
available in the LICENSE.txt file. The MIT license is an approved Open
Source license, which means:

        This software is OSI Certified Open Source Software.  OSI
        Certified is a certification mark of the Open Source Initiative.

More information about OSI certifications and Open Source software is
available at:

        http://www.opensource.org/


REPORTING BUGS
==============

Please report bugs by following the detailed instructions on our Bug
Submission page:

        http://scons.tigris.org/bug-submission.html

You can also send mail to the SCons developers' mailing list:

        dev@scons.tigris.org

But even if you send email to the mailing list please make sure that you
ALSO submit a bug report to the project page bug tracker, because bug
reports in email often get overlooked in the general flood of messages.


MAILING LISTS
=============

An active mailing list for users of SCons is available.  You may send
questions or comments to the list at:

        users@scons.tigris.org

You may subscribe to the mailing list by sending email to:

        users-subscribe@scons.tigris.org

There is also a low-volume mailing list available for announcements
about SCons.  Subscribe by sending email to:

        announce-subscribe@scons.tigris.org

There are other mailing lists available for SCons developers, for
notification of SCons code changes, and for notification of updated
bug reports and project documents.  Please see our mailing lists page
for details.


DONATIONS
=========

If you find SCons helpful, please consider making a donation (of cash,
software, or hardware) to support continued work on the project.
Information is available at:

        http://www.scons.org/donate.html


FOR MORE INFORMATION
====================

Check the SCons web site at:

        http://www.scons.org/


AUTHOR INFO
===========

Steven Knight
knight at baldmt dot com
http://www.baldmt.com/~knight/

With plenty of help from the SCons Development team:
        Chad Austin
        Charles Crain
        Steve Leblanc
        Greg Noel
        Gary Oberbrunner
        Anthony Roach
        Greg Spencer
        Christoph Wiedemann

