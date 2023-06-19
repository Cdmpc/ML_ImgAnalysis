#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BUFLEN 8192
#define SCALE 50
#define MAX_STR 80

unsigned char *buffer; /* read buffer for data transfers */
unsigned long *firstword;
unsigned int *tempoint;
unsigned long *tempoint1;

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        perror("Too many arguements on the cmd line!\n");
        exit(1);
    }

    buffer = (unsigned char *)malloc(BUFLEN); /* alloc buffer on 4-byte boundary */
    firstword = (unsigned long *)buffer;      /* Firstword = First Address on buffer's allocated address space.*/
    tempoint = (unsigned int *)buffer;        /*  */

    /** These functions below, except read are never defined nor called.*/
    read();
    free(buffer);
    return 0;
}

/***************************************************************************/

int read()
{
    unsigned int *Pixel_Plan;
    unsigned int *Pixel_Plan_start;
    unsigned long *Box_Loc;
    unsigned long *Box_Loc_start;
    unsigned long M = 512;
    unsigned long N = 264;
    unsigned long Box_Size = 13;
    unsigned long Sub_Size = 0;
    float Accuracy = 0.001;
    float Sigma = 0;
    float *X_Centroid;
    float *Y_Centroid;
    float *X_Centroid_start;
    float *Y_Centroid_start;
    unsigned int *Boxcnt;
    unsigned int *spotx;
    unsigned int *spoty;
    unsigned int Nboxes;
    int error = 0;
    long n = 0;
    int ii, i, frame = 0, segment = 0;
    int binning, nrows, ncols, dtemp, status, filter, polarizer, finetrack, coollock;
    int nrecs;
    union
    {
        unsigned long lid;
        unsigned char cid[4];
    } tagid;
    long datalength;
    int premature;
    int skips;
    int nfm = 0;
    int mmm, kk, iii;
    int count = 0;
    int inter1, inter2, inter3;
    char str1[MAX_STR];
    char str2[MAX_STR];
    FILE *fptr;

    /** Function signatures of box and do_error methods defined below. */
    extern void box();
    extern int do_error();

    if ((Box_Loc = (unsigned long *)malloc(45 * 45 * sizeof(unsigned long))) == NULL)
    {
        printf("Allocation by malloc Failed.\n");
        exit(0);
    }
    Box_Loc_start = (unsigned long *)Box_Loc;

    if ((Pixel_Plan = (unsigned int *)calloc(512L * 264L, sizeof(unsigned int))) == NULL)
    {
        printf("Allocation by calloc Failed.\n");
        exit(0);
    }
    Pixel_Plan_start = (unsigned int *)Pixel_Plan;

    if ((X_Centroid = (float *)malloc(800 * sizeof(float))) == NULL)
    {
        printf("Allocation by malloc Failed.\n");
        exit(0);
    }
    X_Centroid_start = (float *)X_Centroid;

    if ((Y_Centroid = (float *)malloc(800 * sizeof(float))) == NULL)
    {
        printf("Allocation by malloc Failed.\n");
        exit(0);
    }
    Y_Centroid_start = (float *)Y_Centroid;

    /**
     * taff_tag and its variants were never defined so it is assumed that with the various props it has it is a struct.
     */
    struct Tag
    {
        unsigned long id;
        long datalength;
        int binning, nrows, ncols, dtemp, status;
        unsigned int irig[10];
        char *comment;
    };
    struct TagVol
    {
        unsigned int volnum;
        unsigned long id;
        unsigned int irig[10];
        char *program, mission, instrument;
    };
    struct TagDataSeg
    {
        unsigned long id;
        unsigned int irig[10];
        unsigned long segnum;
        char *comment;
    };
    struct TagSum
    {
        unsigned long id;
        unsigned int irig[10];
        unsigned long nbytes;
    };
    struct Tag taff_tag;
    struct TagVol taff_vol_tag;
    struct TagDataSeg taff_dataseg_tag;
    struct TagSum taff_sum_tag;
    while (1)
    {
        /** Whatever rw_wait does, it takes in an integer value of 1, the buffer length, and
         * the buffer itself (i.e, it's address space since it decays to a pointer). And it appears
         * to return
         */
        if ((error = rw_wait(1, (unsigned)BUFLEN, buffer)))
        {
            /** nfm++ if do_error returns 0, !0 = 1 = true. Otherwise (-1) it exits read(). */
            if (!do_error(error))
                nfm++;
            else
                return -1;
            /*This will cause the while(1) to restart until error = 0 from the rw_wait call. */
            continue;
        }
        /** Will go here if error = 0 from the return value from rw_wait()*/
        nfm = 0;
        if (((buffer[0] & 0x87) == 0x80) && (buffer[1] == 0x01) && (buffer[2] == 0x00) && (buffer[3] == 0x80))
        {
            /** Assuming, the data of the taff_tag struct is organized in buffer, of course they forgot to initliaze it with a file or anything useful.
             * so it is used uninitialized and these values cannot be filled.
             */
            memcpy((void *)&taff_tag, buffer, sizeof(taff_tag));
            tagid.lid = taff_tag.id;
            datalength = taff_tag.datalength;
            binning = taff_tag.binning;
            nrows = taff_tag.nrows;
            ncols = taff_tag.ncols;
            dtemp = taff_tag.dtemp;
            filter = (taff_tag.status & 0x38) >> 3;
            polarizer = (taff_tag.status & 0x04) >> 2;
            finetrack = (taff_tag.status & 0x02) >> 1;
            coollock = taff_tag.status & 0x01;

            printf("\nData Record Tag detected: tag_id = ");
            for (i = 0; i < 4; i++)
                printf("%2.2x ", tagid.cid[i]);
            printf("\n");
            printf("irig =");
            for (i = 0; i < 9; i++)
                printf(" %2.2x", (unsigned int)taff_tag.irig[i]);
            printf("\n");

            printf("datalength = %ld, ", datalength);
            printf("binning = %d, ", binning);
            printf("nrows = %d, ", nrows);
            printf("ncols = %d\n", ncols);
            printf("detector set temperature = %d\n", dtemp);
            printf("filter = %d, ", filter);
            printf("polarizer = %d, ", polarizer);
            printf("finetrack = %d\n", finetrack);
            printf("detector cooler locked = %d\n", coollock);
            printf("comments: %s\n", taff_tag.comment);

            nrecs = (datalength - 128 - 1) / BUFLEN + 1;
            nrecs = 32;

            /*  Should possibly be 2* the above number being that 4 byte long integers */
            /*  are written to file.  This could be fixed by forcing the value of *lp++*/
            /*  in the writfram.c program to be a two byte type int.                   */

            i = 0;
        }
        else if ((buffer[0] == 0x1f) && (buffer[1] == 0x8f) &&
                 (buffer[2] == 0x0f) && (buffer[3] == 0x00))
        {
            memcpy((void *)&taff_vol_tag, buffer, sizeof(taff_vol_tag));

            tagid.lid = taff_vol_tag.id;
            printf("\nVolume Tag detected: tag_id = ");
            for (i = 0; i < 4; i++)
                printf("%2.2x ", tagid.cid[i]);
            printf("\n");
            printf("irig =");
            for (i = 0; i < 9; i++)
                printf(" %2.2x", (unsigned int)taff_vol_tag.irig[i]);
            printf("\n");

            printf("volume number = %d\n", taff_vol_tag.volnum);
            printf("program: %s\n", taff_vol_tag.program);
            printf("mission: %s\n", taff_vol_tag.mission);
            printf("instrument: %s\n", taff_vol_tag.instrument);
            i = 0;
            segment = 0;
            continue;
        }
        else if ((buffer[0] == 0x1f) && (buffer[1] == 0x8f) &&
                 (buffer[2] == 0x0f) && (buffer[3] == 0x08))
        {
            memcpy((void *)&taff_dataseg_tag, buffer, sizeof(taff_dataseg_tag));
            segment++;

            tagid.lid = taff_dataseg_tag.id;
            printf("\nData Segment Tag detected: tag_id = ");
            for (i = 0; i < 4; i++)
                printf("%2.2x ", tagid.cid[i]);
            printf("\n");
            printf("irig =");
            for (i = 0; i < 9; i++)
                printf(" %2.2x", (unsigned)taff_dataseg_tag.irig[i]);
            printf("\n");

            printf("segment number = %ld\n", taff_dataseg_tag.segnum);
            printf("comment: %s\n", taff_dataseg_tag.comment);
            i = 0;
            //      frame = 0;
            continue;
        }
        else if ((buffer[0] == 0x1f) && (buffer[1] == 0xcf) &&
                 (buffer[2] == 0x0f) && (buffer[3] == 0x10))
        {
            memcpy((void *)&taff_sum_tag, buffer, sizeof(taff_sum_tag));

            tagid.lid = taff_sum_tag.id;
            printf("\nSummary Tag detected: tag_id = ");
            for (i = 0; i < 4; i++)
                printf("%2.2x ", tagid.cid[i]);
            printf("\n");
            printf("irig =");
            for (i = 0; i < 9; i++)
                printf(" %2.2x", (unsigned)taff_sum_tag.irig[i]);
            printf("\n");

            printf("byte count = %ld\n", taff_sum_tag.nbytes);
            i = 0;
            continue;
        }
        else
        {
            i = 1;
        }
        premature = 0;
        count++;
        if (count == 1)
            frame++;
        printf("Reading segment %2.2d, frame %3.3d\n", segment, frame);
        /* Read the first half of the frame being that it has already been processed */
        Box_Size = 13;
        Sub_Size = 0;
        Sigma = 0;
        Pixel_Plan = Pixel_Plan_start;
        Box_Loc = Box_Loc_start;
        X_Centroid = X_Centroid_start;
        Y_Centroid = Y_Centroid_start;

        if (count == 1)
        {
            if (error = rw_wait(1, (unsigned)BUFLEN, buffer))
            {
                if (!do_error(error))
                    nfm++;
                else
                    return -1;
                //      frame = 0;
                //        continue;
            }
            nfm = 0;
        }

        if (count == 2)
        {
            for (ii = 0; ii < nrecs - 1; ii++)
            {
                if (error = rw_wait(1, (unsigned)BUFLEN, buffer))
                {
                    if (!do_error(error))
                        nfm++;
                    else
                        return -1;
                    //      frame = 0;
                    continue;
                }
                nfm = 0;
            }
            i = 0;
        } /* if(count == 2) */

        for (; i < nrecs + 1; i++)
        { /*only read half frame due to array size limit*/

            printf("The value of i is: %d \n", i);
            if (error = rw_wait(1, (unsigned)BUFLEN, buffer))
            {
                if (!do_error(error))
                    nfm++;
                else
                    return -1;
                //      frame = 0;
                continue;
            }
            nfm = 0;

            tempoint = (unsigned int *)buffer;
            for (kk = 0; kk < 8; kk++)
            {
                for (mmm = 0; mmm < 512; mmm++)
                {
                    *Pixel_Plan = *tempoint;
                    Pixel_Plan++;
                    tempoint++;
                    //             printf(" %d %d %hu  ",i*8+kk,mmm,*(Pixel_Plan-1));
                }
            }
            if (((buffer[0] & 0x87) == 0x80) && (buffer[1] == 0x01) &&
                (buffer[2] == 0x00) && (buffer[3] == 0x80) /* &&
                    (buffer[0x78]==0x00) && (buffer[0x79]==0x00) */
            )
            {
                premature = 1;
                break;
            }
        } /* end for */

        if (binning == 1000)
        {
            /*Crude box locations*/
            box(Pixel_Plan_start, N, M, Box_Size, Box_Loc);
            Nboxes = *Boxcnt;
            printf("\n");
            printf("Total Number of Boxes: %d \n", Nboxes);
        }
        /* find centroids */
        lvsh_spot_sub(Pixel_Plan_start, N, M, Box_Loc, Nboxes, Box_Size, Sub_Size, Accuracy, Sigma, X_Centroid, Y_Centroid);

        //  Establish name of data file and open it binning - frame . cen where binning is the delay or if 1000, it is a calibration file
        //  and frame is the number of the frame
        if (count == 1)
        {
            if (binning < 10)
            {
                sprintf(str1, "000-");
                str1[2] = 48 + binning;
            }
            if (binning < 100 && binning >= 10)
            {
                sprintf(str1, "000-");
                inter1 = (int)(binning / 10);
                inter2 = binning - 10 * inter1;
                str1[2] = 48 + inter2;
                str1[1] = 48 + inter1;
            }
            if (binning < 1000 && binning >= 100)
            {
                sprintf(str1, "000-");
                inter1 = (int)(binning / 100);
                inter2 = (int)((binning - 100 * inter1) / 10);
                inter3 = binning - 100 * inter1 - 10 * inter2;
                str1[2] = 48 + inter3;
                str1[1] = 48 + inter2;
                str1[0] = 48 + inter1;
            }
            if (frame < 10)
            {
                sprintf(str2, "000.cen");
                str2[2] = 48 + frame;
            }
            if (frame < 100 && frame >= 10)
            {
                sprintf(str2, "000.cen");
                inter1 = (int)(frame / 10);
                inter2 = frame - 10 * inter1;
                str2[2] = 48 + inter2;
                str2[1] = 48 + inter1;
            }
            if (frame < 1000 && frame >= 100)
            {
                sprintf(str2, "000.cen");
                inter1 = (int)(frame / 100);
                inter2 = (int)((frame - 100 * inter1) / 10);
                inter3 = frame - 100 * inter1 - 10 * inter2;
                str2[2] = 48 + inter3;
                str2[1] = 48 + inter2;
                str2[0] = 48 + inter1;
            }
            if (binning == 1000)
            {
                sprintf(str1, "cal-");
            }

            strcat(str1, str2);
            if ((fptr = fopen(str1, "w")) == NULL) /* open file */
            {
                printf("Can't open file %s.", str1);
                exit(0);
            }
        } /* if (count == 1) */

        fprintf(fptr, "%u %u %u \n ", (unsigned int)Nboxes, *spotx, *spoty);
        for (ii = 0; ii < (unsigned int)Nboxes; ii++) /* write to data file*/
        {
            fprintf(fptr, "%d %f %f \n ", ii, *(X_Centroid + ii), *(Y_Centroid + ii));
            printf("%d %f %f \n ", ii, *(X_Centroid + ii), *(Y_Centroid + ii));
        }
        /*
           If done the first half of the frame then rewind the tape and return
           to process the second half of the frame
        */
        if (count == 1)
        {
            backspace();
        }
        if (count == 2)
        {
            fclose(fptr);
            count = 0;
        }

        if (premature || error)
            printf("\nExpected %d records, found only %d.\n", nrecs, i + 1);
        else
            printf("\nFound %d records.\n", i);
        if (error)
            break;
    } /* end while */

    /** Always free up memory allocated with malloc or calloc */
    free(Pixel_Plan);
    free(X_Centroid);
    free(Y_Centroid);
    free(Box_Loc);
} /* end read */

/** Accepts an error code and prints the type of error code, except if error = 1, which means the filemark was detected.
 * @param error The integer code, if error = 1, the filemark was found otherwise, print an error code and return a non-zero integer.
 * @return An integer that is used to verify to other methods that the filemark was found (error = 1) or not (error != 1)
 */
int do_error(int error)
{
    switch (error)
    {
    case 1:
        printf("Filemark Detected.\n");
        return 0;
    case 2:
        printf("Endo of Medium Detected.\n");
        break;
    case 3:
        printf("Blank Tape Detected.\n");
        break;
    default:
        printf("Unrecognized Error Detected.\n");
        break;
    }
    return -1;
}

/**
 * This routine calculates the center of the spots in
 * the pupil plane. The pupil plane is the image of boxes arranged in a grid.
 * It's a rough estimate used to define the boxes used around each of the spots.
 * @param Pixel_Plan pupil plane intensity data
 * @param N row dimension of pupil plane image (assumed it's the last coordinate of the pupil plane in the Y-direction.)
 * @param M column dimension of pupil plane image (assumed it's the last coordinate of the pupil plane in the X-direction.)
 * @param Box_Size pixels across square wavefront processing boxes
 * @param Box_Loc Array of upper-left corners of each box. Each element being: (Top Left X + Top Left Y) as a sum.
 * @param Nboxes number of wavefront processing boxes
 * @param Boxcnt The product of the dimensions of the grid of boxes, (i.e the number of boxes to superimpose)
 */
void box(unsigned int *Pixel_Plan, unsigned long N, unsigned long M, unsigned long Box_Size, unsigned long *Box_Loc, unsigned int *Boxcnt)
{
    /*
    ================================================================================
         Name:         box
         Date:         17.April.1996
         Programmer:   R.J. Hugo
         Environment:  PC/Microsoft C/7.0
         Description:  This routine calculates the center of the spots in
                       the pupil plane.  It is a rough estimate used to
                       define the boxes used around each of the spots

         Revision    Description
                0    17.April.1996:   R.J.H.     initial version
    ================================================================================
         Local Variables/Constants
           CenX[]       - (int) rough centroids in the x-direction
           CenY[]       - (int) rough centroids in the y-direction
           SumX[]       - (static unsigned long) horizontal sum of pixels
           SumY[]       - (static unsigned long) vertical sum of pixels
           Sumtemp      - (unsigned long) temporary location for horizontal sum
           nrecs        - (const int) number of records from tape (4 lines per record)
                                and 128 per frame
           count        - (int) counter used to tell which half of the frame is
                                being read
           imaxx        - (int) number of centroids in the x-direction (columns)
           imaxy        - (int) number of centroids in the y-direction (rows) */

    int CenX[50], CenY[50];

    /* Static var elements are pre initialized to 0 at compile time. */
    static unsigned long SumX[512];
    static unsigned long SumY[512];
    unsigned long Sumtemp;
    int mmm, kk, i, j;
    const int nrecs = 33;
    int count = 0;

    for (i = 0; i < nrecs; i++)
    {
        for (kk = 0; kk < 8; kk++)
        {
            Sumtemp = 0;
            for (mmm = 0; mmm < 512; mmm++)
            {
                SumY[mmm] += *Pixel_Plan;
                Sumtemp += *Pixel_Plan++;
                //             printf(" %d %d %hu ",i*8+kk, mmm, *(Pixel_Plan-1));
            }
            /* Will go in order 0, 1, 2, 3... for each iteration even as i grows. SumX [0 - 262] will have values stored. */
            SumX[i * 8 + kk] = Sumtemp;
        } /* end for kk */
    }     /* end for i */

    /*
       Find the Rough Estimate of the center of each spot which will then
       be used for BoxLocat, i.e calculating each boxes' centroid coordinates (x, y).
       The values of which will be in Cenx[imaxx] and CenY[imaxy].
    */
    int imaxx = -1;
    int imaxy = -1;
    /** If there exists a sum, SumX[i]/SumY[i] such that the preceeding two sums (i - 1 and i - 2) AND
     * their succeeding two sums (i + 1 and i + 2) are smaller than SumX/SumY[i], then the iteration value is a
     * centroid and we store it in the array at the "imaxx/imaxy" index.
     */
    for (i = 7; i < (N - 7); i++)
    {
        if (SumX[i] > SumX[i - 1] && SumX[i] > SumX[i - 2] &&
            SumX[i] > SumX[i + 1] && SumX[i] > SumX[i + 2])
        {
            imaxy += 1;
            CenY[imaxy] = i;
        }
    }
    for (i = 7; i < (M - 7); i++)
    {
        if (SumY[i] > SumY[i - 1] && SumY[i] > SumY[i - 2] &&
            SumY[i] > SumY[i + 1] && SumY[i] > SumY[i + 2])
        {
            imaxx += 1;
            CenX[imaxx] = i;
        }
    }
    /* Since imaxx/imaxy = 0 at some point using indexing, the number of centroids has to be imaxx/imaxy + 1; */
    printf("Number of box locations ix, iy: %d %d \n", imaxx + 1, imaxy + 1);
    *Boxcnt = (unsigned int)((imaxx + 1) * (imaxy + 1));
    // *spotx = (unsigned int)(imaxx + 1);
    // *spoty = (unsigned int)(imaxy + 1);

    /*
      Output the data from the box location routine into the array
      Box_Loc which denotes the upper left hand corner of each of the
      boxes
    */
    for (j = 0; j < imaxy + 1; j++)
    {
        for (i = 0; i < imaxx + 1; i++)
        {
            /* For some reason, it chooses to represent the COORDINATES of the Box_Loc as the
            sum of the (top left X + top left Y) values. */
            if (CenY[j] < 7)
            {
                *Box_Loc = (unsigned long)(CenX[i] - Box_Size / 2);
                printf("Potential Error:  box near edge of array.\n");
            }
            else
            {
                *Box_Loc = (unsigned long)((CenY[j] - 1) * M + CenX[i] -
                                           Box_Size / 2 * M - Box_Size / 2);
            }
            /** Moves the pointer to reference the next element. */
            Box_Loc++;
        }
    }
    /** Resets the pointer back to the start of the array for future iterations. */
    Box_Loc -= (int)*Boxcnt;
} /** END OF BOX METHOD. */

/**
 * This routine calculates the centroids of wavefront sensor spots within a pupil plane image. The centroid
 * locations are calculated relative to the predefined subarray box locations. Should be the function that writes the dot.
 *
 * @param Pupil_Plane    Pointer to the pupil plane intensity data (Should be float[][]) (2D array of doubles.)
 * @param N              Row dimension of the pupil plane image (int)
 * @param M              Column dimension of the pupil plane image (int)
 * @param Box_Locat      Pointer to the wavefront processing box locations (float*)
 * @param Nboxes         Number of wavefront processing boxes (int)
 * @param Box_Size       Dimension of the square window (int)
 * @param Sub_Size       Dimension of the square subarray (int)
 * @param Accuracy       Desired accuracy in spot position (float)
 * @param Sigma          Radius of the Gaussian test spot (float)
 * @param X_Centroid     Pointer to the X coordinates of spot centroids (float*)
 * @param Y_Centroid     Pointer to the Y coordinates of spot centroids (float*)
 *
 * @returns              An integer flag denoting if the centroid spot algorithm was successfully completed, meaning 1.
 *                       If it returns 0, there was a problem with malloc.
 */
int lvsh_spot_sub(unsigned int *Pupil_Plane, unsigned long N, unsigned long M, unsigned long int *Box_Locat, unsigned int Nboxes, unsigned long Box_Size,
                  unsigned long Sub_Size, float Accuracy, float Sigma, float *X_Centroid, float *Y_Centroid)
{
    /* ================================================================================
         Name:         lvsh_spot_sub
         Date:         5.September.1991
         Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
         Environment:  SUN
         Revision    Description
                0    5.September.1991:   R.E.P.     initial version
    ================================================================================
         Output Variables
           X_Centroid    - (float *) centroid position; x (row) coordinate
           Y_Centroid    - (float *) centroid position; y (col) coordinate
    ================================================================================
         Local Variables/Constants
           Subarray     - (float *) wavefront subarray data
           X_Offset     - (float)   x (row) offset of subarray from box position
           Y_Offset     - (float)   y (col) offset of subarray from box position
    ================================================================================
        External Subprograms Called:
            lvsh_refined         - calculates spot position in subarray
            SHE_C_GETSUBARRAY    - extract processing subarray from image frame
    ================================================================================
         External Declarations of nested helper functions:  */

    extern int lvsh_refined(unsigned int *Data, unsigned long n, float *Exponent_Table, float Better_Accuracy, float *Xpos, float *Ypos);
    extern void SHE_C_GETSUBARRAY();
    extern void SHE_C_GETLINE();
    /* =============================================================================
             Local Variables Declarations: */

    unsigned long *Avg_Line;
    float Best_Accuracy;
    float Best_Sigma;
    unsigned long Best_Subbox_Size;
    float *Exponent_Table;
    int i, j;
    int n;
    float X_Offset;
    float Y_Offset;

    /* ========================================================================== */

    /*
    *** Allocate temporary memory for arrays, since it's using malloc to allocate contigous spaces of memory.
        so Subarray and Sum can be used as actual arrays.
    */

    unsigned int *Subarray = (unsigned int *)calloc((int)Box_Size * (int)Box_Size, sizeof(unsigned int));
    if (!Subarray)
        return 0;

    unsigned long *Avg_Line = (unsigned long *)calloc((int)Box_Size, sizeof(unsigned long));
    if (!Avg_Line)
        return 0;

    unsigned long *Sum = (unsigned long *)calloc((int)Box_Size, sizeof(unsigned long));
    if (!Sum)
        return 0;

    unsigned int *Pix = (unsigned int *)calloc((int)Box_Size * (int)Box_Size, sizeof(unsigned int));
    if (!Pix)
        return 0;

    /** Free up Sum here to avoid a memory leak, since Sum will now point to the memory location of Avg_Line.*/
    free(Sum);
    Sum = Avg_Line;

    /*
    *** process each box (each spot)
    */

    Best_Sigma = Sigma;
    Best_Subbox_Size = Sub_Size;
    Best_Accuracy = Accuracy;
    if (Best_Accuracy == 0.0)
        Best_Accuracy = 0.01;

    if (Accuracy == 0.0)
        printf("::: lvsh_aspot ::: automatic ACCURACY = %f\n", Best_Accuracy);

    /*
    *** if we're in automatic Sigma mode we run once through half the data boxes to determine an average
    *** spot cross section and fit a gaussian curve to it. (So it cross-correlates the data with a gaussian curve?)
    */

    if ((Sigma == 0) || (Sub_Size == 0))
    {
        for (i = 0; i < (unsigned int)Nboxes; i++)
        {
            /** So it extracts and manipulates Subarray to be 1 by n. A row vector. */
            SHE_C_GETLINE(Pupil_Plane, N, M, *Box_Locat++, Box_Size, Subarray, Sub_Size);

            /* add line to running sum */
            free(Pix);
            Sum = Avg_Line; /** Average Intesity Profile of a line maybe? */
            Pix = Subarray; /** Pix becomes one line of the subarray from getLine. */
            n = (int)Box_Size;
            while (n--)
                /** Equivalent long way: (*Sum) += (*Pix), then Sum++ and Pix++ to move the ptr locations. */
                *Sum++ += *Pix++;
            Sum -= Box_Size;
            Pix -= Box_Size;
        }
        /* calculate the spot width and appropriate subbox size */
        // printf("The value of Box_Size is %u: \n",Box_Size);
        lvsh_autosigma(Avg_Line, Box_Size, Accuracy, &Best_Sigma, &Best_Subbox_Size);
        printf("::: lvsh_aspot ::: Sigma is %f at Subbox size %d x %d\n", Best_Sigma, Best_Subbox_Size, Best_Subbox_Size);

        lvsh_xsec(Avg_Line, Box_Size);
        Box_Locat -= Nboxes;
    }

    /* set up the exponential table used during centroid determination. */
    printf("Calling refined_table .... \n");
    lvsh_refined_table(Best_Subbox_Size, Best_Sigma, Accuracy, &Exponent_Table, &Best_Accuracy);

    /** Locate the centroids */
    for (i = 0; i < (unsigned int)Nboxes; i++)
    {
        /* extract the subarray of pupil plane data centered around the brightest pixel in the current box*/
        printf("Calling Get_Subarray ....%d \n", i);
        SHE_C_GETSUBARRAY(Pupil_Plane, N, M, *Box_Locat++, Box_Size, Subarray, Best_Subbox_Size, &X_Offset, &Y_Offset);

        /* calculate the spot position within the data subarray */
        printf("Calling  lvsh_refined....%d \n", i);
        /** This method refines the CrossCor calculations. */
        lvsh_refined(Subarray, Best_Subbox_Size, Exponent_Table, Best_Accuracy, X_Centroid, Y_Centroid);

        *X_Centroid++ += (X_Offset + Best_Subbox_Size / 2);
        *Y_Centroid++ += (Y_Offset + Best_Subbox_Size / 2);
        printf("X_Offset %f:\n", X_Offset + Best_Subbox_Size / 2);
        printf("Y_Offset %f:\n", Y_Offset + Best_Subbox_Size / 2);
        printf("Centroid Value %f %f:\n", *(X_Centroid - 1), *(Y_Centroid - 1));
    }
    X_Centroid -= Nboxes;
    Y_Centroid -= Nboxes;

    free(Avg_Line);
    free(Subarray);
    free(Pix);
    free(Exponent_Table);

    return 1;
}
/** END OF LVSH_SUB SPOT METHOD. **/

/** This routine extracts a subarray from the pupil plane image frame. The original search box of size n by n is located at the image coordinates
    specified in Box.  This search box is scanned for its maximum pixel value, and the location of this
    maximum pixel is taken as the center of the n by n subarray.
 * @param Pupil_Plane A matrix version of the image. (Should actually be double**)
 * @param N Row dimension of the image (X)
 * @param M Column dimension of the image (Y)
 * @param Box_Location Offset from start of pupil plane to current box
 * @param Subarray A line of pixel correlation coefficients. (OUTPUT). Extracted data from pupil plane
 * @param Box_Size row dimension of square processing box
 * @param Sub_Size row dimension of square subarray
 * @param X_Offset row offset from box position to subarray (OUTPUT). Row offset from box position to subarray
 * @param Y_Offset column offset from box position to subarray (OUTPUT). Column offset from box position to subarray
 */
void SHE_C_GETSUBARRAY(unsigned int *Pupil_Plane, unsigned long N, unsigned long M, unsigned long int Box_Location, unsigned int *Subarray,
                       unsigned long Box_Size, unsigned long Sub_Size, float *X_Offset, float *Y_Offset)
{

    /*
    ================================================================================
         Name:         SHE_C_GETSUBARRAY
         Date:         25.July.1990
         Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
         Environment:  MicroVAX or SUN
         Revision    Description
                0    25.July.1990:      R.E.P.     initial version

    ================================================================================
        Local Variables/Constants
           i_Box        - (int)     row position of box in pupil plane image
           I_Max        - (int)     row position of maximum pixel in box
           I_Sub        - (int)     row position of subarray in pupil plane image
           J_Box        - (int)     column position of box in pupil plane image
           J_Max        - (int)     column position of maximum pixel in box
           J_Sub        - (int)     column position of subarray in pupil plane image
           Max_Pixel    - (float)   maximum pixel in box
           Next_Row     - (int)     offset from end of row in box to next row in box
           Pixel        - (float)   pixel value
           Ptr          - (float *) pixel in pupil plane
    /* =============================================================================
         Local Variables Declarations: */

    unsigned int *Ptr;
    unsigned int Max_Pixel, Pixel;
    unsigned long Next_Row, I_Box, J_Box, I_Max, J_Max, I_Sub, J_Sub;
    int i, j;

    /* ========================================================================== */
    Next_Row = M - Box_Size;
    /* pointer to first pixel in the current box */
    Ptr = Pupil_Plane + Box_Location;
    printf(" Box_Location = %lu\n", Box_Location);

    I_Box = Box_Location / M;           /* get pixel coordinates  */
    J_Box = Box_Location - (I_Box * M); /* in pupil plane image   */

    printf("I_Box = %u  J_Box = %u \n", I_Box, J_Box);

    I_Max = Box_Size / 2L;
    J_Max = Box_Size / 2L;
    Max_Pixel = *(Ptr + M * Box_Size / 2L + Box_Size / 2L);
    printf("I_Max = %u  J_Max = %u  Max_Pixel = %d \n", I_Box, J_Box, Max_Pixel);

    for (i = 0; i < (int)Box_Size; i++) /* maximum pixel within */
    {                                   /* the box              */
        for (j = 0; j < (int)Box_Size; j++)
        {
            Pixel = *Ptr++;
            if (Pixel > Max_Pixel)
            {
                I_Max = (unsigned long)i;
                J_Max = (unsigned long)j;
                Max_Pixel = Pixel;
            }
        }
        Ptr += (int)Next_Row;
    }

    I_Sub = I_Box + I_Max - (Sub_Size / 2L); /* position subarray to be */
    J_Sub = J_Box + J_Max - (Sub_Size / 2L); /* centered on max. pixel  */
    printf("I_Sub = %u  J_Sub = %u \n", I_Sub, J_Sub);

    if (I_Sub < 0)
        I_Sub = 0; /* do not allow subarray   */
    if (J_Sub < 0)
        J_Sub = 0; /* to fall outside of      */
    if (I_Sub > (N - Sub_Size))
        I_Sub = N - Sub_Size; /* the pupil plane data    */
    if (J_Sub > (M - Sub_Size))
        J_Sub = M - Sub_Size;
    printf("I_Sub = %u  J_Sub = %u \n", I_Sub, J_Sub);

    Ptr = Pupil_Plane + (unsigned long)((I_Sub * M) + J_Sub);
    /* pointer to first pixel in the subarray */

    Next_Row = M - Sub_Size;

    for (i = 0; i < (int)Sub_Size; i++) /* copy subarray data from */
    {                                   /* pupil plane to subarray */
        for (j = 0; j < (int)Sub_Size; j++)
        {
            *Subarray++ = (int)*Ptr++;
            //         printf("Subarray: %d %d %u ",i,j,*(Subarray-1));
        }
        Ptr += (int)Next_Row;
    }
    Subarray -= (int)Sub_Size * (int)Sub_Size;

    *X_Offset = (float)(I_Sub - I_Box); /* calculate offset from */
    *Y_Offset = (float)(J_Sub - J_Box); /* box to subarray       */
    printf("SHE_C_GETSUB - X_Offset:  %u %u %f \n", I_Sub, I_Box, *X_Offset);
    printf("SHE_C_GETSUB - Y_Offset:  %u %u %f \n", J_Sub, J_Box, *Y_Offset);
}
/** This routine extracts a subarray from the pupil plane image frame. Uses a square search window of size n by n to overlap the image coordinates
    specified in Box. It then finds the maximum pixel of the search window, and the location of this maximum pixel is taken as the center of the 1 by n subarray.
 @param Pupil_Plane Pointer to the pupil plane image data (float array).
 @param N Row size of the pupil plane image.
 @param M Column size of the pupil plane image.
 @param Box_Location The top left corner of a box in the plane.
 @param Box_Size Row dimension of the square window.
 @param Subarray An array of values with the intensity of each pixel on the max pixel's line.
 @param Sub_Size Row dimension of the square subarray.
*/
void SHE_C_GETLINE(unsigned int *Pupil_Plane, unsigned long N, unsigned long M, unsigned long int Box_Location, unsigned long Box_Size,
                   unsigned int *Subarray, unsigned long Sub_Size)
{ /*
     ================================================================================
          Name:         SHE_C_GETLINE
          Date:         26.February.1992
          Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
          Environment:  SUN

          Revision    Description
                 0    25.July.1990:      R.E.P.     initial version
                 1    26.February.1990   REP        modified from SHE_C_GETSUBARRAY
     ================================================================================
          Output Variables - could return it.
            Subarray     - (float *) extracted data from pupil plane
     ================================================================================
          Local Variables/Constants
            i_Box        - (int)     row position of box in pupil plane image
            I_Max        - (int)     row position of maximum pixel in box
            I_Sub        - (int)     row position of subarray in pupil plane image
            J_Box        - (int)     column position of box in pupil plane image
            J_Max        - (int)     column position of maximum pixel in box
            J_Sub        - (int)     column position of subarray in pupil plane image
            Max_Pixel    - (float)   maximum pixel in box
            Next_Row     - (int)     offset from end of row in box to next row in box
            Pixel        - (float)   pixel value which will most likely be *Ptr derefenced.
            Ptr          - (float *) pixel in pupil plane
     /* =============================================================================
          Local Variables Declarations: */

    unsigned int *Ptr;
    unsigned int Max_Pixel, Pixel;
    unsigned long Next_Row, I_Box, J_Box, I_Max, J_Max, I_Sub, J_Sub;
    int i, j;

    /* ========================================================================== */

    Next_Row = M - Box_Size;

    Ptr = Pupil_Plane + Box_Location; /* pointer to first pixel in the current box */

    /* Get the first pixel coordinate in the pupil plane image. */
    I_Box = Box_Location / M;
    J_Box = Box_Location - (I_Box * M);

    /** The center coordinates of the square search window */
    I_Max = (unsigned long)(Box_Size / 2);
    J_Max = (unsigned long)(Box_Size / 2);

    /* Creates an initial Max_pixel location and loops through each pixel in the search window, to find the
     maximum pixel. */
    Max_Pixel = *(Ptr + M * (unsigned long)(Box_Size / 2) + (unsigned long)(Box_Size / 2));

    for (i = 0; i < (int)Box_Size; i++)
    {

        for (j = 0; j < (int)Box_Size; j++)
        {
            /* Puts the pointer at the top left pixel corner. */
            Pixel = *Ptr++;
            /** If it finds that the current pointer to a pixel is larger than the max pixel, then it updates
             * the J_Max(x) and I_max(y) coordinates, as well as the Max_Pixel field to the current location.
             */
            if (Pixel > Max_Pixel)
            {
                I_Max = (unsigned long)i;
                J_Max = (unsigned long)j;
                Max_Pixel = Pixel;
            }
        }
        /** Move onto the next row of the box. That is the first pixel in the next row.*/
        Ptr += Next_Row;
    }
    /* Position subarray to be the center of Max_Pixel's location. */
    I_Sub = I_Box + I_Max;
    J_Sub = J_Box + J_Max - (Sub_Size / 2L);
    /* Value checks to ensure the subarray's coordinates do not fall outside the pupil plane ranges. */
    if (I_Sub < 0)
        I_Sub = 0;
    if (J_Sub < 0)
        J_Sub = 0;
    if (I_Sub > (N - Sub_Size))
        I_Sub = N - 1;
    if (J_Sub > (M - Sub_Size))
        J_Sub = M - Sub_Size;

    /*Moves Ptr to the location of the plane, plus the coordinate-sum of the max pixel given by the set subarray locations.*/
    Ptr = Pupil_Plane + (unsigned long)((I_Sub * M) + J_Sub);
    Next_Row = M - Sub_Size;

    /* copy subarray data from the pupil plane to the subarray. */
    for (j = 0; j < Sub_Size; j++)
    {
        *Subarray++ = *Ptr++;
    }
    Subarray -= Sub_Size;
    /** END OF SHE_C_GETLINE METHOD, GO BACK TO LVSH_SPOT_SUB ROUTINE. */
}

/** Given an array of pixel numbers, find the largest pixel from the line of pixels.
 * @param Pixel The starting address location of the array of pixels.
 * @param Npix Number of pixels, or the pixel count.
 */
void lvsh_xsec(unsigned long *Pixel, unsigned long Npix)
{
    unsigned long Max_Pixel;
    int n, j;
    int Nstars;

    for (j = 0; j < (int)Npix; j++)
    {
        printf("val %lu  \n", *(Pixel + j));
    }

    for (j = 0; j < 100; j++)
        printf(" ");
    Max_Pixel = *Pixel;
    n = (int)Npix;
    /** Loop through the set of pixels to find the largest pixel. */
    while (n--)
    {
        if (*Pixel++ > Max_Pixel)
        {
            Max_Pixel = *(Pixel - 1); /** Since Pixel++ happened before this expression.*/
        }
    }
    Pixel -= (int)Npix;

    printf("Max Pixel %lu  \n", Max_Pixel);

    n = (int)Npix;
    /** While loop that prints asteriks, Nstars times = 50 ^ (PIXEL) / Max_Pixel; Then Pixel++ */
    while (n--)
    {
        Nstars = (int)((long)SCALE * *Pixel++ / Max_Pixel);
        while (Nstars--)
            printf("*");
        printf("\n");
    }
} /** END OF LVSH_XSEC() */

/**
 * Calculates spot centroid relative to array center using the fine cross-correlation technique.  The input is an n by n array of floating point data points;
 * the output is the x and y spot centroid position.

* This routine searches for a maximum cross-correlation between the input data and a gaussian (normal) spot with a standard deviation of SIGMA.
* The cross-correlation is calculated at increasingly fine spatial steps until the desired ACCURACY (in pixels) is achieved.

* NOTE: This routine assumes that the centroid of the input data array is within one pixel of the center of the array!  THE INPUT DATA
* MUST MEET THIS CONDITION!  Before passing data to this routine, center it by placing the brightest pixel at the point (n/2,n/2) in the input array.
* @param Data    input data array
* @param n        size of input array ( n by n )
* @param Exponent_Table  array of precalculated exponential values (see lvsh_refined_table)
* @param Better_Accuracy  desired centroid location accuracy (in pixels)  (1 over power of 2 !) (see lvsh_refined_table)
* @param Xpos row position of spot relative to array center (OUTPUT)
* @param Ypos col position of spot relative to array center (OUTPUT)
* @returns An integer flag, 1 if successfuls, 0 if it failed.
*/
int lvsh_refined(unsigned int *Data, unsigned long n, float *Exponent_Table, float Better_Accuracy, float *Xpos, float *Ypos)
{
    /* ================================================================================
         Name:         lvsh_refined
         Date:         20.April.1992
         Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
         Environment:  SUN

        Revision    Description
                0    20.Apr.1992:       REP     initial version
                1    14.May.1993:       REP     added normalization of spot
                                                cross correlation by test spot
                                                autocorrelation
    ================================================================================
         Local Variables/Constants
            Gaussian       - (const float)   -1/(2*Sigma*Sigma) constant for gaussian distribution, (exponent ratio.)
            MaxSum      - (float)   maximum cross-correlation
            Pixel       - (float *) pointer to next data point in input array
            Step        - (float)   resolution of centroid determination
                                    in current run
            Sum         - (float)   current cross-correlation
            Xmax,Ymax   - (float)   location of maximum cross-correlation
            Xoff,Yoff   - (float)   current test centroid location
    /**
     * Assuming the Corr variables below are the total sum of the coefficients, then AutoCorr is a signal overlapping with itself with a time-delay. Usually makes
     * a graph, where it peaks when delay = 0, then drops off. CrossCorr is when the time delay signal is different, and the highest peak is not guranteed
     * to be at delay = 0.
     */
    float AutoCorr;
    float CrossCorr;
    float Exp_i;
    float f_Indexer;
    float Func;
    int i, j;
    int Indexer;
    int Index_Imax;
    int Index_Imin;
    int Index_Jmax;
    int Index_Jmin;
    float MaxSum;
    float Step;
    float Start;
    float Stop;
    float Xmax, Ymax;
    float Xoff, Yoff;

    unsigned int *Pixel;

    /* ========================================================================== */

    /* the "Indexer" variable is the scale factor which coverts a pixel offset into an index in the precalculated Exponent_Table*/
    Indexer = (int)(1.0 / Better_Accuracy);
    f_Indexer = (float)Indexer; /** Float equivalent of Indexer. */

    /** Start at the center of the array. */
    *Xpos = 0.0;
    *Ypos = 0.0;

    /*
    *** Start with a STEP of 1.0 pixels -- i.e. evaluate the cross correlation at the following four points:
    ***
    *** (-1/2,-1/2) ---> (-1/2, +1/2) -(Once a row of the window is done, shift DOWN one pixel and start again)-> (+1/2,-1/2) ---> (+1/2, +1/2)
    ***
    *** relative to Xpos,Ypos (array center).
    ***
    *** Set (XPos, YPos) to the spot that had the highest correlation coefficient.
    ***
    *** On each succeeding loop, we cut the STEP by half and evaluate the cross correlation at four more points:
    *** Ex: 1st Iteration: (STEP = 1/2), 2nd Iteration: (STEP = 1/4), 3rd Iteration: (STEP = 1/8) and so on...
    ***     (-step/2,-step/2)    (-step/2,+step/2)
    ***     (+step/2,-step/2)    (+step/2,+step/2)
    ***
    *** Assuming that the cross-correlation is a relatively smooth spatial function over a few pixels,
    *** Xpos and Ypos will converge to the position of the maximum cross-correlation.
    ***
    */

    /** The pixel range **/
    Start = (float)(-(int)n / 2.);
    Stop = (float)((int)n - (int)n / 2.);

    for (Step = 0.5; Step >= Better_Accuracy; Step /= 2.0)
    {
        MaxSum = 0.0;
        Xoff = *Xpos;
        Yoff = *Ypos;
        Index_Imin = (int)(f_Indexer * (Start - Xoff)); /* Window's starting pixel coordinate in the x-direction. */
        Index_Imax = (int)(f_Indexer * (Stop - Xoff));  /* Window's final pixel coordinate in the x-direction. */
        Index_Jmin = (int)(f_Indexer * (Start - Yoff)); /* Window's starting pixel coordinate in the y-direction. */
        Index_Jmax = (int)(f_Indexer * (Stop - Yoff));  /* Window's final pixel coordinate in the y-direction. */
        AutoCorr = 0.0;
        CrossCorr = 0.0;
        Pixel = Data;
        /*This first nested loop computes the maximum cross-correlation of the two signals. */
        for (i = Index_Imin; i < Index_Imax; i += Indexer)
        {
            Exp_i = *(Exponent_Table + abs(i));
            for (j = Index_Jmin; j < Index_Jmax; j += Indexer)
            {
                Func = Exp_i * *(Exponent_Table + abs(i));
                AutoCorr += Func * Func;
                CrossCorr += (float)(*Pixel) * Func;
                Pixel++;
            }
        }
        CrossCorr /= sqrt(AutoCorr);
        if (CrossCorr > MaxSum)
        {
            /**Update MaxSum and its coordinates, if a cross-correlation sum was found to be larger than the current one.*/
            Xmax = Xoff;
            Ymax = Yoff;
            MaxSum = CrossCorr;
        }

        /** More elaborate CrossCor calculation, if the first loop does the calculation for one window. Then this loop, O(n^4)
         *  would be possible shifting the window and calculating the cross correlations for each window, each window having a set of sums.
         *  Instead of one window, the window would shift and calculate the CC sums for every pixel in every possible window.
         */
        for (Xoff = *Xpos - Step; Xoff <= *Xpos + Step; Xoff += (2. * Step))
        {
            Index_Imin = (int)(f_Indexer * (Start - Xoff));
            Index_Imax = (int)(f_Indexer * (Stop - Xoff));
            for (Yoff = *Ypos - Step; Yoff <= *Ypos + Step; Yoff += (2. * Step))
            {
                Index_Jmin = (int)(f_Indexer * (Start - Yoff));
                Index_Jmax = (int)(f_Indexer * (Stop - Yoff));

                /** Calculate the correlation at the current pixel spot.*/
                AutoCorr = 0.0;
                CrossCorr = 0.0;
                Pixel = Data;
                for (i = Index_Imin; i < Index_Imax; i += Indexer)
                {
                    Exp_i = *(Exponent_Table + abs(i));
                    for (j = Index_Jmin; j < Index_Jmax; j += Indexer)
                    {
                        Func = Exp_i * *(Exponent_Table + abs(i));
                        AutoCorr += Func * Func;
                        CrossCorr += (float)(*Pixel) * Func;
                        Pixel++;
                    }
                }
                CrossCorr /= sqrt(AutoCorr);
                if (CrossCorr > MaxSum)
                {
                    Xmax = Xoff;
                    Ymax = Yoff;
                    MaxSum = CrossCorr;
                }
            }
        }
        *Xpos = Xmax;
        *Ypos = Ymax;
        printf(" Step %f Position %f %f \n", Step, *Xpos, *Ypos);

    } /* for Step */

    return 1;
} /** END OF LVHS_REFINED() METHOD, NOT THE TABLE VERSION. */

/** Precalculates exponentials for all possible gaussian spot offsets for the refined centroid location algorithm. (see lvsh_refined)
 * @param n size of input array ( n by n )
 * @param Sigma standard deviation of reference spot (in pixels) (this is the spot halfwidth)
 * @param Accuracy desired centroid location accuracy (in pixels)  must be less than 1.0
 * @param Exponent_Table Table of calculated exponentials. (OUTPUT)
 * @param Better_Accuracy revised centroid location accuracy ( 1 over a power of 2 ), (OUTPUT)
 * @returns An integer flag, 1 being successful, 0 being failure.
 */
int lvsh_refined_table(unsigned long n, float Sigma, float Accuracy, float **Exponent_Table, float *Better_Accuracy)
{

    /*
    ================================================================================
         Name:         lvsh_refined_table
         Date:         20.April.1992
         Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
         Environment:  SUN
         Revision    Description
                0    20.Apr.1992     R.E.P.     initial version
                1    14.May.1993     R.E.P.     normalized for the power in the gaussian test spot this is essential for small
                                                test spots
    ================================================================================
         Local Variables/Constants
            Const       - (float)   -1/(2*Sigma*Sigma) constant for gaussian
    /* =============================================================================
         Local Variables Declarations: */

    float gaussianExp;
    float *Exponent;
    int i;
    int Limit;
    unsigned long isq;
    float Sum;

    /* ========================================================================== */

    /*
    *** set accuracy to the next power of 2
    *** smaller than the requested accuracy
    */

    //   *Better_Accuracy = exp2( (float)(int)(-2.0+log2(Accuracy)) );
    //   *Better_Accuracy = pow(2.,( (float)(int)(-2.0+
    //                        log(Accuracy)/log(2.)) ));
    *Better_Accuracy = pow(2., ((float)(int)(-1.0 + log(Accuracy) / log(2.))));
    gaussianExp = -1.0 / (2.0 * Sigma * Sigma);

    /* convert constant to indexing for output array */
    gaussianExp *= ((*Better_Accuracy) * (*Better_Accuracy));

    /*
    *** the number of exponential we need to precalculate is equal to the number of possible centroid positions within a
    *** pixel (1/Accuracy) times the total number of pixels across the subbox array (n) divided by 2 (to account for the fact
    *** that the centroid MUST be within one pixel of the center of the subarray.
    */

    //   Limit = (int) (( (int) n+1) / ( 2.*(*Better_Accuracy) ) );
    Limit = 1 + (int)(((int)n + 3) / (2. * (*Better_Accuracy)));

    *Exponent_Table = (float *)malloc(Limit * sizeof(float));
    if (*Exponent_Table == NULL)
    {
        fprintf(stderr, "::: lvsh_refined_table ::: could not allocate memory! \n");
        return 0;
    }

    /* fill the table*/

    Exponent = *Exponent_Table;
    for (i = 0; i < Limit; i++)
    {
        isq = ((unsigned long)i * (unsigned long)i);
        *Exponent++ = exp(gaussianExp * (float)isq);
    }

    return 1;
}
/** Attempts to calculate the optimal half-width, i.e it's standard deviation (sigma) and subbox size for a data spot.
    The data passed in is a cross-section through the spot centered on the spot's brightest pixel.
    This routine searches for a maximum cross-correlation between the input data and a gaussian curve with a deviation of SIGMA.
    The cross-correlation is calculated at increasingly fine spatial steps until the desired ACCURACY (in pixels) is achieved.
    @param Data The input array
    @param n The length of the input vector
    @param Accuracy The desired accuracy of the centroid location (must be < 1)
    @param Sigma The spot's half width as a starting reference.
    @param Size Optimal subbox size
    @return An integer, 1 if it succeeding
*/
int lvsh_autosigma(unsigned long *Data, unsigned long n, float Accuracy, float *Sigma, int *Size)
{
    /*
    ================================================================================
         Name:         lvsh_autosigma
         Date:         26.February.1992
         Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
         Environment:  SUN
         Revision    Description
                0    26.Feb.1992     REP     initial version
                1    14.May.1993     REP     corrected 1/(Sigma*Sigma) to
                                             1/(2*Sigma*Sigma) for gaussians
    ================================================================================
         Local Variables/Constants
            Const          - (float)   -1/2*(Sigma**2) constant for normal curve
            Func           - (float)   value of normal curve at current pixel
            MaxSum         - (float)   maximum cross-correlation
            Pixel          - (float *) pointer to next data point in input array
            Step           - (float)   resolution of sigma in current run
            Sum_AutoCorr   - (float)   current autocorrelation of normal curve
            Sum_CrossCorr  - (float)   current cross-correlation
            Sig            - (float)   current test value of Sigma
    /* =============================================================================
         Local Variables Declarations: */

    int i;
    unsigned long isq;
    float MaxSum;
    float New_Sigma; /** Current ideal sigma placeholder as you loop. */
    float Step;
    float Sig;
    float gaussianConst;
    float Func;
    float Sum_AutoCorr;
    unsigned long *Pixel;
    float Sum_CrossCorr;

    /* ========================================================================== */

    /*
    *** The form of the gaussian is determined by the exponentiation constant 1/sqrt(Sigma). The scale of the gaussian is unimportant
    *** (it is normalized out in lvsh_refined)
    ***  Gaussian Formula Input: (SIGMA = STD, the further away from 0, the more stretched the curve is)
    ***  and (mu = The mean, shifts the peak/trough of the gaussian curve which is always at x = mu. )
    ***  where:   x and is the pixel position relative to brightest pixel (center of the array.)
    */

    /* start with initial guess of SIGMA one eigth the input vector length */
    *Sigma = (float)(n / 8);

    /*
    *** Start with a STEP of n/16 pixels -- i.e: evaluate the cross correlation at
    *** SIGMA values of : (n/8-n/16)      (n/8+n/16)
    ***
    *** Reset New_Sigma to the position which showed the maximum correlation.
    ***
    *** On each succeeding loop, we cut the STEP by half and evaluate the cross correlation at two more points:
    *** (New_Sigma-step/2)  (New_Sigma+step/2)
    ***
    *** In other words, SIGMA will move by up to n/16 pixels in the first iteration, up to n/32 pixels in the second iteration,
    *** n/64 in the third, etc.  Assuming that the cross-correlation is a relatively smooth spatial function, New_Sigma will converge
    *** to the best possible SIGMA value.
    */

    printf(" In Autosigma  \n");
    for (Step = ((float)n) / 16; Step > Accuracy / 2.0; Step /= 2.0)
    {
        MaxSum = 0.0;

        printf(" Step = %f   Accuracy = %f \n ", Step, Accuracy);

        /* compute best SIGMA for the present step */
        for (Sig = *Sigma - Step; Sig <= *Sigma + Step; Sig += Step)
        {
            gaussianConst = -1.0 / (2.0 * Sig * Sig);
            Sum_AutoCorr = 0.0;
            Sum_CrossCorr = 0.0;
            Pixel = Data;
            for (i = -(int)(n / 2); i < (int)(n - n / 2); i++)
            {
                isq = (unsigned long)i * (unsigned long)i;
                Func = exp(gaussianConst * (float)(isq));
                Sum_CrossCorr += (float)(*Pixel++) * Func;
                Sum_AutoCorr += Func * Func;
            }
            Sum_CrossCorr /= sqrt(Sum_AutoCorr);
            if (Sum_CrossCorr > MaxSum)
            {
                New_Sigma = Sig;
                MaxSum = Sum_CrossCorr;
            }
        }

        *Sigma = New_Sigma;
    }

    *Size = (int)(0.5 + 4.0 * *Sigma);
    if ((*Size) % 2 == 0)
        *Size += 1;

    if (*Size > n)
        *Size = 1;
    if (*Size < 1)
        *Size = 1;

    /* printf ( "Sigma = %f   Subbox = %d\n", *Sigma, *Size ); */
    return 1;
} /* END OF LVSH_AUTOSIGMA */