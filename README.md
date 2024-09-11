
## 💖 Sponsor This Project

If you find this project useful or helpful, consider supporting its ongoing development!

You can sponsor me through **GitHub Sponsors**:

[![Sponsor Me](https://img.shields.io/badge/Sponsor-✨-ff69b4?style=for-the-badge&logo=github)](https://github.com/sponsors/jvsiebers)

Your sponsorship helps me dedicate more time to improving and maintaining this project, and I greatly appreciate your support!





# particleDmlc++
## About
This code implements the pseudo Monte Carlo photon transport through multi-leaf collimators as described in the publication

> Siebers JV, Keall PJ, Kim JO, Mohan R. A method for photon beam Monte Carlo multileaf collimator particle transport. 
> Phys Med Biol. 2002 Sep 7;47(17):3225-49. doi: 10.1088/0031-9155/47/17/312. PMID: 12361220.

The code is supplied as a library (libparticleDmlc), and a sample code (particleDmlc++Demo) uses the code (requires libUVA) 
to enable users to see a quick sample of how the code is run.

The important steps of the code are:
1. Getting plan delivery information (control point information, which specified Gantry, Collimator, and MLC positions) into a format recognizable by the code
2. Getting the input particle phase space coordinates (from another MC program, source code module, or phase space file)
3. Transport of particles through the MLC
4. Passing particles onto the next stage (particles to next part of the geometry, e.g. the patient/phantom/EPID, or write particles to a phase space file. 


## Installation Instructions:

1. Make libUVA
```bash
   cd libUVA
   make
```
2. Make libparticleDmlc 
```bash
    cd libparticleDmlc 
    make
```
3. Make demo program

   a. Assumes EGSnrc is installed and `libiaea_phsp.so` has been compiled.

   b. Makefile has a **LOCAL HACK TO GET THIS GOING QUICKLY**:
      ```bash
      HEN_HOUSE=/mnt/d/jvsData/code/EGSnrc/HEN_HOUSE
      Change to your HEN_HOUSE location
      ```

   c. Edit Makefile to include the correct path for `libiaea_phsp.so`:
      ```bash
      cd particleDmlc++Demo
      make
      ```

4. Run the test program
```bash
   cd testRun
   ./runTests
   ```


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


