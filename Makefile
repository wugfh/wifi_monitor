# this Makefile is to make bpf bytecode.
# bpf programe shoule be edit in one file, use clang/llvm to compile the c file
# into bpf bytecode
# now the Makefile compile reflector.c to bpf progream
XDP_TARGETS := reflector
USER_TARGETS :=

LLC ?= llc
CLANG ?= clang
CC ?= gcc

XDP_C = ${XDP_TARGETS:=.c}
XDP_OBJ = ${XDP_C:.c=.o}
USER_C := ${USER_TARGETS:=.c}
USER_OBJ := ${USER_C:.c=.o}

LIBBPF_DIR ?= ./libbpf/src/

OBJECT_LIBBPF = $(LIBBPF_DIR)/libbpf.a

EXTRA_DEPS +=

CFLAGS ?= -I$(LIBBPF_DIR)/build/usr/include/ -g
LDFLAGS ?= -L$(LIBBPF_DIR)

BPF_CFLAGS ?= -I$(LIBBPF_DIR)/build/usr/include/

LIBS = -l:libbpf.a -lelf $(USER_LIBS)

all: llvm-check $(USER_TARGETS) $(XDP_OBJ)

.PHONY: clean $(CLANG) $(LLC)

clean:
	rm -rf $(LIBBPF_DIR)/build
	$(MAKE) -C $(LIBBPF_DIR) clean
	rm -f $(USER_TARGETS) $(XDP_OBJ) $(USER_OBJ)
	rm -f *.ll
	rm -f *~

llvm-check: $(CLANG) $(LLC)
	@for TOOL in $^ ; do \
		if [ ! $$(command -v $${TOOL} 2>/dev/null) ]; then \
			echo "*** ERROR: Cannot find tool $${TOOL}" ;\
			exit 1; \
		else true; fi; \
	done

$(OBJECT_LIBBPF):
	@if [ ! -d $(LIBBPF_DIR) ]; then \
		echo "Error: Need libbpf submodule"; \
		echo "May need to run git submodule update --init"; \
		exit 1; \
	else \
		cd $(LIBBPF_DIR) && $(MAKE) all OBJDIR=.; \
		mkdir -p build; $(MAKE) install_headers DESTDIR=build OBJDIR=.; \
	fi

$(USER_TARGETS): %: %.c  $(OBJECT_LIBBPF) Makefile
	$(CC) -Wall $(CFLAGS) $(LDFLAGS) -o $@ \
	 $< $(LIBS)

$(XDP_OBJ): %.o: %.c  Makefile $(OBJECT_LIBBPF)
	$(CLANG) -S \
	    -target bpf \
	    -D __BPF_TRACING__ \
	    $(BPF_CFLAGS) \
	    -Wall \
	    -Wno-unused-value \
	    -Wno-pointer-sign \
	    -Wno-compare-distinct-pointer-types \
	    -Werror \
	    -O2 -emit-llvm -c -g -o ${@:.o=.ll} $<
	$(LLC) -march=bpf -filetype=obj -o $@ ${@:.o=.ll}
