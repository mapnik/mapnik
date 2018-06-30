#! /usr/bin/env perl
#
#   Re-generate hyperlinks in place:
#
#       perl -i scripts/markdown-hyperlinks.pl CHANGELOG.md
#
#   Generate to another file:
#
#       scripts/markdown-hyperlinks.pl CHANGELOG.md > CHANGELOG-autolink.md

use strict;
use warnings;

my $user = qr/ [a-zA-Z] [a-zA-Z0-9]* /x;
my $repo = qr/ [a-zA-Z] [a-zA-Z0-9.-]* /x;

while (<>) {

    # make links from @username references
    # (except when escaped like \@foobar or `@foobar`)
    s;(?<! \[ | \\ | \` ) \@ ($user) \b (?! \] )
     ;[\@$1](https://github.com/$1);xg;

    # make links from #1234 references (except when escaped like \#5)
    # we can't tell whether the number refers to an issue or a pull request,
    # luckily link to issues/1234 works in either case
    s;(?<! \[ | \\ ) \# ([0-9]+) \b (?! \] )
     ;[\#$1](https://github.com/mapnik/mapnik/issues/$1);xg;

    # make links from commit hashes
    # (accept 7 or 9-40 hex digits, but not 8 which could be a date)
    s;(?<! \[ | / ) \b (([0-9a-f]{7})([0-9a-f]{2,33})?) \b (?! \] )
     ;[$2](https://github.com/mapnik/mapnik/commit/$1);xg;

    # make shortcut links from raw URIs (which GFM turns into proper links,
    # but doesn't contract even though it could)
    # - issues
    s;(?<! \] \( ) (https://github\.com/mapnik/mapnik/(?:issues|pull)/([0-9]+))
     ;[\#$2]($1);xg;
    s;(?<! \] \( ) (https://github\.com/($user/$repo)/(?:issues|pull)/([0-9]+))
     ;[$2\#$3]($1);xg;
    # - commit hashes
    s;(?<! \] \( ) (https://github\.com/mapnik/mapnik/commit/([0-9a-f]{7})[0-9a-f]{0,33}) \b
     ;[$2]($1);xg;
    s;(?<! \] \( ) (https://github\.com/($user/$repo)/commit/([0-9a-f]{7})[0-9a-f]{0,33}) \b
     ;[$2\@$3]($1);xg;

    print;
}
