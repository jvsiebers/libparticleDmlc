/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
/*
   Copyright 2000-2003 Virginia Commonwealth University


   Advisory:
1. The authors make no claim of accuracy of the information in these files or the 
   results derived from use of these files.
2. You are not allowed to re-distribute these files or the information contained within.
3. This methods and information contained in these files was interpreted by the authors 
   from various sources.  It is the users sole responsibility to verify the accuracy 
   of these files.  
4. If you find a error within these files, we ask that you to contact the authors 
   and the distributor of these files.
5. We ask that you acknowledge the source of these files in publications that use results
   derived from the input 

   Please contact us if you have any questions 
*/
/* dmlcRoutines.cc

   Routines to read .dml files and either convert them to 
   a BEAM/MCNP-type input or process the phase space itself

Created: 9 Nov 1999: PJK

Modification History:
        2 Dec 1999: PJK: added arguments for coll
        7 Dec 1999: JVS: JVS adding comments as I am learning the code
        9 Dec 1999: JVS: Adding -density, compile on XENA too
       20 Jan 2000: PJK: Add RR 
        3 Feb 2000: PJK: Removed (int) bug setting (-1,1) to 0
       13 Feb 2000: PJK: Add scattered dose
       18 Feb 2000: PJK: After talking to MA changed to have VARIAN CLOSED throughout motion
       15 Jun 2000: PJK: Added numLeaf flag for 120 and 80 leaf
                       : Added fabs so leaves can travel in both directions
       19 Jun 2000: PJK: Cleaning up into subroutines (much nicer)
       18 August 2000: 0.02: JVS: Reads in MLC parameters from config file
        -- temporary still is limited to single leaf thickness profile
           so can compare results with the 80 leaf results....
           -- Results check WRT the old version (read from #defines) and
              results are nearly identical (definition of PI changed between versions so not identical....)
       21 August 2000: JVS: will work for multi-section leaves IFF period constant (all leaves same thickness)
       23 August 2000: JVS: Break off most routines to dmclRoutines.cc so can run with
                            different MAIN programs....
       07 Sept 2000: JVS: createArray returns OK/FAIL so remove check for NULL
       2 October 2000: PJK: added Russian Roulette structure 
       12 December 2000: PJK: Added keep structure and removed PRIM_ONLY rubbish to transport_phase_space
       04-Jan-2001: JVS: Add routines for particleDmlc
                         Put time back in forward order (shapes that were referred to as [(dmlc->numApertures-1)-s]
                         are now referred to as [s]
       11-Jan-2000: JVS: Orientation changed to match actual beam geometry
       06-April-2001: JVS: Skip reading "Shape =" line from DMLC file as it is not used and can be
                           arbitrary value
                         Make offset apply only for 120 when leaves are not closed...
       09-April-2001: JVS: Improve read-out of Dose Rate
       21-May-2001: JVS: Add read or VarianMlcDataType so leaf offsets are correct....
       28-June-2001: JVS: put softTouch stuff in convertDmlcCoodinates....
       03-August-2001: JVS: don't read poperator for now...should allocate number of segments...
       28-Nov-2001: JVS: Stop reading Field = from .dml file...Pinnacle writes it in a different format.
       12-April-2002: JVS: Add physicalLeafOffset 
       15-April-2002: JVS: put in writeDMLCFile and convertDmlcCoordinatesToIsocenterLocation
       06-Jan-2003: JVS: Strip out un-necessary files for distribution of particleDmlc
       04-Feb 4-2003: JVS: Modified so no longer reads in Hack of "grepped" table
                           requires user to pass table that has the L,K,...lines removed from it 
       (Merged with 10-Nov-2005: ibm...make for c++ with classes
                    jvs....remove write_particle because of conflict with mkf particle_type )
       05-Oct-2006: JVS: replace rand()/RAND_MAX with randomOne so uses same generator for all things
       07-Feb-2007: JVS: Make so will work with C++ Classes.....
       16-Feb-2007: JVS: read_mu_table first looks locally, then to config dirs.
       05-Nov-2007: JVS: eliminate myerrno, use check
       05-Dec-2007: JVS: use %g in error_part, be explitic in using 0.0f instead of 0. (for 64 bit machines, it makes a differance)
    
       Nov 11, 2008: JVS: Remove warnings for c++ class 
                 "warning: declaration of 'someVariable' shadows a member of 'this'"
                 for routines 
		 convertDmlcCoordinatesToMlcLocation
		 projectDMLCtoMLC
		 readDmlcFile
		 convertDmlcCoordinatesToIsocenterLocation
       Dec 15, 2011: JVS/JKG: Add support for Version "H" dml files
       Dec 17,2014: JVS: Add isArc
       May 7, 2015: JVS@UVA: Permit reading in single control point files
       Oct 30, 2015: JVS: Convert dmlc_type to class --- add class definitions here
//
//
// Need to move isArc, isCPInfoSet, and CP _all_ into dmlc_class (currently in particleDmlcClass)
// _and_ need to convert particleDmlc::readDmlcFile so just reads the dmlc_type::readDmlcFile....
*/
/* *********************************************************************** */
/* *********************************************************************** */
// system libraries
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> /* for fabs */
#include <sys/syscall.h>

// other libraries
#include "utilities.h"
#include "option.h"
#include "string_util.h"
#include "common.h"
#include "table.h"
#include "spline_data.h"
#include "routines.h"
#include "read_write_egs.h"
#include "myrandom.h" // for randomOne

using namespace std;

//local libraries
#ifdef C_CLASS
#include "particleDmlcClass.h"
#else

#include "particleDmlc.h"

#endif
// float MLC_Density;
/* ********************************************************************** */
dmlc_type::dmlc_type() {
    dmlc_file_name[0] = 0;
    numApertures = 0;
    num_leaves = 0;
    CP = NULL;
    ALeafBank = NULL;
    BLeafBank = NULL;
}

/* ********************************************************************** */
dmlc_type::dmlc_type(const char *fileName) {
    if (!readDmlcFile(fileName)) {
        throw "ERROR: Reading readDmlcFile";
    }
}

/* ********************************************************************** */
dmlc_type::~dmlc_type() {
    if (NULL != ALeafBank) {
        if (NULL != ALeafBank->matrix) {
            free(ALeafBank->matrix);
            ALeafBank->matrix = NULL;
        }
        delete ALeafBank;
        ALeafBank = NULL;
    }
    if (NULL != BLeafBank) {
        if (NULL != BLeafBank->matrix) {
            free(BLeafBank->matrix);
            BLeafBank->matrix = NULL;
        }
        delete BLeafBank;
        BLeafBank = NULL;
    }
}

/* ********************************************************************** */
bool dmlc_type::readDmlcFile(const char *fileName) {
    strcpy(dmlc_file_name, fileName);
    return (readDmlcFile());
}

bool dmlc_type::readDmlcFile() {
#ifdef DEBUG
    printf("\n Opening file %s", dmlc_file_name);
    printf("\n StringLength = %d, MAX_STR_LEN = %d", strlen(dmlc_file_name), MAX_STR_LEN);
#endif

    FILE *fp;
    if ((fp = fopen(dmlc_file_name, "r")) == NULL) {
        printf("\n ERROR: can't open file %s! \n", dmlc_file_name);
        return false;
    }
    //read header
    if (string_after(fp, "File Rev =", file_rev) != OK) {
        printf("\n ERROR: string after in %s \n", dmlc_file_name);
        return false;
    }
    if (string_after(fp, "Treatment =", treatment) != OK) {
        printf("\n ERROR: string after in %s \n", dmlc_file_name);
        return false;
    }
    if (string_after(fp, "Last Name =", last_name) != OK) {
        printf("\n ERROR: string after in %s \n", dmlc_file_name);
        return false;
    }
    if (string_after(fp, "First Name =", first_name) != OK) {
        printf("\n ERROR: string after in %s \n", dmlc_file_name);
        return false;
    }
    if (string_after(fp, "Patient ID =", patient_id) != OK) {
        printf("\n ERROR: string after in %s \n", dmlc_file_name);
        return false;
    }
    numApertures = (int) get_value(fp, "Number of Fields =");
    if (FAIL == checkStringError()) {
        printf("\n ERROR: get_value Number of Fields =");
        return false;
    }

    if (1 == numApertures) staticMLC = true;
    else staticMLC = false;
    if (!strncmp(treatment, "Dynamic Arc", 11)) {
        isArc = true;
        // staticMLC=false;
    } else {
        if (isArc) {
            printf("\n ERROR: %s: .dml reports Treatment=%s, and isArc was previously set", __FUNCTION__, treatment);
            return false;
        }
    }
    printf("\n\t Treatment: %s", treatment);
    //printf("\n\t isArc = %s", isArc ? "true" : "false");
#ifdef DEBUG
    printf("\n\t isArc = %s", isArc ? "true" : "false");
    printf("\n\t strncmp(treatment,\"Dynamic Arc\",11) = %d", strncmp(treatment,"Dynamic Arc",11));
    printf("\n\t First name: %s",first_name);
    printf("\n\t Last name %s",last_name);
    printf("\n\t ID %s",patient_id);
    printf("\n UVA_CONFIG is set to %s", getenv("UVA_CONFIG"));
    printf("\n numApertures = %d, MAX_FIELD_NUM = %d", numApertures, MAX_FIELD_NUM);
#endif
    printf("\n\t Reading %d dml shapes from %s", numApertures, dmlc_file_name);
    // check numApertures
    if (numApertures < 1 || numApertures > MAX_FIELD_NUM) {
        printf("\n ERROR: num segments (%d) outside allowed range\n", numApertures);
        printf("\n\t MAX_FIELD_NUM = %d", MAX_FIELD_NUM);
        return false;
    }
    // Read version specific information
    if (0 == strncmp(file_rev, "G", 1) ||
        0 == strncmp(file_rev, "F", 1)) {
        num_leaves = (int) get_value(fp, "Number of Leaves =");
        if (FAIL == checkStringError()) {
            printf("\n ERROR: get_value Number of Leave =");
            return false;
        }
    } else if (!strcmp(file_rev, "H")) {
        // For version H, the number of leaves is in the Model #
        // Model = Varian 120M
        char tmpStr[MAX_STR_LEN];
        if (OK != string_after(fp, "Model =", tmpStr)) {
            printf("\n ERROR: getting Model = using string_after in %s \n", tmpStr);
            return false;
        }
        if (1 != sscanf(tmpStr, "%d", &num_leaves)) {
            printf("\n ERROR: reading number of leaves from string %s \n", tmpStr);
            return false;
        }
    } else {
        printf("\n ERROR: unknown File Rev = %s\n", file_rev);
        return false;
    }
    tolerance = get_value(fp, "Tolerance =");
    if (FAIL == checkStringError()) {
        printf("\n ERROR: get_value Tolerance =");
        return false;
    }
#ifdef DEBUG
    printf("\n There are %d segments and %d leaves",numApertures,num_leaves);
    printf("\n UVA_CONFIG is set to %s", getenv("UVA_CONFIG"));
#endif
    // allocate dmlc arrays
    ALeafBank = new TWOD_ARRAY;
    BLeafBank = new TWOD_ARRAY;
    ALeafBank->x_count = BLeafBank->x_count = num_leaves / 2;
    int numFinalApertures = numApertures;
    // if(staticMLC && 1 == numApertures ) numFinalApertures=2;
    ALeafBank->y_count = BLeafBank->y_count = numFinalApertures;
    ALeafBank->matrix = (float *) calloc(ALeafBank->x_count * ALeafBank->y_count, sizeof(float));
    if (ALeafBank->matrix == NULL) {
        printf("\n ERROR: memory allocation for ALeaf leaf bank");
        return false;
    }
    BLeafBank->matrix = (float *) calloc(BLeafBank->x_count * BLeafBank->y_count, sizeof(float));
    if (BLeafBank->matrix == NULL) {
        printf("\n ERROR: memory allocation for BLeaf leaf bank");
        return false;
    }
#ifdef DEBUG
    printf("\n Allocated arrays for dmlc ");
    printf("\n UVA_CONFIG is set to %s", getenv("UVA_CONFIG"));
#endif

    //  int l; // each leaf
    int expectedCarriageGroup = -1;
    char leaf_name[MAX_STR_LEN];
    // for each field
    for (int iCP = 0; iCP < numApertures; iCP++) {
        // printf ("\n Reading Field %d", iCP);
        field[iCP] = iCP;
        /* the field written by Pinnacle in its dml files does NOT follow this convention...jvs 28Nov2001
           field[iCP] = (int) get_value(fp,"Field =");
           if ((field[iCP] != iCP) && (field[iCP] != iCP+1))
           {printf("\n ERROR: field read (%d) does not match index (%d or %d)\n",field[iCP],iCP, iCP+1);return false;}
        */
        index[iCP] = get_value(fp, "Index =");
        if (FAIL == checkStringError()) {
            printf("\n ERROR: get_value Index =");
            return false;
        }
        carriage[iCP] = (int) get_value(fp, "Carriage Group =");
        if (FAIL == checkStringError()) {
            printf("\n ERROR: get_value Carriage Group =");
            return false;
        }
        if (0 == iCP) {
            expectedCarriageGroup = carriage[iCP]; // different systems do different things
        } else {
            if (carriage[iCP] != expectedCarriageGroup) {
                printf("\n ERROR: carriage (%d) does not equal %d, changing carriage untested... \n", carriage[iCP],
                       expectedCarriageGroup);
                return false;
            }
        }
        if (isArc) {
            if (index[iCP] < 0.0 || index[iCP] > 360.0) {
                printf("\n WARNING: dmlcIndex out of range (0,360) for arc, index=%f", index[iCP]);
            }
            fractionalMUs[iCP] = -1.0f; // file has no information on this...will have to set elsewhere and later
        } else {
            // for IMRT, index = fractionalMU, and must be between 0 and 1
            fractionalMUs[iCP] = index[iCP];
            // first value must be 0.0f
            if (0 == iCP && (index[iCP] != 0.0f) && !staticMLC) {
                printf("\n ERROR: Zeroth index read (%f) outside range for an IMRT field\n", index[iCP]);
                return false;
            }
            // last value must be 1.0f
            if ((iCP == numApertures - 1) && (index[iCP] != 1.0f)) {
                printf("\n ERROR: Final index read (%f) outside range for an IMRT field \n", index[iCP]);
                return false;
            }
            // values must be monotonically increasing, positive and in range (0,1)
            if (iCP) //>0 && iCP<numApertures-1)
                if (index[iCP] < 0.0f || index[iCP] < index[iCP - 1] || index[iCP] > 1.0f) {
                    printf("\n ERROR: index read (%f) outside range (0,1) or %f !< (%f=prev))\n",
                           index[iCP], index[iCP], index[iCP - 1]);
                    return false;
                }
        }  // end else part of if(isArc)
        // poperator being skipped for now...causing error when reading in large arrays 8/3/01
        //    if (string_after(fp,"Operator =",poperator[iCP])!=OK)
        //    {printf("\n ERROR: string after in %s \n",dmlc_file_name);return false;}

        collimator[iCP] = get_value(fp, "Collimator =");
        if (FAIL == checkStringError()) {
            printf("\n ERROR: get_value Collimator =");
            return false;
        }

        // read in the leaves
        for (int l = 0; l < num_leaves / 2; l++) {
            sprintf(leaf_name, "Leaf%3dA =", l + 1);  // leaves are indexed from 1
            ARR_VAL(ALeafBank, l, iCP) = get_value(fp, leaf_name);
            if (FAIL == checkStringError()) {
                printf("\n ERROR: get_value %s", leaf_name);
                return false;
            }
        } // end of each ALeaf leaf
        for (int l = 0; l < num_leaves / 2; l++) {
            sprintf(leaf_name, "Leaf%3dB =", l + 1);
            // Note negative value is due to varian convention
            ARR_VAL(BLeafBank, l, iCP) = -1.0f * get_value(fp, leaf_name);
            if (FAIL == checkStringError()) {
                printf("\n ERROR: get_value %s", leaf_name);
                return false;
            }
        } // end of each BLeaf leaf
        // JVS: 4/10/02: Note is not used so eliminate it!!! (note only give num of chars in the comment )
        int noteLength = (int) get_value(fp, "Note =");
        // NB: MUs and Dose Rate on Same line
        /* Nov 20, 2001: JVS: MUs, Dose Rate, and Jaw setting are not used by the
                         MLC routines AND are in non-standardized formats....
                         Thus, will skip reading them for now...
        {
           char tempstring[MAX_STR_LEN];
           if (string_after(fp,"MUs =",tempstring)!=OK)
           { printf("\n ERROR: finding MUs in %s \n",dmlc_file_name);return false;}
           //       printf("\nTemp String = %s", tempstring);
           if( sscanf(tempstring,"%f",&mus[iCP]) != 1)
           { printf("\n ERROR: reading MUs from %s", tempstring); return false;}
           //dose_rate[iCP] = get_value(fp,"Dose rate =");
           char *tString; // look for dose rate on the line
           tString = strstr(tempstring,"Dose Rate =");
           if(tString == NULL) tString = strstr(tempstring,"Dose rate =");
           if(tString == NULL)
           {   printf("\n ERROR: finding Dose rate in file%s\n",dmlc_file_name); return false; }
           // Advance past the Dose Rate on the line
           // read in the dose rate
           if(sscanf( tString+strlen("Dose rate ="),"%f",&dose_rate[iCP]) != 1)
           { printf("\n ERROR: Reading dose rate from tString\n"); return false; }
        }
        */
        // CP Info Note -- specific to .dml files I write myself
        if (noteLength) {
            char iString[2 * MAX_STR_LEN];
            char *pstr = NULL;
            if (isCPInfoSet || 0 == iCP) {
                pstr = checkNextLineForString(fp, "iCP="); // check if iCP line exists
                if (0 == iCP && NULL != pstr) {
                    // cout << "-------------------- CP found in .dml file note ------------- " << endl;
                    isCPInfoSet = true;
                    nCP = numFinalApertures;
                    if (NULL == CP) CP = new controlPoint[nCP];
                }
                if (isCPInfoSet) {
                    if (NULL == pstr) {
                        printf("\n ERROR:%s: NULL pointer when looking for CP info for segment %d", __FUNCTION__, iCP);
                        return false;
                    }
                    strcpy(iString, pstr); // not sure this is needed
                    if (!CP[iCP].setControlPoint(iString)) {
                        cout << " ERROR:" << __FUNCTION__ << " translating iString to controlPoint" << endl;
                        return false;
                    }
                    fractionalMUs[iCP] = CP[iCP].getFractionalMUs();
                }
            }
        }
        // Note: Shape can be non-zero and it is not used for anything, thus, will skip reading it
        /*
        shape[iCP] = (int) get_value(fp,"Shape =");
        if (shape[iCP]!=0)
        { printf("\n ERROR: shape (%d) does not equal 0\n",shape[iCP]);return false;}
        */

        mag[iCP] = get_value(fp, "Magnification =");
        if (FAIL == checkStringError()) {
            printf("\n ERROR: get_value Magnification =");
            return false;
        }
        if (mag[iCP] != 1.0f) {
            printf("\n WARNING: mag (%f) does not equal 1.00 for segment %d\n", mag[iCP], iCP + 1);
            if (mag[iCP] == 0.0f) mag[iCP] = 1.00f; // Varis export sets Mag = 0.0
        }
        // March 18, 2002: JVS: Add warning....so can read in files exported from Varis
        if (mag[iCP] != 1.0f) {
            printf("\n ERROR: mag (%f) does not equal 1.00 for segment %d\n", mag[iCP], iCP + 1);
            return false;
        }
    } // end of segments
#ifdef DEBUG
    //  printf("\n Note %d mus %f dr %f shape %d mag %f",note[0],mus[0],dose_rate[0],shape[0],mag[0]);
#endif
    if (checkStringError() == FAIL) {
        printf("\n ERROR: string routines returned FAIL\n");
        return false;
    }
    fclose(fp);
    // finished reading dmlc file
#ifdef DEBUG
    printf("\n Normal completion of read_dmlc_file ");
#endif
    return true;
} // end of read_dmlcFile
/* ********************************************************************** */
#ifdef C_CLASS
int particleDmlc::convertDmlcCoordinatesToMlcLocation()// mlcType *machineMLC, dmlc_type *dmlc)
#else

int convertDmlcCoordinatesToMlcLocation(mlcType *machineMLC, dmlc_type *dmlc)
#endif
{
    /* Read in Varian Table of Specified Position vs. Actual Projected Position
       ActualPositionAtMLC = TableLookupofSpecifiedToActualPosition/MagFactor;
       When the ALeafPosition =BLeafPosition, then the table look up is avoided and
       ActualPositionAtMLC/=MagFactor.  This is termed the 'soft touch" routines */
    VarianMlcDataType *VarianMLC;
    VarianMLC = new(VarianMlcDataType);
    if (readVarianMLCTable(VarianMLC) != OK) {
        printf("\n ERROR: Reading VarianMLC Config\n");
        return (FAIL);
    }
    printf("\n\t Converting Leaf Positions from Isocenter to Physical Locations using mlctable.txt");
    printf("\n\t\t Physical Leaf Offset of %g applied", machineMLC->physicalLeafOffset);
    float sumPhysicalGap = 0.0f;
    int nGapSamples = 0;
    for (int iCP = 0; iCP < dmlc->numApertures; iCP++) {
        for (int l = 0; l < dmlc->num_leaves / 2; l++) {
            // Ensure Leaves are specified properly (initially open )
            float lightFieldGap = ARR_VAL(dmlc->ALeafBank, l, iCP) - ARR_VAL(dmlc->BLeafBank, l, iCP);
#ifdef DEBUG
            if(l==22){ printf("\nGap 1 = %f",  ARR_VAL(dmlc->ALeafBank,l,iCP)-ARR_VAL(dmlc->BLeafBank,l,iCP));
                      printf("\ta=%f=tb=%f", ARR_VAL(dmlc->ALeafBank,l,iCP), ARR_VAL(dmlc->BLeafBank,l,iCP));
             }
#endif
            if (lightFieldGap < 0.0f) {
                printf("\n ERROR- (before) leaves are crossed over at leaf %d for shape %d!", l + 1, iCP + 1);
                printf("\n      - ALeaf %f, BLeaf %f, LightFieldGap (%f)", ARR_VAL(dmlc->ALeafBank, l, iCP),
                       ARR_VAL(dmlc->BLeafBank, l, iCP), lightFieldGap);
                return (FAIL);
            }
            float aNew, bNew;
            if (lightFieldGap > 0.0f) /* Table look up only if leaves are not touching... */
            {
                aNew = (-1.0f * table_value(-1.0f * ARR_VAL(dmlc->ALeafBank, l, iCP), &VarianMLC->data));
                /* mlcTable.txt (like the dml file) is in coordinates such that negative
                   values indicate a value beyond the centerline.  When the dml file was read in,
                   these coordinates were translated to beamline coordinates. (that is, the
                   B-leaf was multipled by -1.  Thus, when looking up in the mlcTable.txt file,
                   we must multiply by -1 to get the correct lookup of the leaf tip position,
                   then multiply the result by -1 to transform it back into the beamline
                   coordinate system ...My coordinates seem reversed, but it is consistent
                   the beamline coordinates....Oct 19, 2001: JVS */
                bNew = table_value(ARR_VAL(dmlc->BLeafBank, l, iCP), &VarianMLC->data);
                // if(l==22) printf("\n Looked Up New leaf positions from the table");
            } else  /* This is like the Varian "Soft-touch" routine... if aLeaf == -bLeaf, then don't use
                  the table look up..........closed is closed */
            {
                aNew = ARR_VAL(dmlc->ALeafBank, l, iCP);
                bNew = ARR_VAL(dmlc->BLeafBank, l, iCP);
                //            if(l==22) printf("\n Leaves Are Touching");
            }
            /* Back project all leaves to the MLC location */
            //      if(l==22) { printf("\nGap 2 = %f",  aNew - bNew); 			   printf("\ta=%f=tb=%f", aNew, bNew); }
            ARR_VAL(dmlc->ALeafBank, l, iCP) = aNew / VarianMLC->MagFactor;
            ARR_VAL(dmlc->BLeafBank, l, iCP) = bNew / VarianMLC->MagFactor;

            /* Check the physical distance between the leaves... */
            float physicalLeafGap = ARR_VAL(dmlc->ALeafBank, l, iCP) - ARR_VAL(dmlc->BLeafBank, l, iCP);
            /* April 11, 2002: JVS: Add in Offset due to difference between "Actual" positions in dynalog and real position" */
            if (physicalLeafGap > 0) {
                // float physicalLeafOffset=0.008;
                ARR_VAL(dmlc->ALeafBank, l, iCP) += machineMLC->physicalLeafOffset;
                ARR_VAL(dmlc->BLeafBank, l, iCP) -= machineMLC->physicalLeafOffset;
                physicalLeafGap = ARR_VAL(dmlc->ALeafBank, l, iCP) - ARR_VAL(dmlc->BLeafBank, l, iCP);
            }
            //      if(l==22) { printf("\nGap 3 = %f",  ARR_VAL(dmlc->ALeafBank,l,iCP)-ARR_VAL(dmlc->BLeafBank,l,iCP)); 			   printf("\ta=%f=tb=%f", ARR_VAL(dmlc->ALeafBank,l,iCP), ARR_VAL(dmlc->BLeafBank,l,iCP));}
            sumPhysicalGap += physicalLeafGap;
            nGapSamples++;
            if (physicalLeafGap < 0.0f) {
                printf("\n ERROR- (After) leaves are crossed over at leaf %d for shape %d!", l + 1, iCP + 1);
                printf("\n      - ALeaf %f, BLeaf %f, diff (%f)", ARR_VAL(dmlc->ALeafBank, l, iCP),
                       ARR_VAL(dmlc->BLeafBank, l, iCP), physicalLeafGap);
                return (FAIL);
            }
        }
    }
    if (nGapSamples)
        printf("\n\t Mean Leaf Gap = %f", (float) sumPhysicalGap / (float) nGapSamples);
    delete VarianMLC;
    return (OK);
}
/* ********************************************************************************************* */
#ifdef C_CLASS
int particleDmlc::projectDMLCtoMLC() // 11/08 mlcType *machineMLC, dmlc_type *dmlc)
#else

int projectDMLCtoMLC(mlcType *machineMLC, dmlc_type *dmlc)
#endif
{
    /* N.B. Reversed from PJK r_leaves, l_leaves code, A is leading, B is following... */
    // NOTE: This routine DOES NOT use mlcTable.txt for lookup.
    printf("\n----- %s is deprecated ---", __FUNCTION__);
    if (1) return (FAIL);
    float Magnification =
            machineMLC->distance / machineMLC->specificationDistance; // have co-ordinates at MLC mid-plane
    printf("\n\t Converting leaf positions from Isocenter to Physical Location using offset");
    printf("\n\t\t Magnification Factor of %f", Magnification);
    printf("\n\t\t Offset of %f", machineMLC->closedOffset / 2.0);
    float sumPhysicalGap = 0.0f;
    int nGapSamples = 0;
    for (int iCP = 0; iCP < dmlc->numApertures; iCP++) {
        // Account for leaves being Varian closed, ie not!
        for (int l = 0; l < dmlc->num_leaves / 2; l++) {
            // changed as leaves are more open than we think all the time
            // float diff = ARR_VAL(dmlc->BLeafBank,l,(dmlc->numApertures-1)-s)-ARR_VAL(dmlc->ALeafBank,l,(dmlc->numApertures-1)-s);
            float diff = ARR_VAL(dmlc->ALeafBank, l, iCP) - ARR_VAL(dmlc->BLeafBank, l, iCP);
            //      if(l==22){ printf("\nGap 1 = %f",  ARR_VAL(dmlc->ALeafBank,l,iCP)-ARR_VAL(dmlc->BLeafBank,l,iCP)); 			   printf("\ta=%f=tb=%f", ARR_VAL(dmlc->ALeafBank,l,iCP), ARR_VAL(dmlc->BLeafBank,l,iCP));	  }
            if (diff < 0.0f) {
                printf("\n ERROR- (before) leaves are crossed over at leaf %d for shape %d!", l + 1, iCP + 1);
                printf("\n      - ALeaf %f, BLeaf %f, diff (%f)", ARR_VAL(dmlc->ALeafBank, l, iCP),
                       ARR_VAL(dmlc->BLeafBank, l, iCP), diff);
                return (FAIL);
            }
            if (dmlc->num_leaves != 120 || diff > 0.0f) // For 120 leaf MLC, offset does not apply to closed leaves
            {
                ARR_VAL(dmlc->ALeafBank, l, iCP) += machineMLC->closedOffset / 2.0f;
                ARR_VAL(dmlc->BLeafBank, l, iCP) -= machineMLC->closedOffset / 2.0f;
            }
            //      if(l==22) { printf("\nGap 2 = %f",  ARR_VAL(dmlc->ALeafBank,l,iCP)-ARR_VAL(dmlc->BLeafBank,l,iCP)); 			   printf("\ta=%f=tb=%f", ARR_VAL(dmlc->ALeafBank,l,iCP), ARR_VAL(dmlc->BLeafBank,l,iCP));	  }
            // now project size back to mlc plane
            ARR_VAL(dmlc->ALeafBank, l, iCP) *= Magnification;
            ARR_VAL(dmlc->BLeafBank, l, iCP) *= Magnification;
            //      if(l==22) { printf("\nGap 3 = %f",  ARR_VAL(dmlc->ALeafBank,l,iCP)-ARR_VAL(dmlc->BLeafBank,l,iCP)); 			   printf("\ta=%f=tb=%f", ARR_VAL(dmlc->ALeafBank,l,iCP), ARR_VAL(dmlc->BLeafBank,l,iCP));	  }
            // can never check too much BLeaf Jeff!
            float diff2 = ARR_VAL(dmlc->ALeafBank, l, iCP) - ARR_VAL(dmlc->BLeafBank, l, iCP);
            sumPhysicalGap += diff2;
            nGapSamples++;
            if (diff2 < 0.0f) {
                printf("\n ERROR- (after) leaves are crossed over at leaf %d for shape %d!", l + 1, iCP + 1);
                printf("\n      - ALeaf %f, BLeaf %f, diff (%f)", ARR_VAL(dmlc->ALeafBank, l, iCP),
                       ARR_VAL(dmlc->BLeafBank, l, iCP), diff2);
                return (FAIL);
            }
        }
    }
    if (nGapSamples)
        printf("\n Mean Leaf Gap = %f", sumPhysicalGap / (float) nGapSamples);
    return (OK);
}
/* *********************************************************************** */
// #define DEBUG
#ifdef C_CLASS
int particleDmlc::readDmlcFile() // 11/08 dmlc_type* dmlc)
#else

int readDmlcFile(dmlc_type *dmlc)
#endif
{
    isCPInfoSet = false;
    // open file
    strcpy(dmlc->dmlc_file_name, dmlFileName.c_str());
#ifdef DEBUG
    // printf("\n UVA_CONFIG is set to %s", getenv("UVA_CONFIG"));
    printf("\n Opening file %s", dmlc->dmlc_file_name);
    printf("\n StringLength = %d, MAX_STR_LEN = %d", strlen(dmlc->dmlc_file_name), MAX_STR_LEN);
#endif

    FILE *fp;
    if ((fp = fopen(dmlc->dmlc_file_name, "r")) == NULL) {
        printf("\n ERROR: can't open file %s! \n", dmlc->dmlc_file_name);
        return (FAIL);
    }
#ifdef DEBUG
    printf("\n After Open file: UVA_CONFIG is set to %s", getenv("UVA_CONFIG"));
#endif
    //
    //read header
    if (string_after(fp, "File Rev =", dmlc->file_rev) != OK) {
        printf("\n ERROR: string after in %s \n", dmlc->dmlc_file_name);
        return (FAIL);
    }
    if (string_after(fp, "Treatment =", dmlc->treatment) != OK) {
        printf("\n ERROR: string after in %s \n", dmlc->dmlc_file_name);
        return (FAIL);
    }
    if (string_after(fp, "Last Name =", dmlc->last_name) != OK) {
        printf("\n ERROR: string after in %s \n", dmlc->dmlc_file_name);
        return (FAIL);
    }
    if (string_after(fp, "First Name =", dmlc->first_name) != OK) {
        printf("\n ERROR: string after in %s \n", dmlc->dmlc_file_name);
        return (FAIL);
    }
    if (string_after(fp, "Patient ID =", dmlc->patient_id) != OK) {
        printf("\n ERROR: string after in %s \n", dmlc->dmlc_file_name);
        return (FAIL);
    }
    dmlc->numApertures = (int) get_value(fp, "Number of Fields =");
    if (FAIL == checkStringError()) {
        printf("\n ERROR: get_value Number of Fields =");
        return (FAIL);
    }

    if (1 == dmlc->numApertures) dmlc->staticMLC = true;
    else dmlc->staticMLC = false;
    if (!strncmp(dmlc->treatment, "Dynamic Arc", 11)) {
        isArc = true;
        // dmlc->staticMLC=false;
    } else {
        if (isArc) {
            printf("\n ERROR: %s: .dml reports Treatment=%s, and isArc was previously set", __FUNCTION__,
                   dmlc->treatment);
            return (FAIL);
        }
    }
    printf("\n\t Treatment: %s", dmlc->treatment);
    //printf("\n\t isArc = %s", isArc ? "true" : "false");
#ifdef DEBUG
    printf("\n\t isArc = %s", isArc ? "true" : "false");
    printf("\n\t strncmp(dmlc->treatment,\"Dynamic Arc\",11) = %d", strncmp(dmlc->treatment,"Dynamic Arc",11));
    printf("\n\t First name: %s",dmlc->first_name);
    printf("\n\t Last name %s",dmlc->last_name);
    printf("\n\t ID %s",dmlc->patient_id);
    printf("\n UVA_CONFIG is set to %s", getenv("UVA_CONFIG"));
    printf("\n numApertures = %d, MAX_FIELD_NUM = %d", dmlc->numApertures, MAX_FIELD_NUM);
#endif
    printf("\n\t Reading %d dml shapes from %s", dmlc->numApertures, dmlc->dmlc_file_name);
    // check numApertures
    if (dmlc->numApertures < 1 || dmlc->numApertures > MAX_FIELD_NUM) {
        printf("\n ERROR: num segments (%d) outside allowed range\n", dmlc->numApertures);
        printf("\n\t MAX_FIELD_NUM = %d", MAX_FIELD_NUM);
        return (FAIL);
    }
    // Read version specific information
    if (0 == strncmp(dmlc->file_rev, "G", 1) ||
        0 == strncmp(dmlc->file_rev, "F", 1)) {
        dmlc->num_leaves = (int) get_value(fp, "Number of Leaves =");
        if (FAIL == checkStringError()) {
            printf("\n ERROR: get_value Number of Leave =");
            return (FAIL);
        }
    } else if (!strcmp(dmlc->file_rev, "H")) {
        // For version H, the number of leaves is in the Model #
        // Model = Varian 120M
        char tmpStr[MAX_STR_LEN];
        if (OK != string_after(fp, "Model =", tmpStr)) {
            printf("\n ERROR: getting Model = using string_after in %s \n", tmpStr);
            return (FAIL);
        }
        if (1 != sscanf(tmpStr, "%d", &dmlc->num_leaves)) {
            printf("\n ERROR: reading number of leaves from string %s \n", tmpStr);
            return (FAIL);
        }
    } else {
        printf("\n ERROR: unknown File Rev = %s\n", dmlc->file_rev);
        return (FAIL);
    }
    dmlc->tolerance = get_value(fp, "Tolerance =");
    if (FAIL == checkStringError()) {
        printf("\n ERROR: get_value Tolerance =");
        return (FAIL);
    }
#ifdef DEBUG
    printf("\n There are %d segments and %d leaves",dmlc->numApertures,dmlc->num_leaves);
    printf("\n UVA_CONFIG is set to %s", getenv("UVA_CONFIG"));
#endif
    // allocate dmlc arrays
    dmlc->ALeafBank = new TWOD_ARRAY;
    dmlc->BLeafBank = new TWOD_ARRAY;
    dmlc->ALeafBank->x_count = dmlc->BLeafBank->x_count = dmlc->num_leaves / 2;
    int numFinalApertures = dmlc->numApertures;
    // if(dmlc->staticMLC && 1 == dmlc->numApertures ) numFinalApertures=2;
    dmlc->ALeafBank->y_count = dmlc->BLeafBank->y_count = numFinalApertures;
    dmlc->ALeafBank->matrix = (float *) calloc(dmlc->ALeafBank->x_count * dmlc->ALeafBank->y_count, sizeof(float));
    if (dmlc->ALeafBank->matrix == NULL) {
        printf("\n ERROR: memory allocation for ALeaf leaf bank");
        return (FAIL);
    }
    dmlc->BLeafBank->matrix = (float *) calloc(dmlc->BLeafBank->x_count * dmlc->BLeafBank->y_count, sizeof(float));
    if (dmlc->BLeafBank->matrix == NULL) {
        printf("\n ERROR: memory allocation for BLeaf leaf bank");
        return (FAIL);
    }
#ifdef DEBUG
    printf("\n Allocated arrays for dmlc ");
    printf("\n UVA_CONFIG is set to %s", getenv("UVA_CONFIG"));
#endif

    //  int l; // each leaf
    int expectedCarriageGroup = -1;
    char leaf_name[MAX_STR_LEN];
    // for each field
    for (int iCP = 0; iCP < dmlc->numApertures; iCP++) {
        // printf ("\n Reading Field %d", iCP);
        dmlc->field[iCP] = iCP;
        /* the field written by Pinnacle in its dml files does NOT follow this convention...jvs 28Nov2001
           dmlc->field[iCP] = (int) get_value(fp,"Field =");
           if ((dmlc->field[iCP] != iCP) && (dmlc->field[iCP] != iCP+1))
           {printf("\n ERROR: field read (%d) does not match index (%d or %d)\n",dmlc->field[iCP],iCP, iCP+1);return(FAIL);}
        */
        dmlc->index[iCP] = get_value(fp, "Index =");
        if (FAIL == checkStringError()) {
            printf("\n ERROR: get_value Index =");
            return (FAIL);
        }
        dmlc->carriage[iCP] = (int) get_value(fp, "Carriage Group =");
        if (FAIL == checkStringError()) {
            printf("\n ERROR: get_value Carriage Group =");
            return (FAIL);
        }
        if (0 == iCP) {
            expectedCarriageGroup = dmlc->carriage[iCP]; // different systems do different things
        } else {
            if (dmlc->carriage[iCP] != expectedCarriageGroup) {
                printf("\n ERROR: carriage (%d) does not equal %d, changing carriage untested... \n",
                       dmlc->carriage[iCP], expectedCarriageGroup);
                return (FAIL);
            }
        }
        if (isArc) {
            if (dmlc->index[iCP] < 0.0 || dmlc->index[iCP] > 360.0) {
                printf("\n WARNING: dmlcIndex out of range (0,360) for arc, index=%f", dmlc->index[iCP]);
            }
            dmlc->fractionalMUs[iCP] = -1.0f; // file has no information on this...will have to set elsewhere and later
        } else {
            // for IMRT, index = fractionalMU, and must be between 0 and 1
            dmlc->fractionalMUs[iCP] = dmlc->index[iCP];
            // first value must be 0.0f
            if (0 == iCP && (dmlc->index[iCP] != 0.0f) && !dmlc->staticMLC) {
                printf("\n ERROR: Zeroth index read (%f) outside range for an IMRT field\n", dmlc->index[iCP]);
                return (FAIL);
            }
            // last value must be 1.0f
            if ((iCP == dmlc->numApertures - 1) && (dmlc->index[iCP] != 1.0f)) {
                printf("\n ERROR: Final index read (%f) outside range for an IMRT field \n", dmlc->index[iCP]);
                return (FAIL);
            }
            // values must be monotonically increasing, positive and in range (0,1)
            if (iCP) //>0 && iCP<dmlc->numApertures-1)
                if (dmlc->index[iCP] < 0.0f || dmlc->index[iCP] < dmlc->index[iCP - 1] || dmlc->index[iCP] > 1.0f) {
                    printf("\n ERROR: index read (%f) outside range (0,1) or %f !< (%f=prev))\n",
                           dmlc->index[iCP], dmlc->index[iCP], dmlc->index[iCP - 1]);
                    return (FAIL);
                }
        }  // end else part of if(isArc)
        // poperator being skipped for now...causing error when reading in large arrays 8/3/01
        //    if (string_after(fp,"Operator =",dmlc->poperator[iCP])!=OK)
        //    {printf("\n ERROR: string after in %s \n",dmlc->dmlc_file_name);return(FAIL);}

        dmlc->collimator[iCP] = get_value(fp, "Collimator =");
        if (FAIL == checkStringError()) {
            printf("\n ERROR: get_value Collimator =");
            return (FAIL);
        }

        // read in the leaves
        for (int l = 0; l < dmlc->num_leaves / 2; l++) {
            sprintf(leaf_name, "Leaf%3dA =", l + 1);  // leaves are indexed from 1
            ARR_VAL(dmlc->ALeafBank, l, iCP) = get_value(fp, leaf_name);
            if (FAIL == checkStringError()) {
                printf("\n ERROR: get_value %s", leaf_name);
                return (FAIL);
            }
        } // end of each ALeaf leaf
        for (int l = 0; l < dmlc->num_leaves / 2; l++) {
            sprintf(leaf_name, "Leaf%3dB =", l + 1);
            // Note negative value is due to varian convention
            ARR_VAL(dmlc->BLeafBank, l, iCP) = -1.0f * get_value(fp, leaf_name);
            if (FAIL == checkStringError()) {
                printf("\n ERROR: get_value %s", leaf_name);
                return (FAIL);
            }
        } // end of each BLeaf leaf
        // JVS: 4/10/02: Note is not used so eliminate it!!! (note only give num of chars in the comment )
        int noteLength = (int) get_value(fp, "Note =");
        // NB: MUs and Dose Rate on Same line
        /* Nov 20, 2001: JVS: MUs, Dose Rate, and Jaw setting are not used by the
                         MLC routines AND are in non-standardized formats....
                         Thus, will skip reading them for now...
        {
           char tempstring[MAX_STR_LEN];
           if (string_after(fp,"MUs =",tempstring)!=OK)
           { printf("\n ERROR: finding MUs in %s \n",dmlc->dmlc_file_name);return(FAIL);}
           //       printf("\nTemp String = %s", tempstring);
           if( sscanf(tempstring,"%f",&dmlc->mus[iCP]) != 1)
           { printf("\n ERROR: reading MUs from %s", tempstring); return(FAIL);}
           //dmlc->dose_rate[iCP] = get_value(fp,"Dose rate =");
           char *tString; // look for dose rate on the line
           tString = strstr(tempstring,"Dose Rate =");
           if(tString == NULL) tString = strstr(tempstring,"Dose rate =");
           if(tString == NULL)
           {   printf("\n ERROR: finding Dose rate in file%s\n",dmlc->dmlc_file_name); return(FAIL); }
           // Advance past the Dose Rate on the line
           // read in the dose rate
           if(sscanf( tString+strlen("Dose rate ="),"%f",&dmlc->dose_rate[iCP]) != 1)
           { printf("\n ERROR: Reading dose rate from tString\n"); return(FAIL); }
        }
        */
#define CPInfoNote
#ifdef CPInfoNote
        if (noteLength) {
            char iString[2 * MAX_STR_LEN];
            char *pstr = NULL;
            if (isCPInfoSet || 0 == iCP) {
                pstr = checkNextLineForString(fp, "iCP="); // check if iCP line exists
                if (0 == iCP && NULL != pstr) {
                    // cout << "-------------------- CP found in .dml file note ------------- " << endl;
                    isCPInfoSet = true;
                    nCP = numFinalApertures;
                    if (NULL == CP) CP = new controlPoint[nCP];
                }
                if (isCPInfoSet) {
                    if (NULL == pstr) {
                        printf("\n ERROR:%s: NULL pointer when looking for CP info for segment %d", __FUNCTION__, iCP);
                        return (FAIL);
                    }
                    strcpy(iString, pstr); // not sure this is needed
                    if (!CP[iCP].setControlPoint(iString)) {
                        cout << " ERROR:" << __FUNCTION__ << " translating iString to controlPoint" << endl;
                        return (FAIL);
                    }
                    dmlc->fractionalMUs[iCP] = CP[iCP].getFractionalMUs();
                }
            }
        }
#endif
        // Note: Shape can be non-zero and it is not used for anything, thus, will skip reading it
        /*
        dmlc->shape[iCP] = (int) get_value(fp,"Shape =");
        if (dmlc->shape[iCP]!=0)
        { printf("\n ERROR: shape (%d) does not equal 0\n",dmlc->shape[iCP]);return(FAIL);}
        */

        dmlc->mag[iCP] = get_value(fp, "Magnification =");
        if (FAIL == checkStringError()) {
            printf("\n ERROR: get_value Magnification =");
            return (FAIL);
        }
        if (dmlc->mag[iCP] != 1.0f) {
            printf("\n WARNING: mag (%f) does not equal 1.00 for segment %d\n", dmlc->mag[iCP], iCP + 1);
            if (dmlc->mag[iCP] == 0.0f) dmlc->mag[iCP] = 1.00f; // Varis export sets Mag = 0.0
        }
        // March 18, 2002: JVS: Add warning....so can read in files exported from Varis
        if (dmlc->mag[iCP] != 1.0f) {
            printf("\n ERROR: mag (%f) does not equal 1.00 for segment %d\n", dmlc->mag[iCP], iCP + 1);
            return (FAIL);
        }
    } // end of segments
#ifdef DEBUG
    //  printf("\n Note %d mus %f dr %f shape %d mag %f",dmlc->note[0],dmlc->mus[0],dmlc->dose_rate[0],dmlc->shape[0],dmlc->mag[0]);
#endif
    if (checkStringError() == FAIL) {
        printf("\n ERROR: string routines returned FAIL\n");
        return (FAIL);
    }
    fclose(fp);
    // finished reading dmlc file
#ifdef DEBUG
    printf("\n Normal completion of read_dmlc_file ");
#endif
    return (OK);
} // end of read_dmlc_file
/* *********************************************************************** */
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::read_mu_table(char *fname, table_type* pE_mu, int read_col, float Density)
#else

int read_mu_table(char *fname, table_type *pE_mu, int read_col, float Density)
#endif
{  // reads attenuation coefficients (mu values)
    // Input Format is from ???
    // NB. read_mu returns actual mu as multiples table by Density (mu = attenuation coefficient)
    /* JVS: Feb 4, 2003: Modified so no longer reads in Hack of "grepped" table
       require user to pass table that has the L,K,...lines removed from it */

    // see if data file exists
    //FILE *fpg;
    //if( (fpg = fopen(fname,"r") ) == NULL )
    char tmpFileName[MAX_STR_LEN]; /// use tmpFileName since openDMLCConfig over-writes the name
    strcpy(tmpFileName, fname);
    FILE *fpg;
    if ((fpg = fopen(fname, "r")) == NULL) {
        fpg = openDMLCConfigFile(tmpFileName);
        if (NULL == fpg) {
            printf("\n ERROR: read_mu_table: opening file %s \n", fname);
            return (FAIL);
        }
    }

    // read all mu values
    int tmp;
    float data[MAX_DATA][N_PER_LINE];
    int iread = 0;
    do {
        tmp = array_read(fpg, data[iread], N_PER_LINE);
        if (tmp == 8) {
            //printf("\n %f %f %f", data[iread][0],data[iread][1],data[iread][2]);
            iread++;
        }
    } while (tmp != FAIL && iread < MAX_DATA);

    fclose(fpg); // Close the output file......

    if (iread == MAX_DATA) {
        fprintf(stderr, "\n ERROR: too many entries in data table");
        return (FAIL);
    }

    pE_mu->n_elements = iread;
    for (int i = 0; i < iread; i++) {
        pE_mu->x[i] = data[i][0];
        pE_mu->y[i] = Density * data[i][read_col];
    }
#ifdef DEBUG
    printf("\n Normal completion of read_mu_table");
#endif
    return (OK);
} // end of read_mu_table
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::russian_roulette(particle_type* part)
#else

int russian_roulette(particle_type *part, russianRoulette *rr)
#endif
{
#ifdef C_CLASS
    // rr is a member of the class
    rr->totalParticlesToWrite++; // keep track of total # times routine called
    if( part->weight <= rr->rrWeight*rr->rrMaxPartWeight ) // check if need to run RR
    {
#endif
    rr->numParticlesToPlayRR++; // increment the number to that are playing the game
    //See MCNP manual p 2-116.
    float cutVal = rr->rrWeight * rr->rrMaxPartWeight;
    if (randomOne() <= part->weight / cutVal) { // russian roulette based on weight
        rr->numParticlesThatLive++;
        part->weight = cutVal;
    } else
        part->weight = 0.0f;

    // error checking
    if (part->weight > 1.0f) {
        printf("\n ERROR: Weight >1 in RR");
        error_part(part);
        return (FAIL);
    } else if (part->weight < 0.0f) {
        printf("\n ERROR: Weight <0 in RR");
        error_part(part);
        return (FAIL);
    }
#ifdef C_CLASS
    }
#endif
    return (OK);
}
/* *********************************************************************** */
#ifdef C_CLASS
// #error "write_part is not defined for the C++ Class"
#else

int write_part(ps_header_type *h, particle_type *p, FILE *ostrm, russianRoulette *rr) {
    if (p->weight > 1.0f) {
        printf("\n ERROR: Weight >1 (%f) charge %d, energy %f", p->weight, p->charge, p->energy);
        return (FAIL);
    } else if (p->weight < 0.0f) {
        printf("\n ERROR: Weight <0 (%f) charge %d", p->weight, p->charge);
        return (FAIL);
    } else if (p->weight == 0.0f) {
        return (NO_COUNT);
    } else // p->weight (0,1]
    {
        // Feb 7, 2007: JVS: Logic change here, but should be same result
        rr->totalParticlesToWrite++;
        if (p->weight <= rr->rrWeight * rr->rrMaxPartWeight) // check if need to run RR
        {
#ifdef C_CLASS
            if (russian_roulette(p)!=OK)
#else
            if (russian_roulette(p, rr) != OK)
#endif
            {
                printf("\n ERROR: Playing RR");
                return (FAIL);
            }
        }
        if (p->weight > 0.0f) { // write only surviving particles
            if (write_a_particle(h, p, ostrm) != OK) {
                printf("\n ERROR: Writing particle");
                return (FAIL);
            }
        }
    }
    return (OK);
}

#endif
/* *********************************************************************** */
#ifdef C_CLASS
void particleDmlc::error_part(particle_type *p, particle_type *s)
#else

void error_part(particle_type *p, particle_type *s)
#endif
{
    printf("\n Part 1.");
    error_part(p);
    printf("\n Part 2.");
    error_part(s);
}
/* *********************************************************************** */
#ifdef C_CLASS
void particleDmlc::error_part(particle_type *p)
#else

void error_part(particle_type *p)
#endif
{
    printf("\n Output suspect particle");
    printf("\n\tX %g\tY %g\t U %g\t V %g\t (W %g)", p->x, p->y, p->u, p->v,
           sqrt(1.0f - (p->u * p->u + p->v * p->v)));
    printf("\n\tEn %g\tWt %g\tZl %g\t Q %d", p->energy, p->weight, p->zlast, p->charge);
    printf("\n");
}
/* ******************************************************************* */
#ifdef C_CLASS
int particleDmlc::convertDmlcCoordinatesToIsocenterLocation() // 11/08 mlcType *machineMLC, dmlc_type *dmlc)
#else

int convertDmlcCoordinatesToIsocenterLocation(mlcType *machineMLC, dmlc_type *dmlc)
#endif
{
    /* this provides the inverse function of convertDmlcCoordinatesToDMLCLocation...
       created so we can create DMLC files from dynalog files */

    /* Read in Varian Table of Specified Position vs. Actual Projected Position
       ActualPositionAtMLC = TableLookupofSpecifiedToActualPosition/MagFactor;
    */
    VarianMlcDataType *VarianMLC;
    VarianMLC = new(VarianMlcDataType);
    if (readVarianMLCTable(VarianMLC) != OK) {
        printf("\n ERROR: Reading VarianMLC Config\n");
        return (FAIL);
    }
    /* ******************************* */
    printf("\n\t\t Converting Leaf Positions from Physical Locations to Isocenter using mlctable.txt");
    float LeafTolerance = 0.000f;
    for (int iCP = 0; iCP < dmlc->numApertures; iCP++) {
        for (int l = 0; l < dmlc->num_leaves / 2; l++) {
            // Ensure Leaves are specified properly (initially open )
            // Physical distance between the leaves
            float physicalLeafGap = ARR_VAL(dmlc->ALeafBank, l, iCP) - ARR_VAL(dmlc->BLeafBank, l, iCP);

            if (physicalLeafGap < 0.0f) {
                if (fabs(physicalLeafGap) < LeafTolerance) {

                    printf("\n WARNING- (before) Actual leaves are crossed over at leaf %d for shape %d!", l + 1,
                           iCP + 1);
                    printf("\n      - ALeaf %f, BLeaf %f, Physical Gap (%f)", ARR_VAL(dmlc->ALeafBank, l, iCP),
                           ARR_VAL(dmlc->BLeafBank, l, iCP), physicalLeafGap);
                    printf("\n Reassigning each leaf to the mean position");
                    float meanPosition = 0.5f * (ARR_VAL(dmlc->ALeafBank, l, iCP) + ARR_VAL(dmlc->BLeafBank, l, iCP));
                    ARR_VAL(dmlc->ALeafBank, l, iCP) = ARR_VAL(dmlc->BLeafBank, l, iCP) = meanPosition;
                } else {
                    printf("\n ERROR- (before) leaves are crossed over at leaf %d for shape %d!", l + 1, iCP + 1);
                    printf("\n      - ALeaf %f, BLeaf %f,  Actual Physical Gap (%f)", ARR_VAL(dmlc->ALeafBank, l, iCP),
                           ARR_VAL(dmlc->BLeafBank, l, iCP), physicalLeafGap);
                    printf("\n      - Segment %d, Dose Fraction = %f", iCP, dmlc->index[iCP]);
                    return (FAIL);
                }
            }
            if (physicalLeafGap > 0) {
                ARR_VAL(dmlc->ALeafBank, l, iCP) -= machineMLC->physicalLeafOffset;
                ARR_VAL(dmlc->BLeafBank, l, iCP) += machineMLC->physicalLeafOffset;
                physicalLeafGap = ARR_VAL(dmlc->ALeafBank, l, iCP) - ARR_VAL(dmlc->BLeafBank, l, iCP);
            }
            //
            // Magnify leaves to the isocenter location
            /* Project all leaves to the Isocenter location */
            //if(l==0 && iCP==20) printf ("\n ALeaf = %f \t BLeaf = %f", ARR_VAL(dmlc->ALeafBank,l,iCP), ARR_VAL(dmlc->BLeafBank,l,iCP));
            ARR_VAL(dmlc->ALeafBank, l, iCP) *= VarianMLC->MagFactor;
            ARR_VAL(dmlc->BLeafBank, l, iCP) *= VarianMLC->MagFactor;
            //if(l==0 && iCP==20)printf ("\n After Mag of %f ALeaf = %f \t BLeaf = %f", VarianMLC->MagFactor, ARR_VAL(dmlc->ALeafBank,l,iCP), ARR_VAL(dmlc->BLeafBank,l,iCP));
            float aNew, bNew;
            if (physicalLeafGap > 0.0f) /* Table look up only if leaves are not touching...In Desired Positions */
            {
                aNew = -1.0f * tableValueGivenYFindX(-1.0f * ARR_VAL(dmlc->ALeafBank, l, iCP), &VarianMLC->data);
                /* mlcTable.txt (like the dml file) is in coordinates such that negative
                   values indicate a value beyond the centerline.  When the dml file was read in,
                   these coordinates were translated to beamline coordinates. (that is, the
                   B-leaf was multipled by -1.  Thus, when looking up in the mlcTable.txt file,
                   we must multiply by -1 to get the correct lookup of the leaf tip position,
                   then multiply the result by -1 to transform it back into the beamline
                   coordinate system ...My coordinates seem reversed, but it is consistent
                   the beamline coordinates....Oct 19, 2001: JVS */
                bNew = tableValueGivenYFindX(ARR_VAL(dmlc->BLeafBank, l, iCP), &VarianMLC->data);
            } else  /* This is like the Varian "Soft-touch" routine... if aLeaf == -bLeaf, then don't use
                  the table look up..........closed is closed */
            {
                aNew = ARR_VAL(dmlc->ALeafBank, l, iCP);
                bNew = ARR_VAL(dmlc->BLeafBank, l, iCP);
                //
            }
            ARR_VAL(dmlc->ALeafBank, l, iCP) = aNew;
            ARR_VAL(dmlc->BLeafBank, l, iCP) = bNew;


            /* Check the light Field distance between the leaves... */
            float lightFieldLeafGap = ARR_VAL(dmlc->ALeafBank, l, iCP) - ARR_VAL(dmlc->BLeafBank, l, iCP);
            /* temp patch for 1mmGap field: jvs */
            if (lightFieldLeafGap < 0.0f && (dmlc->index[iCP] < 0.01 || dmlc->index[iCP] > 0.99)) {

                printf("\n WARNING: leaves overlap after for leaf %d shape %d", l + 1, iCP + 1);
                float meanPosition = 0.5f * (ARR_VAL(dmlc->ALeafBank, l, iCP) + ARR_VAL(dmlc->BLeafBank, l, iCP));
                ARR_VAL(dmlc->ALeafBank, l, iCP) = ARR_VAL(dmlc->BLeafBank, l, iCP) = meanPosition;
                lightFieldLeafGap = ARR_VAL(dmlc->ALeafBank, l, iCP) - ARR_VAL(dmlc->BLeafBank, l, iCP);
            }
            /* end of temp patch for 1mmGap fields */
            if (lightFieldLeafGap < 0.0f) {
                printf("\n ERROR- (After) leaves are crossed over at leaf %d for shape %d!", l + 1, iCP + 1);
                printf("\n      - ALeaf %f, BLeaf %f, diff (%f)", ARR_VAL(dmlc->ALeafBank, l, iCP),
                       ARR_VAL(dmlc->BLeafBank, l, iCP), lightFieldLeafGap);
                printf("\n      - Segment %d, Dose Fraction = %f", iCP, dmlc->index[iCP]);
                return (FAIL);
            }
        }
    }
    delete VarianMLC;
    return (OK);
}
/* ************************************************************************************** */
