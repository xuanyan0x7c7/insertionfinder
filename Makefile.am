SUBDIRS = src

install-data-hook:
	cd data/algorithms && exec_prefix=$(prefix) bash generate.sh
	mkdir -p $(prefix)/share/$(PACKAGE)/$(VERSION)/algorithms
	cp data/algorithms/*.algs $(prefix)/share/$(PACKAGE)/$(VERSION)/algorithms
