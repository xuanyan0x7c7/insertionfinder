dist_algorithms_DATA = \
	2x2CP.txt 2x2EP.txt 3CP-large.txt 3CP-normal.txt \
	3EP.txt C-other.txt CO.txt E-other.txt EO.txt other.txt
algorithms_DATA = \
	2x2CP.algs 2x2EP.algs 3CP-large.algs 3CP-normal.algs \
	3EP.algs C-other.algs CO.algs E-other.algs EO.algs other.algs
algorithmsdir = $(datadir)/$(PACKAGE)/$(VERSION)/algorithms

.txt.algs:
	@echo Generating algorithm file $@
	@../../src/insertionfinder --generate-algfile -f $^ -a $@

clean-local:
	rm -f *.algs
