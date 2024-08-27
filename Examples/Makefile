all:
# if any Makefile failed, stop building all...
	@for m in `find . -mindepth 2 -name Makefile`; do\
		$(MAKE) -C `dirname $$m`; \
		if [ $$? -ne 0 ]; then exit 1; fi; \
	done

clean:
	-@for m in `find . -mindepth 2 -name Makefile`; do\
		$(MAKE) -C `dirname $$m` clean; \
	done

pristine:
	-@for m in `find . -mindepth 2 -name Makefile`; do\
		$(MAKE) -C `dirname $$m` pristine; \
	done
