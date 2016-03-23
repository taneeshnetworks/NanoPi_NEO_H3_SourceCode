
# define sources root directory before everything
SRC_ROOT = .

# change compiler and linker option before you include rule.mk
#
# link to libscript.a when you need to fetch configuration
# from test_script
#
#CFLAGS := $(CFLAGS) -g
LDFLAGS := -lscript -ldirectfb -lpthread -ldisp

# include rule.mk
include $(SRC_ROOT)/rule.mk

CFLAGS := $(CFLAGS) -I$(SYSROOT)/usr/include/directfb
# define objects collection
libscript_objs = core/script.o
libdisp_objs = view/${CPU}/display.o

core_objs = core/core.o core/script_parser.o \
			view/view.o view/df_view.o


launcher_objs = core/launcher.o

# add targets to all
.PHONY: all
all: libscript.a libdisp.a core launcher
	make -C testcases
	make -C lib/tinyalsa-master

libscript.a: $(libscript_objs)
	@echo "  AR\t$^ -> $@"
	$(AR) $(ARFLAGS) $(LIBDIR)/$@ $^

$(libscript_objs): %.o: %.c
	$(COMPILE_MSG)
	$(COMPILEX)

libdisp.a: ${libdisp_objs}
	@echo "  AR\t$^ -> $@"
	$(AR) $(ARFLAGS) $(LIBDIR)/$@ $^

$(libdisp_objs): %.o: %.c
	$(COMPILE_MSG)
	$(COMPILEX)

core: $(core_objs)
	$(LINK_MSG)
	$(LINKX)

$(core_objs): %.o: %.c
	$(COMPILE_MSG)
	$(COMPILEX)

launcher: $(launcher_objs)
	$(LINK_MSG)
	$(LINKX)

$(launcher_objs): %.o: %.c
	$(COMPILE_MSG)
	$(COMPILEX)

.PHONY: clean
clean:
	@make -C lib/tinyalsa-master clean
	@rm -f $(BINDIR)/*
	@rm -rf ./include/asm/arch
	@echo "  CLEAN *.o"
	@find . -name *.o -exec rm -f {} \;
	@echo "  CLEAN *.d"
	@find . -name *.d -exec rm -f {} \;
	@echo "  CLEAN *.d.*"
	@find . -name *.d.* -exec rm -f {} \;


# generate dependence
SRCDIRS = core/ view/ view/${CPU}/ $(shell find testcases/ -type d)
SOURCES = $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

-include $(SOURCES:.c=.d)

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
