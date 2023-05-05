help:
	@echo "MAKE targets:"
	@echo ""
	@echo "all | world\t\t\tBuild everything (try -j$NUMCPU!)"
	@echo "clean\t\t\t\tClean up the tree before rebuilding"
	@echo "distclean\t\t\tClean up the tree before releasing/uploading"
	@echo "install-deps\t\t\tInstall needed libraries (ft8_lib and termbox2)"
	@echo "install-deps-sudo\t\tInstall needed libraries, using sudo"
