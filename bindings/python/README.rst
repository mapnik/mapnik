======================
Mapnik Python Bindings
======================

Official `Mapnik`_ python bindings.

.. image:: https://secure.travis-ci.org/mapnik/mapnik.png
   :target: http://travis-ci.org/mapnik/mapnik

.. contents::


Installation
============

Prerequisites
-------------

Before installing, you need:

* the ``mapnik-config`` utility in your ``$PATH``
* ``Boost.Python`` linked to your python interpreter (see `Boost notes`_)
* ``cairo`` / ``cairomm`` (optionnal but enabled if you compiled mapnik with cairo support)
* mapnik
* The current python interpreter


Version notes
-------------

The python bindings are tied to the mapnik library version.

========================  ====================
 Mapnik library version     Python package
========================  ====================
2.0.x                     mapnik2 (==2.0.1.3)
2.1.0                     mapnik (==2.1.0)
========================  ====================

You can find your installed version with:

.. code-block:: bash

    $ mapnik-config -v


Boost notes
-----------

To specify which ``boost_python`` lib to link against, you can use:

.. code-block:: bash

    $ export MAPNIK2_BOOST_PYTHON="libboost_python.so.1"

Where you have on your filesystem

.. code-block:: bash

    /usr/lib/libboost_python.so.1

Don't forget that you can play with ``LDFLAGS``/``CFLAGS``/``LD_LIBRARY_PATH`` to indicate non standart locations for the requirements if it applies.


Standard installation
---------------------
You can use either ``easy_install`` or ``pip``.
Choose the one you prefer into:

.. code-block:: bash

    $ easy_install mapnik         # to use the last known version
    $ easy_install mapnik==2.1.0  # to pin the version
    $ pip install mapnik          # to use the last known version
    $ pip install mapnik==2.1.0   # to pin the version

Standard installation works with virtualenv too.

If an error occurs, please read carefully `Prerequisites`_, `Version notes`_ and `Boost notes`_ sections before submitting an issue to the `Mapnik tracker`_.


Buildout
--------

Some developers use buildout_ to ease deployments.

* Add ``mapnik`` to the list of eggs to install, e.g.

.. code-block:: ini

    [buildout]
    parts = somepart

    [somepart]
    recipe = minitage.recipe.scripts
    ...
    # (options like include dirs)
    ...
    eggs =
        ...
        mapnik

* Re-run buildout, e.g. with:

.. code-block:: bash

    $ ./bin/buildout


Credits
=======

Companies
---------

|makinacom|_

* `Planet Makina Corpus <http://www.makina-corpus.org>`_
* `Contact us <mailto:python@makina-corpus.org>`_

.. |makinacom| image:: http://depot.makina-corpus.org/public/logo.gif
.. _makinacom:  http://www.makina-corpus.com


Authors
-------

Contributors
------------

* kiorky <kiorky@cryptelium.net>
* noirbizarre <noirbizarre+mapnik@gmail.com>


.. _buildout: http://buildout.org
.. _Mapnik: http://www.mapnik.org
.. _Mapnik tracker: https://github.com/mapnik/mapnik/issues
