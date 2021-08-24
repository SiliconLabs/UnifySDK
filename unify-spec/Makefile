all : html-spec pdf-spec

help :
	@echo "make pdf-spec     : build pdf version"
	@echo "make html-spec    : build html version"
	@echo -n
	@echo "make release      : display release instructions"

release :
	@echo To release, git tag -a "verA_BB-YYYYMMDD" and git push --tags.
	@echo This will trigger a Jenkins build of pdf and html.

Doxyfile-pdf : Doxyfile.in
	-mkdir -p build/pdf
	sed -e 's/__INPUT_PLACEHOLDER__/doc\//g' \
	    -e 's/__OUTPUT_DIRECTORY_PLACEHOLDER__/build\/pdf/g' \
		-e 's/__GENERATE_LATEX_PLACEHOLDER__/YES/g' \
		-e 's/__GENERATE_HTML_PLACEHOLDER__/NO/g' \
		-e 's/__PROJECT_NUMBER_PLACEHOLDER__/\"$(build_version)\"/g' \
		Doxyfile.in >build/pdf/Doxyfile-pdf

pdf-spec : Doxyfile-pdf $(wildcard doc/*.md doc/pdf_only/*.md)
	doxygen build/pdf/Doxyfile-pdf
	cd build/pdf/latex && make
	cp build/pdf/latex/refman.pdf ./unify-spec.pdf

html-spec: Doxyfile-html $(wildcard doc/*.md doc/html_only/*.md)
	doxygen build/html/Doxyfile-html

Doxyfile-html:
	-mkdir -p build/html
	sed -e 's/__INPUT_PLACEHOLDER__/doc\/ doc\/html_only\//g' \
	    -e 's/__OUTPUT_DIRECTORY_PLACEHOLDER__/build\//g' \
		-e 's/__GENERATE_LATEX_PLACEHOLDER__/NO/g' \
		-e 's/__GENERATE_HTML_PLACEHOLDER__/YES/g' \
		-e 's/__PROJECT_NUMBER_PLACEHOLDER__/$(build_version)/g' \
		Doxyfile.in >build/html/Doxyfile-html

clean :
	@rm -r build/ ./unify-spec.pdf || true

build_version = $(shell git describe --tags --match "ver*")

.PHONY : help clean release

# Fail the build if doxygen exits with an error
.SHELLFLAGS = -ec
