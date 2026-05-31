#include "kernels.h"
#include "cpp_common.h"

//when dividing threads by threadblock, always round up
int divUp(int a, int b) { return (a + b - 1) / b; }

//constrain to 0:255 eight-bit range
__device__
unsigned char clip(int n) { return n > 255 ? 255 : (n < 0 ? 0 : n); }

//vm to vary from -0.1 to +0.0
//Scale this to 0 to 255
__device__
unsigned char colorize_vm(float vm) {
        vm = vm + 0.101;  //move up to positive
        vm = vm * 2550;  //increase range to 0:255 for 0 <= n < 0.1 mV
        int n = int ( vm );
        return  n > 255 ? 255 : (n < 0 ? 0 : n);
}
/*
unsigned char colorize_vm(float vm) {
        //80mV range starting from -80mV to 0 //The plus term is to allow negative Vms up to this value to be colorized
        vm = 1.0e+1*(vm + 0.8e-1) / 0.8;  //subtract negative range, divide by total range
        vm = vm * 255;
        int n = int ( vm );
        return  n > 255 ? 255 : (n < 0 ? 0 : n);
}
*/


// limit idx to range 0 <= idx <= W*H thread-count for a layer
__device__
int idxClip(int idx, int idxMax) {
  return idx >(idxMax - 1) ? (idxMax - 1) : (idx < 0 ? 0 : idx);
}

//given (x=col, y=row) calculate an index into a 1D 'flattened' linear array of width*height entries, respecting bounds
__device__
int flatten(int col, int row, int width, int height) {
  return idxClip(col, width) + idxClip(row, height)*width;
}

__global__
void resetKernel(lif_data_type *lif_data, int w, cellParamStruct cellParams ) {
  const int col = blockIdx.x*blockDim.x + threadIdx.x;
  const int row = blockIdx.y*blockDim.y + threadIdx.y;
  if ((col >= w) || (row >= w)) return;
  int id = row*w + col;
  lif_data[id].d_vm = E_L;
  lif_data[id].d_vms = E_L;
  lif_data[id].d_Isra = 0.0;
  lif_data[id].d_Gref = 0.0;
  lif_data[id].d_Isyn = 0;
  lif_data[id].d_Isyns = 0;
  lif_data[id].d_delay_2 = 0;
  lif_data[id].d_delay_1 = 0;
  lif_data[id].d_delay_0 = 0;
  lif_data[id].d_curandVal = 0;
  lif_data[id].int_misc = 0;
  lif_data[id].float_misc = 0.0;  //use as needed
  lif_data[id].spikeCount = 0;  //use as needed
  curand_init( cellParams.SEED, id, 0, &lif_data[id].d_curandState);
  //lif_data[id].Cm = cellParams.Cm * ( 1.0 + 0.5 * (curand_uniform(&lif_data[id].d_curandState) - 0.5));  //dither membrane caps to spread cell response times a bit
  lif_data[id].Cm = cellParams.Cm ;
}


__global__
void loadPatKernel(lif_data_type *layer, int w, float *pattern ) {
  const int col = blockIdx.x*blockDim.x + threadIdx.x;
  const int row = blockIdx.y*blockDim.y + threadIdx.y;
  if ((col >= w) || (row >= w)) return;
  int id = row*w + col;
  layer[id].float_misc = pattern[id];
}

__global__
void loadVisualField(lif_data_type *layer, int w, float *pattern ) {
  const int col = blockIdx.x*blockDim.x + threadIdx.x;
  const int row = blockIdx.y*blockDim.y + threadIdx.y;
  if ((col >= w) || (row >= w)) return;
  int id = row*w + col;
  layer[id].state = pattern[id];
}

__global__
//en_params = false: run-time clear, only clear weights & states.  en_params = true: initialization, also reload the params values from synParams
void resetSynapse( synapse_data_type *synapse, float *weights, int w, synParamStruct synParams, int radius, int rfield_type ) {

  const int col = blockIdx.x*blockDim.x + threadIdx.x;
  const int row = blockIdx.y*blockDim.y + threadIdx.y;
  if ((col >= w) || (row >= w)) return;
  int id = row*w + col;
  //initialize these params only on the first call of this routine from Main, at which time synParams points to the right value set.
  //on all other passes, called later in the program, synParams is obsolete so don't overwrite these values
  //maybe figure out how to make these parameters to the synapse kernel call so that they don't need a memory slot?
  synapse[id].imax_syn = synParams.imax_syn;
  synapse[id].tau_syn = synParams.tau_syn;
  synapse[id].ws = synParams.ws; //0: weight-sharing   1:each synapse has a unique weight

  int x, y;
  int xrow, ycol;
  float distance;
  int offset;

  //set up to initialize convolutional weights, e.g. learning is turned off.
  //with learning turned on, weight-sharing is no longer possible and the weight matrix is much larger.
  if ( synParams.ws == 1 ) {
     switch ( rfield_type ) {
        case R_121: //one to one mapping
			xrow = radius;
			ycol = radius;
                        offset = (2*radius+1)*xrow + ycol;
                        x = xrow - radius; y = ycol - radius;
			weights[offset] = 0;
                        distance = sqrt( float(x*x + y*y) ) - 1;
			if ( distance < radius  ) {
				if ( x == 0 && y  == 0 ) {
			              weights[offset] = synParams.imax_syn;
				}
			}
                break;
        case R_Z: //zero, no transmission
                  for ( xrow = 0; xrow < (2*radius+1); xrow++){
                     for ( ycol = 0; ycol < (2*radius+1); ycol++){
                         offset = (2*radius+1)*xrow + ycol;
                         x = xrow - radius; y = ycol - radius;
                         distance = sqrt( float(x*x + y*y) ) - 1;
                         weights[offset] =  0.0;
                     }
                   }
                break;
        case R_H:  //horizontally sensitive
              for ( xrow = 0; xrow < (2*radius+1); xrow++){
                  for ( ycol = 0; ycol < (2*radius+1); ycol++){
                      offset = (2*radius+1)*xrow + ycol;
                      x = xrow - radius; y = ycol - radius;
                      distance = sqrt( float(x*x + y*y) );
                      if ( x <=  1 && x >= -1 ){   
                          weights[offset] =  distance < radius ? (1.0 - (1.7*distance)/radius) : 0;  // excitatory center
			  if ( weights[offset] < 0 ) { weights[offset] = 0;}
                      } else {
                          weights[offset] =  distance < radius ? -1.0 : 0;  //inhibitory wings
                      }
                  }
              }
              break;
        case R_V:  //vertically sensitive
	      for ( xrow = 0; xrow < (2*radius+1); xrow++){
                  for ( ycol = 0; ycol < (2*radius+1); ycol++){
                      offset = (2*radius+1)*xrow + ycol;
                      x = xrow - radius; y = ycol - radius;
                      distance = sqrt( float(x*x + y*y) );
                      if ( y <=  1 && y >= -1 ){   
                          weights[offset] =  distance < radius ? (1.0 - (1.7*distance)/radius) : 0;  // excitatory center
			  if ( weights[offset] < 0 ) { weights[offset] = 0;}
                      } else {
                          weights[offset] =  distance < radius ? -1.0 : 0;  //inhibitory wings
                      }
                  }
                }
                break;
        case R_BS:  // backslash
		  for ( xrow = 0; xrow < (2*radius+1); xrow++){
                     for ( ycol = 0; ycol < (2*radius+1); ycol++){
                         offset = (2*radius+1)*xrow + ycol;
                         x = xrow - radius; y = ycol - radius;
                         distance = sqrt( float(x*x + y*y) );
                         //if ( abs(x - y ) < BAR_WIDTH && ( abs(x) < BAR_HEIGHT ) ){   //3 cells wide seems to be the sweet spot
                         if ( abs(x - y ) < BAR_WIDTH ){   
                             weights[offset] =  distance < radius ? 1.0 * (1 - (1.0*distance-1)/radius) : 0;  // excitatory center scaled up by 1.5 to compensate for diagnonal losses
                         } else {
                             weights[offset] =  distance < radius ? -1.0 * (1 - (1.0*distance-1)/radius) : 0;  //inhibitory wings
                         }
                     }
                 }
                break;
        case R_FS:  // forward slash
		for ( xrow = 0; xrow < (2*radius+1); xrow++){
                    for ( ycol = 0; ycol < (2*radius+1); ycol++){
                        offset = (2*radius+1)*xrow + ycol;
                        x = xrow - radius; y = ycol - radius;
                        distance = sqrt( float(x*x + y*y) );
                        //if ( abs(x + y ) < BAR_WIDTH && ( abs(x) < BAR_HEIGHT ) ){   //3 cells wide seems to be the sweet spot
                        if ( abs(x + y ) < BAR_WIDTH  ){   
                            weights[offset] =  distance < radius ? 1.0 * (1 - (1.0*distance-1)/radius) : 0;  // excitatory center scaled up by 1.5 to compensate for diagnonal losses
                        } else {
                            weights[offset] =  distance < radius ? -1.0 * (1 - (1.0*distance-1)/radius) : 0;  //inhibitory wings
                        }
                    }
                }

                break;
        case R_AE: //all excitatory
                  for ( xrow = 0; xrow < (2*radius+1); xrow++){
                     for ( ycol = 0; ycol < (2*radius+1); ycol++){
                         offset = (2*radius+1)*xrow + ycol;
                         x = xrow - radius; y = ycol - radius;
                         distance = sqrt( float(x*x + y*y) ) - 1;
                         weights[offset] =  distance < radius ? (1 - (1.0*distance)/radius) : 0;
			 weights[offset] *= synParams.imax_syn;
                     }
                   }
                break;
        case R_CS:  //symmetrical center-surround
		for ( xrow = 0; xrow < (2*radius+1); xrow++){
                    for ( ycol = 0; ycol < (2*radius+1); ycol++){
                        offset = (2*radius+1)*xrow + ycol;
                        x = xrow - radius; y = ycol - radius;
                        distance = sqrt( float(x*x + y*y) ) - 1;
                        weights[offset] =  distance < radius ? (1 - (1.6 * distance)/radius) : 0;  //center-surround  normal attentive operation
			weights[offset] *= synParams.imax_syn;
                     }
                 }
                break;
        case R_CS_B:  //symmetrical center-surround with small center
		for ( xrow = 0; xrow < (2*radius+1); xrow++){
                    for ( ycol = 0; ycol < (2*radius+1); ycol++){
                        offset = (2*radius+1)*xrow + ycol;
                        x = xrow - radius; y = ycol - radius;
                        distance = sqrt( float(x*x + y*y) ) - 1;
                        weights[offset] =  distance < radius ? (1 - (3.0*distance)/radius) : 0;  //center-surround  normal attentive operation
			weights[offset] *= synParams.imax_syn;
                     }
                 }
                break;
        case R_NN: //nearest neighbor
                  for ( xrow = 0; xrow < (2*radius+1); xrow++){
                     for ( ycol = 0; ycol < (2*radius+1); ycol++){
                         offset = (2*radius+1)*xrow + ycol;
                         x = xrow - radius; y = ycol - radius;
                         distance = sqrt( float(x*x + y*y) ) - 1;
			 weights[offset] = 0;
                         weights[offset] =  distance < radius ? (1 - (1.0*distance)/radius) : 0;
			 weights[offset] *= synParams.imax_syn;
                     }
                   }
		/*
		for ( xrow = 0; xrow < (2*radius+1); xrow++){
                    for ( ycol = 0; ycol < (2*radius+1); ycol++){
                        offset = (2*radius+1)*xrow + ycol;
			weights[offset] = 0;
			if ( x == 1  && y  == 0 ) { weights[offset] = synParams.imax_syn; }
			if ( x == -1 && y  == 0 ) { weights[offset] = synParams.imax_syn; }
			if ( x == 0  && y  == 1 ) { weights[offset] = synParams.imax_syn; }
			if ( x == 0 && y == -1 ) { weights[offset] = synParams.imax_syn; }
			//the diagonals
			if ( x == 1  && y  == 1 ) { weights[offset] = 0.71 * synParams.imax_syn; }
			if ( x == 1  && y == -1 ) { weights[offset] = 0.71 * synParams.imax_syn; }
			if ( x == -1 && y  == 1 ) { weights[offset] = 0.71 * synParams.imax_syn; }
			if ( x == -1 && y == -1 ) { weights[offset] = 0.71 * synParams.imax_syn; }
                     }
                 }
		 */
                break;
        default: break ;
     }//switch
  }//if ws
}

//Clears the synapse states
void resetSnaps(synapse_data_type *synapse_ptr, float *weights, int w, synParamStruct synParams, int radius, int rfield_type ) {
  const dim3 blockSize(TX, TY);
  const dim3 gridSize(divUp(w, TX), divUp(w, TY));

  resetSynapse<<<gridSize, blockSize>>>( synapse_ptr, weights, w, synParams, radius, rfield_type );
}


//in-line include containing procedural code
#include "synapseKernel_a.inc"
#include "synapseKernel_a_s2b.inc"
#include "somaKernel_aelif.inc"

//the following are host-side routines that interact with the device

//synapseKernel_a: fixed weight with selectable receptive field pattern
//w is the dimension of the post-synaptic layer.  ratio specifies the relative sizes of the pre and post-synaptic layers {0: 1->1   1: 3->1   2: 1-> 3}
//ratio=0 is 1 to 1.  Both layers must have the same dimension w.  The synapse array  must also have dimension w.
//ratio=1 maps a 100x100 region of the 300x300 presynaptic layer onto the 100x100 postsynaptic layer.
//        synapse-array dimension must match the receiving array dimension.
//ratio=2 maps each cell in the 100x100 presynaptic layer onto 3x3 cells of the postsynaptic layer, so that the presynaptic layer activates the entire postsynaptic layer.

void synapseKernelLauncher_a(cudaStream_t stream, float *syn_currents, lif_data_type *lif_data_a, lif_data_type *lif_data_b, synapse_data_type *synapse, float *weights, int w, int ratio,  syn_switchesStruct syn_switches, int radius) {
  const dim3 blockSize(TX, TY);
  const dim3 gridSize(divUp(w, TX), divUp(w, TY));
  switch ( ratio ) {
	  //S2B and B2S do not support weight sharing
	  case S2B:  
              //printf("*************Calling synapseKernel_a_s2b with name=%d\ten_syn=%d*************\n", syn_switches.name, syn_switches.en_syn);
              synapseKernel_a_s2b<<<gridSize, blockSize, 0, stream>>>(syn_currents, synapse, lif_data_a, lif_data_b, weights, w, syn_switches, radius);
	      break;
	  default:
              //printf("Calling synapseKernel_a with name=%d\ten_syn=%d\n", syn_switches.name, syn_switches.en_syn);
              synapseKernel_a<<<gridSize, blockSize, 0, stream>>>(syn_currents, synapse, lif_data_a, lif_data_b, weights, w, ratio, syn_switches, radius);
	      break;
  }
}

void resetCells(lif_data_type *lif_data, int w, cellParamStruct cellParams ) {
  const dim3 blockSize(TX, TY);
  const dim3 gridSize(divUp(w, TX), divUp(w, TY));
  resetKernel<<<gridSize, blockSize>>>(lif_data, w, cellParams );
}

void I_loadPat (lif_data_type *layer, int w, float *pattern){
  const dim3 blockSize(TX, TY);
  const dim3 gridSize(divUp(w, TX), divUp(w, TY));
  loadPatKernel<<<gridSize, blockSize>>>(layer, w, pattern);
}

void cellKernelLauncher_aelif(cudaStream_t stream, uchar4 *d_out, lif_data_type *lif_data, int w, cell_switchesStruct cell_switches, cellParamStruct cellParams ) {
  const dim3 blockSize(TX, TY);
  const dim3 gridSize(divUp(w, TX), divUp(w, TY));
  somaKernel_aelif<<<gridSize, blockSize, 0, stream>>>( d_out,  lif_data, w, cell_switches, cellParams);
}

