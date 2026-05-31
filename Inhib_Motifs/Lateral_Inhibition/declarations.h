
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "../../src/cpp_common.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define PI 3.1415926

int DTS_PER_RENDER = 5; //10: 17 seconds  1: 97seconds  6: 21 seconds

//the aelif kernel uses this as an ouput channel to transmit its firing pattern
int*  d_firing_pat;

//support variables associated with the network architecture
//potentially referenced by main,  activity, anything on the C side

cellParamStruct layer1_params;
synParamStruct  syn_params, syn_b2s_params, syn_s2b_params;

cell_switchesStruct cell_switchesi, cell_switches1, cell_switches2;
syn_switchesStruct syn_switches1, syn_switches2 ;  

layer I_layer;       //input layer
layer A_layers[100]; //center-surround path
layer B_layers[100]; //all-excitatory path

//These are cudaMalloced to create an array of lif_data_types
lif_data_type *I_cellLayer;    //input layer to A primary
			       
//the big layers
lif_data_type *A_cellLayer1;   // A1 primary cell layer (first spiking layer)
lif_data_type *A_cellLayer2;   
lif_data_type *A_cellLayer3;   
lif_data_type *A_cellLayer4;   
lif_data_type *A_cellLayer5;   

//the small layers
lif_data_type *B_cellLayer1;   
lif_data_type *B_cellLayer2;   
lif_data_type *B_cellLayer3;   
lif_data_type *B_cellLayer4;   
lif_data_type *B_cellLayer5;   

synapse_data_type *syn_I_A1;  //I onto A1
synapse_data_type *syn_A1_A2; //A1 onto A2
synapse_data_type *syn_A2_A3; //A2 onto A3
synapse_data_type *syn_A3_A4; //A3 onto A4
synapse_data_type *syn_A4_A5; //A3 onto A4

synapse_data_type *syn_A1_B1; 
synapse_data_type *syn_A2_B2; 
synapse_data_type *syn_A3_B3; 
synapse_data_type *syn_A4_B4; 
synapse_data_type *syn_A5_B5; 

synapse_data_type *syn_B1_A2; 
synapse_data_type *syn_B2_A3; 
synapse_data_type *syn_B3_A4; 
synapse_data_type *syn_B4_A5; 
synapse_data_type *syn_B5_A6; 

//individual synaptic currents
float *syn_a1, *syn_a2, *syn_a3, *syn_a4, *syn_a5;
float *syn_b2s1, *syn_b2s2, *syn_b2s3, *syn_b2s4, *syn_b2s5;
float *syn_s2b1, *syn_s2b2, *syn_s2b3, *syn_s2b4, *syn_s2b5;

//synaptic weights for each synapse group
float *weights_a1, *weights_a2, *weights_a3, *weights_a4, *weights_a5;
float *weights_b1, *weights_b2, *weights_b3, *weights_b4, *weights_b5;
float *weights_c1, *weights_c2, *weights_c3, *weights_c4, *weights_c5;

cudaStream_t cell_stream[50];
cudaStream_t syn_stream[50];

