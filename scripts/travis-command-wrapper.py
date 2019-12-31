#!/usr/bin/python
#
# Author:       Patrick Ohly <patrick.ohly@intel.com>
# Copyright:    Copyright (C) 2015 Intel Corporation
#
# This file is licensed under the MIT license, see COPYING.MIT in
# this source distribution for the terms.

# Runs a command, pipes its output to stdout, and injects status
# reports at regular time interval.
#
# This ensures that TravisCI does not abort the command just because
# it is silent for more than 10 minutes, as it can happen with bitbake
# when working on a single complex task, like "bitbake linux-yocto".
#
# Piping bitbake stdout has the advantage that bitbake enters
# non-interactive output mode, which it would do when run by TravisCI
# directly.
#
# Finally, the default status messages give some sense of memory
# and disk usage, which is critical in the rather constrained
# TravisCI environments.

import errno
import optparse
import signal
import subprocess
import sys
import time

parser = optparse.OptionParser()
parser.add_option("-s", "--status",
                  help="invoked in a shell when it is time for a status report",
                  # 200 columns is readable in the TravisCI Web UI without wrapping.
                  # Depends of course on screen and font size. Resizing top output
                  # only works (and is needed) on the more recent Trusty TravisCI
                  # environment.
                  default="date; free; df -h .; COLUMNS=200 LINES=30 top -w -b -n 1 2>/dev/null || top -n 1; ps x --cols 200 --forest",
                  metavar="SHELL-CMD")
parser.add_option("-i", "--interval",
                  help="repeat status at intervals of this amount of seconds, 0 to disable",
                  default=300,
                  metavar="SECONDS", type="int")
parser.add_option("-d", "--deadline",
                  help="stop execution when reaching the given time",
                  default=time.time,
                  metavar="SECONDS-SINCE-EPOCH", type="int")

(options, args) = parser.parse_args()

def check_deadline(now):
    if options.deadline > 0 and options.deadline < now:
        print("\n\n*** travis-cmd-wrapper: deadline reached, shutting down ***\n\n")
        sys.exit(1)
    else:
        print("deadline not reached: %s > %s" % (options.deadline,now))

# Set up status alarm. When we have a deadline, we need to check more often
# and/or sooner. Sending a SIGALRM manually will also trigger a status report
# (not really possible in TravisCI, but may be useful elsewhere).
now = time.time()
next_status = now + options.interval
alarm_interval = max(options.interval, 0)
if options.deadline:
    check_deadline(now)
    if options.deadline < now + 60:
        # Wake up a little too late, to be sure that we trigger the if check.
        deadline_alarm_interval = max(int(options.deadline + 2 - now), 1)
    elif next_status > 60:
        deadline_alarm_interval = 60
    if deadline_alarm_interval < alarm_interval:
        alarm_interval = deadline_alarm_interval

def status(signum, frame):
    global next_status
    now = time.time()
    if options.interval < 0 or now >= next_status:
        subprocess.call(options.status, shell=True)
        next_status = now + options.interval
    check_deadline(now)
    if alarm_interval > 0:
        signal.alarm(alarm_interval)

# Run command.
try:
    cmd = subprocess.Popen(args, stdout=subprocess.PIPE)

    # Arm timer and handler.
    signal.signal(signal.SIGALRM, status)
    if alarm_interval > 0:
        signal.alarm(alarm_interval)

    while cmd.poll() is None:
        try:
            line = cmd.stdout.readline()
            sys.stdout.write(line)
            sys.stdout.flush()
        except IOError as ex:
            if ex.errno != errno.EINTR:
                raise
finally:
    # If we go down, so must our child...
    if cmd.poll() is None:
        cmd.kill()

exit(1 if cmd.returncode else 0)