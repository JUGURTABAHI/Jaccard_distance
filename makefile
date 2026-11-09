.PHONY: clean dist

dist: clean
	tar -hzcf "$(CURDIR).tar.gz" hashtable/* holdall/* jdis_test/* opt/* Rapport.pdf makefile

clean:
	$(MAKE) -C jdis_test clean
