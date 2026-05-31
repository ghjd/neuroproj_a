#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //for sleep function
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <curand_kernel.h>
#include <math.h>


#include "kernel_common.h"


//the expected order of synapse parameters in the control file
//must match kernel.h cellParamList struct definition
char synParamList[NUM_CELL_PARAMS][100] = {
    "imax_syn",
    "tau_syn",
    "ws"
};

//the expected order of parameters in the control file
//must match kernel.h cellParamList struct definition
char cellParamList[NUM_CELL_PARAMS][100] = {
    "SEED",
     "Cm",
     "Vth",
     "delta_Vth",
     "tau_sra",
     "sra_a",
     "sra_b",
     "tau_gref",
     "delta_gref",
     "sigma_i_noise"
};



//read cell parameters from text file fileName
//load parameters into host-side data structure of type cellParams
int readCellParamsFromFile( char* fileName, cellParams* structPtr ) {
    FILE* ptr = fopen(fileName, "r");
    if (ptr == NULL) {
        printf("no such file.");
        return 0;
    }

    char paramName[100];
    float  value;

    int index = 0;
    char param[100];
    //struct cellParams *structPtr;
        while ( index < NUM_CELL_PARAMS ) {  //loop through each of the params
        //printf("\n[INFO][%d]filename %s parameter %s \n", index, fileName, cellParamList[index] );
        if (fscanf(ptr, "%s\t%f", param, &value) == 2){
                //printf("[INFO][%d]filename %s parameter %s line %s\n",index, fileName, cellParamList[index], param );
                if ( strcmp( param, cellParamList[index] ) != 0 ) {
                     printf("[ERROR]%s appears to be out of order for parameter %s, expecting %s\n",fileName, param, cellParamList[index] );
                     return 1;
                }
                switch( index ) {
                        //SEED is unsigned long int.  Everything else is float
                        case 0: { structPtr->SEED = (unsigned long int) value; break;}
                        case 1: { structPtr->Cm =  value; break;}
                        case 2: { structPtr->Vth =  value;  break;}
                        case 3: { structPtr->delta_Vth =  value; break;}
                        case 4: { structPtr->tau_sra =  value; break;}
                        case 5: { structPtr->sra_a =  value; break;}
                        case 6: { structPtr->sra_b =  value; break;}
                        case 7: { structPtr->tau_gref =  value; break;}
                        case 8: { structPtr->delta_gref =  value; break;}
                        case 9: { structPtr->sigma_i_noise =  value; break;}
                        default: ;
                }
        } else {
            printf("[ERROR]%s appears to have a problem with parameter %s\n",fileName, cellParamList[index] );
            return 1;
        }
        index++;
    }
    return 0;
}

int loadSynParams( char* fileName, synParams* structPtr ) {
    FILE* ptr = fopen( fileName, "r");
    if (ptr == NULL) {
        printf("[ERROR]loadSynParams unable to load %s: file not found.",fileName);
        return 0;
    }

    char paramName[100];
    float  value;

    int index = 0;
    char param[100];
    while ( index < NUM_SYN_PARAMS ) {  //loop through each of the params
        if (fscanf(ptr, "%s\t%f", param, &value) == 2){
                //printf("[INFO][%d]filename %s parameter %s line %s\n",index, fileName, synParamList[index], param );
                if ( strcmp( param, synParamList[index] ) != 0 ) {
                     printf("[ERROR]%s appears to be out of order for parameter %s, expecting %s\n",fileName, param, synParamList[index] );
                     return 1;
                }
                switch( index ) {
                        case 0: { structPtr->imax_syn = value; break;}
                        case 1: { structPtr->tau_syn = value;  break;}
                        default: ;
                }
        } else {
            printf("[ERROR]%s appears to have a problem with parameter %s\n",fileName, synParamList[index] );
            return 1;
        }
        index++;
    }
    return 0;
}

void display_nul() {}  //the work gets done in the primary display call.  This is a place-holder for all the other windows.


void initPixelBuffer( GLuint* pboPtr, GLuint* texPtr, cudaGraphicsResource** cuda_pbo_resourcePtr, int W) {
  glGenBuffers(1, pboPtr); glBindBuffer(GL_PIXEL_UNPACK_BUFFER, (GLuint) *pboPtr); glBufferData(GL_PIXEL_UNPACK_BUFFER, W*W*sizeof(GLubyte)* 4, 0, GL_STREAM_DRAW);
  glGenTextures(1, texPtr); glBindTexture(GL_TEXTURE_2D, *texPtr ); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  cudaGraphicsGLRegisterBuffer( cuda_pbo_resourcePtr, (GLuint) *pboPtr, cudaGraphicsMapFlagsWriteDiscard);
}

void exitfunc() {
}

void draw_texture( int w, int h) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
  glTexCoord2f(0.0f, 1.0f); glVertex2f(0, h);
  glTexCoord2f(1.0f, 1.0f); glVertex2f(w, h);
  glTexCoord2f(1.0f, 0.0f); glVertex2f(w, 0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}
