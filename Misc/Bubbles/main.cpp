
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //for sleep function
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <curand_kernel.h>
#include <math.h>


//declarations, etc
#include "./kernel_common.h"
#include "./kernels.h"
#include "./cpp_common.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define PI 3.1415926

int DTS_PER_RENDER = 20; 

//the aelif kernel uses this as an ouput channel to transmit its firing pattern
int*  d_firing_pat;

//used by genPats
float in_pat[BIG*BIG];
float*  d_in_pat;
float in_pat_i[BIG*BIG];
float*  d_in_pat_i;
float in_pat_j[SMALL*SMALL];
float*  d_in_pat_j;

#include "declarations.inc"


cudaStream_t cell_stream[20];
cudaStream_t syn_stream[20];


//project-specific subroutines
#include "genPats.inc"
#include "activity.inc"



void initGLUT(int *argc, char **argv ) {
  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
#include "windows.inc"
  glewInit();
}

void resetNetwork() {
  char fileName[100];
  int retCode;
#include "resets.inc"
}



void render() {

#include "glut1.inc"
  if ( pause_animation == 0 ) {
      for (int i = 0; i < DTS_PER_RENDER; ++i) {   //Each pass through the loop is one render every DTS_PER_RENDER DTs
#include "syn_launches.inc"
	//sync so that all the synapse kernel launches are forced to complete before cell kernels start to launch
        cudaDeviceSynchronize();
#include "cell_launches.inc"
	//sync so that all the cell kernel launches are forced to complete before synapse kernels start to launch
        cudaDeviceSynchronize();
      }
  }
#include "glut2.inc"
}


void display() {
  char title[128];
  render();
#include "glut3.inc"
}

int main(int argc, char** argv) {
  for (int i=0; i < 20; i++)cudaStreamCreate (&syn_stream[i]);
  for (int i=0; i < 20; i++)cudaStreamCreate (&cell_stream[i]);
 
#include "mallocs.inc"
  cudaMalloc((void **)&d_in_pat, BIG*BIG*sizeof( float ));
  cudaMalloc((void **)&d_in_pat_i, BIG*BIG*sizeof( float ));
  cudaMalloc((void **)&d_in_pat_j, SMALL*SMALL*sizeof( float ));


  char fileName[100];
  int retCode;
										      
  resetNetwork();
  initGLUT(&argc, argv); 
  glutIdleFunc(idle);

#include "glut4.inc"

  glutMainLoop();
  atexit(exitfunc);
  return 0;
}
