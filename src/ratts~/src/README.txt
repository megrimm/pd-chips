    README for pd external library 'ratts'

    Last updated for version 0.02.

DESCRIPTION
    "Realtime Analog Text-to-Speech" externals library for PD, built on code
    by Nick Ing-Simmons [1,2], Jon Iles [2], Orm Finnendahl [3], and Travis
    Newhouse [4]; based on work by D.H. Klatt [5,6], L.C. Klatt [6], J.N.
    Holmes, I. Mattingly, and J. Shearme [7].

EXTERNALS
    * klatt~
        A port of Jon Iles' and Nick Ing-Simmons' implementation of the
        Klatt Cascade-Parallel Formant Speech Synthesizer, as originally
        described by Dennis Klatt.

        See the attached patch "klatt-test.pd" for some idea of what it
        does. For detailed information on the available parameters and what
        they do, see the original README for the "klatt-3.04" package, in
        the distribution subdirectory "klatt3.04".

    * holmes
    * phones2holmes
        Two objects derived from Nick Ing-Simmons' implementation of the
        phoneme-to-vocal-tract-parameters system described by Holmes et. al.
        (1964).

    * guessphones
        Automatic text-to-phoneme conversion rules for english derived from
        [8].

    * number2text
        Converts floats to their english textual representations.

    * rattstok
        Tokenizer for raw text.

    * spellout
        Helps you learn your letters ;-)

    * toupper, tolower
        Normalize case of input messages.

    * rattshash, rattshread, rattshwrite
        Auto-resizable linked-list-entry hash table, based on Orm
        Finnendahl's "maphash". Suitable for large hash tables (>= 200K
        entries).

PROGRAMS
    * dict/mkdicttxt
        Creates a "cr" format dictionary file readable by [rattshash] from a
        text source file. Currently supports the following source
        dictionaries:

        1. "cmudict"
            Available from:

                  ftp://ftp.cs.cmu.edu:project/fgdata/dict

            Latest seems to be cmudict.0.6.Z

            The Carnegie Mellon Pronouncing Dictionary [cmudict.0.1] is
            Copyright 1993 by Carnegie Mellon University. Use of this
            dictionary, for any research or commercial purpose, is
            completely unrestricted. If you make use of or redistribute this
            material, we would appreciate acknowlegement of its origin.

        2. "beep"
            Available from:

                  ftp://svr-ftp.eng.cam.ac.uk/comp.speech/data

            Latest seems to be beep-0.4.tar.gz

            This is a direct desendant of CUVOLAD (british pronounciation)
            (as used by older releases of Nick Ing-Simmons' rsynth program),
            and so has a more restrictive copyright than the CMU dictionary.

        To create a dictionary readable by "rattshash" from one of the above
        sources, run:

              mkdicttxt DIALECT SOURCEFILE DESTFILE

        where DIALECT is one of:

                a : american english
                b : british english

ABSTRACTIONS
    * rattsdict
        Shareable dictionary based on 'rattshash'.

    * ratts~
        Full text-to-speech abstraction.

PLATFORMS
    * linux/x86
        This is what I run, so ratts really ought to work here.

    * MacOS X
        Adam T. Lindsay reported a successful build of ratts-0.01 on MaxOS
        X.

    * Other Platforms
        See REQUIREMENTS, below.

REQUIREMENTS
    In order to build the "ratts" library, you will need the following:

    * A C compiler
        Tested with gcc-2.95.4 under linux/x86 (Debian).

    * /bin/sh , sed
        In order to run the 'configure' script.

    * A make program
        Tested with GNU make v3.79.1 under linux/x86 (Debian).

    * PD
        Tested with pd v0.35.0 under linux/x86 (Debian). PD is available
        from:

         http://www.crca.ucsd.edu/~msp/software.html

INSTALLATION
    Issue the following commands to the shell:

       cd PACKAGENAME-X.YY  (or wherever you extracted the distribution)
       ./configure
       make
       make install

BUILD OPTIONS
    The 'configure' script supports the following options, among others:

    * --enable-object-externals , --disable-object-externals
        Whether to build single-object externals in addition to the ratts
        library. This may not work correctly, as it has not been extensively
        tested.

    * --with-pd-dir=DIR
        PD base directory.

    * --with-pd-include=DIR
        Directory where the PD include files live.

    * --with-pd-extdir=DIR
        Where to put the externals on 'make install'.

    * --enable-debug , --disable-debug
        Whether to enable verbose debugging messages. Default=no.

ACKNOWLEDGEMENTS
    PD by Miller Puckette and others.

    Based on original text-to-speech work by D.H. Klatt, L.C. Klatt, and
    J.N. Holmes, I. Mattingly, and J. Shearme.

    Original C port of the Klatt synthesiser code by Jon Iles and Nick
    Ing-Simmons.

    Original C ports of the Holmes/Mattingly/Shearme phonetic interpreter
    and the NRL english-to-phoneme rules, the numeric interpretation code in
    "saynum.c", the tokenization code in "suspect.c", and diverse other code
    by Nick Ing-Simmons.

    "rattshash", "rattshread", and "rattshwrite" externals based on
    "maphash" distribution by Orm Finnendahl, which is based on the "mapper"
    object by Travis Newhouse.

    Ideas, black magic, and other nuggets of information drawn from code by
    Guenter Geiger, Larry Troxler, and iohannes m zmoelnig.

KNOWN BUGS
    * General
        Very hard on the PD symbol-table: every word is its own symbol,
        likewise every phone-string. Tests with the "beep" dictionary (231K
        entries) nonetheless did not seem to greatly disturb PD -- way to
        go, Miller!

        Only tested under linux.

        Dynamic buffering is safe but slow.

        Build procedure isn't entirely clean.

    * rattstok
        None known.

    * rattshash
        Hashes may block on insert operations if autosize flag is set.
        Workaround: turn off autosize if you need the cpu time for other
        things.

    * toupper, tolower
        None known.

    * guessphones
        Could probably be less CPU-intensive. On the other hand, it's quite
        nice to be able to just drop it in and see it work ;-)

    * number2text
        Uses Nick's "darray", which may re-allocate; a linked-list-of-blocks
        might be less spiky, but would probably be slower in the average
        case.

    * phones2holmes
        No buffering of phone-strings avoids re-allocation but could get
        ugly for long utterances. Also uses darray.

    * holmes
        Still not great for a potential "singing voice" abstraction, but
        triplet-output from [phones2holmes] should help.

    * klatt~
        Way too many parameters.

        Disturbingly cpu-intensive.

        Ownership issues with the code -- it may be soon replaced by the
        synthesizer from Nick Ing-Simmons' upcoming rsynth-3.0.

REFERENCES
    [1] John Iles and Nick Ing-Simmons, "klatt3.04: Klatt Cascade-Parallel
        Format Synthesizer", April 1994.

         URL: http://www.cs.bham.ac.uk/~jpi/download/klatt.3.04.tar.gz

    [2] Nick Ing-Simmons, "rsynth-2.0", November, 1994.

         URL: ftp://svr-ftp.eng.cam.ac.uk/pub/comp.speech/synthesis/rsynth-2.0.tar.gz

    [3] Orm Finnendahl, "maphash-0.1", February, 2002.

         URL: http://icem-www.folkwang-hochschule.de/~finnendahl/pd.html

    [4] Travis Newhouse, "mapper-1.0", September, 1999.

         URL: http://www-cse.ucsd.edu/~newhouse

    [5] D. H. Klatt (1980) "Software for a cascade/parallel formant
        synthesizer.", Journal of the Acoustical Society of America 67(3),
        March 1980, pp 971--995.

    [6] D. H. Klatt L. C. Klatt (1990) "Analysis, synthesis and perception
        of voice quality variations among female and male talkers." Journal
        of the Acoustical Society of America 87(2), February 1990, pp
        820--857.

    [7] J. N. Holmes, I. Mattingly, and J. Shearme (1964) "Speech Synthesis
        by Rule." Language and Speech 7, pp 127--143.

    [8] Naval Research Laboratory (1976), "AUTOMATIC TRANSLATION OF ENGLISH
        TEXT TO PHONETICS BY MEANS OF LETTER-TO-SOUND RULES", NRL Report
        7948, Naval Research Laboratory, Washington, D.C., January 21st,
        1976.

AUTHOR / MAINTAINER
        Bryan Jurish <moocow@ling.uni-potsdam.de>

