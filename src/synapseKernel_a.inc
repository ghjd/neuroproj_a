

/*
     syn_currents: float array with one entry per synapse in the synapse group. (receiving-array width)^2 * (2*radius+1)^2
     synapse:      array of synapse_data_type data-structures, containing parameters for all synapses of this group. One per receivinc cell, (receiving-array width)^2   
                   =>Can this be once per synapse-group, rather than once per receiving cell?
                   1 per cell in the receiving array, needed to containing the plastic synaptic weight.  If ws=1, in principle only one is needed per synapse group
     lif_pre:	   pointer to the presynaptic cell array
     lif_post:	   pointer to the postsynaptic cell array
     weights:      if ws=1, this supplies the (2*radius+1)^2 weights shared b all receiving cells.  malloc (2*radius+1)^2
                   if ws=0, each cell needs individual synaptic weights.  So malloc (2*radius+1)^2 * (receiving-array width)^2
Called once per cell in the receiving layer, to calculate incoming synaptic currents. 
Receiving cell[col][row] is idx = flatten(col, row, w, w), with w=layer dimension equaling (row*w + col)   0 <= col < w, 0 <= row < w , col defining X dimension and row defining Y dimension
Each cell receives from an array of synapses with dimension (2*radius+1)^2, centered at (0,0) and extending to -radius and +radius on each axis.
syn_array is a linear array spanning Width^2 cells in the layer, each with (2*radius+1)^2 synapses.
A particular cell receives synapses from synapse_array starting at idx*(2*radius+1)^2, and extending to (idx+1)*(2*radius+1)^2 - 1
The loop runs from -radius <= x < radius,   -radius <= y = radius, centered at 0, 0.
A particular synapse for a particular cell will be synapse_array[ (idx*(2*radius+1)^2) + (x+radius)*(2*radius+1) + (y+radius)
The presynaptic cell driving a synapse is located at presyn_row = row + x, presyn_col = col + y in the source layer, also of dimension Width^2. Here, row & col are the location of the postsynaptic cell.

Weight arrays are pre-calculated by generateReceptiveFields and stored in constant memory.  
For fixed weights, e.g. convolutions, this reduces memory requirement and takes advantage of constant memory's high bandwidth.
Weight array receptive fields must be calculated using the same receptive-field radius.
X and Y indices looped like this, row & column index into a (2*RADIUS+1)^2 array to put the synapse at the presynaptic cell's index
    float h_weights[(2*RADIUS+1)*(2*RADIUS+1)];
    for ( row = 0; row < (2*radius+1); row++){
      for ( col = 0; col < (2*radius+1); col++){
        h_weights[(2*radius+1)*row + col] = weight;
This must line  up with how synapseKernel_a references and uses the weights.

The synapse kernel loop is this:
  for ( x = -radius; x <= radius; x++){   // 2*radius+1 stepping through rows
     for ( y = -radius; y <=  radius; y++){  stepping through columns
          presyn_row = row + x;  //referencing into the presynaptic cell layer. x=y=0 will pick the same row, col cell in each layer.
          presyn_col = col + y;
          int offset = _2rp1*(x+radius) + (y+radius);
          //weight-array reference,  This is a (2*radius+1)^2 linear array,
          weight = h_weights[offset];

         //the syn_currents reference into the multi-cell linear array
         syn_currents[ (idx * _2rp1 * _2rp1) + (x+radius)*_2rp1 + (y+radius)]
                 (idx * _2rp1 * _2rp1) = picks the region of the linear array associated with the postsynaptic cell
                      (x+radius)*w moves ahead x rows.  Adding radius because the loop has -radius <= x < radius, so 0 <= (x+radius) <= 2*radius for (2*radius+1) entries
                          (y+radius) picks the column

Finally, loop through all the synapses in this group for the postsynaptic cell and add them to its cumulative synaptic current:
    for ( x = 0; x < _2rp1; x++){
       for ( y = 0; y <  _2rp1; y++){
          Isyn +=  syn_currents[ (idx * _2rp1 * _2rp1) + x*_2rp1 + y];
*/
__global__
void synapseKernel_a(float *syn_currents, synapse_data_type *synapse, lif_data_type *lif_pre, lif_data_type *lif_post, float *weights, int w, int ratio, syn_switchesStruct switches, int radius ) {
  // global indices
  const int col = threadIdx.x + blockDim.x * blockIdx.x;  //column # of this cell in cell layer of dimension w
  const int row = threadIdx.y + blockDim.y * blockIdx.y;  //row    # of this cell in cell layer of dimension w
  if ((col >= w) || (row >= w)) return;
  const int idx = flatten(col, row, w, w);

  int _2rp1 = 2*radius + 1; //a commonly used value
  int sq_2rp1 = _2rp1 * _2rp1; //a commonly used value

  lif_data_type * presyn_ptr;  //pointer to pre- and post-synaptic cells of type lif_data_type
  int x, y;
  int presyn_row, presyn_col;

  float tau_syn = synapse[idx].tau_syn;

  //figure out the size of the presynaptic array from W and ratio, for periodic boundry correction calculation
  //note that S2B is handled in synapseKernel_a_s2b.inc
  int pre_array_size = (ratio == S2S ) ? SMALL : BIG; //B2B, B2S have BIG pre-synaptic arrays 
  //projects presyn region of radius * radius onto postsyn cell (row, col)
  //at the moment, the weights data-structure must be calculated with the same radius by resetSnaps
  float weight = 0;
  for ( x = -radius; x <= radius; x++){   // 2*radius+1
     for ( y = -radius; y <=  radius; y++){
       //if ( !(x == 0 & y == 0) ) {  //skip the self-connection.  FIXME implement logic so that this only occurs for recurrent connectivity, i.e. lif_pre == lif_post
         switch ( ratio ) {
              case B2S : //big to small
                  presyn_row = 3*row + x;
                  presyn_col = 3*col + y;
	          break;
              default: //one to one mapping, B2B or S2S or S2B
                  presyn_row = row + x;
                  presyn_col = col + y;
	          break;
	  }

          int offset = _2rp1*(x+radius) + (y+radius); //row,col location within an array, from 0 to (2r+1)^2
	  int idx_offset = (idx * sq_2rp1 ) + offset; //jump past other post-syn cell's input weight matrices, then move to the row and column of this one. Assuming weight sharing, all post-cells use identical weight matrices
	  weight = weights[offset]; //weight-sharing version
	  if ( synapse[idx].ws == 0 ) {
	       weight = weights[idx_offset]; //all synapses have unique weight
	  }

          //periodic boundry correction.  mark out-of-bounds with a -1 value to be detected later.
          if ( presyn_row < 0 ) { presyn_row =  switches.en_periodic ? pre_array_size + presyn_row : -1; }  
          if ( presyn_row >= pre_array_size) { presyn_row =  switches.en_periodic ? presyn_row - pre_array_size : -1; }
          if ( presyn_col < 0 ) { presyn_col =  switches.en_periodic ? pre_array_size + presyn_col : -1; }
          if ( presyn_col >= pre_array_size) { presyn_col =  switches.en_periodic ? presyn_col - pre_array_size : -1; }
          presyn_ptr =   &lif_pre[flatten(presyn_col, presyn_row,pre_array_size, pre_array_size)] ;
	  bool spike = ( presyn_ptr->d_delay_0 &  0x0000000000000001 );
          //if there is a spike at the presynaptic cell's parametrized delay-chain location, set synaptic current to imax. Otherwise, decay the current
          if ( !(presyn_row == -1 || presyn_col == -1) ) {  //if not out-of-bounds, flagged by -1
             syn_currents[ idx_offset ]  =  spike ? weight : syn_currents[ idx_offset] * ( 1.0 - dT / tau_syn );
          } else { syn_currents[ idx_offset] = 0;}  //non-wrapped out-of-bounds links set to zero
	  //do the weight modification for learning
	  if ( switches.en_learn && synapse[idx].ws == 0 && spike ) {
              if ( lif_post[idx].d_vm > -0.055 ) { //incoming spike into a spiking receiving neuron through this synapse
                      weights[idx_offset] = 0.0000000001;  //learning rate should be parametrized eventually
              }
          }
       //}//if ( !(x == 0 & y == 0) )
     }//col
  }//row

  //calculate the incoming synaptic current
  float Isyn = 0;
  if ( (row > 0 & row < w  & col  > 0 & col < w) || switches.en_periodic ){
    for ( x = 0; x < _2rp1; x++){
       for ( y = 0; y <  _2rp1; y++){
	  Isyn +=  syn_currents[ (idx * sq_2rp1) + x*_2rp1 + y];
       }
    }
  }
  if ( switches.en_syn == 0 ) { Isyn = 0; }
  lif_post[idx].d_Isyn += Isyn;  
}//synapseKernel_a
