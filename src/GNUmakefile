all clean distclean help world install install-deps install-deps-sudo:
	@${MAKE} --no-print-directory -C .. $@

../bin/ft8goblin:
	${MAKE) -C .. all

run: ../bin/ft8goblin
	@echo "Attempting to start local build without installing..."
	cd ../; ./startup
