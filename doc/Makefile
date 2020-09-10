O := build

all: html singlehtml

html singlehtml linkcheck::
	sphinx-build $(SPHINXFLAGS) -d $(O)/.doctrees -b $@ ./ $(O)/$@

html singlehtml::
	@echo HTML builds are available in $(O)/$@/index.html

check: linkcheck

clean:
	rm -rf $(O)

.PHONY: all check clean html linkcheck singlehtml
