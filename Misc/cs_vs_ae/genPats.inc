#ifndef GEN_PATS_H
#define GEN_PATS_H
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define STIM 0.5e-9

void genPats( float * pattern, float * d_pattern, lif_data_type* cell_layer, int width, int sel, bool cue  ) {  //cue:0  complete pattern  cue:1 apprporiate segment for cuing a memory
   //printf("[INFO]GenPats Width=%d\tSel=%d\tCue=%s\n", width, sel, cue ? "true" : "false" );
   int row, col, pat;
     for( row = 0; row < width; row++){
       for( col = 0; col < width; col++){
	 //two commonly used coordinates, anchored at the center of the array
	 int x = row - width/2;
	 int y = col - width/2;
	 pattern[width* row + col] = 0.0; //initialize to zero, set as necessary below
	 //BAR_WIDTH is narrow access
	 //BAR_HEIGHT is long access
         switch ( sel ) {
           case 0:  //empty
	       break;
           case 5:  //horizontal bar through h/2
	       if ( abs(x) < BAR_WIDTH && abs(y) < BAR_HEIGHT ) { pattern[width* row + col] = STIM; }
	       if ( cue ) { if ( col > 55 ) {  pattern[width* row + col] = 0; } }
	       break;
           case 6:  //vertical bar through w/2
	       if ( abs( col - width/2) < BAR_WIDTH && abs(row-width/2) < BAR_HEIGHT ) { pattern[width* row + col] = STIM; }
	       if ( cue ) { if ( row > 55 ) {  pattern[width* row + col] = 0; } }
	       break;
           case 7:  //diagonal \ bar
	         if ( abs(x - y) < BAR_WIDTH && abs(x) < BAR_HEIGHT ) { pattern[width* row + col] = STIM; }
	         if ( cue ) { if ( col > 55 ) { pattern[width* row + col] = 0; } }
	       break;
           case 8:  //diagonal / bar
	         if ( abs(x + y) < BAR_WIDTH && abs(x) < BAR_HEIGHT ) { pattern[width* row + col] = STIM; }
	         if ( cue ) { if ( col > 55 ) { pattern[width* row + col] = 0; } }
	       break;
           case 9:  //horizontal sin wave
	       if ((abs(x + 8*cos( 2*3.1415926 * (float)col/75) - 75)) < BAR_WIDTH && (abs(y) < BAR_HEIGHT )) { pattern[width* row + col] = STIM; }
	       if ((abs(x + 8*cos( 2*3.1415926 * (float)col/75) + 75)) < BAR_WIDTH && (abs(y) < BAR_HEIGHT )) { pattern[width* row + col] = STIM; }
	       if ( cue ) { if ( row < width/2 && col > 55 ) {  pattern[width* row + col] = 0; } }
	       if ( cue ) { if ( row > width/2 && col < 245 ) {  pattern[width* row + col] = 0; } }
	       break;
           case 10:  //vertical sin wave
	       if ((abs(y + 8*cos( 2*3.1415926 * (float)row/75) - 75)) < BAR_WIDTH && (abs(x) < BAR_HEIGHT )) { pattern[width* row + col] = STIM; }
	       if ((abs(y + 8*cos( 2*3.1415926 * (float)row/75) + 75)) < BAR_WIDTH && (abs(x) < BAR_HEIGHT )) { pattern[width* row + col] = STIM; }
	       if ( cue ) { if ( col < width/2 && row > 55 ) {  pattern[width* row + col] = 0; } }
	       if ( cue ) { if ( col > width/2 && row < 245 ) {  pattern[width* row + col] = 0; } }
	       break;
           case 11:  //short vertical line through center
	       if ( abs( col - width/2 - 4) < BAR_WIDTH && abs(row-width/2) < 50 ) { pattern[width* row + col] = STIM; }
	       if ( cue ) { if ( row > 12 ) {  pattern[width* row + col] = 0; } }
	       break;
           case 12:  //two short vertical lines 
	       if ( abs( col - width/2 - 20) < BAR_WIDTH && abs(row-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( abs( col - width/2 + 20) < BAR_WIDTH && abs(row-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( cue ) { if ( row > 12 ) {  pattern[width* row + col] = 0; } }
	       break;
           case 13:  //three short vertical lines 
	       if ( abs( col - width/2 - 30) < BAR_WIDTH && abs(row-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( abs( col - width/2 + 30)  < BAR_WIDTH && abs(row-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( abs( col - width/2 + 4)   < BAR_WIDTH && abs(row-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( cue ) { if ( row > 12 ) {  pattern[width* row + col] = 0; } }
	       break;
           case 14:  //short horizontal line through center
	       if ( abs( row - width/2 - 4) < BAR_WIDTH && abs(col-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( cue ) { if ( col > 12 ) {  pattern[width* row + col] = 0; } }
	       break;
           case 15:  //two short horizontal lines 
	       if ( abs( row - width/2 - 20)  < BAR_WIDTH && abs(col-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( abs( row - width/2 + 20)  < BAR_WIDTH && abs(col-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( cue ) { if ( col > 12 ) {  pattern[width* row + col] = 0; } }
	       break;
           case 16:  //three short horizontal lines 
	       if ( abs( row - width/2 - 30)  < BAR_WIDTH && abs(col-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( abs( row - width/2 + 30)  < BAR_WIDTH && abs(col-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( abs( row - width/2 + 4)   < BAR_WIDTH && abs(col-width/2) < 51 ) { pattern[width* row + col] = STIM; }
	       if ( cue ) { if ( col > 12 ) {  pattern[width* row + col] = 0; } }
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
