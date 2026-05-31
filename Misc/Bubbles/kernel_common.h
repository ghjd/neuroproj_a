#ifndef KERNEL_COMMON_H
#define KERNEL_COMMON_H

#include <stdio.h>
#include <cuda.h>
#include <curand_kernel.h>
#include "cuda_fp16.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <curand_kernel.h>

#define ISTIM 0.6e-9

//define the two cell-array sizes that are supported
#define BIG 1000
#define SMALL 100

//defines the height of stimulus bar when in bar mode
#define BAR_HEIGHT  115
//the width comes out twice this
#define BAR_WIDTH 2

//this is the commonly used receptive field radius  //max 16
#define RADIUS 16

//threadblock dimensions  (8, 32) 14.99   (32,32):21.08
#define TX 32
#define TY 32

//receptive field types
#define R_Z    0
#define R_H    1
#define R_V    2
#define R_BS   3
#define R_FS   4
#define R_AE   5
#define R_CS   6
#define R_CS_B 7
#define R_121  8
#define R_NN   9


//constants
#define dT 1.0e-4
#define E_L -60.0e-3
//#define E_K -86.1e-3
//lower post-spike reset voltage results in better bursting behavior. Eventually separate reset voltage from E_K
#define E_K -75.0e-3
#define G_L 8.0e-9

//calcium t current related params  for bursting
#define G_T 15.0e-7
//H is the non-inactivated fraction of Ca_t channels.  Slow to rise (VM < V_H), fast to fall (Vm > V_H).
// dhdt = ( Vm > V_H ) ? -1.0 * lif_data[idx].d_h_t / H_TAU_F : ( (1.0 - lif_data[idx].d_h_t) / H_TAU_R); //Smith 2000 eqn 3
// set this so H_t is mostly charged up in the Theta time-scale, 10Hz.  Setting H_tau to 100mS actually doesn't give it enough time to fully charge in a Theta cycle
#define H_TAU_R 0.06
//H_TAU_F defines the duration of the burst.  Must conclude under 50mS to allow one burst per theta cycle
#define H_TAU_F 0.008
//this is the t-current inactivation threshold voltage.  Set it slightly above voltage to which inhibition drives Vm, but not so much that E_K spike resets have much effect on H
//V_H should be as high as possible until post-spike hyperpolarization starts affecting it. Trial/error gives -0.067 for this tuning
#define V_H     -0.065
//T-current floor.  It only flows if Vm > V_T.  Set it just below resting potential E_L  V_theta in the article
#define V_T     -0.065



#define delayTap   0x0000000000000001  //how many dT's of delay		1,2,4,8, 10, ...
#define delayBank  0            //0: 1:64 dTs of delay  1: 65:128 dTs of delay  2: 129:196 dT of delay

//host struct for loading synapse parameters
//gets passed down as parameters to device
#define NUM_SYN_PARAMS 2
typedef struct synParams{
    float imax_syn;
    float tau_syn;
    float ws;  	//set in resetSnaps rather than loadSynParams
} synParamStruct ;

//device-side cell struct
typedef struct lif_data {
  //unique cell parameters
  float Cm;             //Cell membrane capacitance.  Defined in cellParams, then randomized a bit
  //dynamic variables
  float d_vm;           //membrane voltage
  float d_Isra;         //spike-rate adaptation current
  float d_Gref;         //refractory conductance
  float d_Isyn;         //synaptic current into soma
  //calcium params for bursting
  float d_h_t;          //calcium T-current state variable
  float d_vms;          //10mS smoothing filter d_vm to become d_vms, averaged version without the quick spike transients
  float d_Isyns;        //5mS smoothing filter d_Isyn to become d_Isyns, averaged version without the quick spike transients
  curandState d_curandState;  //curand state value
  unsigned int d_curandVal;    //curand prng random value
  unsigned long int d_delay_2;       // 64-bit word  for up to 19.2mS delay
  unsigned long int d_delay_1;       // 64-bit word  for up to 12.8mS delay
  unsigned long int d_delay_0;       // 64-bit word  for up to 6.4mS delay
  int spikeCount;		     //how many spikes since last clearing of this register
  int int_misc;                      //general purpose integer
  float float_misc;                  //general purpose float
  float now;                         //the current time (iterations * dT)
  bool hasBeenSet = false;     //marks whether a cell has ever been part of a training pattern
  bool setThisCycle = false;   //marks whether a cell is part of the current training pattern
  bool disable = false;        //turn off this cell to supress xtalk
  float state = 0.0;	       //used by I_ layer to specify state of this cell in the input pattern.  ??replace this with some other mechanism to save memory??
  int spikes = 0;  	       //accumulate number of spikes since last being cleared
} lif_data_type ;


//host struct for loading cell parameters
#define NUM_CELL_PARAMS 10 
typedef struct cellParams{
    unsigned long int SEED;
    float Cm;
    float Vth;
    float delta_Vth;
    float tau_sra;
    float sra_a;
    float sra_b;
    float tau_gref;
    float delta_gref;
    float sigma_i_noise;
} cellParamStruct ;

typedef struct cell_switches{
        int printf_cell = 0;    //cell idx to print from
        bool en_motion_sensitivity = false; 
        bool en_noise = false;   //to kernel enable Vm noise for spontaneous activity
        bool en_theta = false;  // to kernel   enable theta-rythm process in lgn, trn
        bool en_cancel_xtalk = true;    // keep track whether this cell is in more than one training pattern. Suppress activity in this case.
        bool en_learn = false;   //tell the cell whether synaptic plasticity is enabled.  Used by xtalk suppression
        bool en_bursting = false;   //tell the cell whether synaptic plasticity is enabled.  Used by xtalk suppression
        bool en_intrinsic_osc = false;   //enable instrinsic oscillations within the cell (sin1, sin2, sin3)
        bool en_spontaneous_activation = false;  //enable constant intrinsic stimulus, defined within the soma kernel
	int en_input = 0;   //use this to specify an input layer driven by genPats or whatever 0: no external input 1:disk of activation 2:from float_misc
	int2 loc = {0,0};
	int  misc;
	//three sine waves (or any other function you need), calculated in main or somewhere
	float sin_1, sin_2, sin_3;
} cell_switchesStruct ;

typedef struct syn_switches{     //the run-time state of a particular synapse group
        bool en_periodic = false; 
        bool en_noise = false;   //to kernel enable Vm noise for spontaneous activity
        bool en_p2p = false;     //point to point connectivity rather than receptive-field radius connectivity
        bool en_learn = false;   //enable synaptic plasticity in P_ synapses
        bool en_syn = true;      //global enable/disable for this synapse group
	int ratio = 0;		 //layer to layer size 0: 1->1   1: 3->1   2: 1-> 3  never changes, should be kept as a param somehow?
	int  name = 0;
} syn_switchesStruct ;

//device-side synapse struct
//defines the behavior of a particular synapse group
typedef struct syn_data {
  //parameters
  float imax_syn;
  float tau_syn;
  int ws = 1;	//true if weight-sharing(convolution), which reduces and changes memory access pattern for weight values
} synapse_data_type;


typedef struct {
    //0:vm   1:isyn
    int win[3];
    GLuint  pbo[2];
    GLuint  tex[2];
    struct cudaGraphicsResource *cuda_pbo_resource[2];
    uchar4 *vm ;
    uchar4 *isyn;
    uchar4 *dummy1, *dummy2;
    lif_data_type *cells;  //doesn't work for some reason
} layer ;

#endif //KERNEL_COMMON
