
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
#include "../../src//kernel_common.h"
#include "../../src/kernels.h"
#include "declarations.h"

//subroutines
#include "misc.inc"
#include "activity.inc"



void render() {

  //the cell Vm arrays
  cudaGraphicsMapResources(1, &I_layer.cuda_pbo_resource[0], 0);     cudaGraphicsResourceGetMappedPointer((void **)&I_layer.vm,     NULL, I_layer.cuda_pbo_resource[0]);
  cudaGraphicsMapResources(1, &A_layers[1].cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&A_layers[1].vm, NULL, A_layers[1].cuda_pbo_resource[0]);
  cudaGraphicsMapResources(1, &A_layers[2].cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&A_layers[2].vm, NULL, A_layers[2].cuda_pbo_resource[0]);
  cudaGraphicsMapResources(1, &A_layers[3].cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&A_layers[3].vm, NULL, A_layers[3].cuda_pbo_resource[0]);
  cudaGraphicsMapResources(1, &A_layers[4].cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&A_layers[4].vm, NULL, A_layers[4].cuda_pbo_resource[0]);
  cudaGraphicsMapResources(1, &A_layers[5].cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&A_layers[5].vm, NULL, A_layers[5].cuda_pbo_resource[0]);

  cudaGraphicsMapResources(1, &B_layers[1].cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&B_layers[1].vm, NULL, B_layers[1].cuda_pbo_resource[0]);
  cudaGraphicsMapResources(1, &B_layers[2].cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&B_layers[2].vm, NULL, B_layers[2].cuda_pbo_resource[0]);
  cudaGraphicsMapResources(1, &B_layers[3].cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&B_layers[3].vm, NULL, B_layers[3].cuda_pbo_resource[0]);
  cudaGraphicsMapResources(1, &B_layers[4].cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&B_layers[4].vm, NULL, B_layers[4].cuda_pbo_resource[0]);
  cudaGraphicsMapResources(1, &B_layers[5].cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&B_layers[5].vm, NULL, B_layers[5].cuda_pbo_resource[0]);



  if ( pause_animation == 0 ) {
      for (int i = 0; i < DTS_PER_RENDER; ++i) {   //Each pass through the loop is one render every DTS_PER_RENDER DTs
						   
        synapseKernelLauncher_a (syn_stream[0], syn_a1, I_cellLayer,  A_cellLayer1, syn_I_A1,  weights_a1, BIG,   B2B, syn_switches1, RADIUS );  
        synapseKernelLauncher_a (syn_stream[1], syn_a2, A_cellLayer1, A_cellLayer2, syn_A1_A2, weights_a2, BIG,   B2B, syn_switches2, RADIUS );   
        synapseKernelLauncher_a (syn_stream[2], syn_a3, A_cellLayer2, A_cellLayer3, syn_A2_A3, weights_a3, BIG,   B2B, syn_switches2, RADIUS );   
        synapseKernelLauncher_a (syn_stream[3], syn_a4, A_cellLayer3, A_cellLayer4, syn_A3_A4, weights_a4, BIG,   B2B, syn_switches2, RADIUS );   
        synapseKernelLauncher_a (syn_stream[5], syn_a5, A_cellLayer4, A_cellLayer5, syn_A4_A5, weights_a5, BIG,   B2B, syn_switches2, RADIUS );   

        synapseKernelLauncher_a (syn_stream[6], syn_b2s1, A_cellLayer1, B_cellLayer1, syn_A1_B1, weights_b1, SMALL, B2S, syn_switches1, RADIUS );   //B2S takes the postsynaptic array size
        synapseKernelLauncher_a (syn_stream[7], syn_b2s2, A_cellLayer2, B_cellLayer2, syn_A2_B2, weights_b2, SMALL, B2S, syn_switches1, RADIUS );   //B2S takes the postsynaptic array size
        synapseKernelLauncher_a (syn_stream[8], syn_b2s3, A_cellLayer3, B_cellLayer3, syn_A3_B3, weights_b3, SMALL, B2S, syn_switches1, RADIUS );   //B2S takes the postsynaptic array size
        synapseKernelLauncher_a (syn_stream[9], syn_b2s4, A_cellLayer4, B_cellLayer4, syn_A4_B4, weights_b4, SMALL, B2S, syn_switches1, RADIUS );   //B2S takes the postsynaptic array size
        synapseKernelLauncher_a (syn_stream[10], syn_b2s5, A_cellLayer5, B_cellLayer5, syn_A5_B5, weights_b5, SMALL, B2S, syn_switches1, RADIUS );   //B2S takes the postsynaptic array size

        synapseKernelLauncher_a (syn_stream[11],syn_s2b1, B_cellLayer1, A_cellLayer1, syn_B1_A2, weights_c1, SMALL, S2B, syn_switches1, RADIUS );   //S2B takes the presynaptic array size
        synapseKernelLauncher_a (syn_stream[12],syn_s2b2, B_cellLayer2, A_cellLayer2, syn_B2_A3, weights_c2, SMALL, S2B, syn_switches1, RADIUS );   //S2B takes the presynaptic array size
        synapseKernelLauncher_a (syn_stream[13],syn_s2b3, B_cellLayer3, A_cellLayer3, syn_B3_A4, weights_c3, SMALL, S2B, syn_switches1, RADIUS );   //S2B takes the presynaptic array size
        synapseKernelLauncher_a (syn_stream[14],syn_s2b4, B_cellLayer4, A_cellLayer4, syn_B4_A5, weights_c4, SMALL, S2B, syn_switches1, RADIUS );   //S2B takes the presynaptic array size
        synapseKernelLauncher_a (syn_stream[15],syn_s2b5, B_cellLayer5, A_cellLayer5, syn_B5_A6, weights_c5, SMALL, S2B, syn_switches1, RADIUS );   //S2B takes the presynaptic array size
																		  
	//sync so that all the synapse kernel launches are forced to complete before cell kernels start to launch
        cudaDeviceSynchronize();
												      
        cellKernelLauncher_aelif(cell_stream[0], I_layer.vm,       I_cellLayer,  BIG,    cell_switchesi,  layer1_params ); 
        cellKernelLauncher_aelif(cell_stream[1], A_layers[1].vm,   A_cellLayer1, BIG,    cell_switches1,  layer1_params ); 
        cellKernelLauncher_aelif(cell_stream[2], A_layers[2].vm,   A_cellLayer2, BIG,    cell_switches1,  layer1_params ); 
        cellKernelLauncher_aelif(cell_stream[3], A_layers[3].vm,   A_cellLayer3, BIG,    cell_switches1,  layer1_params ); 
        cellKernelLauncher_aelif(cell_stream[4], A_layers[4].vm,   A_cellLayer4, BIG,    cell_switches1,  layer1_params ); 
        cellKernelLauncher_aelif(cell_stream[5], A_layers[5].vm,   A_cellLayer5, BIG,    cell_switches1,  layer1_params ); 

        cellKernelLauncher_aelif(cell_stream[6], B_layers[1].vm,   B_cellLayer1, SMALL,  cell_switches1,  layer1_params ); 
        cellKernelLauncher_aelif(cell_stream[7], B_layers[2].vm,   B_cellLayer2, SMALL,  cell_switches1,  layer1_params ); 
        cellKernelLauncher_aelif(cell_stream[8], B_layers[3].vm,   B_cellLayer3, SMALL,  cell_switches1,  layer1_params ); 
        cellKernelLauncher_aelif(cell_stream[9], B_layers[4].vm,   B_cellLayer4, SMALL,  cell_switches1,  layer1_params ); 
        cellKernelLauncher_aelif(cell_stream[5], B_layers[5].vm,   B_cellLayer5, SMALL,  cell_switches1,  layer1_params ); 
	
	//sync so that all the cell kernel launches are forced to complete before synapse kernels start to launch
        cudaDeviceSynchronize();
      }
  }
  cudaGraphicsUnmapResources(1, &I_layer.cuda_pbo_resource[0],     0);
  cudaGraphicsUnmapResources(1, &A_layers[1].cuda_pbo_resource[0], 0);
  cudaGraphicsUnmapResources(1, &A_layers[2].cuda_pbo_resource[0], 0);
  cudaGraphicsUnmapResources(1, &A_layers[3].cuda_pbo_resource[0], 0);
  cudaGraphicsUnmapResources(1, &A_layers[4].cuda_pbo_resource[0], 0);

  cudaGraphicsUnmapResources(1, &B_layers[1].cuda_pbo_resource[0], 0);
  cudaGraphicsUnmapResources(1, &B_layers[2].cuda_pbo_resource[0], 0);
  cudaGraphicsUnmapResources(1, &B_layers[3].cuda_pbo_resource[0], 0);
  cudaGraphicsUnmapResources(1, &B_layers[4].cuda_pbo_resource[0], 0);
  cudaGraphicsUnmapResources(1, &B_layers[5].cuda_pbo_resource[0], 0);

}

void display() {
  char title[128];

  render();

  glutSetWindow(    I_layer.win[0]);  sprintf(title, "%2.3f Seconds", SimTime); draw_texture( BIG, BIG); glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers(); //this one is necessary, can't be commented since not display_nul
  glutSetWindow(A_layers[1].win[0]);  sprintf(title, "A" ); draw_texture( BIG, BIG);     glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers(); 
  glutSetWindow(A_layers[2].win[0]);  sprintf(title, "A" ); draw_texture( BIG, BIG);     glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers(); 
  glutSetWindow(A_layers[3].win[0]);  sprintf(title, "C" ); draw_texture( BIG, BIG);     glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers();
  glutSetWindow(A_layers[4].win[0]);  sprintf(title, "D" ); draw_texture( BIG, BIG);     glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers();
  glutSetWindow(A_layers[5].win[0]);  sprintf(title, "D" ); draw_texture( BIG, BIG);     glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers();

  glutSetWindow(B_layers[1].win[0]);  sprintf(title, "B" ); draw_texture( SMALL, SMALL); glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers();
  glutSetWindow(B_layers[2].win[0]);  sprintf(title, "B" ); draw_texture( SMALL, SMALL); glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers();
  glutSetWindow(B_layers[3].win[0]);  sprintf(title, "B" ); draw_texture( SMALL, SMALL); glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers();
  glutSetWindow(B_layers[4].win[0]);  sprintf(title, "B" ); draw_texture( SMALL, SMALL); glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers();
  glutSetWindow(B_layers[5].win[0]);  sprintf(title, "B" ); draw_texture( SMALL, SMALL); glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers();


}

int main(int argc, char** argv) {
  for (int i=0; i < 50; i++)cudaStreamCreate (&syn_stream[i]);
  for (int i=0; i < 50; i++)cudaStreamCreate (&cell_stream[i]);

  cell_switchesi.en_input = 1;
  cell_switchesi.loc = {150, 150};

  cell_switches1.en_noise = false;
  cell_switches1.en_theta = false;
  syn_switches1.en_learn = false;
  syn_switches2.en_learn = true;

  cudaMalloc((void **)&I_cellLayer,   BIG*BIG*    sizeof( lif_data_type ));
  cudaMalloc((void **)&A_cellLayer1,  BIG*BIG*    sizeof( lif_data_type ));
  cudaMalloc((void **)&A_cellLayer2,  BIG*BIG*    sizeof( lif_data_type ));
  cudaMalloc((void **)&A_cellLayer3,  BIG*BIG*    sizeof( lif_data_type ));
  cudaMalloc((void **)&A_cellLayer4,  BIG*BIG*    sizeof( lif_data_type ));
  cudaMalloc((void **)&A_cellLayer5,  BIG*BIG*    sizeof( lif_data_type ));

  cudaMalloc((void **)&B_cellLayer1,  SMALL*SMALL*sizeof( lif_data_type ));
  cudaMalloc((void **)&B_cellLayer2,  SMALL*SMALL*sizeof( lif_data_type ));
  cudaMalloc((void **)&B_cellLayer3,  SMALL*SMALL*sizeof( lif_data_type ));
  cudaMalloc((void **)&B_cellLayer4,  SMALL*SMALL*sizeof( lif_data_type ));
  cudaMalloc((void **)&B_cellLayer5,  SMALL*SMALL*sizeof( lif_data_type ));

  cudaMalloc((void **)&syn_I_A1,  BIG*BIG*    sizeof( synapse_data_type ));   
  cudaMalloc((void **)&syn_A1_A2, BIG*BIG*    sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_A2_A3, BIG*BIG*    sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_A3_A4, BIG*BIG*    sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_A4_A5, BIG*BIG*    sizeof( synapse_data_type ));  

  cudaMalloc((void **)&syn_A1_B1, SMALL*SMALL*sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_A2_B2, SMALL*SMALL*sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_A3_B3, SMALL*SMALL*sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_A4_B4, SMALL*SMALL*sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_A5_B5, SMALL*SMALL*sizeof( synapse_data_type ));  

  cudaMalloc((void **)&syn_B1_A2, BIG*BIG*    sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_B2_A3, BIG*BIG*    sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_B3_A4, BIG*BIG*    sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_B4_A5, BIG*BIG*    sizeof( synapse_data_type ));  
  cudaMalloc((void **)&syn_B5_A6, BIG*BIG*    sizeof( synapse_data_type ));  


  //synaptic currents
  cudaMalloc((void **)&syn_a1 , BIG*BIG*    (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&syn_a2 , BIG*BIG*    (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&syn_a3 , BIG*BIG*    (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&syn_a4 , BIG*BIG*    (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&syn_a5 , BIG*BIG*    (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));

  cudaMalloc((void **)&syn_b2s1 , SMALL*SMALL*(2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&syn_b2s2 , SMALL*SMALL*(2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&syn_b2s3 , SMALL*SMALL*(2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&syn_b2s4 , SMALL*SMALL*(2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&syn_b2s5 , SMALL*SMALL*(2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));

  cudaMalloc((void **)&syn_s2b1 , SMALL*SMALL *sizeof( float )); //S2B layer has one synaptic current for each presynaptic cell in SMALL layer, which get fanned out to the post-synaptic BIG cell array
  cudaMalloc((void **)&syn_s2b2 , SMALL*SMALL *sizeof( float )); //S2B layer has one synaptic current for each presynaptic cell in SMALL layer, which get fanned out to the post-synaptic BIG cell array
  cudaMalloc((void **)&syn_s2b3 , SMALL*SMALL *sizeof( float )); //S2B layer has one synaptic current for each presynaptic cell in SMALL layer, which get fanned out to the post-synaptic BIG cell array
  cudaMalloc((void **)&syn_s2b4 , SMALL*SMALL *sizeof( float )); //S2B layer has one synaptic current for each presynaptic cell in SMALL layer, which get fanned out to the post-synaptic BIG cell array
  cudaMalloc((void **)&syn_s2b5 , SMALL*SMALL *sizeof( float )); //S2B layer has one synaptic current for each presynaptic cell in SMALL layer, which get fanned out to the post-synaptic BIG cell array

  //weight matrices
  //for weight-sharing, the arrays are (2r+1)^2
  //when learning is enabled, the arrays are (cells-in-layer)*(2r+1)^2, so that there is an individual value for each synapse
  cudaMalloc((void **)&weights_a1 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_a2 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_a3 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_a4 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float )); 
  cudaMalloc((void **)&weights_a5 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float )); 

  cudaMalloc((void **)&weights_b1 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_b2 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_b3 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_b4 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_b5 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));

  cudaMalloc((void **)&weights_c1 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_c2 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_c3 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_c4 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));
  cudaMalloc((void **)&weights_c5 , (2*RADIUS+1) * (2*RADIUS+1) *sizeof( float ));

  char fileName[100];
  int retCode;
										      
  resetNetwork();

  printInstructions();
  initGLUT(&argc, argv); 
  glutIdleFunc(idle);

  //display must be called only once.  The others must call display_nul
 
  glutSetWindow(    I_layer.win[0]); glutKeyboardFunc(keyboard); initPixelBuffer(     &I_layer.pbo[0],     &I_layer.tex[0],     &I_layer.cuda_pbo_resource[0], BIG );  gluOrtho2D(0, BIG, BIG, 0);    glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display);
  glutSetWindow(A_layers[1].win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &A_layers[1].pbo[0], &A_layers[1].tex[0], &A_layers[1].cuda_pbo_resource[0], BIG );  gluOrtho2D(0, BIG, BIG, 0);    glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display_nul);
  glutSetWindow(A_layers[2].win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &A_layers[2].pbo[0], &A_layers[2].tex[0], &A_layers[2].cuda_pbo_resource[0], BIG );  gluOrtho2D(0, BIG, BIG, 0);    glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display_nul);
  glutSetWindow(A_layers[3].win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &A_layers[3].pbo[0], &A_layers[3].tex[0], &A_layers[3].cuda_pbo_resource[0], BIG );  gluOrtho2D(0, BIG, BIG, 0);    glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display_nul);
  glutSetWindow(A_layers[4].win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &A_layers[4].pbo[0], &A_layers[4].tex[0], &A_layers[4].cuda_pbo_resource[0], BIG );  gluOrtho2D(0, BIG, BIG, 0);    glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display_nul);
  glutSetWindow(A_layers[5].win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &A_layers[5].pbo[0], &A_layers[5].tex[0], &A_layers[5].cuda_pbo_resource[0], BIG );  gluOrtho2D(0, BIG, BIG, 0);    glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display_nul);

  glutSetWindow(B_layers[1].win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &B_layers[1].pbo[0], &B_layers[1].tex[0], &B_layers[1].cuda_pbo_resource[0], SMALL );gluOrtho2D(0, SMALL, SMALL, 0);glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display_nul);
  glutSetWindow(B_layers[2].win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &B_layers[2].pbo[0], &B_layers[2].tex[0], &B_layers[2].cuda_pbo_resource[0], SMALL );gluOrtho2D(0, SMALL, SMALL, 0);glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display_nul);
  glutSetWindow(B_layers[3].win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &B_layers[3].pbo[0], &B_layers[3].tex[0], &B_layers[3].cuda_pbo_resource[0], SMALL );gluOrtho2D(0, SMALL, SMALL, 0);glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display_nul);
  glutSetWindow(B_layers[4].win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &B_layers[4].pbo[0], &B_layers[4].tex[0], &B_layers[4].cuda_pbo_resource[0], SMALL );gluOrtho2D(0, SMALL, SMALL, 0);glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display_nul);
  glutSetWindow(B_layers[5].win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &B_layers[5].pbo[0], &B_layers[5].tex[0], &B_layers[5].cuda_pbo_resource[0], SMALL );gluOrtho2D(0, SMALL, SMALL, 0);glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc(display_nul);

  glutMainLoop();
  atexit(exitfunc);
  return 0;
}
