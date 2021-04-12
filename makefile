AR=ar -cvq
MD=@mkdir -p
RM=@rm -rf
CP=@cp -f
LN=@ln -sf
CC=g++
LK=g++
CCNAME=gcc-$(shell $(CC) -dumpversion)
CCFLAGS=-std=c++11 -fPIC -c -w -O2
LKFLAGS=
LDFLAGS=-shared -Wl,-soname,$(DLLFILE)
ARFLAGS=
LIBRARY=pthread z bz2 lz4 lzma
DEFINES=LINUX GCC
PRJNAME=openanno
PRJNICK=oa
VERSION=0.0.1
SRVFILE=$(PRJNICK)server
CLTFILE=$(PRJNICK)client
EXEFILE=$(PRJNAME)
LIBFILE=lib$(PRJNAME).a
DLLFILE=lib$(PRJNAME).so

OUTPATH=tmp/$(CCNAME)
OBJPATH=$(OUTPATH)/obj
BINPATH=$(OUTPATH)/bin
LIBPATH=$(OUTPATH)/lib
DLLPATH=$(OUTPATH)/dll
SRCROOT=common
INCROOT=.
INCPATH=$(addprefix src/, . $(SRCROOT))
FINDHPP=$(wildcard  $(addprefix src/$(dir)/, *.h   */*.h   */*/*.h   */*/*/*.h))
FINDCPP=$(wildcard  $(addprefix src/$(dir)/, *.cpp */*.cpp */*/*.cpp */*/*/*.cpp))
LIBINCS=$(foreach   dir, $(INCROOT), $(FINDHPP))
LIBSRCS=$(foreach   dir, $(SRCROOT), $(FINDCPP))
LIBOBJS=$(addprefix $(OBJPATH)/, $(LIBSRCS:.cpp=.o))
EXESRCS=$(foreach   dir, locals, $(FINDCPP))
EXEOBJS=$(addprefix $(OBJPATH)/, $(EXESRCS:.cpp=.o))
SRVSRCS=$(foreach   dir, server, $(FINDCPP))
SRVOBJS=$(addprefix $(OBJPATH)/, $(SRVSRCS:.cpp=.o))
CLTSRCS=$(foreach   dir, client, $(FINDCPP))
CLTOBJS=$(addprefix $(OBJPATH)/, $(CLTSRCS:.cpp=.o))

all: $(DLLFILE) $(LIBFILE) $(EXEFILE)

shared: $(DLLFILE)

static: $(LIBFILE)

local:  $(EXEFILE)

server: $(SRVFILE)

client: $(CLTFILE)

clean:
	$(RM)   $(OBJPATH)

$(DLLFILE): $(LIBOBJS)
	$(MD)   $(DLLPATH)
	$(RM)   $(DLLPATH)/$@
	$(LK)   $(LIBOBJS) -o $(DLLPATH)/$@ $(LDFLAGS) $(addprefix -L,$(LIBPATH)) $(addprefix -l,$(LIBRARY))

$(LIBFILE): $(LIBOBJS)
	$(MD)   $(LIBPATH)
	$(RM)   $(LIBPATH)/$@
	$(AR)   $(LIBPATH)/$@ $(LIBOBJS)

$(EXEFILE): $(EXEOBJS)
	$(MD)   $(BINPATH)
	$(LK)   $(EXEOBJS) -o $(BINPATH)/$@ $(LKFLAGS) $(addprefix -L,$(LIBPATH)) $(addprefix -l,$(PRJNAME)) $(addprefix -l,$(LIBRARY))

$(SRVFILE): $(SRVOBJS)
	$(MD)   $(BINPATH)
	$(LK)   $(SRVOBJS) -o $(BINPATH)/$@ $(LKFLAGS) $(addprefix -L,$(LIBPATH)) $(addprefix -l,$(PRJNAME)) $(addprefix -l,$(LIBRARY))

$(CLTFILE): $(CLTOBJS)
	$(MD)   $(BINPATH)
	$(LK)   $(CLTOBJS) -o $(BINPATH)/$@ $(LKFLAGS) $(addprefix -L,$(LIBPATH)) $(addprefix -l,$(PRJNAME)) $(addprefix -l,$(LIBRARY))

$(OBJPATH)/%.o: %.cpp $(LIBINCS)
	$(MD)   $(@D)
	$(CC)   $< -o $@ $(CCFLAGS) $(addprefix -D,$(DEFINES)) $(addprefix -I,$(INCPATH))
