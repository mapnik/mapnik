BenestroffSansTest-Regular.ttf is a renamed subset font derived from
NotoSans-Regular.ttf for the "Bénestroff − Centre" fallback regression.
Noto Sans is licensed under the SIL Open Font License, version 1.1:
https://scripts.sil.org/OFL

BenestroffSymbolsTest-symbols.ttf is a renamed subset font derived from
NotoSansSymbols-Regular.ttf for the "Bénestroff − Centre" fallback regression.
Its embedded family name is `Benestroff Symbols Test Symbols`.

BenestroffSymbolsTest-minus.ttf is a renamed subset font derived from
NotoSansBengaliUI-Regular.ttf for the same fallback regression.
Its embedded family name is `Benestroff Symbols Test Minus`.

Subset recreation commands
==========================

The HarfBuzz CLI commands used for the committed Benestroff fixtures are
recorded below.

BenestroffSansTest-Regular.ttf
------------------------------

```bash
hb-subset NotoSans-Regular.ttf \
  --output-file=BenestroffSansTest-Regular.ttf \
  --text='Bénestroff Centre'
```

BenestroffSymbolsTest-symbols.ttf
---------------------------------

```bash
hb-subset NotoSansSymbols-Regular.ttf \
  --output-file=BenestroffSymbolsTest-symbols.ttf \
  --gids=534
```

The subset is then renamed to use the embedded family name
`Benestroff Symbols Test Symbols`.

```bash
uv run --with fonttools python scripts/rename_font_family.py \
  --family='Benestroff Symbols Test Symbols' \
  BenestroffSymbolsTest-symbols.ttf
```

BenestroffSymbolsTest-minus.ttf
-------------------------------

```bash
hb-subset NotoSansBengaliUI-Regular.ttf \
  --output-file=BenestroffSymbolsTest-minus.ttf \
  --unicodes=U+2212
```

The subset is then renamed to use the embedded family name
`Benestroff Symbols Test Minus`.

```bash
uv run --with fonttools python scripts/rename_font_family.py \
  --family='Benestroff Symbols Test Minus' \
  BenestroffSymbolsTest-minus.ttf
```

Those two outputs are used directly by the fallback test, so no merge step is
needed.

Khmer and Myanmar fixtures
==========================

KhmerFallbackProbe.ttf and KhmerFallbackFull.ttf are renamed subset fonts
derived from KhmerOS.ttf for deterministic text fallback shaping tests.

MyanmarFallbackProbe.ttf and MyanmarFallbackFull.ttf are renamed subset fonts
derived from NotoSansMyanmarUI-Regular.ttf for deterministic text fallback
shaping tests.

The HarfBuzz CLI commands used for the committed Khmer and Myanmar fixtures are
recorded below.

KhmerFallbackProbe.ttf
----------------------

```bash
hb-subset test/data/fonts/KhmerOS/KhmerOS.ttf \
  --output-file=KhmerFallbackProbe.ttf \
  --unicodes=U+1782,U+1794,U+1797,U+1799
```

The subset is then renamed to use the embedded family name `Khmer Fallback
Probe`.

```bash
uv run --with fonttools python scripts/rename_font_family.py \
  --family='Khmer Fallback Probe' \
  KhmerFallbackProbe.ttf
```

KhmerFallbackFull.ttf
---------------------

```bash
hb-subset test/data/fonts/KhmerOS/KhmerOS.ttf \
  --output-file=KhmerFallbackFull.ttf \
  --unicodes=U+1782,U+1794,U+1797,U+1799,U+17B6,U+17B9,U+17CA,U+17D2
```

The subset is then renamed to use the embedded family name `Khmer Fallback
Full`.

```bash
uv run --with fonttools python scripts/rename_font_family.py \
  --family='Khmer Fallback Full' \
  KhmerFallbackFull.ttf
```

MyanmarFallbackProbe.ttf
------------------------

```bash
hb-subset /Users/ram/Code/mapnik/wasm/demo/osm-tiles/data/openstreetmap-carto/fonts/NotoSansMyanmarUI-Regular.ttf \
  --output-file=MyanmarFallbackProbe.ttf \
  --unicodes=U+1001,U+1004
```

The subset is then renamed to use the embedded family name `Myanmar Fallback
Probe`.

```bash
uv run --with fonttools python scripts/rename_font_family.py \
  --family='Myanmar Fallback Probe' \
  MyanmarFallbackProbe.ttf
```

MyanmarFallbackFull.ttf
-----------------------

```bash
hb-subset /Users/ram/Code/mapnik/wasm/demo/osm-tiles/data/openstreetmap-carto/fonts/NotoSansMyanmarUI-Regular.ttf \
  --output-file=MyanmarFallbackFull.ttf \
  --unicodes=U+1001,U+1004,U+1039,U+103A
```

The subset is then renamed to use the embedded family name `Myanmar Fallback
Full`.

```bash
uv run --with fonttools python scripts/rename_font_family.py \
  --family='Myanmar Fallback Full' \
  MyanmarFallbackFull.ttf
```

Those outputs are used directly by the fallback tests, so no merge step is
needed.
