all: libs
tls: libs
OUTDIR=./sdk
LIBDIR=$(OUTDIR)/lib
INCLUDEDIR=$(OUTDIR)/include
MODULES=$(sort $(patsubst ./%/,%,$(dir $(wildcard ./*/Makefile))))
LIBS=$(addprefix $(LIBDIR)/,$(MODULES))
LIBS:=$(addsuffix .a,$(LIBS))
tls:TLS=tls

clean-%:
	-cd $* && make clean

clean: $(addprefix clean-,$(MODULES))
	-rm -R $(OUTDIR)

$(LIBDIR)/%.a:%
	cd $< && make $(TLS)
	-mv $</$<.a $@
	mkdir -p $(INCLUDEDIR)/$<
	cp $</*.h $(INCLUDEDIR)/$<

outdir:
	mkdir -p $(LIBDIR)
	mkdir -p $(INCLUDEDIR)

libs: outdir $(LIBS)
	echo $(LIBS)

.PHONY: test
test: 
	echo $(LIBS)
