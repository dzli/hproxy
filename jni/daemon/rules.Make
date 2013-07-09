#
# defaults for compilers and tools
#
TOOLSDIR=
CC=$(TOOLSDIR)gcc
CXX=$(TOOLSDIR)g++
RANLIB=$(TOOLSDIR)ranlib
AR=$(TOOLSDIR)ar
LD=$(TOOLSDIR)ld

#
# Added soname into shared libraries
#
LDSONAME=-soname $@

#
# defaults to using g++
#
ifndef USEGCC
CC=$(TOOLSDIR)gcc
LDGPLUSPLUS=g++
endif

#
# use g++ to link libraries
#
ifdef LDGPLUSPLUS
LD=$(TOOLSDIR)g++
LDSONAME=-Wl,-soname -Wl,$@
endif

#
# always use ld to like *.o files
#
LDO = $(TOOLSDIR)ld


INCS= -I $(TOPDIR)/src/include

# dependency directory
DEPDIR=.deps


# disabled optimziation if debugging 
ifdef DEBUG 
NOOPTIMIZE=1 
endif 

#ifeq ($(NOOPTIMIZE),1) 
ifndef NOOPTIMIZE
CFLAGS += -O2 
endif 

# add debug flags 
ifdef DEBUG 
CFLAGS += -g 
endif 


# disable assert statements
ifndef DEBUG
CFLAGS += -DNDEBUG
endif

ifndef NOREENTRANT
CFLAGS += -D_REENTRANT
endif

#CFLAGS += -D_GNU_SOURCE
CFLAGS += -Wall
#CFLAGS += -Werror

CFLAGS += $(INCS) $(EXTRA_INCS) $(CINCS) $(EXTRA_CFLAGS)

ifndef DBINDIR
DBINDIR=$(TOPDIR)/build/bin
endif

ifndef SOLIBDIR
SOLIBDIR=$(TOPDIR)/build/lib
endif

ifndef ALIBDIR
ALIBDIR=$(TOPDIR)/build/lib
endif

.SUFFIXES:

#
# Added subdirs target
#
ifdef SUBDIRS
SUBDIRSTARGET=subdirs
endif

# All target
.PHONY: all


all:: \
        $(EXTRA_PREBUILD) \
        $(SUBDIRSTARGET) \
        $(HOST_PROG) \
        $(HOST_PROG1) \
        $(HOST_PROG2) \
        $(TARGET_O) \
        $(TARGET_O1) \
        $(TARGET_SO) \
        $(TARGET_SO1) \
        $(APACHE_SO) \
        $(TARGET_A) \
        $(TARGET_A1) \
        $(TARGET_A2) \
        $(TARGET_PROG) \
        $(TARGET_PROG1) \
        $(TARGET_PROG2) \
        $(TARGET_PROG3) \
        $(TARGET_PROG4) \
        $(TARGET_PROG5) \
        $(TARGET_PROG6) \
        $(TARGET_PROG7) \
        $(TARGET_PROG8) \
        $(TARGET_PROG9) \
        $(TARGET_PROG10) \
        $(TARGET_PROG11) \
        $(TARGET_PROG12) \
        $(EXTRA_POSTBUILD)

#
# enter each subdirs and make
#
ifdef SUBDIRS
.PHONY: subdirs

GOALSTR=
ifneq ("$(MAKECMDGOALS)","")
GOALSTR=($(MAKECMDGOALS))
endif

#PRINTDIR=--no-print-directory
subdirs: $(SUBDIRS)

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	@if [ -f "$@/Makefile" ]; then \
		echo "===> $@ $(GOALSTR)"; \
	$(MAKE) -C $@ $(MAKECMDGOALS) || exit 1; \
	echo "<=== $@ $(GOALSTR)"; \
	fi;
endif

#
# Add silent variables to suppress verbose output
#
ifdef SILENT
Q = @
define silentc
	@echo "compiling $@"
endef
define silentl
	@echo "linking $@"
endef
else
Q =
silentc =
silentl =
endif

libdirs=\

#
# ld flags used for linking .so files
#
LDDFLAGS = \
    -shared \
    $(libdirs) \
    $(EXTRA_LDFLAGS)
    
#
# LD flags for linking executables
#
LDFLAGS = \
    $(libdirs) \
    $(EXTRA_LDFLAGS) \

#
# Add system libraries 
#
SYSLIBS= \
    $(EXTRA_SYSLIBS)

#
# TARGET_O: TARGET_O_OBJECTS for compiling .o files into one single .o file
#
ifdef TARGET_O

$(TARGET_O): $(TARGET_O_OBJECTS) $(TARGET_O_EXTRAS)
	$(silentl)
	$(Q)$(LDO) -r -o $@ $(sort $(TARGET_O_OBJECTS)) $(TARGET_O_EXTRAS)
endif

# 
# TARGET_O1: TARGET_O1_OBJECTS for compiling .o files into one single .o file
#
ifdef TARGET_O1

$(TARGET_O1): $(TARGET_O1_OBJECTS) $(TARGET_O1_EXTRAS)
	$(silentl)
	$(Q)$(LDO) -r -o $@ $(sort $(TARGET_O1_OBJECTS)) $(TARGET_O1_EXTRAS)
endif

# 
# TARGET_PROG: TARGET_PROG_OBJECTS for compiling .o files into an executable
#
ifdef TARGET_PROG

$(TARGET_PROG): $(TARGET_PROG_OBJECTS) $(TARGET_PROG_EXTRAS)
	$(silentc)
	$(Q)$(CC) -o $@ $(LDFLAGS) $(TARGET_PROG_EXTRA_LDFLAGS) $(sort $(TARGET_PROG_OBJECTS)) $(TARGET_PROG_EXTRAS) $(SYSLIBS)
ifndef NOPROGCOPY
	$(Q)cp -f $@ $(DBINDIR)
endif
endif

#
# TARGET_PROG1: TARGET_PROG1_OBJECTS for compiling .o files into an executable
#
ifdef TARGET_PROG1

$(TARGET_PROG1): $(TARGET_PROG1_OBJECTS) $(TARGET_PROG1_EXTRAS)
	$(silentc)
	$(Q)$(CC) -o $@ $(LDFLAGS) $(TARGET_PROG1_EXTRA_LDFLAGS) $(sort $(TARGET_PROG1_OBJECTS)) $(TARGET_PROG1_EXTRAS) $(SYSLIBS)
ifndef NOPROGCOPY
	$(Q)cp -f $@ $(DBINDIR)
endif
endif

#
# TARGET_PROG2: TARGET_PROG2_OBJECTS for compiling .o files into an executable
#
ifdef TARGET_PROG2

$(TARGET_PROG2): $(TARGET_PROG2_OBJECTS) $(TARGET_PROG2_EXTRAS)
	$(silentc)
	$(Q)$(CC) -o $@ $(LDFLAGS) $(TARGET_PROG2_EXTRA_LDFLAGS) $(sort $(TARGET_PROG2_OBJECTS)) $(TARGET_PROG2_EXTRAS) $(SYSLIBS)
ifndef NOPROGCOPY
	$(Q)cp -f $@ $(DBINDIR)
endif
endif

#
# TARGET_PROG3: TARGET_PROG3_OBJECTS for compiling .o files into an executable
#
ifdef TARGET_PROG3

$(TARGET_PROG3): $(TARGET_PROG3_OBJECTS) $(TARGET_PROG3_EXTRAS)
	$(silentc)
	$(Q)$(CC) -o $@ $(LDFLAGS) $(TARGET_PROG3_EXTRA_LDFLAGS) $(sort $(TARGET_PROG3_OBJECTS)) $(TARGET_PROG3_EXTRAS) $(SYSLIBS)
ifndef NOPROGCOPY
	$(Q)cp -f $@ $(DBINDIR)
endif
endif

#
# TARGET_PROG4: TARGET_PROG4_OBJECTS for compiling .o files into an executable
#
ifdef TARGET_PROG4

$(TARGET_PROG4): $(TARGET_PROG4_OBJECTS) $(TARGET_PROG4_EXTRAS)
	$(silentc)
	$(Q)$(CC) -o $@ $(LDFLAGS) $(TARGET_PROG4_EXTRA_LDFLAGS) $(sort $(TARGET_PROG4_OBJECTS)) $(TARGET_PROG4_EXTRAS) $(SYSLIBS)
ifndef NOPROGCOPY
	$(Q)cp -f $@ $(DBINDIR)
endif
endif

#
# TARGET_PROG5: TARGET_PROG5_OBJECTS for compiling .o files into an executable
#
ifdef TARGET_PROG5

$(TARGET_PROG5): $(TARGET_PROG5_OBJECTS) $(TARGET_PROG5_EXTRAS)
	$(silentc)
	$(Q)$(CC) -o $@ $(LDFLAGS) $(TARGET_PROG5_EXTRA_LDFLAGS) $(sort $(TARGET_PROG5_OBJECTS)) $(TARGET_PROG5_EXTRAS) $(SYSLIBS)
ifndef NOPROGCOPY
	$(Q)cp -f $@ $(DBINDIR)
endif
endif


#
# TARGET_SO: TARGET_SO_OBJECTS for compiling .o files into a shared library (.so file)
#
ifdef TARGET_SO

$(TARGET_SO): $(TARGET_SO_OBJECTS) $(TARGET_SO_EXTRAS)
	$(silentl)
	$(Q)$(LD) $(LDSONAME) -fPIC $(LDDFLAGS) -o $@ $(sort $(TARGET_SO_OBJECTS)) $(TARGET_SO_EXTRAS) -lc
ifndef NOSOLIBCOPY
	$(Q)cp -f $@ $(SOLIBDIR)
endif
endif

#
# TARGET_SO1: TARGET_SO1_OBJECTS for compiling .o files into a shared library (.so file)
#
ifdef TARGET_SO1

$(TARGET_SO1): $(TARGET_SO1_OBJECTS) $(TARGET_SO1_EXTRAS)
	$(silentl)
	$(Q)$(LD) $(LDSONAME) -fPIC $(LDDFLAGS) -o $@ $(sort $(TARGET_SO1_OBJECTS)) $(TARGET_SO1_EXTRAS) -lc
ifndef NOSOLIBCOPY
	$(Q)cp -f $@ $(SOLIBDIR)
endif
endif

#
# TARGET_A: TARGET_A_OBJECTS for compiling .o files into a static library (.a file)
#
ifdef TARGET_A

$(TARGET_A): $(TARGET_A_OBJECTS) $(TARGET_A_EXTRAS)
	$(silentl)
	$(Q)-rm -f $@
	$(Q)$(AR) r $@ $(sort $(TARGET_A_OBJECTS)) $(TARGET_A_EXTRAS)
	$(Q)$(RANLIB) $@
ifndef NOALIBCOPY
	$(Q)cp -f $@ $(ALIBDIR)
endif
endif

#
# TARGET_A1: TARGET_A1_OBJECTS for compiling .o files into a static library (.a file)
#
ifdef TARGET_A1

$(TARGET_A1): $(TARGET_A1_OBJECTS) $(TARGET_A1_EXTRAS)
	$(silentl)
	$(Q)-rm -f $@
	$(Q)$(AR) r $@ $(sort $(TARGET_A1_OBJECTS)) $(TARGET_A1_EXTRAS)
	$(Q)$(RANLIB) $@
ifndef NOALIBCOPY
	$(Q)cp -f $@ $(ALIBDIR)
endif
endif

#
# TARGET_A2: TARGET_A2_OBJECTS for compiling .o files into a static library (.a file)
#
ifdef TARGET_A2

$(TARGET_A2): $(TARGET_A2_OBJECTS) $(TARGET_A2_EXTRAS)
	$(silentl)
	$(Q)-rm -f $@
	$(Q)$(AR) r $@ $(sort $(TARGET_A2_OBJECTS)) $(TARGET_A2_EXTRAS)
	$(Q)$(RANLIB) $@
ifndef NOALIBCOPY
	$(Q)cp -f $@ $(ALIBDIR)
endif
endif


#
# Generating dependency files in .deps/ directory while compiling
#
define generatedep
    @-[ -d $(DEPDIR) ] || mkdir -p $(DEPDIR) 2>&1 > /dev/null
    @-if [ -f "$*.d" ]; then \
        OUTFILE=`echo $*.d | sed -e 's/\//_/g'` && \
        (cp $*.d $(DEPDIR)/$$OUTFILE; \
            sed -e 's/#.*//' -e 's/\.\./_/g' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $(DEPDIR)/$$OUTFILE; \
                rm -f $*.d) \
    fi
endef

#
# C compiler command
#
define compilec
    $(silentc)
    $(Q)$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.d)
    $(generatedep)
endef

#
# C++ compiler command
#
define compilecxx
    $(silentc)
    $(Q)$(CXX) $(CFLAGS) -c $< -o $@ -MD -MF $(@:.o=.d)
    $(generatedep)
endef

#
# C++ rule
#
%.o:%.cpp
	$(compilecxx)

#
# C rule
#
%.o:%.c
	$(compilec)

#
# clean
#
.PHONY: clean
clean:: $(SUBDIRSTARGET)
	$(Q)-rm -rf *.o *.a *~
ifdef EXTRA_CLEAN
	$(Q)-rm -rf $(EXTRA_CLEAN)
endif
ifdef TARGET_O
	$(Q)-rm -rf $(TARGET_O) $(TARGET_O_OBJECTS)
endif
ifdef TARGET_O1
	$(Q)-rm -rf $(TARGET_O1) $(TARGET_O1_OBJECTS)
endif
ifdef HOST_PROG
	$(Q)-rm -rf $(HOST_PROG) $(HOST_PROG_OBJECTS)
endif
ifdef HOST_PROG1
	$(Q)-rm -rf $(HOST_PROG1) $(HOST_PROG1_OBJECTS)
endif
ifdef HOST_PROG2
	$(Q)-rm -rf $(HOST_PROG2) $(HOST_PROG2_OBJECTS)
endif
ifdef TARGET_PROG
	$(Q)-rm -rf $(TARGET_PROG) $(TARGET_PROG_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG)
endif
endif
ifdef TARGET_PROG1
	$(Q)-rm -rf $(TARGET_PROG1) $(TARGET_PROG1_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG1)
endif
endif
ifdef TARGET_PROG2
	$(Q)-rm -rf $(TARGET_PROG2) $(TARGET_PROG2_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG2)
endif
endif
ifdef TARGET_PROG3
	$(Q)-rm -rf $(TARGET_PROG3) $(TARGET_PROG3_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG3)
endif
endif
ifdef TARGET_PROG4
	$(Q)-rm -rf $(TARGET_PROG4) $(TARGET_PROG4_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG4)
endif
endif
ifdef TARGET_PROG5
	$(Q)-rm -rf $(TARGET_PROG5) $(TARGET_PROG5_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG5)
endif
endif
ifdef TARGET_PROG6
	$(Q)-rm -rf $(TARGET_PROG6) $(TARGET_PROG6_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG6)
endif
endif
ifdef TARGET_PROG7
	$(Q)-rm -rf $(TARGET_PROG7) $(TARGET_PROG7_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG7)
endif
endif
ifdef TARGET_PROG8
	$(Q)-rm -rf $(TARGET_PROG8) $(TARGET_PROG8_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG8)
endif
endif
ifdef TARGET_PROG9
	$(Q)-rm -rf $(TARGET_PROG9) $(TARGET_PROG9_OBJECTS)
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG9)
endif
ifdef TARGET_PROG10
	$(Q)-rm -rf $(TARGET_PROG10) $(TARGET_PROG10_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG10)
endif
endif
ifdef TARGET_PROG11
	$(Q)-rm -rf $(TARGET_PROG11) $(TARGET_PROG11_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG11)
endif
endif
ifdef TARGET_PROG12
	$(Q)-rm -rf $(TARGET_PROG12) $(TARGET_PROG12_OBJECTS)
ifndef NOPROGCOPY
	$(Q)-rm -f $(DBINDIR)/$(TARGET_PROG12)
endif
endif
ifdef TARGET_A
	$(Q)-rm -rf $(TARGET_A) $(TARGET_A_OBJECTS)
ifndef NOALIBCOPY
	$(Q)-rm -f $(ALIBDIR)/$(TARGET_A)
endif
endif
ifdef TARGET_A1
	$(Q)-rm -rf $(TARGET_A1) $(TARGET_A1_OBJECTS)
ifndef NOALIBCOPY
	$(Q)-rm -f $(ALIBDIR)/$(TARGET_A1)
endif
endif
ifdef TARGET_A2
	$(Q)-rm -rf $(TARGET_A2) $(TARGET_A2_OBJECTS)
ifndef NOALIBCOPY
	$(Q)-rm -f $(ALIBDIR)/$(TARGET_A2)
endif
endif
ifdef TARGET_SO
	$(Q)-rm -rf $(TARGET_SO) $(TARGET_SO_OBJECTS)
ifndef NOSOLIBCOPY
	$(Q)-rm -f $(SOLIBDIR)/$(TARGET_SO)
endif
endif
ifdef TARGET_SO1
	$(Q)-rm -rf $(TARGET_SO1) $(TARGET_SO1_OBJECTS)
ifndef NOSOLIBCOPY
	$(Q)-rm -f $(SOLIBDIR)/$(TARGET_SO1)
endif
endif

# 
# realclean
#
.PHONY: realclean
realclean: clean
	$(Q)-rm -rf $(DEPDIR) *.d

#
# added dependency files
#
-include $(DEPDIR)/*.d

