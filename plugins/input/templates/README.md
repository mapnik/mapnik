## template plugins

Directory to hold sample plugin templates.

These are NOT intended to be used except for testing by developers.

Build these plugins with the Mapnik build system:

    ./configure SAMPLE_INPUT_PLUGINS=True
    make install

Or develop them locally using the `Makefile` provided.

Only an ultra-simple hello world is available currently,
but planned are example plugins templates for file-based
and sql-based datasources.
