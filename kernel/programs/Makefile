PROGLIB_DIR=__libprog
PROGLIB=$(PROGLIB_DIR)/libprog.a

all: $(PROGLIB)
	for dir in [^_]*; do [ -d "$$dir" ] && make -C "$$dir" ; done

install: $(PROGLIB)
	for dir in [^_]*; do [ -d "$$dir" ] && \
		make -C "$$dir" clean && make -C "$$dir" install ; done

.PHONY: $(PROGLIB)
$(PROGLIB):
	make -C $(PROGLIB_DIR)
