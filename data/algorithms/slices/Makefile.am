dist_algorithms_slices_DATA = basic.txt
algorithms_slices_DATA = basic.algs
algorithms_slicesdir = $(datadir)/${PACKAGE}/${VERSION}/algorithms/slices

.txt.algs:
	@echo Generating algorithm file slices/$@
	@../../../src/insertionfinder${EXEEXT} --generate --symmetrics-only -f $^ -a $@

clean-local:
	rm -f *.algs
