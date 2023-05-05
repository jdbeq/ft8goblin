
obj/%.o: src/%.c $(wildcard *.h)
	@echo "[CC] $< -> $@"
	@${CC} ${CFLAGS} -o $@ -c $<
