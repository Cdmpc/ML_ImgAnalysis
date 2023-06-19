#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "scsisubs.h"
#define globals
#include "globals.h"
#define subs386
#include "386subs.h"
#define irigsubs
#include "irigsubs.h"

#define BUFLENGTH (8192)
#define ABS(i) (((i) < 0) ? -i : i)
#define SCALE 50
#define MAX_STR 80

unsigned char *buffer; /* read buffer for data transfers */
unsigned long *firstword;
unsigned int *tempoint;
unsigned long *tempoint1;

/***************************************************************************/

main(int argc, char **argv)
{
   if (argc > 1)
      set_verbose(1);                       /* set SCSI error reporting to "verbose" */
   buffer = my_malloc((unsigned)BUFLENGTH); /* alloc buffer on 4-byte boundary */
   firstword = (unsigned long *)buffer;
   tempoint = (unsigned int *)buffer;

   /** These two functions below are */
   init_scsi();
   rewind_unit();
   read();

   scsi_cleanup();
   return 0;
}

/***************************************************************************/

read()

{
   /*  char *malloc();   Does not work on the PC */
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
   int binning, nrows, ncols, dtemp, status, polarizer, finetrack, coollock;
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

   extern void box();

   if ((Box_Loc = (unsigned long *)malloc(45 * 45 * sizeof(unsigned long))) == NULL)
   {
      printf("Allocation by malloc Failed.\n");
      exit(0);
   }
   Box_Loc_start = (unsigned long *)Box_Loc;

   if ((Pixel_Plan = (unsigned int *)halloc(512L * 264L, sizeof(unsigned int))) == NULL)
   {
      printf("Allocation by halloc Failed.\n");
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

   while (1)
   {
      if (error = rw_wait(1, (unsigned)BUFLENGTH, buffer))
      {
         if (!do_error(error))
            nfm++;
         else
            return (-1);
         //      frame = 0;
         continue;
      }
      nfm = 0;
      if (((buffer[0] & 0x87) == 0x80) && (buffer[1] == 0x01) &&
          (buffer[2] == 0x00) && (buffer[3] == 0x80))
      {
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
            printf(" %2.2x", (unsigned)taff_tag.irig[i]);
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

         nrecs = (datalength - 128 - 1) / BUFLENGTH + 1;
         nrecs = 32;

         /*  Should possibly be 2* the above number being that 4 byte long integers */
         /*  are written to file.  This could be fixed by forcing the value of *lp++*/
         /*  in the writfram.c program to be a two byte type int.                   */

         i = 0;
         //      getch();
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
            printf(" %2.2x", (unsigned)taff_vol_tag.irig[i]);
         printf("\n");

         printf("volume number = %d\n", taff_vol_tag.volnum);
         printf("program: %s\n", taff_vol_tag.program);
         printf("mission: %s\n", taff_vol_tag.mission);
         printf("instrument: %s\n", taff_vol_tag.instrument);
         i = 0;
         segment = 0;
         //      getch();
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
         //      getch();
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
         //      getch();
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
      //    getch();
      /*
         Read the first half of the frame being that it has already been processed
      */
      Box_Size = 13;
      Sub_Size = 0;
      Sigma = 0;
      Pixel_Plan = Pixel_Plan_start;
      Box_Loc = Box_Loc_start;
      X_Centroid = X_Centroid_start;
      Y_Centroid = Y_Centroid_start;

      if (count == 1)
      {
         if (error = rw_wait(1, (unsigned)BUFLENGTH, buffer))
         {
            if (!do_error(error))
               nfm++;
            else
               return (-1);
            //      frame = 0;
            //        continue;
         }
         nfm = 0;
      }

      if (count == 2)
      {
         for (ii = 0; ii < nrecs - 1; ii++)
         {
            if (error = rw_wait(1, (unsigned)BUFLENGTH, buffer))
            {
               if (!do_error(error))
                  nfm++;
               else
                  return (-1);
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
         getch();
         if (error = rw_wait(1, (unsigned)BUFLENGTH, buffer))
         {
            if (!do_error(error))
               nfm++;
            else
               return (-1);
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
            //         getch();
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
         box(Pixel_Plan_start, /* crude box locations */
             N, M,
             Box_Size,
             Box_Loc,
             Boxcnt,
             spotx,
             spoty);

         Nboxes = *Boxcnt;
         printf("\n");
         printf("Total Number of Boxes: %d \n", Nboxes);
         //      getch();
      }

      lvsh_spot_sub(Pixel_Plan_start, /* find centroids */
                    N, M,
                    Box_Loc,
                    Nboxes,
                    Box_Size,
                    Sub_Size,
                    Accuracy,
                    Sigma,
                    X_Centroid,
                    Y_Centroid);

      //
      //  Establish name of data file and open it
      //  binning - frame . cen
      //  where binning is the delay or if 1000, it is a calibration file
      //  and frame is the number of the frame
      //
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
         getch();
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
     //  fclose(fptr);
   hfree((void *)Pixel_Plan);
   free(X_Centroid);
   free(Y_Centroid);
   free(Box_Loc);
} /* end read */

/***************************************************************************/

int do_error(int error)
{
   switch (error)
   {
   case 1:
      printf("Filemark Detected.\n");
      return (0);
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
   return (-1);
}

/***************************************************************************/

void box(Pixel_Plan,
         N, M,
         Box_Size,
         Box_Loc,
         Boxcnt,
         spotx,
         spoty)

    unsigned int *Pixel_Plan;
unsigned long int *Box_Loc;
unsigned long N, M, Box_Size;
unsigned int *Boxcnt;
unsigned int *spotx;
unsigned int *spoty;

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
        Input Variables
          Pixel_Plan    - (unsigned int _huge *) pupil plane intensity data
          N             - (int)     row dimension of pupil plane image
          M             - (int)     column dimension of pupil plane image
          Box_Size      - (int)     pixels across square wavefront processing boxes
   ================================================================================
        Output Variables
          Box_Loc       - (unsigned int *) wavefront processing box locations
          Nboxes        - (int)     number of wavefront processing boxes
   ================================================================================
        Local Variables/Constants
          CenX[]       - (int) rough centroids in the x-direction
          CenY[]       - (int) rough centroids in the y-direction
          SumX[]       - (static unsigned long) horizontal sum of pixels
          SumY[]       - (static unsigned long) vertical sum of pixels
          Sumtemp      - (unsigned long) temporary location for horizontal sum
          nrecs        - (int) number of records from tape (4 lines per record)
                               and 128 per frame
          count        - (int) counter used to tell which half of the frame is
                               being read
          imaxx        - (int) number of centroids in the x-direction (columns)
          imaxy        - (int) number of centroids in the y-direction (rows)
   ================================================================================
        External Variables/Constants
   ================================================================================
        Intrinsic Functions Called:
   ================================================================================
        External Subprograms Called:

   ================================================================================
        External Declarations:  */

   /* =============================================================================
        Local Parameters/Constants Declarations: */

   /* =============================================================================
        Local Variables Declarations: */

   int CenX[50], CenY[50];
   static unsigned long SumX[512];
   static unsigned long SumY[512];
   unsigned long Sumtemp;
   int mmm, kk, i, j, nrecs = 33;
   int count = 0;
   int imaxx, imaxy;

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
         SumX[i * 8 + kk] = Sumtemp;
         //         getch();
      } /* end for kk */
   }    /* end for i */

   /*
      Find the Rough Estimate of the center of each spot which will then
      be used for BoxLocat
   */
   imaxx = -1;
   imaxy = -1;
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
   printf("Number of box locations ix, iy: %d %d \n", imaxx + 1, imaxy + 1);
   //     getch();
   *Boxcnt = (unsigned int)((imaxx + 1) * (imaxy + 1));
   *spotx = (unsigned int)(imaxx + 1);
   *spoty = (unsigned int)(imaxy + 1);
   /*
     Output the data from the box location routine into the array
     Box_Loc which denotes the upper left hand corner of each of the
     boxes
   */
   for (j = 0; j < imaxy + 1; j++)
   {
      for (i = 0; i < imaxx + 1; i++)
      {
         //          printf("Values:  %d %d  ",CenY[j],CenX[i]);
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
         Box_Loc++;
      }
      //          getch();
   }
   Box_Loc -= (int)*Boxcnt;
}

int lvsh_spot_sub(Pupil_Plane,
                  N, M,
                  Box_Locat,
                  Nboxes,
                  Box_Size,
                  Sub_Size,
                  Accuracy,
                  Sigma,
                  X_Centroid,
                  Y_Centroid)

unsigned int *Pupil_Plane;
// float  *Pupil_Plane;
unsigned long N;
unsigned long M;
unsigned long int *Box_Locat;
unsigned int Nboxes;
unsigned long Box_Size;
unsigned long Sub_Size;
float Accuracy;
float Sigma;
float *X_Centroid;
float *Y_Centroid;
{

   /*
   ================================================================================
        Name:         lvsh_spot_sub
        Date:         5.September.1991
        Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
        Environment:  SUN
        Description:  This routine calculates the centroids of wavefront
                      sensor spots within a pupil plane image.  The centroid
                      locations are returned relative to the predefined
                      subarray box locations.

        Revision    Description
               0    5.September.1991:   R.E.P.     initial version

   ================================================================================
        Input Variables
          Pupil_Plane   - (float *) pupil plane intensity data
          N             - (int)     row dimension of pupil plane image
          M             - (int)     column dimension of pupil plane image
          Box_Locat     - (float *) wavefront processing box locations
          Nboxes        - (int)     number of wavefront processing boxes
          Box_Size      - (int)     pixels across square wavefront processing boxes
          Sub_Size      - (int)     pixels across square wavefront subarrays
          Accuracy      - (float)   desired accuracy in spot position
          Sigma         - (float)   radius of gaussian test spot
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
        External Variables/Constants
   ================================================================================
        Intrinsic Functions Called:
   ================================================================================
        External Subprograms Called:
           lvsh_refined         - calculates spot position in subarray
           SHE_C_GETSUBARRAY    - extract processing subarray from image frame
   ================================================================================
        External Declarations:  */

   //         extern void lvsh_refined ( );
   extern void SHE_C_GETSUBARRAY();
   extern void SHE_C_GETLINE();
   //         char  *malloc();

   /* =============================================================================
        Local Parameters/Constants Declarations: */
   /* =============================================================================
        Local Variables Declarations: */

   unsigned long *Avg_Line;
   float Best_Accuracy;
   float Best_Sigma;
   unsigned long Best_Subbox_Size;
   float *Exponent_Table;
   int i, j;
   int n;
   unsigned int *Pix;
   unsigned int *Subarray;
   unsigned long *Sum;
   float X_Offset;
   float Y_Offset;

   /* ========================================================================== */

   /*
   *** allocate temporary memory
   */

   Subarray = (unsigned int *)malloc((int)Box_Size * (int)Box_Size *
                                     sizeof(unsigned int));
   if (!Subarray)
      return (0);

   Avg_Line = (unsigned long *)malloc((int)Box_Size * sizeof(unsigned long));
   if (!Avg_Line)
      return (0);

   Sum = (unsigned long *)malloc((int)Box_Size * sizeof(unsigned long));
   if (!Sum)
      return (0);

   Pix = (unsigned int *)malloc((int)Box_Size * (int)Box_Size *
                                sizeof(unsigned int));
   if (!Pix)
      return (0);

   Sum = Avg_Line;
   for (i = 0; i < Box_Size; i++)
      *Sum++ = 0;

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
   *** if we're in automatic Sigma mode
   *** we run once through half the data
   *** boxes to determine an average
   *** spot cross section and fit a
   *** gaussian to it
   */

   if ((Sigma == 0) || (Sub_Size == 0))
   {
      for (i = 0; i < (unsigned int)Nboxes; i++)
      {
         /*
         *** extract a line of pixels centered on
         *** the brightest pixel in the current box
         */

         SHE_C_GETLINE(Pupil_Plane,
                       N, M,
                       *Box_Locat++,
                       Box_Size,
                       Subarray,
                       Box_Size);

         /*
         *** add line to running sum
         */

         Sum = Avg_Line;
         Pix = Subarray;
         n = (int)Box_Size;
         while (n--)
            *Sum++ += *Pix++;
         Sum -= Box_Size;
         Pix -= Box_Size;
      }

      /*
      *** calculate the spot width and
      *** appropriate  subbox size
      */

      //      printf("The value of Box_Size is %u: \n",Box_Size);
      //      getch();
      lvsh_autosigma(Avg_Line,
                     Box_Size,
                     Accuracy,
                     &Best_Sigma,
                     &Best_Subbox_Size);

      printf("::: lvsh_aspot ::: Sigma is %f at Subbox size %d x %d\n",
             Best_Sigma, Best_Subbox_Size, Best_Subbox_Size);
      lvsh_xsec(Avg_Line, Box_Size);

      Box_Locat -= Nboxes;
   }

   /*
   *** set up the exponential table
   *** used during centroid determination
   */

   printf("Calling refined_table .... \n");
   //   getch();
   lvsh_refined_table(Best_Subbox_Size,
                      Best_Sigma,
                      Accuracy,
                      &Exponent_Table,
                      &Best_Accuracy);

   /*
   *** locate the centroids
   */

   for (i = 0; i < (unsigned int)Nboxes; i++)
   {
      /*
      *** extract the subarray of pupil plane data
      *** centered around the brightest pixel in
      *** the current box
      */

      printf("Calling Get_Subarray ....%d \n", i);
      getch();
      SHE_C_GETSUBARRAY(Pupil_Plane,
                        N, M,
                        *Box_Locat++,
                        Box_Size,
                        Subarray,
                        Best_Subbox_Size,
                        &X_Offset,
                        &Y_Offset);

      /*
      *** calculate the spot position within the
      *** data subarray
      */
      printf("Calling  lvsh_refined....%d \n", i);
      lvsh_refined(Subarray,
                   Best_Subbox_Size,
                   Exponent_Table,
                   Best_Accuracy,
                   X_Centroid,
                   Y_Centroid);

      *X_Centroid++ += (X_Offset + Best_Subbox_Size / 2);
      *Y_Centroid++ += (Y_Offset + Best_Subbox_Size / 2);
      printf("X_Offset %f:\n", X_Offset + Best_Subbox_Size / 2);
      printf("X_Offset %f:\n", Y_Offset + Best_Subbox_Size / 2);
      printf("Centroid Value %f %f:\n", *(X_Centroid - 1), *(Y_Centroid - 1));
   }
   X_Centroid -= Nboxes;
   Y_Centroid -= Nboxes;

   free(Avg_Line);
   free(Subarray);
   free(Pix);
   free(Sum);
   free(Exponent_Table);

   return (1);
}

void SHE_C_GETSUBARRAY(Pupil_Plane,
                       N, M,
                       Box_Location,
                       Box_Size,
                       Subarray,
                       Sub_Size,
                       X_Offset,
                       Y_Offset)

    unsigned int *Pupil_Plane;
// float *Pupil_Plane;
unsigned long N;
unsigned long M;
unsigned long int Box_Location;
unsigned int *Subarray;
unsigned long Box_Size;
unsigned long Sub_Size;
float *X_Offset;
float *Y_Offset;
{

   /*
   ================================================================================
        Name:         SHE_C_GETSUBARRAY
        Date:         25.July.1990
        Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
        Environment:  MicroVAX or SUN
        Description:  This routine extracts a subarray from the pupil
                      plane image frame.  The original search box of
                      size n by n is located at the image coordinates
                      specified in Box.  This search box is scanned for
                      its maximum pixel value, and the location of this
                      maximum pixel is taken as the center of the nn by
                      nn subarray.

        Revision    Description
               0    25.July.1990:      R.E.P.     initial version

   ================================================================================
        Input Variables
          Pupil_Plane  - (float *) pupil plane image data
          N            - (int)     row size of pupil plane image
          M            - (int)     column size of pupil plane image
          Box_Location - (int)   offset from start of pupil plane to current box
          Box_Size     - (int)     row dimension of square processing box
          Sub_Size     - (int)     row dimension of square subarray
   ================================================================================
        Output Variables
          Subarray     - (float *) extracted data from pupil plane
          X_Offset     - (float *) row offset from box position to subarray
          Y_Offset     - (float *) column offset from box position to subarray
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
   ================================================================================
        External Variables/Constants
   ================================================================================
        Intrinsic Functions Called:
   ================================================================================
        External Subprograms Called:
   ================================================================================
        External Declarations:  */
   /* =============================================================================
        Local Parameters/Constants Declarations: */
   /* =============================================================================
        Local Variables Declarations: */

   unsigned int *Ptr;
   unsigned int Max_Pixel, Pixel;
   unsigned long Next_Row, I_Box, J_Box, I_Max, J_Max, I_Sub, J_Sub;
   int i, j;

   /* ========================================================================== */

   Next_Row = M - Box_Size;

   Ptr = Pupil_Plane + Box_Location; /* pointer to first pixel */
                                     /* in the current box     */
   printf(" Box_Location = %lu  \n", Box_Location);

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
            //            printf("Max Pixel now = %8.8hu \n",Max_Pixel);
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
   /* pointer to first pixel */
   /* in the subarray        */

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
   getch();
}

void SHE_C_GETLINE(Pupil_Plane,
                   N, M,
                   Box_Location,
                   Box_Size,
                   Subarray,
                   Sub_Size)

    // float *Pupil_Plane;
    unsigned int *Pupil_Plane;
unsigned long N;
unsigned long M;
unsigned long Box_Size;
unsigned long Sub_Size;
unsigned long int Box_Location;
unsigned int *Subarray;
{

   /*
   ================================================================================
        Name:         SHE_C_GETLINE
        Date:         26.February.1992
        Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
        Environment:  SUN
        Description:  This routine extracts a subarray from the pupil
                      plane image frame.  The original search box of
                      size n by n is located at the image coordinates
                      specified in Box.  This search box is scanned for
                      its maximum pixel value, and the location of this
                      maximum pixel is taken as the center of the by
                      1 by nn subarray.

        Revision    Description
               0    25.July.1990:      R.E.P.     initial version
               1    26.February.1991   REP        modified from SHE_C_GETSUBARRAY

   ================================================================================
        Input Variables
          Pupil_Plane  - (float *) pupil plane image data
          N            - (int)     row size of pupil plane image
          M            - (int)     column size of pupil plane image
          Box_Location - (int)   offset from start of pupil plane to current box
          Box_Size     - (int)     row dimension of square processing box
          Sub_Size     - (int)     row dimension of square subarray
   ================================================================================
        Output Variables
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
          Pixel        - (float)   pixel value
          Ptr          - (float *) pixel in pupil plane
   ================================================================================
        External Variables/Constants
   ================================================================================
        Intrinsic Functions Called:
   ================================================================================
        External Subprograms Called:
   ================================================================================
        External Declarations:  */
   /* =============================================================================
        Local Parameters/Constants Declarations: */
   /* =============================================================================
        Local Variables Declarations: */

   unsigned int *Ptr;
   unsigned int Max_Pixel, Pixel;
   unsigned long Next_Row, I_Box, J_Box, I_Max, J_Max, I_Sub, J_Sub;
   int i, j;

   /* ========================================================================== */

   Next_Row = M - Box_Size;

   Ptr = Pupil_Plane + Box_Location; /* pointer to first pixel */
                                     /* in the current box     */

   I_Box = Box_Location / M;           /* get pixel coordinates  */
   J_Box = Box_Location - (I_Box * M); /* in pupil plane image   */

   I_Max = Box_Size / 2L;
   J_Max = Box_Size / 2L;
   Max_Pixel = *(Ptr + M * Box_Size / 2L + Box_Size / 2L);

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
      Ptr += Next_Row;
   }

   I_Sub = I_Box + I_Max;                   /* position subarray to be */
   J_Sub = J_Box + J_Max - (Sub_Size / 2L); /* centered on max. pixel  */

   if (I_Sub < 0)
      I_Sub = 0; /* do not allow subarray   */
   if (J_Sub < 0)
      J_Sub = 0; /* to fall outside of      */
   if (I_Sub > (N - Sub_Size))
      I_Sub = N - 1; /* the pupil plane data    */
   if (J_Sub > (M - Sub_Size))
      J_Sub = M - Sub_Size;

   Ptr = Pupil_Plane + (unsigned long)((I_Sub * M) + J_Sub);

   /* pointer to first pixel */
   /* in the subarray        */

   Next_Row = M - Sub_Size;

   //   Ptr      = Pupil_Plane +  Box_Location;       /* pointer to first pixel */

   //   Ptr = Pupil_Plane + Box_Location+I_Max*M;     /* pointer to first pixel */

   /* copy subarray data from */
   /* pupil plane to subarray */
   for (j = 0; j < Sub_Size; j++)
   {
      *Subarray++ = *Ptr++;
   }

   Subarray -= Sub_Size;
}

// #define SCALE 50

lvsh_xsec(Pixel, Npix)

    unsigned long *Pixel;
unsigned long Npix;

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
   while (n--)
      if (*Pixel++ > Max_Pixel)
         Max_Pixel = *(Pixel - 1);
   Pixel -= (int)Npix;

   printf("Max Pixel %lu  \n", Max_Pixel);
   getch();

   n = (int)Npix;
   while (n--)
   {
      Nstars = (int)((long)SCALE * *Pixel++ / Max_Pixel);
      while (Nstars--)
         printf("*");
      printf("\n");
   }
}

// #include "vinclude.h"

// #define ABS(i)  ( ((i)<0)?-i:i )

// #include <math.h>

int lvsh_refined(Data, n, Exponent_Table, Better_Accuracy, Xpos, Ypos)

unsigned int *Data;
unsigned long n;
float *Exponent_Table;
float Better_Accuracy;
float *Xpos;
float *Ypos;
{

   /*
   ================================================================================
        Name:         lvsh_refined
        Date:         20.April.1992
        Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
        Environment:  SUN
        Description:  Calculates spot centroid relative to array
                      center using the fine cross-correlation
                      technique.  The input is an n by n array of
                      floating point data points; the output is
                      the x and y spot centroid position.

                      This routine searches for a maximum cross-
                      correlation between the input data and a
                      gaussian (normal) spot with a deviation of
                      SIGMA.  The cross-correlation is calculated
                      at increasingly fine spatial steps until
                      the desired ACCURACY (in pixels) is achieved.

                      NOTE: This routine assumes that the centroid
                      of the input data array is within one pixel
                      of the center of the array!  THE INPUT DATA
                      MUST MEET THIS CONDITION!  Before passing
                      data to this routine, center it by placing
                      the brightest pixel at the point (n/2,n/2)
                      in the input array.


        Revision    Description
               0    20.Apr.1992:       REP     initial version
               1    14.May.1993:       REP     added normalization of spot
                                               cross correlation by test spot
                                               autocorrelation
   ================================================================================
        Input Variables
           Data           - (float *)  input data array
           n              - (int)      size of input array ( n by n )
           Exponent_Table - (float) *) array of precalculated exponential values
                                       (see lvsh_refined_table )
           Better_Accuracy- (float)    desired centroid location accuracy
                                       (in pixels)  (1 over power of 2 !)
                                       (see lvsh_refined_table )
   ================================================================================
        Output Variables
           Xpos       - (float *)    row position of spot relative to array center
           Ypos       - (float *)    col position of spot relative to array center
   ================================================================================
        Local Variables/Constants
           Const       - (float)   -1/(2*Sigma*Sigma) constant for gaussian
           MaxSum      - (float)   maximum cross-correlation
           Pixel       - (float *) pointer to next data point in input array
           Step        - (float)   resolution of centroid determination
                                   in current run
           Sum         - (float)   current cross-correlation
           Xmax,Ymax   - (float)   location of maximum cross-correlation
           Xoff,Yoff   - (float)   current test centroid location
   ================================================================================
        External Variables/Constants
   ================================================================================
        Intrinsic Functions Called:
   ================================================================================
        External Subprograms Called:
   ================================================================================
        External Declarations:  */
   /* =============================================================================
        Local Parameters/Constants Declarations: */
   /* =============================================================================
        Local Variables Declarations: */

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

   /*
   *** the "Indexer" variable is the scale factor
   *** which coverts a pixel offset into an index
   *** in the precalculated Exponent_Table
   */

   Indexer = (int)(1.0 / Better_Accuracy);
   f_Indexer = (float)Indexer;

   /*
   *** start at array center
   */

   *Xpos = 0.0;
   *Ypos = 0.0;

   /*
   *** Start with a STEP of 1.0 pixels -- i.e.
   *** evaluate the cross correlation at the
   *** following four points:
   ***
   ***           (-1/2,-1/2)    (-1/2,+1/2)
   ***           (+1/2,-1/2)    (+1/2,+1/2)
   ***
   *** relative to Xpos,Ypos (array center).
   ***
   *** Reset Xpos,Ypos to the position which
   *** showed the maximum correlation.
   ***
   *** On each succeeding loop, we cut the
   *** STEP by half and evaluate the cross
   *** correlation at four more points:
   ***
   ***     (-step/2,-step/2)    (-step/2,+step/2)
   ***     (+step/2,-step/2)    (+step/2,+step/2)
   ***
   *** relative to Xpos, Ypos.
   ***
   *** In other words, the centroid position will
   *** move by up to 1/2 pixel in the first iteration,
   *** up to 1/4 pixel in the second iteration,
   *** 1/8 in the third, etc.  Assuming that the
   *** cross-correlation is a relatively smooth
   *** spatial function over a few pixels, Xpos and
   *** Ypos will converge to the position of the
   *** maximum cross-correlation.
   ***
   */

   Start = (float)(-(int)n / 2.);
   Stop = (float)((int)n - (int)n / 2.);

   for (Step = 0.5; Step >= Better_Accuracy; Step /= 2.0)
   {
      MaxSum = 0.0;
      Xoff = *Xpos;
      Yoff = *Ypos;
      Index_Imin = (int)(f_Indexer * (Start - Xoff));
      Index_Imax = (int)(f_Indexer * (Stop - Xoff));
      Index_Jmin = (int)(f_Indexer * (Start - Yoff));
      Index_Jmax = (int)(f_Indexer * (Stop - Yoff));
      AutoCorr = 0.0;
      CrossCorr = 0.0;
      Pixel = Data;
      for (i = Index_Imin; i < Index_Imax; i += Indexer)
      {
         Exp_i = *(Exponent_Table + (int)ABS(i));
         for (j = Index_Jmin; j < Index_Jmax; j += Indexer)
         {
            Func = Exp_i * *(Exponent_Table + (int)ABS(j));
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

      for (Xoff = *Xpos - Step; Xoff <= *Xpos + Step; Xoff += (2. * Step))
      {
         Index_Imin = (int)(f_Indexer * (Start - Xoff));
         Index_Imax = (int)(f_Indexer * (Stop - Xoff));

         for (Yoff = *Ypos - Step; Yoff <= *Ypos + Step; Yoff += (2. * Step))
         {
            Index_Jmin = (int)(f_Indexer * (Start - Yoff));
            Index_Jmax = (int)(f_Indexer * (Stop - Yoff));

            /*
            *** calculate correlation at current postion
            */

            AutoCorr = 0.0;
            CrossCorr = 0.0;
            Pixel = Data;
            for (i = Index_Imin; i < Index_Imax; i += Indexer)
            {
               Exp_i = *(Exponent_Table + (int)ABS(i));
               for (j = Index_Jmin; j < Index_Jmax; j += Indexer)
               {
                  Func = Exp_i * *(Exponent_Table + (int)ABS(j));
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

   return (1);
}

//  #include "vinclude.h"

//  #include <math.h>

int lvsh_refined_table(n, Sigma, Accuracy, Exponent_Table, Better_Accuracy)

unsigned long n;
float Sigma;
float Accuracy;
float **Exponent_Table;
// float *Exponent_Table;
float *Better_Accuracy;
{

   /*
   ================================================================================
        Name:         lvsh_refined_table
        Date:         20.April.1992
        Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
        Environment:  SUN
        Description:  precalculates exponentials for all possible
                      gaussian spot offsets for the refined
                      centroid location algorithm. (see lvsh_refined)


        Revision    Description
               0    20.Apr.1992     R.E.P.     initial version
               1    14.May.1993     R.E.P.     normalized for the power
                                               in the gaussian test spot;
                                               this is essential for small
                                               test spots
   ================================================================================
        Input Variables
           n          - (int)        size of input array ( n by n )
           Sigma      - (float)      deviation of reference spot (in pixels)
                                     (this is the spot halfwidth)
           Accuracy   - (float)      desired centroid location accuracy
                                     (in pixels)  must be less than 1.0
   ================================================================================
        Output Variables
           Exponent_Table  - (float **)  array of calculated exponentials
           Better_Accuracy - (float **)  revised centroid location accuracy
                                         ( 1 over a power of 2 )
   ================================================================================
        Local Variables/Constants
           Const       - (float)   -1/(2*Sigma*Sigma) constant for gaussian
   ================================================================================
        External Variables/Constants
   ================================================================================
        Intrinsic Functions Called:
   ================================================================================
        External Subprograms Called:
   ================================================================================
        External Declarations:  */
   /* =============================================================================
        Local Parameters/Constants Declarations: */
   /* =============================================================================
        Local Variables Declarations: */

   float Const;
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
   *Better_Accuracy = pow(2., ((float)(int)(-1.0 +
                                            log(Accuracy) / log(2.))));

   /*
   *** The form of the gaussian is determined
   *** by the exponentiation constant 1/(2*Sigma*Sigma).
   *** The scale of the gaussian is unimportant
   *** for our purposes and is ignored.
   ***
   ***               2
   ***             -r /(2*Sigma*Sigma)
   *** Gaussian = e
   ***
   ***     where:   r is the pixel distance to
   **               gaussian center
   */

   Const = -1.0 / (2.0 * Sigma * Sigma);

   /*
   *** convert constant to indexing for output array
   */

   Const *= ((*Better_Accuracy) * (*Better_Accuracy));

   /*
   *** the number of exponential we need to
   *** precalculate is equal to the number of
   *** possible centroid positions within a
   *** pixel (1/Accuracy) times the total number
   *** of pixels across the subbox array (n)
   *** divided by 2 (to account for the fact
   *** that the centroid MUST be within one
   *** pixel of the center of the subarray
   */

   //   Limit = (int) (( (int) n+1) / ( 2.*(*Better_Accuracy) ) );
   Limit = 1 + (int)(((int)n + 3) / (2. * (*Better_Accuracy)));

   *Exponent_Table = (float *)malloc(Limit * sizeof(float));
   if (*Exponent_Table == NULL)
   {
      fprintf(stderr, "::: lvsh_refined_table ::: could not allocate memory! \n");
      return (0);
   }

   /*
   *** fill the table
   */

   Exponent = *Exponent_Table;
   for (i = 0; i < Limit; i++)
   {
      isq = ((unsigned long)i * (unsigned long)i);
      *Exponent++ = exp(Const * (float)isq);
   }

   return (1);
}

//  #include "vinclude.h"

//  #include <math.h>

int lvsh_autosigma(Data, n, Accuracy, Sigma, Size)

unsigned long *Data;
unsigned long n;
float Accuracy;
float *Sigma;
int *Size;
{

   /*
   ================================================================================
        Name:         lvsh_autosigma
        Date:         26.February.1992
        Programmer:   R.E.Pierson / APPLIED TECHNOLOGY ASSOCIATES
        Environment:  SUN
        Description:  attempts to calculate the optimal half-width
                      (sigma) and subbox size for a data spot.  The
                      data passed in is a cross-section through the
                      spot centered on the spot's brightest pixel.

                      This routine searches for a maximum cross-
                      correlation between the input data and a
                      gaussian curve with a deviation of SIGMA.
                      SIGMA.  The cross-correlation is calculated
                      at increasingly fine spatial steps until
                      the desired ACCURACY (in pixels) is achieved.


        Revision    Description
               0    26.Feb.1992     REP     initial version
               1    14.May.1993     REP     corrected 1/(Sigma*Sigma) to
                                            1/(2*Sigma*Sigma) for gaussians
   ================================================================================
        Input Variables
           Data       - (float *)    input data array
           n          - (int)        length of input vector
           Accuracy   - (float)      desired centroid location accuracy
                                     (in pixels)  must be less than 1.0
   ================================================================================
        Output Variables
           Sigma      - (float)      spot half width
           Size       - (int)        optimal subbox size
   ================================================================================
        Local Variables/Constants
           Const          - (float)   -1/Sigma**2 constant for normal curve
           Func           - (float)   value of normal curve at current pixel
           MaxSum         - (float)   maximum cross-correlation
           Pixel          - (float *) pointer to next data point in input array
           Step           - (float)   resolution of sigma in current run
           Sum_AutoCorr   - (float)   current autocorrelation of normal curve
           Sum_CrossCorr  - (float)   current cross-correlation
           Sig            - (float)   current test value of Sigma
   ================================================================================
        External Variables/Constants
   ================================================================================
        Intrinsic Functions Called:
   ================================================================================
        External Subprograms Called:
   ================================================================================
        External Declarations:  */
   /* =============================================================================
        Local Parameters/Constants Declarations: */
   /* =============================================================================
        Local Variables Declarations: */

   int i;
   unsigned long isq;
   float MaxSum;
   float New_Sigma;
   float Step;
   float Sig;

   register float Const;
   register float Func;
   register float Sum_AutoCorr;
   unsigned long *Pixel;
   register float Sum_CrossCorr;

   /* ========================================================================== */

   /*
   *** The form of the gaussian is determined
   *** by the exponentiation constant 1/sqrt(Sigma).
   *** The scale of the gaussian is unimportant
   *** (it is normalized out in lvsh_refined)
   ***
   ***                2
   ***             -(x )/(2*Sigma*Sigma)
   *** Gaussian = e
   ***
   ***     where:   x and is the pixel position
   ***              relative to brightest pixel
   ***              (center of the array)
   ***
   */

   /*
   *** start with initial guess of SIGMA
   *** one eigth the input vector length
   */

   *Sigma = ((float)n) / 8;

   /*
   *** Start with a STEP of n/16 pixels -- i.e.
   *** evaluate the cross correlation at
   *** SIGMA values of :
   ***
   ***           (n/8-n/16)      (n/8+n/16)
   ***
   *** Reset New_Sigma to the position which
   *** showed the maximum correlation.
   ***
   *** On each succeeding loop, we cut the
   *** STEP by half and evaluate the cross
   *** correlation at two more points:
   ***
   ***   (New_Sigma-step/2)  (New_Sigma+step/2)
   ***
   *** In other words, SIGMA will move by
   *** up to n/16 pixels in the first iteration,
   *** up to n/32 pixels in the second iteration,
   *** n/64 in the third, etc.  Assuming that the
   *** cross-correlation is a relatively smooth
   *** spatial function, New_Sigma will converge
   *** to the best possible SIGMA value.
   ***
   */

   /*
            Pixel = Data;
            for ( i=-n/2 ; i<(n-n/2) ; i++ )
               printf ( "i = %d   Sub = %f \n", i, *Pixel++ );
   */

   printf(" In Autosigma  \n");
   //   printf("The value of n is %u: \n",n);
   for (Step = ((float)n) / 16; Step > Accuracy / 2.0; Step /= 2.0)
   {
      MaxSum = 0.0;

      printf(" Step = %f   Accuracy = %f \n ", Step, Accuracy);
      //      getch();
      /* compute best SIGMA for the present step */

      for (Sig = *Sigma - Step; Sig <= *Sigma + Step; Sig += Step)
      {
         Const = -1.0 / (2.0 * Sig * Sig);
         Sum_AutoCorr = 0.0;
         Sum_CrossCorr = 0.0;
         Pixel = Data;
         for (i = -(int)(n / 2); i < (int)(n - n / 2); i++)
         {
            isq = (unsigned long)i * (unsigned long)i;
            Func = exp(Const * (float)(isq));
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

   /*
      printf ( "Sigma = %f   Subbox = %d\n", *Sigma, *Size );
   */

   return (1);
}
