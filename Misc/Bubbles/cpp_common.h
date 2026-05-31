
#ifndef CPP_COMMON_H
#define CPP_COMMON_H

//the expected order of synapse parameters in the control file
//must match kernel.h cellParamList struct definition
extern char *synParamList;

//the expected order of parameters in the control file
//must match kernel.h cellParamList struct definition
extern char *cellParamList;

//host struct for loading cell parameters
#define NUM_CELL_PARAMS 10

//synapse ratio values to specify 1:1 mapping, big to small, or small to big
#define B2B 0
#define B2S 1
#define S2S 2
#define S2B 3

void display_nul() ;
void initPixelBuffer( GLuint* pboPtr, GLuint* texPtr, cudaGraphicsResource** cuda_pbo_resourcePtr, int W);
void exitfunc() ;
void draw_texture( int w, int h);

int readCellParamsFromFile( char* fileName, cellParams* structPtr );
int loadSynParams( char* fileName, synParams* structPtr );

#endif
