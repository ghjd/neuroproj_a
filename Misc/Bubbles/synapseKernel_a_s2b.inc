


__global__
void synapseKernel_a_s2b(float *syn_currents, synapse_data_type *synapse, lif_data_type *lif_pre, lif_data_type *lif_post, float *weights, int w_pre, syn_switchesStruct switches, int radius ) {

/*
     This kernel defines a synapse-group through which a small cell array can drive a big cell array, hard-coded to be 3X3 size of the small array.
     The kernel is called to launch one thread per cell in the presynaptic array.  It will then calculate the postsynaptic currents into
     each cell of the postsynaptic array.
     The kernel executes a thread that calculates current Isyn  of one presynaptic cell as an exponentially decaying jump to Imax in response to a spike.
     Isyn (per presynaptic cell) is then propagated through the weights array to produce specific currents for each postsynaptic cell.
     The synapse-group of type P_syn_data contains functional parameters for width*width synapses, each of which
     contact a contiguous non-overlapping 3X3 patch of the post-synaptic array.  The 3X3 post-synaptic cells all receive the same current, through a single weight.
	(not correctly implemented yet.  Looks like only one postsynaptic cell in the 3X3 patch gets a synaptic current)

     synapse (group): array of synapse_data_type data-structures, containing parameters for all synapses of this group. One per presynaptic cell, (w_pre)^2   
		      Only contains a few parameters anymore.  Maybe can be deprecated?
                      1 per cell in the receiving array, needed to containing the plastic synaptic weight.  If ws=1, in principle only one is needed per synapse group
     lif_pre:         (w_pre)^2 presynaptic cell array.  Each thread of this kernel handles one cell
     lif_post:        (3*w_pre)^2 postsynaptic cell array
     syn_currents:    float array with one entry per cell in the synapse group. Malloced as (pre-syn array width)^2 * (2*radius+1)^2 
	              but I think only (pre-syn array width)^2 are needed here.
     weights:         each presynaptic cell needs a synaptic weight group. WS=0  malloc (2*radius+1)^2 * (w_pre)^2 
	                                                                   WS=1  malloc (2*radius+1)^2 , all presynaptic cells share the same weight values

     Note that with WS=1, weights are initialized by resetSnaps into the appropriate receptive field.
     With WS=0, weights are initialized to 0 and grow in response to learning.

     This synapse-group must not be the first to be launched, since it does not have the accumulation logic.

     This synapse-group must not be the first to be launched for a postsynaptic layer, since it does not have the accumulation logic.

     This kernel works backwards from the B2S. A single spiking & decaying presynaptic current is fed through the weights to reach the postsynaptic cells.
     With B2S, 3X3 presynaptic currents are summed to arrive at a single postsynaptic cell.

     Set radius=0 when R_121 is used.  This tells the synapse group to do the 9X9 fan-out

*/


  // global indices
  //row and column of pre-synaptic cell
  const int pre_col = threadIdx.x + blockDim.x * blockIdx.x;
  const int pre_row = threadIdx.y + blockDim.y * blockIdx.y;
  if ((pre_col >= w_pre) || (pre_row >= w_pre)) return;   //block threads of non-existant cells
  const int pre_idx = flatten(pre_col, pre_row, w_pre, w_pre);  //index into 1D presynaptic array correspondign to (pre_row, pre_col) for this cell
  int post_idx;
  //int x_post_idx;

  float tau_syn = synapse[pre_idx].tau_syn;

  //float * postsyn_ptr = NULL;

  int en_syn = switches.en_syn;
  int en_learn = switches.en_learn;
  int x, y; 
  //int v, w;

  int w_post = 3*w_pre;      //w is presynaptic layer dimension
  int _2rp1 =  2*radius + 1; //a commonly used value
  int post_row, post_col;
  //int x_post_row, x_post_col;


  //first, figure out if the pre-syn cell is firing.
  lif_data_type * presyn_ptr =   &lif_pre[flatten(pre_col, pre_row,w_pre, w_pre)] ;  //pointer to pre- and post-synaptic cells of type lif_data_type
  bool spike = ( presyn_ptr->d_delay_0 &  0x0000000000000001 );
  float isyn = spike  ? 1.0 : syn_currents[ pre_idx] * ( 1.0 - dT / tau_syn ); //alternatively, add to existing isyn at spike, for cumulative signal
  syn_currents[ pre_idx ]  = isyn;
  for ( x = -radius; x <= radius; x++){   // 2*radius+1
     for ( y = -radius; y <=  radius; y++){
         //calculate target post-syn cell's row & column from pre-syn coordinates, then calculate it's idx in post-syn coordinates.
         post_row = 3*pre_row + x;
         post_col = 3*pre_col + y;
         post_idx = flatten(post_col, post_row, w_post, w_post);  //index into lif_post array correspondign to (pre_row, pre_col)
	 int offset = _2rp1*(x+radius) + (y+radius);  //for WS=1, all cells share the same (2*radius+1)^2 weight matrix
	 //if WS=0 with en_learn, each cell has a unique (2*radius+1)^2 region of the (2*radius+1)^2 * (w_pre)^2 weight matrix
	 if ( synapse[pre_idx].ws == 0 ) {
		 offset = (pre_idx * _2rp1 * _2rp1 ) + offset;
	 }


         //periodic boundry correction for post-syn locations
	 //pulled in by one cell around the boundry to make room for the 3X3 fan-out
         if ( post_row < 1 ) { post_row =  switches.en_periodic ? w_post + post_row : -1; }  //mark out-of-bounds with a -1 value to be detected later.
         if ( post_row >= w_post-1) { post_row =  switches.en_periodic ? post_row - w_post : -1; }
         if ( post_col < 1 ) { post_col =  switches.en_periodic ? w_post + post_col : -1; }
         if ( post_col >= w_post-1) { post_col =  switches.en_periodic ? post_col - w_post : -1; }
	 //if there is a spike at the presynaptic cell's parametrized delay-chain location, set synaptic current to imax. Otherwise, decay the current
         //pointer to the current current-state of the synapse between lif_post(post_row, post_col) and lif_pre(pre_row, pre_col)
         //indexed in postsyn coordinates because post is the larger array
         if (!(post_row == -1 || post_col == -1) && en_syn  ) {  //if not out-of-bounds,
           lif_post[post_idx].d_Isyn +=   isyn * weights[offset];
	   /*
	   messes up xtalk in the sequence_generator sim
           for ( v = -1; v <= 1; v++) { 
             for ( w = -1; w <= 1; w++) { 
                  x_post_row = post_row + v;
                  x_post_col = post_col + w;
                  x_post_idx = flatten(x_post_col, x_post_row, w_post, w_post);  //index into lif_post array correspondign to (pre_row, pre_col)
                  lif_post[x_post_idx].d_Isyn +=   isyn * weights[offset];
	      }
            }
	   */
         } 
	 //do the learning
         if ( !(post_row == -1 || post_col == -1) && en_learn && spike ) {
              if ( lif_post[post_idx].d_vm > -0.055 ) { //incoming spike into a spiking postsynaptic neuron through this synapse
                      weights[offset] = 0.000000001;  //learning rate should be parametrized eventually
             }
         }
     }//col
  }//row
}//synapseKernel_a_s2b
