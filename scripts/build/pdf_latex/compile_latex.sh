#!/bin/bash

# Size images in pdf latex
grep -RiIl '\\includegraphics\[height=.*,keepaspectratio=true\]' | xargs sed -i 's/\\includegraphics\[height=.*,keepaspectratio=true\]/\\includegraphics[width=\\textwidth,keepaspectratio=true]/g'

# Compile pdf and return OK status code even though it encounters errors
make LATEX_CMD="pdflatex -interaction=nonstopmode"|| (exit 0)
echo "Ignoring errors in compiling PDF latex."

# Check compiled pdf exists
test -f "./refman.pdf"

# Change name to target
cp ./refman.pdf ../$1.pdf