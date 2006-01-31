"""SCons.Job

This module defines the Serial and Parallel classes that execute tasks to
complete a build. The Jobs class provides a higher level interface to start,
stop, and wait on jobs.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Job.py 0.96.1.D001 2004/08/23 09:55:29 knight"

class Jobs:
    """An instance of this class initializes N jobs, and provides
    methods for starting, stopping, and waiting on all N jobs.
    """

    def __init__(self, num, taskmaster):
        """
        create 'num' jobs using the given taskmaster.

        If 'num' is 1 or less, then a serial job will be used,
        otherwise a parallel job with 'num' worker threads will
        be used.

        The 'num_jobs' attribute will be set to the actual number of jobs
        allocated.  If more than one job is requested but the Parallel
        class can't do it, it gets reset to 1.  Wrapping interfaces that
        care should check the value of 'num_jobs' after initialization.
        """

        self.job = None
        if num > 1:
            try:
                self.job = Parallel(taskmaster, num)
                self.num_jobs = num
            except NameError:
                pass
        if self.job is None:
            self.job = Serial(taskmaster)
            self.num_jobs = 1

    def run(self):
        """run the job"""
        try:
            self.job.start()
        except KeyboardInterrupt:
            # mask any further keyboard interrupts so that scons
            # can shutdown cleanly:
            # (this only masks the keyboard interrupt for Python,
            #  child processes can still get the keyboard interrupt)
            import signal
            signal.signal(signal.SIGINT, signal.SIG_IGN)
            raise

class Serial:
    """This class is used to execute tasks in series, and is more efficient
    than Parallel, but is only appropriate for non-parallel builds. Only
    one instance of this class should be in existence at a time.

    This class is not thread safe.
    """

    def __init__(self, taskmaster):
        """Create a new serial job given a taskmaster. 

        The taskmaster's next_task() method should return the next task
        that needs to be executed, or None if there are no more tasks. The
        taskmaster's executed() method will be called for each task when it
        is successfully executed or failed() will be called if it failed to
        execute (e.g. execute() raised an exception). The taskmaster's
        is_blocked() method will not be called.  """
        
        self.taskmaster = taskmaster

    def start(self):
        """Start the job. This will begin pulling tasks from the taskmaster
        and executing them, and return when there are no more tasks. If a task
        fails to execute (i.e. execute() raises an exception), then the job will
        stop."""
        
        while 1:
            task = self.taskmaster.next_task()

            if task is None:
                break

            try:
                task.prepare()
                task.execute()
            except KeyboardInterrupt:
                raise
            except:
                task.exception_set()
                # Let the failed() callback function arrange for the
                # build to stop if that's appropriate.
                task.failed()
            else:
                task.executed()

            task.postprocess()


# Trap import failure so that everything in the Job module but the
# Parallel class (and its dependent classes) will work if the interpreter
# doesn't support threads.
try:
    import Queue
    import threading
except ImportError:
    pass
else:
    class Worker(threading.Thread):
        """A worker thread waits on a task to be posted to its request queue,
        dequeues the task, executes it, and posts a tuple including the task
        and a boolean indicating whether the task executed successfully. """

        def __init__(self, requestQueue, resultsQueue):
            threading.Thread.__init__(self)
            self.setDaemon(1)
            self.requestQueue = requestQueue
            self.resultsQueue = resultsQueue
            self.start()

        def run(self):
            while 1:
                task = self.requestQueue.get()

                try:
                    task.execute()
                except KeyboardInterrupt:
                    # be explicit here for test/interrupts.py
                    ok = False
                except:
                    task.exception_set()
                    ok = 0
                else:
                    ok = 1

                self.resultsQueue.put((task, ok))

    class ThreadPool:
        """This class is responsible for spawning and managing worker threads."""

        def __init__(self, num):
            """Create the request and reply queues, and 'num' worker threads."""
            self.requestQueue = Queue.Queue(0)
            self.resultsQueue = Queue.Queue(0)

            # Create worker threads
            for i in range(num):
                Worker(self.requestQueue, self.resultsQueue)

        def put(self, obj):
            """Put task into request queue."""
            self.requestQueue.put(obj)

        def get(self, block = 1):
            """Remove and return a result tuple from the results queue."""
            return self.resultsQueue.get(block)
            
        def get_nowait(self):
            """Remove and result a result tuple from the results queue 
            without blocking."""
            return self.get(0)

    class Parallel:
        """This class is used to execute tasks in parallel, and is somewhat 
        less efficient than Serial, but is appropriate for parallel builds.

        This class is thread safe.
        """

        def __init__(self, taskmaster, num):
            """Create a new parallel job given a taskmaster.

            The taskmaster's next_task() method should return the next task
            that needs to be executed, or None if there are no more tasks. The
            taskmaster's executed() method will be called for each task when it
            is successfully executed or failed() will be called if the task
            failed to execute (i.e. execute() raised an exception).  The
            taskmaster's is_blocked() method should return true iff there are
            more tasks, but they can't be executed until one or more other
            tasks have been executed. next_task() will be called iff
            is_blocked() returned false.

            Note: calls to taskmaster are serialized, but calls to execute() on
            distinct tasks are not serialized, because that is the whole point
            of parallel jobs: they can execute multiple tasks
            simultaneously. """

            self.taskmaster = taskmaster
            self.tp = ThreadPool(num)

            self.jobs = 0
            self.maxjobs = num

        def start(self):
            """Start the job. This will begin pulling tasks from the
            taskmaster and executing them, and return when there are no
            more tasks. If a task fails to execute (i.e. execute() raises
            an exception), then the job will stop."""

            while 1:
                if self.jobs < self.maxjobs:
                    task = self.taskmaster.next_task()
                    if task is None:
                        break

                    # prepare task for execution
                    try:
                        task.prepare()
                    except KeyboardInterrupt:
                        raise
                    except:
                        # Let the failed() callback function arrange for the
                        # build to stop if that's appropriate.
                        task.failed()

                    # dispatch task
                    self.tp.put(task)
                    self.jobs = self.jobs + 1

                while 1:
                    try:
                        task, ok = self.tp.get_nowait()
                    except Queue.Empty:
                        if not (self.jobs is self.maxjobs or self.taskmaster.is_blocked()):
                            break
                        task, ok = self.tp.get()

                    self.jobs = self.jobs - 1
                    if ok:
                        task.executed()
                    else:
                        task.failed()

                    task.postprocess()
