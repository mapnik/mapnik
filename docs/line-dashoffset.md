# Offsetting line dash patterns

`LineSymbolizer` supports `stroke-dashoffset` in the AGG and Cairo renderers.
The offset is a distance into the repeating `stroke-dasharray` pattern, in
pixels, and defaults to zero. It scales with the renderer's scale factor,
just like the dash and gap lengths.

For example, `stroke-dasharray="8,8" stroke-dashoffset="8"` starts with an
8-pixel gap, followed by an 8-pixel dash. This also works with round line caps.
Negative offsets move in the opposite direction and wrap around the pattern.
Offsets can be expressions such as `stroke-dashoffset="[phase]"`.

To draw alternating red and blue dashes, use two symbolizers on the same line:

```xml
<Rule>
  <LineSymbolizer stroke="red" stroke-width="3" stroke-dasharray="8,8"/>
  <LineSymbolizer stroke="blue" stroke-width="3" stroke-dasharray="8,8"
                  stroke-dashoffset="8"/>
</Rule>
```

The dash pattern restarts for each subpath. An offset has no effect on a solid
line. Use the default `rasterizer="full"`; the AGG `fast` line rasterizer does
not support dash patterns.

In CartoCSS, the corresponding property is `line-dash-offset` (it requires a
Carto reference version that exposes the property).
