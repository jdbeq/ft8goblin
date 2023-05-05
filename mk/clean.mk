
distclean: clean

clean:
	@echo "Cleaning..."
## Try to enforce cleaning before other rules
ifneq ($(filter clean,$(MAKECMDGOALS)),)
	${RM} -f logs/*.log logs/*.debug run/*.pid run/*.pipe
	${RM} -f ${extra_clean}
	${MAKE} ${extra_clean_targets}
endif
