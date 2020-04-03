SUBDIRS = extras slices
dist_algorithms_DATA = \
	2x2CP.txt 2x2EP.txt 3CP.txt 3CP-pure.txt \
	3EP.txt C-other.txt CO.txt E-other.txt EO.txt \
	no-parity-other.txt parity.txt center.txt
algorithms_DATA = \
	2x2CP.algs 2x2EP.algs 3CP.algs 3CP-pure.algs \
	3EP.algs C-other.algs CO.algs E-other.algs EO.algs \
	no-parity-other.algs parity.algs center.algs
algorithmsdir = $(datadir)/$(PACKAGE)/$(VERSION)/algorithms

.txt.algs:
	@echo Generating algorithm file $@
	@../../src/insertionfinder${EXEEXT} --generate -f $^ -a $@

clean-local:
	rm -f *.algs
