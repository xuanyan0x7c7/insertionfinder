dist_algorithms_extras_DATA = no-parity-other.txt parity.txt
algorithms_extras_DATA = no-parity-other.algs parity.algs
algorithms_extrasdir = $(datadir)/${PACKAGE}/algorithms/extras

.txt.algs:
	@echo Generating algorithm file extras/$@
	@../../../src/insertionfinder${EXEEXT} --generate -f $^ -a $@

clean-local:
	rm -f *.algs
