.EXPORT_ALL_VARIABLES:

include ./Rules.make

SUBS = base \
	control \
	main

all: outdir
	@(for c in $(SUBS); do \
		echo ""; \
		echo "********************* $$c *********************"; \
		$(MAKE) -C $$c || exit 1; \
	done)
	@echo ""
	@echo "Build complete: $(BIN_DIR)/stream_daemon  [ARCH=$(ARCH)]"

outdir:
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(BIN_DIR)

clean:
	@(for c in $(SUBS); do \
		echo ""; \
		echo "********************* $$c *********************"; \
		$(MAKE) -C $$c clean; \
	done)

dep:
	@(for c in $(SUBS); do \
		echo ""; \
		echo "********************* $$c *********************"; \
		$(MAKE) -C $$c dep; \
	done)
