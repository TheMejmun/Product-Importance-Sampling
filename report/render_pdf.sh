#!/bin/bash
pandoc ./REPORT.md --lua-filter=pandoc_br.lua -o ./REPORT.pdf --pdf-engine=xelatex -V geometry:margin=1in --columns=200