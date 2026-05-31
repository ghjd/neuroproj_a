#ifndef GEN_PATS_H
#define GEN_PATS_H
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define STIM 0.5e-9
#define LINE_WIDTH 1
#define LINE_LENGTH 23

void genPats( float * pattern, float * d_pattern, lif_data_type* cell_layer, int width, int sel, int2 loc  ) {  //cue:0  complete pattern  cue:1 apprporiate segment for cuing a memory
   //printf("[INFO]GenPats Width=%d\tSel=%d\tCue=%s\n", width, sel, cue ? "true" : "false" );
   int row, col, pat;
   int x0, y0, P, R;
   int x1, y1, x2, y2, run;
   int xz, yz;  //use these as the root location for a drawing
   float slope ;
   int inRange, onLine, rise, yPrime;
     for( row = 0; row < width; row++){
       for( col = 0; col < width; col++){
	 //two commonly used coordinates, anchored at the center of the array
	 int x = row - width/2;
	 int y = col - width/2;
	 pattern[width* row + col] = 0.0; //initialize to zero, set as necessary below
	 //LINE_WIDTH is narrow access
	 //LINE_LENGTH is long access
         switch ( sel ) {
           case 0:  //empty
	       break;
           case 1:  //movable spot
               x0 = x + loc.y; y0 = y - loc.x; R = 8;//funny math here puts (0,0) at the center, increasing X to the right, increasing Y upwards
               P = int( sqrt( x0 * x0  + y0 * y0 ));
               if ( P <= R) { pattern[width* row + col] = STIM; }
               break;
           case 2:  //movable horizontal line segment
               x0 = x + loc.y; y0 = y - loc.x; R = 10;
               if (( y0 >= -R && y0 <= R) && (x0 >= -1 && x0 <= 0)) { pattern[width* row + col] = STIM; }
               break;
           case 3:  //movable vertical line segment
               x0 = x + loc.y; y0 = y - loc.x; R = 10;
               if (( x0 >= -R && x0 <= R) && (y0 >= -1 && y0 <= 0)) { pattern[width* row + col] = STIM; }
               break;
           case 4:  //movable / line segment
               xz = -loc.y; yz = loc.x; R=10;
               x0 = xz-R; x1 = xz+R; y0 = yz+R; y1 = yz-R; //upward diag
               slope = (float)(y1 - y0)/(float)(x1-x0); inRange = (x >= x0 & x <= x1) & (y >= y1 & y <= y0) ? 1 : 0;
               yPrime = y0 + (x-x0)*slope; onLine = y >= yPrime-1 & y <= yPrime+1 ? 1 : 0;
               if (inRange & onLine) { pattern[width* row + col] = STIM; }
               break;
           case 5:  //movable \ line segment
               xz = -loc.y; yz = loc.x; R=10;
               x0 = xz-R; x1 = xz+R; y0 = yz-R; y1 = yz+R; //downward diag
               slope = (float)(y1 - y0)/(float)(x1-x0); inRange = (x >= x0 & x <= x1) & (y <= y1 & y >= y0) ? 1 : 0;
               yPrime = y0 + (x-x0)*slope; onLine = y >= yPrime-1 & y <= yPrime+1 ? 1 : 0;
               if (inRange & onLine) { pattern[width* row + col] = STIM; }
               break;
           case 6:  //horizontal sin wave
               if ((abs(x + 3*cos( 2*PI * (float)col/10) + PI/4.1 + loc.y)) < LINE_WIDTH && (abs(y-loc.x) < LINE_LENGTH )) { pattern[width* row + col] = STIM; }
               break;
           case 7:  //vertical sin wave
               if ((abs(y + 3*cos( 2*PI * (float)row/10) + PI/4.1 - loc.x)) < LINE_WIDTH && (abs(x + loc.y) < LINE_LENGTH )) { pattern[width* row + col] = STIM; }
               break;

           default :
	       break;
	 }//case sel
       }//for col
     }//for row
     cudaMemcpy ( d_pattern, pattern, width*width*sizeof( float ), cudaMemcpyHostToDevice );
     I_loadPat( cell_layer, width, d_pattern);
}


#endif
