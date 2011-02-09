#!/bin/sh

echo '' >> ~/.bash_profile
echo '# Settings for Mapnik.framework Installer to enable Mapnik programs and python bindings' >> ~/.bash_profile
echo 'export PATH=/Library/Frameworks/Mapnik.framework/Programs:$PATH' >> ~/.bash_profile
echo 'export PYTHONPATH=/Library/Frameworks/Mapnik.framework/Python:$PYTHONPATH' >> ~/.bash_profile