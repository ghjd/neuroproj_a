
double SimTime  = 0;
int   iterations = 0;
int   iterations_prev = 0;
int   deltaIterations = 0;


//used by stimulus-spot generator
int2 loc = {0, 0};   //contains loc.x, loc.y for communicating mouse or track location to the device

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

void newPat (int state, int count ){
	int bigPat, smallPat;
	switch ( count ){
               case  0: bigPat = 1; //spot
                        break;
               case  1: bigPat = 3; //vertical
                        break;
               case  2: bigPat = 4; //backslash
                        break;
               case  3: bigPat = 5; //forward slash
                        break;
               case  4: bigPat = 6; //horizontal squigle
                        break;
               case  5: bigPat = 7; //vertical squigle
                        break;
               case  6: bigPat = 2; //horizontal bar
                        break;
               default:
                        break;
       }
       //rand() % (max_number + 1 - minimum_number) + minimum_number
       rand(); rand(); rand(); 
       loc.x = rand() % (BIG - 4*RADIUS + 1 ) + 2*RADIUS;
       loc.y = rand() % (BIG - 4*RADIUS + 1 ) + 2*RADIUS;
       loc.x = loc.x - BIG/2;
       loc.y = loc.y - BIG/2;
       printf("[newPat][%d][%d]\t[%d]\n",state, count, bigPat );
       genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, bigPat, loc );
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
          cell_switches_i.en_input = 2;     //enable input patterns from genPat in somaKernel_aelif.inc .  On through the entire sequence
	  syn_switches_i.en_syn == 1;
          syn_switches_a0a1.en_syn = 1; 
          syn_switches_a1a0.en_syn = 1; 
          syn_switches_b0a1.en_syn = 1; 
  }



    deltaIterations = iterations - iterations_prev;
    //time is a float, so == doesn't work. 
    //calculate time intervals based on iterations, which is DTS_PER_RENDER*DT.  Set DTS_PER_RENDER to 10, so each iteration is 1mS

    //train with lateral and {big/small, small/big} synapses disabled. set syn_switches_a0a0.en_syn=0 in main at beginning of time
    if ( A_stim_state == 0 ) {  //
       //printf("A_stim_state=%d\tA_counter=%d\n",A_stim_state, A_counter);
       //void genPats( float * pattern, float * d_pattern, lif_data_type* cell_layer, int width, int sel, int2 loc  )
       if (  deltaIterations ==  0  ) { 
	       DTS_PER_RENDER = 10;
       }
       if (  deltaIterations ==  5  ) {  //set up 
	       newPat(A_stim_state, A_counter); 
       }
       if (  deltaIterations ==  100 ) { //turn off stimulus
	       genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, 0, loc );
       }
       if (  deltaIterations ==  110 ) { 
       }
       if (  deltaIterations ==  115 ) { 
         iterations_prev = iterations; deltaIterations = 0; 
         if ( A_counter == 5 ) {
           A_stim_state = 1;
           A_counter = 0; 
           printf("*********Entering State %d\n", A_stim_state);
         } else { A_counter++;  printf("************State %d\tCount %d\n", A_stim_state, A_counter);}
       }
    }    
    if ( A_stim_state == 1 ) {  //
       if (  deltaIterations ==  0  ) { 
	       DTS_PER_RENDER = 40;
               genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, 0, loc ); //send in a blank pattern
       }
       if (  deltaIterations ==  5  ) {  //set up horizontal line segment input pattern
	       newPat(A_stim_state, 6); 
       }
       if (  deltaIterations ==  30  ) {  //turn off horizontal bar input pattern
               genPats( in_pat_i, d_in_pat_i, I_cellLayer_0,  BIG, 0, loc ); //send in a blank pattern
       }
       if (  deltaIterations ==  150  ) { 
           iterations_prev = iterations; deltaIterations = 0; 
           A_stim_state = 0;
           A_counter = 0; 
           printf("*********Entering State %d\n", A_stim_state);
       }
    }    


/***************************************************************************************************/

    /*
    if ( (iterations % 50)==0 || iterations == 10 ) {   //every (DTS_PER_RENDER * dT * mod-value )
       printf("*** Time=%.4f Seconds iterations[%d] ***\n", SimTime, iterations );
    }
    */
}//idle

