
all:
	if [ ! -f src/Makefile ]; then \
	    (RUNME;) \
	fi
	(cd src; make $@)

clean:
	(cd src; make $@)

clobber:
	rm src/Makefile
	(cd src; make $@)
# DO NOT DELETE
