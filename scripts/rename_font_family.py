#!/usr/bin/env python3

from __future__ import annotations

import argparse

from fontTools.ttLib import TTFont


NAME_IDS = (1, 2, 4, 6, 16, 17)
NAME_RECORDS = ((3, 1, 0x409), (1, 0, 0))


def rename_font(path: str, family: str, style: str) -> None:
    full = f"{family} {style}"
    postscript = family.replace(" ", "") + "-" + style
    tt = TTFont(path)
    name_table = tt["name"]
    # Rewrite the common family/style name fields for both Windows and Mac records.
    for name_id, value in (
        (1, family),
        (2, style),
        (4, full),
        (6, postscript),
        (16, family),
        (17, style),
    ):
        for platform_id, encoding_id, language_id in NAME_RECORDS:
            name_table.setName(value, name_id, platform_id, encoding_id, language_id)
    tt.save(path)


def main() -> None:
    parser = argparse.ArgumentParser(description="Rewrite the family name inside a TrueType/OpenType font.")
    parser.add_argument("path", help="Font file to rewrite in place")
    parser.add_argument("--family", required=True, help="Family name to write into the font")
    parser.add_argument("--style", default="Regular", help="Style name to write into the font")
    args = parser.parse_args()
    rename_font(args.path, args.family, args.style)


if __name__ == "__main__":
    main()
