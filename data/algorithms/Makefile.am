dist_algorithms_DATA = \
	2x2CP.txt 2x2EP.txt 3CP-large.txt 3CP-normal.txt \
	3EP.txt C-other.txt CO.txt E-other.txt EO.txt other.txt
algorithmsdir = $(prefix)/share/$(PACKAGE)/$(VERSION)/algorithms

.txt.algs:
	@echo Generating algorithm file $@
	@../../src/insertionfinder --generate-algfile -f $^ -a $@

clean-local:
	rm -f *.algs
