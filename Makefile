all: libs
tls: libs
OUTDIR=./sdk
LIBDIR=$(OUTDIR)/lib
INCLUDEDIR=$(OUTDIR)/include
EXCEPTIONS=./tests/
MODULES=$(sort $(patsubst ./%/,%,$(filter-out $(EXCEPTIONS),$(dir $(wildcard ./*/Makefile)))))
LIBS=$(addprefix $(LIBDIR)/,$(MODULES))
LIBS:=$(addsuffix .a,$(LIBS))
tls:TLS=tls

clean-%:
	-cd $* && make clean
	-cd tests && make clean
	-rm -rf obj

clean: $(addprefix clean-,$(MODULES))
	-rm -R $(OUTDIR)

$(LIBDIR)/%.a:%
	cd $< && make $(TLS)
	-mkdir -p obj
	-cp $</*.o obj/
	mkdir -p $(INCLUDEDIR)/$<
	cp $</*.h $(INCLUDEDIR)/$<

outdir:
	mkdir -p $(LIBDIR)
	mkdir -p $(INCLUDEDIR)

libs: outdir $(LIBS)
	$(AR) rcs $(LIBDIR)/dumaislib.a obj/*.o
	echo $(LIBS)

.PHONY: test
test: libs 
	cd tests && make $(TLS)
