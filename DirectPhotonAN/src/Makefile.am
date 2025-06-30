AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
  -I$(includedir) \
  -isystem$(OFFLINE_MAIN)/include \
  -isystem$(ROOTSYS)/include

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib \
  -L$(OFFLINE_MAIN)/lib64 \
  -lcalo_io \
  -lcalotrigger_io \
  -lcentrality_io \
  -lCLHEP \
  -lglobalvertex \
  -lglobalvertex_io \
  -lffarawmodules \
  -lg4dst \
  -lg4eval

ROOTDICTS = \
  Debugger_Dict.cc

lib_LTLIBRARIES = \
  libDirectPhotonAN.la

pcmdir = $(libdir)

nobase_dist_pcm_DATA = \
  Debugger_Dict_rdict.pcm

libDirectPhotonAN_la_SOURCES = \
  $(ROOTDICTS) \
  DirectPhotonAN.cc \
  Debugger.cc

pkginclude_HEADERS = \
  DirectPhotonAN.h \
  Debugger.h

BUILT_SOURCES = testexternals.cc Debugger_Dict.cc Debugger_Dict.h Debugger_Dict_rdict.pcm

libDirectPhotonAN_la_LIBADD = \
  -lphool \
  -lmbd_io \
  -lsph_onnx \
  -lSubsysReco

noinst_PROGRAMS = \
  testexternals

testexternals_SOURCES = testexternals.cc
testexternals_LDADD   = libDirectPhotonAN.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

%_Dict.cc %_Dict.h: %.h %LinkDef.h
	rootcint -f $*_Dict.cc @CINTDEFS@ -c $(DEFAULT_INCLUDES) $(AM_CPPFLAGS) $^

#just to get the dependency
%_Dict_rdict.pcm: %_Dict.cc ;

clean-local:
	rm -f *Dict* $(BUILT_SOURCES) *.pcm
