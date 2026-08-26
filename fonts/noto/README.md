NotoSansSinhala-Subset.ttf is kept in-tree as a deterministic text fallback
shaping fixture. Its embedded family name is `Noto Sans Sinhala Subset`.

It was subset from the upstream source font `NotoSansSinhalaUI-Regular.ttf`
with:

```bash
hb-subset NotoSansSinhalaUI-Regular.ttf \
  --output-file=NotoSansSinhala-Subset.ttf \
  --unicodes=U+0048,U+0065,U+006C,U+006F,U+0020,U+0DC1,U+0DCA,U+200D,U+0DBB,U+0DD3,U+0DBD,U+0D82,U+0D9A,U+0DCF
```

The embedded family name was then rewritten with:

```bash
uv run --with fonttools python scripts/rename_font_family.py \
  --family 'Noto Sans Sinhala Subset' \
  NotoSansSinhala-Subset.ttf
```

The subset keeps the glyphs needed by the focused Sinhala fallback test.
