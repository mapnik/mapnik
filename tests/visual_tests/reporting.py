# -*- coding: utf-8 -*-

import sys

class Reporting:
    DIFF = 1
    NOT_FOUND = 2
    OTHER = 3
    REPLACE = 4
    LOAD_ERROR = 5
    def __init__(self, quiet):
        self.quiet = quiet
        self.passed = 0
        self.failed = 0
        self.overwrite_failures = False
        self.errors = [ #(type, actual, expected, diff, message)
         ]

    def format_time(self, render_time):
        if render_time > 0.5:
            return "%.2fs" % render_time
        else:
            return "%.0fms" % (render_time * 1000)

    def result_fail(self, actual, expected, diff, render_time):
        self.failed += 1
        if self.quiet:
            sys.stderr.write('\x1b[31m.\x1b[0m')
        else:
            print '\x1b[31m✘\x1b[0m (\x1b[34m%s, %u different pixels\x1b[0m)' % (self.format_time(render_time), diff)

        if self.overwrite_failures:
            self.errors.append((self.REPLACE, actual, expected, diff, None))
            contents = open(actual, 'r').read()
            open(expected, 'wb').write(contents)
        else:
            self.errors.append((self.DIFF, actual, expected, diff, None))

    def result_pass(self, actual, expected, diff, render_time):
        self.passed += 1
        if self.quiet:
            sys.stderr.write('\x1b[32m.\x1b[0m')
        else:
            print '\x1b[32m✓\x1b[0m (\x1b[34m%s\x1b[0m)' % self.format_time(render_time)

    def not_found(self, actual, expected):
        self.failed += 1
        self.errors.append((self.NOT_FOUND, actual, expected, 0, None))
        if self.quiet:
            sys.stderr.write('\x1b[33m.\x1b[0m')
        else:
            print '\x1b[33m?\x1b[0m (\x1b[34mReference file not found\x1b[0m)'
        if self.generate:
            contents = open(actual, 'r').read()
            open(expected, 'wb').write(contents)

    def other_error(self, expected, message):
        self.failed += 1
        self.errors.append((self.OTHER, None, expected, 0, message))
        if self.quiet:
            sys.stderr.write('\x1b[31m.\x1b[0m')
        else:
            print '\x1b[31m✘\x1b[0m (\x1b[34m%s\x1b[0m)' % message

    def load_error(self, name, message):
        self.failed += 1
        self.errors.append((self.LOAD_ERROR, name, None, 0, message))
        if self.quiet:
            sys.stderr.write('\x1b[31m.\x1b[0m')
        else:
            print '"%s": error while loading style file: \x1b[31m✘\x1b[0m (\x1b[34m%s\x1b[0m)' % (name, message)

    def summary(self):
        if len(self.errors) == 0:
            print '\nAll %s visual tests passed: \x1b[1;32m✓ \x1b[0m' % self.passed
            return 0
        print "\nVisual rendering: %s failed / %s passed" % (len(self.errors), self.passed)
        for idx, error in enumerate(self.errors):
            if error[0] == self.OTHER:
                print str(idx+1) + ") \x1b[31mfailure to run test \"%s\":\x1b[0m %s" % (error[2], error[4])
            elif error[0] == self.NOT_FOUND:
                if self.generate:
                    print str(idx+1) + ") Generating reference image: '%s'" % error[2]
                else:
                    print str(idx+1) + ")Could not verify %s: No reference image found!" % error[1]
                continue
            elif error[0] == self.DIFF:
                print str(idx+1) + ") \x1b[34m%s different pixels\x1b[0m:\n\t%s (\x1b[31mactual\x1b[0m)\n\t%s (\x1b[32mexpected\x1b[0m)" % (error[3], error[1], error[2])
            elif error[0] == self.REPLACE:
                print str(idx+1) + ") \x1b[31mreplaced reference with new version:\x1b[0m %s" % error[2]
            elif error[0] == self.LOAD_ERROR:
                print str(idx+1) + ") \x1b[31mfailure to load style \"%s\":\x1b[0m %s" % (error[1], error[4])
        return 1

    def show_file(self, postfix, renderer):
        if not self.quiet:
            print "\"%s\" with %s..." % (postfix, renderer),