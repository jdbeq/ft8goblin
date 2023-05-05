
install:
	@for i in ${real_bins}; do \
		install -m 0755 $$i ${bin_install_path}/$$i; \
	done
	# XXX: Test for libraries
	# XXX: Install missing libraries
	# XXX: Install headers for installed libraries
