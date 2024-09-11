particleDmlc++
Installation Instructions:

1. Make libUVA
   >cd libUVA
   >make
2. Make libparticleDmlc
   >cd libparticleDmlc
   >make
3. Make demo program
   >cd particleDmlc++Demo
   a. Assumes EGSnrc is installed and libiaea_phsp.so has been compiled
   b. Makefile has a LOCAL HACK TO GET THIS GOING QUICKLY
      HEN_HOUSE=/mnt/d/jvsData/code/EGSnrc/HEN_HOUSE
      Change to your HEN_HOUSE location
   c.  Edit Makefile to include correct path for libiaea_phsp.so)
   >make
4. Run the test program
   >cd testRun
   >./runTests


-------------------------------------------------------
Notes:
-------------------------------------------------------
General:
This code was written before cmake existed.  For rapid distribution, I am leaving it as is, using a standard
Makefile.

Minimal effort has been made to clean up the Makefiles.  Additionally, libUVA contains many functions/modules that
are not necessary for libparticleDmlc, however, cleaning it out takes time.

particleDmlc++Demo
As I use it, particleDmlc++Demo reads the mlc command files (.dml, .dma) to interpret
the MLC leaf sequences.  The default output format for these files (e.g. from Pinnacle) DOES NOT contain
control point weight information in them for Arc treatments.

To accommodate this, I wrote code to output the weight information into the note-lines in the
.dml file.  This can be seen by looking at the smartArc.dml demo file (grep iCP smartArc.dml).

For your use, you will need to write some code to get the CP weight info into the code.

The information about the leaf sequences does not need to come from a .dml/.dma file.  It could come directly from the
DICOM plan file.  (I think I have some code, not included here) that directly uses DICOM (or, at least converts the
DICOM to .dml).    For efficiency in releasing this code, it is not included here.

libparticleDmlc is most efficiently used w/o reading/writing phase space files.  Instead directly it into your
source head model.  Take your particle type, convert it to the particle type expected by the code, run the particle,
then convert back to your particle type.

As of ~2022, the code now also has a very simplistic implementation of Jaw tracking in it.

Implementing other MLC leaf geometries (e.g. HDMLC or other vendor MLCs) is possible by using different config files.


