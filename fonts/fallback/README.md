KhmerFallbackProbe.ttf and KhmerFallbackFull.ttf are renamed subset fonts
derived from KhmerOS.ttf for deterministic text fallback shaping tests.

MyanmarFallbackProbe.ttf and MyanmarFallbackFull.ttf are renamed subset fonts
derived from NotoSansMyanmar-Regular.ttf for deterministic text fallback
shaping tests.

BenestroffSansTest-Regular.ttf is a renamed subset font derived from
NotoSans-Regular.ttf for the "Bénestroff − Centre" fallback regression.

BenestroffSymbolsTest-Regular.ttf is a renamed merged subset font combining the
relevant glyph from NotoSansSymbols-Regular.ttf with the minus sign from
NotoSansBengaliUI-Regular.ttf so the same regression can be exercised with a
tiny two-font fixture.

FallbackRegularTest-Regular.ttf is a renamed subset font derived from
NotoSans-Regular.ttf for deterministic fallback shaping regressions that need
U+200C, U+202F, and U+0020 from the regular face.

BengaliJoinerTest-Regular.ttf is a renamed subset font derived from
NotoSansBengaliUI-Regular.ttf for the Bengali joiner-substitution regression
based on the label "দুশান্‌বে".

MongolianProblemTest-Regular.ttf is a renamed subset font derived from
NotoSansMongolian-Regular.ttf for the Mongolian shaping regression based on the
label "᠑᠐ ᠳᠦᠭᠡᠷ ᠪᠠᠭ".
