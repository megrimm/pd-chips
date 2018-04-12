# File: ./src/Makefile.am
# Package: ratts
# Description:
#   + src-level automake file
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Options & Subdirectories
#-----------------------------------------------------------------------

## --- recursion subdirectories
#SUBDIRS = 

## --- pseudo-deps for '.SUFFIXES'
SUFFIXES = .@PDEXT@

#-----------------------------------------------------------------------
# Flags and variables
#-----------------------------------------------------------------------
PDEXT    = @PDEXT@
EXEEXT   = .@PDEXT@

#-----------------------------------------------------------------------
# pd externals (hacked _PROGRAMS target)
#-----------------------------------------------------------------------

## --- externals
pdexterns_PROGRAMS = @PD_OBJECT_EXTERNALS@ ratts

EXTRA_PROGRAMS = \
	klatt~ \
	holmes \
	holmes-feat \
	holmes-mask \
	phones2holmes \
	guessphones \
	number2text \
	rattshash \
	rattstok \
	toupper \
	spellout

## --- patches
pdexterns_DATA = \
	ratts~.pd \
	rattsdict.pd \
	holmes-features.pd

## --- documentation
pddoc_DATA = \
	ratts-help.pd \
	klatt~-help.pd \
	holmes-help.pd \
	holmes-feat-help.pd \
	holmes-mask-help.pd \
	phones2holmes-help.pd \
	guessphones-help.pd \
	holmes-phones.pd \
	number2text-help.pd \
	rattshash-help.pd \
	rattshread-help.pd \
	rattshwrite-help.pd \
	toupper-help.pd \
	spellout-help.pd \
	rattstok-help.pd


#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------

klatt__SOURCES = \
	proto.h \
	parwave.c parwave.h \
	klatt_frame.c klatt_frame.h \
	klatt~.c

holmes_SOURCES = \
	proto.h \
	parwave.c parwave.h \
	klatt_frame.c klatt_frame.h \
	useconfig.h \
	elements.c elements.h phfeat.h phfeat.c \
	Elements.def \
	squeue.c squeue.h \
	dsqueue.c dsqueue.h \
	rholmes.c rholmes.h \
	pars.def \
	pd_holmes.c pd_holmes.h \
	alhash.c alhash.h hashsizes.def \
	holmes.c

holmes_feat_SOURCES = \
	proto.h \
	useconfig.h \
	parwave.c parwave.h \
	klatt_frame.c klatt_frame.h \
	elements.c elements.h phfeat.h phfeat.c \
	Elements.def \
	squeue.c squeue.h \
	dsqueue.c dsqueue.h \
	rholmes.c rholmes.h \
	pars.def \
	pd_holmes.c pd_holmes.h \
	alhash.c alhash.h hashsizes.def \
	holmes-feat.c

holmes_mask_SOURCES = \
	proto.h \
	useconfig.h \
	parwave.c parwave.h \
	klatt_frame.c klatt_frame.h \
	elements.c elements.h phfeat.h phfeat.c \
	Elements.def \
	squeue.c squeue.h \
	dsqueue.c dsqueue.h \
	rholmes.c rholmes.h \
	pars.def \
	pd_holmes.c pd_holmes.h \
	alhash.c alhash.h hashsizes.def \
	holmes-mask.c

phones2holmes_SOURCES = \
	proto.h \
	useconfig.h \
	elements.c elements.h phfeat.h phfeat.c \
	Elements.def \
	pd_phtoelm.c pd_phtoelm.h \
	phtoelm.c phtoelm.h phtoelm.def \
	darray.c darray.h \
	trie.c trie.h \
	squeue.c squeue.h \
	dsqueue.c dsqueue.h \
	phones2holmes.c

guessphones_SOURCES = \
	proto.h \
	useconfig.h \
	text.c text.h english.c \
	darray.h darray.c \
	phoneutils.h phoneutils.c \
	guessphones.c

number2text_SOURCES = \
	proto.h \
	useconfig.h \
	darray.h darray.c \
	phoneutils.h phoneutils.c \
	saynum.h saynum.c \
	number2text.c

rattstok_SOURCES = \
	suspect.h suspect.c \
	rattstok.c

spellout_SOURCES = \
	ASCII.h ASCII.c \
	spellout.c

toupper_SOURCES = \
	toupper.c

rattshash_SOURCES = \
	alhash.c alhash.h hashsizes.def \
	ratts_keyval.c ratts_keyval.h \
	rattshash.h \
	rattshash.c

ratts_SOURCES = \
	proto.h \
	parwave.c parwave.h \
	klatt_frame.c klatt_frame.h \
	useconfig.h \
	elements.c elements.h \
	Elements.def \
	squeue.c squeue.h \
	dsqueue.c dsqueue.h \
	rholmes.c rholmes.h \
	phfeat.c phfeat.h \
	pd_holmes.c pd_holmes.h \
	alhash.c alhash.h hashsizes.def \
	pars.def \
	phtoelm.c phtoelm.h phtoelm.def \
	pd_phtoelm.c pd_phtoelm.h \
	darray.c darray.h \
	phoneutils.h phoneutils.c \
	trie.c trie.h \
	text.c text.h english.c \
	saynum.h saynum.c \
	ratts_keyval.c ratts_keyval.h \
	rattshash.h \
	suspect.h suspect.c \
	ASCII.h ASCII.c \
	klatt~.c \
	holmes.c \
	holmes-feat.c \
	holmes-mask.c \
	phones2holmes.c \
	guessphones.c \
	number2text.c \
	rattshash.c \
	rattstok.c \
	toupper.c \
	spellout.c \
	ratts.c


#ratts_SOURCES = $(sort $(klatt__SOURCES) $(holmes_SOURCES)) ratts.c
# ratts_SOURCES = \
#	$(klatt_holmes_shared_sources) \
#	klatt~.c
#	$(holmes_only_sources) \
#	ratts.c

#-----------------------------------------------------------------------
# external compilation : flags
#-----------------------------------------------------------------------
DEFS    = @DEFS@
AFLAGS  = @AFLAGS@
DFLAGS  = @DFLAGS@
IFLAGS  = @IFLAGS@
LFLAGS  = @LFLAGS@
OFLAGS  = @OFLAGS@
WFLAGS  = -Wall -Winline

#GLIB_IFLAGS = @GLIB_IFLAGS@
#GLIB_LFLAGS = @GLIB_LFLAGS@

AM_CPPFLAGS = $(IFLAGS) $(GLIB_IFLAGS) $(DFLAGS)
AM_CFLAGS   = $(OFLAGS) $(WFLAGS) $(AFLAGS)

klatt__LDFLAGS = $(LFLAGS)
klatt__LDADD   = $(GLIB_LFLAGS)

holmes_LDFLAGS = $(LFLAGS)
holmes_LDADD   = $(GLIB_LFLAGS)

ratts_LDFLAGS = $(LFLAGS)
ratts_LDADD   = $(GLIB_LFLAGS)

#-----------------------------------------------------------------------
# additional hacks
#-----------------------------------------------------------------------
holmes-features.pd: make-holmes-features.perl phfeat.h
	perl make-holmes-features.perl < phfeat.h > $@

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------
## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean: built by 'make'
CLEANFILES = *$(EXEEXT)

## --- distclean: built by 'configure'
DISTCLEANFILES = \
	config.log	\
	config.cache	\
	config.status

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = *~ \
	$(PODS:.pod=.txt) \
	holmes-features.pd \
	Makefile Makefile.in \
	aclocal.m4 \
	configure \
	install-sh \
	stamp-h.in \
	config.h.in

maintainer-clean-local:
	rm -rf autom4te.cache

#CVSCLEAN_SUBDIRS = $(SUBDIRS)

#CVSCLEANFILES = Makefile.in Makefile


#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	make-holmes-features.perl \
	$(pddoc_DATA) \
	$(pdexterns_DATA) \
	klatt-test.pd ksflink.pd klatt-parfile.pd

## --- recursion subdirectories for 'make dist'
DIST_SUBDIRS = $(SUBDIRS)

## --- dist-hook: when another 'Makefile.am' is overkill
#DISTHOOK_DIRS = foo
#DISTHOOK_FILES = foo/bar.txt foo/baz.txt
#dist-hook:
#	for d in $(DISTHOOK_DIRS); do\
#	  mkdir -p $(distdir)/$$d ;\
#	done
#	for f in $(DISTHOOK_FILES); do\
#	  cp -p $(srcdir)/$$f $(distdir)/$$f ;\
#	done

#dist-bz2: dist-bzip2 ;


#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean cvsclean-hook

cvsclean: maintainer-clean ;

