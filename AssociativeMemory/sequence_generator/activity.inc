
double SimTime  = 0;
int   iterations = 0;
int   iterations_prev = 0;
int   deltaIterations = 0;


//used by stimulus-spot generator
int2 loc = {1, 1};   //contains loc.x, loc.y for communicating mouse or track location to the device
double loc_x , loc_y ; //current locatin in floating point for track path calculation
int    loc_x_0 = 50.0;  //putting 1 in here causes boundry problems
int    loc_y_0 = 400.0;


unsigned int pause_animation = 0;

void keyboard(unsigned char key, int x, int y) {
  if (key == 'p') {pause_animation = ( pause_animation == 0 ) ? 1 : 0; printf("Pause Animation = %d\n",pause_animation);}
  if (key == 27) exit(0);
  glutPostRedisplay();
}

void mouseDrag(int x, int y) {
  glutPostRedisplay();
}

void printInstructions() {
  printf("Instructions:\n"
         "Pause: p\n"
         "Exit                           : Esc\n");
}


void mouse(int button, int state, int x, int y){}

int cue = 0;
void newPat (int state, int count, int cue ){
	int bigPat, smallPat;
	switch ( count ){
               case  0: smallPat = 11;
	       		if ( state == 0 ) {bigPat = 9;} else { bigPat = 6; }
                        break;
               case  1: smallPat = 15;
	       		if ( state == 0 ) {bigPat = 6;} else { bigPat = 7; }
                        break;
               case  2: smallPat = 13;
	       		if ( state == 0 ) {bigPat = 7;} else { bigPat = 8; }
                        break;
               case  3: smallPat = 14;
	       		if ( state == 0 ) {bigPat = 8;} else { bigPat = 5; }
                        break;
               case  4: smallPat = 12;
	       		if ( state == 0 ) {bigPat = 5;} else { bigPat = 10; }
                        break;
               case  5: smallPat = 16;
	       		if ( state == 0 ) {bigPat = 10;} else { bigPat = 9; }
                        break;
               default:
                        break;
       }
       printf("[newPat][%d][%d]\tb[%d]\ts[%d]\n",state, count, bigPat, smallPat );
       genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, bigPat, cue );
       genPats( in_pat_j, d_in_pat_j, J_cellLayer_0,  SMALL, smallPat,cue );
}//newPat


int A_stimulus_type = 0;     //state control of stimulus loop.
int A_stim_state = 0;        //state control of stimulus loop.
int A_counter = 0;

void idle(void) {
  ++iterations;  //increments every render, (DTS_PER_RENDER * dT) seconds
  SimTime += DTS_PER_RENDER * dT;  //this routine gets called every DTS_PER_RENDER dTs
  if ( iterations == 1 ) {
          pause_animation = 0; //set to 1 if you want an initial pause
          printf("********************\n");
	  printf("DTS_PER+RENDER=%d\tdT=%.4f\n",DTS_PER_RENDER, dT );
          printf("********************\n");
	  syn_switches_i.en_syn == 1;
	  syn_switches_i.en_learn == 0;
	  syn_switches_1.en_learn == 0;
          cell_switches_i.en_input = 2;     //enable input patterns from genPat in somaKernel_aelif.inc .  On through the entire sequence
          syn_switches_a0a0.en_syn = 0; syn_switches_a0b0.en_syn = 0; 
          syn_switches_b0b0.en_syn = 0; syn_switches_b0a0.en_syn = 0; 
	  //synaps-group names are used only for debug, no functional purpose
	  syn_switches_i.name = 0;
	  syn_switches_1.name = 1;
	  syn_switches_a0b0.name = 2;
	  syn_switches_b0a0.name = 3;
  }



    deltaIterations = iterations - iterations_prev;
    //time is a float, so == doesn't work. 
    //calculate time intervals based on iterations, which is DTS_PER_RENDER*DT.  Set DTS_PER_RENDER to 10, so each iteration is 1mS

    //train with lateral and {big/small, small/big} synapses disabled. set syn_switches_a0a0.en_syn=0 in main at beginning of time
    if ( A_stim_state == 0 ) {  //
       //printf("A_stim_state=%d\tA_counter=%d\n",A_stim_state, A_counter);
       cell_switches2.en_cancel_xtalk = true;  //tell cells in learning layers to cancel xtalk. This gets left on the entire sim

       if (  deltaIterations ==  10  ) { 
	       newPat(A_stim_state, A_counter, 0); 
	       syn_switches_a0a0.en_learn = syn_switches_b0b0.en_learn = syn_switches_a0b0.en_learn = cell_switches2.en_learn = 1;
       }
       if (  deltaIterations ==  70 ) { //turn off stimulus
	       genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, 0, 0 );
	       genPats( in_pat_j, d_in_pat_j, J_cellLayer_0,  SMALL, 0, 0 );
               syn_switches_a0a0.en_learn = syn_switches_b0b0.en_learn = syn_switches_a0b0.en_learn = cell_switches2.en_learn = 0;
       }
       if (  deltaIterations ==  80 ) { 
         iterations_prev = iterations; deltaIterations = 0; 
         if ( A_counter == 5 ) {
           A_stim_state = 2;
           A_counter = 0;
           printf("*********Entering State %d\n", A_stim_state);
         } else { A_counter++; }
       }
    }    

    //this is a debug state, check if learned pattern in A produces learned pattern in B
    if ( A_stim_state == 1 ) {  
       syn_switches_a0a0.en_learn = syn_switches_b0b0.en_learn = syn_switches_a0b0.en_learn = syn_switches_b0a0.en_learn = 0;
       if (  deltaIterations ==  10 ) { 
	       newPat(A_stim_state, A_counter, 1); 
	       genPats( in_pat_j, d_in_pat_j, J_cellLayer_0,  SMALL, 0, 0 );
               syn_switches_a0a0.en_syn = 1; syn_switches_a0b0.en_syn = 1; 
               syn_switches_b0b0.en_syn = 1; syn_switches_b0a0.en_syn = 0; 
       }
       if (  deltaIterations ==  50 ) {  //turn off the stimulus
	       genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, 0, 0 );
	       genPats( in_pat_j, d_in_pat_j, J_cellLayer_0,  SMALL, 0, 0 );
       }
       if (  deltaIterations ==  90 ) { 
               syn_switches_a0a0.en_syn = 0; syn_switches_a0b0.en_syn = 0; 
               syn_switches_b0b0.en_syn = 0; syn_switches_b0a0.en_syn = 0; 
       }
       if (  deltaIterations ==  100) {
         iterations_prev = iterations; deltaIterations = 0; 
         if ( A_counter == 5 ) {
           A_stim_state = 2;
           A_counter = 0;
           printf("*********Entering State %d\n\n", A_stim_state);
         } else { A_counter++; }
       }
    }

    //train the s2b synapses. Turn off learning in the a1 & b1 lateral, and a2b synapses.
    if ( A_stim_state == 2 ) {  //
	//only ff synapses enabled during training.  lateral, s2b and b2s turned off
       syn_switches_a0a0.en_syn = 0; syn_switches_a0b0.en_syn = 0; 
       syn_switches_b0b0.en_syn = 0; syn_switches_b0a0.en_syn = 0;
       syn_switches_a0a0.en_learn = syn_switches_b0b0.en_learn = syn_switches_a0b0.en_learn = 0; 
       if (  deltaIterations ==  10  ) { 
	       syn_switches_b0a0.en_learn = 1;
	       newPat(A_stim_state, A_counter, 0); 
       }
       if (  deltaIterations ==  70 ) { //turn off stimulus and learning
	       genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, 0, 0 );
	       genPats( in_pat_j, d_in_pat_j, J_cellLayer_0,  SMALL, 0, 0 );
	       syn_switches_b0a0.en_learn = 0;
       }
       if (  deltaIterations ==  80 ) { 
         iterations_prev = iterations; deltaIterations = 0; 
         if ( A_counter == 5 ) {
           A_stim_state = 4; //skip 2 if test-recall pass is not required
           A_counter = 0;
           syn_switches_b0a0.en_learn = 0;
           printf("*********Entering State %d\n", A_stim_state);
         } else { A_counter++; }
       }
    }    

    //this is a debug state, check if learned pattern in B produces learned pattern in A
    if ( A_stim_state == 3 ) {  
       syn_switches_a0a0.en_learn = syn_switches_b0b0.en_learn = syn_switches_a0b0.en_learn = syn_switches_b0a0.en_learn = 0;
       if (  deltaIterations ==  10 ) { 
	       newPat(A_stim_state, A_counter, 1); 
	       genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, 0, 0 );  //turn off stim through the big path
               syn_switches_a0a0.en_syn = 1; syn_switches_a0b0.en_syn = 0; 
               syn_switches_b0b0.en_syn = 1; syn_switches_b0a0.en_syn = 1; 
       }
       if (  deltaIterations ==  50 ) {  //turn off the stimulus
	       genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, 0, 0 );
	       genPats( in_pat_j, d_in_pat_j, J_cellLayer_0,  SMALL, 0, 0 );
       }
       if (  deltaIterations ==  90 ) { 
               syn_switches_a0a0.en_syn = 0; syn_switches_a0b0.en_syn = 0; 
               syn_switches_b0b0.en_syn = 0; syn_switches_b0a0.en_syn = 0; 
       }
       if (  deltaIterations ==  100) {
         iterations_prev = iterations; deltaIterations = 0; 
         if ( A_counter == 5 ) {
           A_stim_state = 4;
           A_counter = 0;
           printf("*********Entering State %d\n\n", A_stim_state);
         } else { A_counter++; }
       }
    }

    //cue the first pattern in the big array
    if ( A_stim_state == 4 ) {  
       if (  deltaIterations ==  10 ) { 
               genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, 9, 1 );
	       syn_switches_a0a0.en_syn = 1; syn_switches_a0b0.en_syn = 1; 
	       syn_switches_b0b0.en_syn = 1; syn_switches_b0a0.en_syn = 0;
       }
       if (  deltaIterations ==  60 ) { 
	       genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, 0, 0 );
       }
       if (  deltaIterations ==  130 ) { syn_switches_a0b0.en_syn = 0; syn_switches_a0a0.en_syn = 1; syn_switches_b0b0.en_syn = 1; syn_switches_b0a0.en_syn = 0;}
       if (  deltaIterations ==  160) {
         iterations_prev = iterations; deltaIterations = 0; 
         A_stim_state = 5;
         printf("*********Entering State %d\n\n", A_stim_state);
       } else { A_counter++; }
    }//state 3

    //alternate between the two arrays
    if ( A_stim_state == 5 ) {  
       if (  deltaIterations ==  1 ) {   //b is remembering
		syn_switches_a0a0.en_syn = 0; syn_switches_a0b0.en_syn = 0; 
		syn_switches_b0b0.en_syn = 1; syn_switches_b0a0.en_syn = 0; 
       }
       if (  deltaIterations ==  20 ) { //b drives onto a
		syn_switches_a0a0.en_syn = 1; syn_switches_a0b0.en_syn = 0; 
		syn_switches_b0b0.en_syn = 1; syn_switches_b0a0.en_syn = 1; 
       }
       if (  deltaIterations ==  50 ) { //a is remembering
		syn_switches_a0a0.en_syn = 1; syn_switches_a0b0.en_syn = 0; 
		syn_switches_b0b0.en_syn = 0; syn_switches_b0a0.en_syn = 0; 
       }
       if (  deltaIterations ==  80 ) { //a drives onto b
		syn_switches_a0a0.en_syn = 1; syn_switches_a0b0.en_syn = 1; 
		syn_switches_b0b0.en_syn = 1; syn_switches_b0a0.en_syn = 0; 
       }
       if (  deltaIterations ==  100) {
         iterations_prev = iterations; deltaIterations = 0; 
         printf("*********Completed one sequence step*******************\n");
       }
    }//state 4


/***************************************************************************************************/

    /*
    if ( (iterations % 50)==0 || iterations == 10 ) {   //every (DTS_PER_RENDER * dT * mod-value )
       printf("*** Time=%.4f Seconds iterations[%d] ***\n", SimTime, iterations );
    }
    */
}//idle

