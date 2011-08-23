#!/bin/bash

# TODO
# - find safer way to do this: http://trac.mapnik.org/ticket/776
# - check for ~/.profile so that adding a new bash_profile does not override

# backup bash_profile
cp ~/.bash_profile ~/.bash_profile.mapnik.backup

# add installer Programs to PATH
# by appending new entries to bash_profile
echo '' >> ~/.bash_profile
echo '# Settings for Mapnik.framework Installer to enable Mapnik programs and python bindings' >> ~/.bash_profile
echo 'export PATH=/Library/Frameworks/Mapnik.framework/Programs:$PATH' >> ~/.bash_profile
echo 'export PYTHONPATH=/Library/Frameworks/Mapnik.framework/Python:$PYTHONPATH' >> ~/.bash_profile