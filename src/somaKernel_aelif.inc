__global__
void somaKernel_aelif( uchar4 *d_out, lif_data_type *lif_data, int w, cell_switchesStruct cell_switches, cellParamStruct cellParams ) {
  // global indices
  const int col = threadIdx.x + blockDim.x * blockIdx.x;
  const int row = threadIdx.y + blockDim.y * blockIdx.y;
  if ((col >= w) || (row >= w)) return;
  const int idx = flatten(col, row, w, w);

  //d_out is membrane voltage mapped into RGB for display.
  // assign default color values for d_out (black)
  d_out[idx].x = 0;
  d_out[idx].z = 0;
  d_out[idx].y = 0;
  d_out[idx].w = 255;

  //float Cm = cellParams.Cm ;  //all identical
  float Cm = lif_data[idx].Cm;  //dithered

  float Inoise = cell_switches.en_noise ? (cellParams.sigma_i_noise * curand_uniform(&lif_data[idx].d_curandState)) : 0;
  float Istim = 0;

  if ( cell_switches.en_input == 1 ) { //disk of excitation centered at loc
       int x, y;
       x = row - cell_switches.loc.y;
       y = col - cell_switches.loc.x;
       Istim = sqrt( float(x*x + y*y)) < RADIUS ? 0.5e-9 : 0;
   }
   if ( cell_switches.en_input == 2 ) { Istim = lif_data[idx].float_misc; }
   if ( cell_switches.en_intrinsic_osc ) {
       Istim += (cell_switches.sin_1 + cell_switches.sin_2 + cell_switches.sin_3);
   }
   if ( cell_switches.en_spontaneous_activation ) {
       Istim += 0.6e-9;
   }
  
  //the axon delay chain shifter.  Do the shift after all synapses reference the value, hence after syncthreads
  lif_data[idx].d_delay_2 = lif_data[idx].d_delay_2 << 1 |   ((lif_data[idx].d_delay_1 >> 63) & 1);  //using 64-bit unsigned long ints
  lif_data[idx].d_delay_1 = lif_data[idx].d_delay_1 << 1 |   ((lif_data[idx].d_delay_0 >> 63) & 1);  //using 64-bit unsigned long ints

  float Vm = lif_data[idx].d_vm ;
  float Isyn = lif_data[idx].disable ? lif_data[idx].d_Isyn * 0.0 : lif_data[idx].d_Isyn ;  //set the synaptic weight of an xtalk cell to very low 
  float Isra = lif_data[idx].d_Isra ;
  float Gref = lif_data[idx].d_Gref ;
  float dIsra_dt = (cellParams.sra_a * (Vm - E_L) - Isra)/cellParams.tau_sra;
  float dGref_dt = Gref / cellParams.tau_gref;
  float Iref = Gref * (  E_K  - Vm );
  float exp_term = cellParams.delta_Vth * powf( 2.71, ((Vm - cellParams.Vth)/cellParams.delta_Vth));

  //10mS filter d_vm to become d_vms, averaged version without the quick spike transients
  float Vms =  lif_data[idx].d_vms; //used twice more below, so avoid another memory access by putting Vms in a register
  lif_data[idx].d_vms += dT * (Vm - Vms)/0.010;  //0.00# mS filtering time constant

  //5mS filter isyn to become d_isyns, averaged version without the quick spike transients
  float Isyns = lif_data[idx].d_Isyns;
  lif_data[idx].d_Isyns += dT * ((Isyn+Istim) - Isyns)/0.005;  //0.00# mS filtering time constant, drive towards total incoming current

  //calculate the calcium T current for bursting behavior
  //g.d. smith, s. murray sherman "Fourier Analysis of Sinusoidally Driven Thalamocortical Relay Neurons and a Minimal Integrate-and-Fire-or-Burst Model" Journ. of neurophysiology v83:issue1 2000
  //h should fall if Vm > V_H, and rise if Vm < V_H.  Fall slew rate H_TAU_F should be faster than rise slew rate H_TAU_R
  float dhdt = ( Vms > V_H ) ? -1.0 * lif_data[idx].d_h_t / H_TAU_F : ( (1.0 - lif_data[idx].d_h_t) / H_TAU_R); //Smith 2000 eqn 4
  lif_data[idx].d_h_t += dT * dhdt;
  float I_ca_t = ( Vms > V_T ) ? G_T * lif_data[idx].d_h_t * (Vms - V_T) : 0.0; //eqn 5
  float x_I_ca_t = ( Isyns > 0.0 ) ? I_ca_t : 0.0; //guard-rail: only apply calcium current if some Isyn is coming in.  Otherwise non-stimulated cells will start firing at removal of inhibition
  if ( cell_switches.en_bursting == 0 ) { x_I_ca_t = 0; }

  Gref = Gref - dT * dGref_dt;


  float dvdt = (dT /Cm) * ( G_L*((E_L - Vm) + exp_term) - Isra + Iref + Istim + Isyn + Inoise  + x_I_ca_t);
  if ( !cell_switches.en_learn ) {  lif_data[idx].setThisCycle = false; }  //setThisCycle is low until a spike occurs during this learning period
                                                                           //hasBeenSet is sticky, stays true through sequencial learning cycles
  if ( Vm == 0  )  {  //if spike
	  Vm = E_K; 
	  Isra = Isra + cellParams.sra_b;
	  Gref = Gref + cellParams.delta_gref;
	  lif_data[idx].d_delay_0 = (lif_data[idx].d_delay_0 << 1) |  0x1;  //set lsb of delay chain
	  if ( cell_switches.en_cancel_xtalk && cell_switches.en_learn ) {  //xtalk suppression logic
	          if ( lif_data[idx].hasBeenSet && !lif_data[idx].setThisCycle ) {   //not the first pattern this cell has participated in, disable it to avoid xtalk
		      lif_data[idx].disable = true; 
		  }
		  lif_data[idx].hasBeenSet = true; 
		  lif_data[idx].setThisCycle = true; 
	  }
	  lif_data[idx].spikes++;
  } else {
      Vm =  Vm + dvdt;
      Isra = Isra + dT * dIsra_dt;
      lif_data[idx].d_delay_0 = (lif_data[idx].d_delay_0 << 1) ;
  }
  if ( cell_switches.en_cancel_xtalk == 0 ) {  //clear xtalk canceling parameters if cancel_xtalk is deactivated (between training of different synapse groups )
	 lif_data[idx].hasBeenSet = false; 
	 lif_data[idx].setThisCycle = false; 
  }
  if ( Vm > 0.0 )  { Vm = 0; }  //this is a spike, defined by Vm=0
  if ( Vm < 1.5*E_K )  { Vm = 1.5*E_K; }  //limit how negative the membrane voltage can be driven. A bit below Potasium reversal voltage is the practical limit
  lif_data[idx].d_vm =  Vm;
  lif_data[idx].d_Isra = Isra;
  lif_data[idx].d_Gref = Gref;

  if ( cell_switches.misc > 0 && cell_switches.printf_cell == idx ) {
      switch (cell_switches.misc) { //printf of various aspects of a cell, used by Osc/Burst_calibration for plotting waveforms
	  case 1:  printf("%.4f\t%.12f\n", lif_data[idx].now, lif_data[idx].d_h_t * 1.0e-1) ;
	      break;
	  case 2:  printf("%.4f\t%.4f\n", lif_data[idx].now, Vm) ;
	      break;
	  case 3:  printf("%.4f\t%.4f\n", lif_data[idx].now, Vms) ;
	      break;
	  case 4:  printf("%.4f\t%.4f\n", lif_data[idx].now, x_I_ca_t*5.0e+7);
	      break;
	  case 5:  
	         //printf("%.4f\t%.4f\n", lif_data[idx].now, lif_data[idx].d_Isyns * 1.0e+9);
	         printf("%.4f\t%.4f\n", lif_data[idx].now, Istim * 1.0e+8);
	      break;
	  default:
	      break;
      }
  }

  //after a lot of tinkering, this looks similar to matlab, with decent contrast
  unsigned char intensity = colorize_vm(Vm);
  d_out[idx].x = clip(8*(intensity - 80));
  d_out[idx].y = clip(8*(intensity - 55) );
  d_out[idx].z = clip(5*intensity );

  lif_data[idx].now += dT;
  lif_data[idx].d_Isyn = 0;  //clear before next synapse cycle
}
