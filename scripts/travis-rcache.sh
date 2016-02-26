#! /bin/bash
#
# Given the following values set by Travis:
#   TRAVIS_REPO_SLUG = lightmare/mapnik
#   TRAVIS_BRANCH = experimental
#   TRAVIS_JOB_NUMBER = 89.3
#
# And these set in repository settings on Travis:
#   RCACHE_REMOTE = host::prefix
#   RCACHE_PASSWORD = puzzles
#
# rcache variables will default to:
#   RCACHE_MODULE = host::prefix:lightmare
#   RCACHE_USER = lightmare
#   RCACHE_REPO = mapnik
#   RCACHE_TAG = 3
#
# ccache directory will be rsynced with:
#   host::prefix:lightmare/mapnik/experimental/3
#
# If RCACHE_PASSWORD is empty, cache won't be uploaded.
# If RCACHE_REMOTE is empty, RCACHE_MODULE must be supplied.
# RCACHE_TAG may be used to make several jobs share a cache.
#

: ${RCACHE_DIR:=$HOME/.rcache}
: ${RCACHE_TAG:=${TRAVIS_JOB_NUMBER##*.}}
: ${RCACHE_REPO:=${TRAVIS_REPO_SLUG#*/}}
: ${RCACHE_USER:=${TRAVIS_REPO_SLUG%%/*}}
: ${RCACHE_MODULE:=${RCACHE_REMOTE:+$RCACHE_REMOTE:$RCACHE_USER}}

rcache_download () {
    test -n "$RCACHE_MODULE" || return
    export CCACHE_COMPRESS=1
    export CCACHE_DIR="$RCACHE_DIR/$RCACHE_REPO/$TRAVIS_BRANCH/$RCACHE_TAG"
    RCACHE_REMOTE_BRANCH_EXISTS=0

    # try to download cache for the current branch (or merge-base)
    rcache_download_branch "$TRAVIS_BRANCH" &&
        RCACHE_REMOTE_BRANCH_EXISTS=1 &&
        return

    # if that fails, try to download cache for master
    test "$TRAVIS_BRANCH" != "master" &&
        rcache_download_branch "master"
}

rcache_download_branch () {
    local rbranch="$1"
    shift
    mkdir -p "$RCACHE_DIR/$RCACHE_REPO/$TRAVIS_BRANCH"

    # RCACHE_PASSWORD, being an encrypted environment variable, is not
    # available to pull requests from forks. That's why there is rsync
    # user "travis-ci" with un-encrypted password and read-only access.
    RSYNC_PASSWORD="ic+sivart" \
    rsync -auz --relative --no-implied-dirs --human-readable --stats \
        "travis-ci@$RCACHE_MODULE/$RCACHE_REPO/$rbranch/./$RCACHE_TAG/" \
        "$RCACHE_DIR/$RCACHE_REPO/$TRAVIS_BRANCH/"
}

rcache_upload () {
    test -n "$RCACHE_MODULE" || return
    test -n "$RCACHE_PASSWORD" || return

    if test "$TRAVIS_BRANCH" = "master"; then
        RSYNC_PASSWORD="$RCACHE_PASSWORD" \
        rsync -auz --relative --no-implied-dirs --human-readable --stats \
            --del --delete-excluded --exclude="tmp" \
            "$RCACHE_DIR/./$RCACHE_REPO/$TRAVIS_BRANCH/$RCACHE_TAG/" \
            "$RCACHE_USER@$RCACHE_MODULE/"
    else
        # We can only leverage --link-dest if we sync relative to the
        # branch directory, which must exist on the remote side before
        # the transfer.
        if test "$RCACHE_REMOTE_BRANCH_EXISTS" != 1; then
            RSYNC_PASSWORD="$RCACHE_PASSWORD" \
            rsync -du --relative --no-implied-dirs \
                "$RCACHE_DIR/./$RCACHE_REPO/$TRAVIS_BRANCH" \
                "$RCACHE_USER@$RCACHE_MODULE/"
        fi

        RSYNC_PASSWORD="$RCACHE_PASSWORD" \
        rsync -auz --relative --no-implied-dirs --human-readable --stats \
            --del --delete-excluded --exclude="tmp" \
            --link-dest="../master/" \
            "$RCACHE_DIR/$RCACHE_REPO/$TRAVIS_BRANCH/./$RCACHE_TAG/" \
            "$RCACHE_USER@$RCACHE_MODULE/$RCACHE_REPO/$TRAVIS_BRANCH/"
    fi
}
