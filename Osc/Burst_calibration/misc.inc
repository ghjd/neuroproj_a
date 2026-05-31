

void initGLUT(int *argc, char **argv ) {
  int Yrow0 = 100; 
  int Yrow1 = Yrow0 + 320; 
  int LeftEdge = 100;

  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  
  //the big arrays
  glutInitWindowSize(BIG, BIG);
  glutInitWindowPosition(LeftEdge + 70, Yrow0); 	I_layer.win[0] = glutCreateWindow("Window 1");
  glutInitWindowPosition(LeftEdge + 370, Yrow0);  	A_layers[1].win[0] = glutCreateWindow("Window 1");
  glutInitWindowPosition(LeftEdge + 670, Yrow0); 	A_layers[2].win[0] = glutCreateWindow("Window 1");
  glewInit();
}

void resetNetwork() {
  char fileName[100];
  int retCode;
  strcpy(fileName, "cell_params_layer_300.txt"); retCode = readCellParamsFromFile( fileName, &layer1_params );
  resetCells( I_cellLayer,  BIG, layer1_params );
  resetCells( A_cellLayer1, BIG, layer1_params );
  resetCells( A_cellLayer2, BIG, layer1_params );  

  strcpy(fileName, "syn_params.txt");       retCode = loadSynParams( fileName, &syn_params ) ;
  syn_params.ws = 1;
  resetSnaps( syn_I_A1  , weights_a1,   BIG, syn_params, RADIUS, R_AE );
  resetSnaps( syn_A1_A2 , weights_a2,   BIG, syn_params, RADIUS, R_AE );
  syn_params.ws = 1;

}

