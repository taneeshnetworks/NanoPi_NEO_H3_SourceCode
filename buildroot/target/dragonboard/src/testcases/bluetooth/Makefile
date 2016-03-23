
# define sources root directory before everything
SRC_ROOT := ../..

# include rule.mk
include $(SRC_ROOT)/rule.mk

.PHONY: all
all:
	cp bttester.sh $(BINDIR)/
	cp hcitool $(BINDIR)/
	cp cciconfig $(BINDIR)/
	cp bluetooltester $(BINDIR)/

	cp -f ap6210/*.hcd $(BINDIR)/
	cp -f ap6330/*.hcd $(BINDIR)/
	cp -f ap6335/*.hcd $(BINDIR)/