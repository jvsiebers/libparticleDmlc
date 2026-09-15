
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

Currently, will put libraries in x86_64-linux directory.  Update Makefile(s) to change

1. Make all the codes

   a. Method 1: All in one Makefile
      ```bash 
       make
    ```
   
   b. Method 2: Per library Makefile

      i.Make libUVA
      ```bash
         cd libUVA
         make
      ```
     ii. Make libparticleDmlc 
      ```bash
         cd libparticleDmlc 
         make
     ```
     ii. Make libiaea_phsp
      ```bash
         cd libiaea_phsp 
         make
     ```
3. Make demo program
      ```bash
      cd particleDmlc++Demo
      make
      ```

4. Run the test program
```bash
   cd particleDmlc++Demo/testRun
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

The `native` subdirectory begins the replacement interface used by modern callers. It accepts a versioned,
array-based delivery record and deliberately contains no DICOM or DML parser. Configure it independently with
`cmake -S native -B build/native`, then build and run its tests with CMake/CTest. The legacy Makefile and demo remain
available as behavioral references while transport is moved behind this boundary.

The boundary implements the commissioned particleDmlc equivalent-thickness
model directly. It classifies the two MLC sections, samples forced interaction
in their summed longitudinal thickness, attenuates a forward scattered photon
through `remaining thickness / w`, and applies the historical infinitely-thin-
MLC reference-plane projection. This model uses the existing section tables and
does not require physical placement of each MLC material plane. A non-forward
scattered photon returns explicit empty state. Generic material-path
attenuation, deterministic propagation, and exact-interval location are also
available as optional extension points for an alternate solid-geometry model.

The production primary finalizer attenuates photons through the same summed
equivalent thickness. Incident electrons and positrons pass unchanged only
through zero thickness and are otherwise discarded. Generated Compton
electrons are used to close the sampled energy and momentum record but are not
retained by the first-release run policy; the historical fixed `20.6 MeV/cm`
energy-loss approximation is not part of the maintained implementation.

The native build also produces separate host and EGSnrc adapter libraries. The host adapter owns bounded product
queues and host-random callbacks. The EGSnrc adapter converts the native charged-particle kinetic-energy convention
to explicit EGSnrc total-energy stack entries, retains region/latch/lineage sidecars, and provides a bounded LIFO
test context. Its provenance-tagged dispatcher passes primary and scattered-photon entries to one synchronous host
callback and discards generated Compton electrons under the first-release policy. An optional target compiles that
callback against a selected EGSnrc `egs_config1.h`. The supplied Mortran routine owns the only common-block action:
it sets `LATCHI` immediately before calling `SHOWER`. The same target exports a configured Fortran scalar entry
point that accepts explicit lineage, product kind, and electron rest mass before invoking the validated dispatcher.
An isolated Fox DOSXYZ probe routes photon, electron, and positron basis particles through this round trip with
release and sanitizer dose output byte-identical to stock DOSXYZ. The transport core does not map Fortran common
blocks.

The host-adapter library also exports a versioned producer context. It deep-
copies one DICOM-derived delivery record and commissioned machine, selects one
explicit MLC device, samples the caller's exact fractional MU once, and queues
tagged primary then scattered-photon output. It uses libParticleDmlc's
commissioned leaf-equivalent-thickness tables directly; callers do not provide
physical z locations for individual MLC planes. The producer reports exact
host-random consumption and explicit generated-Compton-electron discard. Static
jaws remain the responsibility of upstream BEAM transport. When either DICOM
jaw pair moves, the producer samples one X and one Y pair at the same fractional
MU and applies the reviewed legacy ideal-aperture approximation at the incident
particle plane. DICOM X jaw coordinates are already aligned with the BEAM
transport X axis; DICOM Y is antiparallel to BEAM transport Y, so the producer
negates and swaps the Y banks before aperture testing. A jaw-blocked particle
consumes no random values and produces an empty queue. The producer leaves the
queue empty after any failed production call. C++ release and ASan/UBSan tests
plus a Python/shared-library acceptance probe exercise this boundary. Producer
contexts and their callback state are worker-owned and must not be called
concurrently; independent contexts may share immutable delivery and machine
definitions and run concurrently. An eight-worker test covers synchronized
context construction, transport, product drain, and destruction with isolated
random and sampled-weight callback state.

The separately built `mcdose_particle_dmlc_source_session` library adds a
transport-neutral scalar source lifecycle above that producer. An upstream
callback supplies incident particles, fractional meterset, candidate product
identifiers, and explicit history-boundary state. The session drains all MLC
products before requesting another incident particle and retains the source
history ID and boundary state on each descendant, so correlated source reuse
remains one statistical history. It deliberately has no DICOM, BEAMnrc, or
EGSnrc ABI dependency. A null producer supports direct pass-through; a
non-null producer is caller-owned and applies the commissioned MLC transport.
Batch operation and BEAM/EGSnrc consumer adapters are separate future layers.

An optional `MCDOSE_PARTICLE_DMLC_BUILD_STARTUP_LOADER=ON` target links SQLite
3.37 or newer outside the transport core. It strictly loads the versioned
Python-created delivery/machine artifact, verifies its complete canonical
payload SHA-256, calls the same deep-copying producer constructor, and releases
all database and parsing storage before transport. Release and sanitizer-linked
acceptance cover valid producer execution plus altered provenance, schema, and
application identity.

Enabling both the startup loader and EGSnrc Mortran bridge also builds
`mcdose_particle_dmlc_egsnrc_source_mortran_bridge`. It owns one startup-loaded
source context, calls EGSnrc's random stream on demand, converts configured
Fortran scalar widths, and returns exactly one queued product per call. Native
failures include bounded diagnostics and the incident scalar values. The
companion `mcdose_particle_dmlc_source_v1.mortran` keeps the opaque handle in a
small dedicated common block and does not map the DOSXYZ stack. The configured
bridge test also requires exact weight preservation for an open transport path.
Configured CTest runs set
`MCDOSE_PARTICLE_DMLC_TEST_SOURCE_STARTUP_ARTIFACT` to a generated synthetic
startup database; CMake reports when those artifact-dependent tests are not
registered.

The producer also exposes one optional, one-time sampled-weight callback. It is
called only after the exact fractional-MU state and all device openings have
been sampled, and before classification or transport. The separate
`mcdose_particle_dmlc_source_output_correction` integration library loads the
fixed-size `mcdose.particle-dmlc-source-output-correction/v1` artifact and
implements the migrated field-size evaluator without adding DICOM or
commissioning-config parsing to the transport core. The artifact is bound to
the exact startup payload SHA-256. The corrected EGSnrc source constructor
requires that binding, owns the evaluator configuration for the complete source
context lifetime, and scales all retained products once. The ordinary source
constructor retains identity weighting. Release and ASan/UBSan tests cover
Python-generated artifact loading, strict dimensions and integrity, startup
binding, constructor failure, and the exact corrected-to-identity weight ratio.

`native/integration/egsnrc_dosxyznrc_source21_v1.patch` is pinned to EGSnrc
commit `9edee3ebfda3d81d0e8eb033a7e76bf9a70e41ef`. It adds the explicit
`mcdose_particle_dmlc` source-21 token, forwards BEAM's scoring-plane ray and
exact fractional MU, drains the native queue before sampling BEAM again, and
preserves the queue and fractional MU across DOSXYZ parallel scheduler chunks.
Stock reset behavior remains unchanged for source 20 and non-native source 21.
It also adds opt-in source-2 patient-frame modes for IAEA files written after
delivery transforms such as MLC transport. Mode 1 performs an exact
translation-only handoff equivalent to legacy source 102. Mode 2 applies the
configured rotation before the isocenter translation. Both preserve stored Z
and both signs of W and disable only the source-plane W and `BEAM_SIZE`
filters. The unflagged source-2 path is unchanged.

The same pinned patch builds DOSXYZ for up to 100 media and 512 voxels per
axis, matching the runtime-manifest capacity contract. Its phantom reader
accepts both standard EGSnrc base-62 medium rows and legacy UVA fixed-width
three-digit medium rows. Mixed encodings, media outside the phantom's declared
range, and medium counts above the compiled limit are rejected before
transport.

The patch leaves the normal DOSXYZ `SHOWER` call in place and is an
EGSnrc-derived AGPL integration artifact. It applies with zero fuzz and passes
complete Mortran preprocessing, Fortran compilation, release linking, and
configured release/sanitizer scalar tests in an isolated tree. Complete
synthetic runs under fresh WSL and Fox configurations also transport all 100
BEAM products through the native path into DOSXYZ with zero misses or blocks;
their finite 41 x 41 x 80 dose files are byte-identical. The files under
`native/tests/fixtures` are wiring-smoke inputs only; they are not commissioned
or dosimetric reference data.

A deidentified three-beam HFS commissioned-input smoke also completes this
native path on Fox with two DOSXYZ workers per beam. All six workers transport
their requested histories over full-CT calculation grids, report zero geometry
misses, and produce `.pardose` normalization counts that exactly match the BEAM
primary-history counts. This is integration and accounting evidence, not
statistical convergence, absolute-dose validation, or clinical commissioning.

libparticleDmlc is most efficiently used w/o reading/writing phase space files.  Instead directly it into your
source head model.  Take your particle type, convert it to the particle type expected by the code, run the particle,
then convert back to your particle type.

As of ~2022, the code now also has a very simplistic implementation of Jaw tracking in it.

Implementing other MLC leaf geometries (e.g. HDMLC or other vendor MLCs) is possible by using different config files.


