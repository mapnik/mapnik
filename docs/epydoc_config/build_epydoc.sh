epydoc --no-private --no-frames --no-sourcecode --name mapnik --url http://mapnik.org --css mapnik_epydoc.css mapnik -o ../api_docs/python
svn add ../api_docs/python/*
svn propset svn:mime-type text/html ../api_docs/python/*.html
svn propset svn:mime-type text/css ../api_docs/python/*.css
svn propset svn:mime-type image/png ../api_docs/python/*.png