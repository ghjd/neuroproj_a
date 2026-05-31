
double SimTime  = 0;
double SimTime_prev  = 0;

int2 loc = {1, 1};   //contains loc.x, loc.y for communicating mouse or track location to the device
double loc_x , loc_y ; //current locatin in floating point for track path calculation
int    loc_x_0 = 50.0;  //putting 1 in here causes boundry problems
int    loc_y_0 = 400.0;

int iterations = 0;

unsigned int pause_animation = 0;

void keyboard(unsigned char key, int x, int y) {
  if (key == 'p') {pause_animation = ( pause_animation == 0 ) ? 1 : 0; printf("Pause Animation = %d\n",pause_animation);}
  if (key == 'r') { resetNetwork(); }
  if (key == 27) exit(0);
  glutPostRedisplay();
}

void mouseDrag(int x, int y) {
  loc.x = x;
  loc.y = y;
  //printf("mouseDrag X=%d\tY=%d\n", x, y);
  glutPostRedisplay();
}


void printInstructions() {
  printf("Instructions:\n"
         "Pause: p\n"
         "Exit                           : Esc\n");
}


void mouse(int button, int state, int x, int y){}

void idle(void) {
  int step = 2;
  ++iterations;  //increments every render, (DTS_PER_RENDER * dT) seconds
  SimTime += DTS_PER_RENDER * dT;  //this routine gets called every DTS_PER_RENDER dTs
  //start in pause state after one iteration to initialize the image
  
  if ( iterations == 1 ) { 
	  pause_animation = 0; //set to 1 if you want an initial pause 
	  //A_loc = {100, 100};
	  printf("********************\n");
	  printf("********************\n");
	  //printf("pausing at [%d][%d]...\n",A_loc.x, A_loc.y);
  }
  


  //move the stimulus spot every 80mS
  if ( (SimTime - SimTime_prev) > 0.05 )  {   
       SimTime_prev = SimTime;
       //rand() % (max_number + 1 - minimum_number) + minimum_number
       int2 loc;
       rand(); rand(); rand(); rand();
       loc.x = rand() % (BIG - 4*RADIUS + 1 ) + 2*RADIUS;
       loc.y = rand() % (BIG - 4*RADIUS + 1 ) + 2*RADIUS;
       rand(); rand(); rand();
       cell_switchesi.loc = loc;
       printf("[StimSpot][%d][%d]\n",cell_switches1.loc.x, cell_switches1.loc.y);
   }

/***************************************************************************************************/


    if ( (iterations % 100)==0 || iterations == 10 ) {   //every (DTS_PER_RENDER * dT * 100 )
       printf("*** Time=%.4f Seconds iterations[%d] ***\n", SimTime, iterations );
    }
    //automatically terminate simulation when doing automated tuning. Use 0.5uS for nci
    //if ( SimTime > 0.010 ) {   exit(0); }
}//idle

