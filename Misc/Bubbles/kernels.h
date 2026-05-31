#ifndef KERNEL1_H
#define KERNEL1_H

#include "kernel_common.h"

void resetCells(lif_data_type *lif_data, int w, cellParamStruct cellParams );
void resetSnaps (synapse_data_type *synapse_ptr, float *weights, int w, synParamStruct synParams, int radius, int rfield_type);
void synapseKernelLauncher_a(cudaStream_t stream, float *syn_currents, lif_data_type *lif_data_a, lif_data_type *lif_data_b, synapse_data_type *synapse, float *weights, int w, int ratio, syn_switchesStruct syn_switches, int radius );
void I_loadPat (lif_data_type *layer, int w, float *pattern );
void cellKernelLauncher_aelif(cudaStream_t stream, uchar4 *d_out,  lif_data_type *lif_data, int w, cell_switchesStruct cell_switches, cellParamStruct cellParams ); //was cellKernelLauncher2


#endif
