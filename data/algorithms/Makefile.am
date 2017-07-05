dist_algorithms_DATA = 2x2CP.algs 2x2EP.algs 3CP-large.algs 3CP-normal.algs \
	3EP.algs C-other.algs CO.algs E-other.algs EO.algs other.algs
algorithmsdir = $(prefix)/share/$(PACKAGE)/$(VERSION)/algorithms

2x2CP.algs: 2x2CP.txt
	../../src/insertionfinder --generate-algfile -f 2x2CP.txt -a 2x2CP.algs
2x2EP.algs: 2x2EP.txt
	../../src/insertionfinder --generate-algfile -f 2x2EP.txt -a 2x2EP.algs
3CP-large.algs: 3CP-large.txt
	../../src/insertionfinder --generate-algfile -f 3CP-large.txt -a 3CP-large.algs
3CP-normal.algs: 3CP-normal.txt
	../../src/insertionfinder --generate-algfile -f 3CP-normal.txt -a 3CP-normal.algs
3EP.algs: 3EP.txt
	../../src/insertionfinder --generate-algfile -f 3EP.txt -a 3EP.algs
C-other.algs: C-other.txt
	../../src/insertionfinder --generate-algfile -f C-other.txt -a C-other.algs
CO.algs: CO.txt
	../../src/insertionfinder --generate-algfile -f CO.txt -a CO.algs
E-other.algs: E-other.txt
	../../src/insertionfinder --generate-algfile -f E-other.txt -a E-other.algs
EO.algs: EO.txt
	../../src/insertionfinder --generate-algfile -f EO.txt -a EO.algs
other.algs: other.txt
	../../src/insertionfinder --generate-algfile -f other.txt -a other.algs

clean-local:
	rm *.algs
