

void initGLUT(int *argc, char **argv ) {
  int Yrow0 = 100; 
  int Yrow1 = Yrow0 + 300; 
  int LeftEdge = 100;

  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  
  //the small arrays
  glutInitWindowSize(SMALL, SMALL);
  glutInitWindowPosition(LeftEdge + 620, Yrow1); 	B_layers[1].win[0] = glutCreateWindow("Window 3");
  glutInitWindowPosition(LeftEdge + 920, Yrow1); 	B_layers[2].win[0] = glutCreateWindow("Window 3");
  glutInitWindowPosition(LeftEdge + 1220, Yrow1); 	B_layers[3].win[0] = glutCreateWindow("Window 3");
  glutInitWindowPosition(LeftEdge + 1520, Yrow1); 	B_layers[4].win[0] = glutCreateWindow("Window 3");

  //the big arrays
  glutInitWindowSize(BIG, BIG);
  glutInitWindowPosition(LeftEdge + 70, Yrow0); 	I_layer.win[0] = glutCreateWindow("Window 1");
  glutInitWindowPosition(LeftEdge + 370, Yrow0);  	A_layers[1].win[0] = glutCreateWindow("Window 2");
  glutInitWindowPosition(LeftEdge + 670, Yrow0); 	A_layers[2].win[0] = glutCreateWindow("Window 3");
  glutInitWindowPosition(LeftEdge + 970, Yrow0); 	A_layers[3].win[0] = glutCreateWindow("Window 4");
  glutInitWindowPosition(LeftEdge + 1270, Yrow0); 	A_layers[4].win[0] = glutCreateWindow("Window 5");
  glutInitWindowPosition(LeftEdge + 1570, Yrow0); 	A_layers[5].win[0] = glutCreateWindow("Window 6");

  glewInit();
}

void resetNetwork() {
  char fileName[100];
  int retCode;
  strcpy(fileName, "cell_params_layer_BIG.txt"); retCode = readCellParamsFromFile( fileName, &layer1_params );
  resetCells( I_cellLayer,  BIG, layer1_params );
  resetCells( A_cellLayer1, BIG, layer1_params );
  resetCells( A_cellLayer2, BIG, layer1_params );  
  resetCells( A_cellLayer3, BIG, layer1_params );  
  resetCells( A_cellLayer4, BIG, layer1_params );  
  resetCells( A_cellLayer5, BIG, layer1_params );  
  resetCells( B_cellLayer2, SMALL,layer1_params );  
  resetCells( B_cellLayer3, SMALL,layer1_params );  
  resetCells( B_cellLayer4, SMALL,layer1_params );  
  resetCells( B_cellLayer5, SMALL,layer1_params );  

  strcpy(fileName, "syn_params.txt");       retCode = loadSynParams( fileName, &syn_params ) ;
  strcpy(fileName, "syn_inhib_params.txt"); retCode = loadSynParams( fileName, &syn_inhib_params ) ;
  syn_params.ws = 1;
  syn_inhib_params.ws = 1;
  resetSnaps( syn_I_A1  , weights_a1,   BIG,   syn_params, RADIUS, R_AE );
  resetSnaps( syn_A1_A2 , weights_a2,   BIG,   syn_params, RADIUS, R_AE );
  resetSnaps( syn_A2_A3 , weights_a3,   BIG,   syn_params, RADIUS, R_AE );
  resetSnaps( syn_A3_A4 , weights_a4,   BIG,   syn_params, RADIUS, R_AE );
  resetSnaps( syn_A4_A5 , weights_a5,   BIG,   syn_params, RADIUS, R_AE );

  resetSnaps( syn_A2_B2 , weights_b1,   SMALL, syn_params, RADIUS, R_AE );
  resetSnaps( syn_A3_B3 , weights_b2,   SMALL, syn_params, RADIUS, R_AE );
  resetSnaps( syn_A4_B4 , weights_b3,   SMALL, syn_params, RADIUS, R_AE );
  resetSnaps( syn_A5_B5 , weights_b4,   SMALL, syn_params, RADIUS, R_AE );

  resetSnaps( syn_B1_A2 , weights_c1,   SMALL, syn_inhib_params, RADIUS, R_AE );
  resetSnaps( syn_B2_A3 , weights_c2,   SMALL, syn_inhib_params, RADIUS, R_AE );
  resetSnaps( syn_B3_A4 , weights_c3,   SMALL, syn_inhib_params, RADIUS, R_AE );
  resetSnaps( syn_B4_A5 , weights_c4,   SMALL, syn_inhib_params, RADIUS, R_AE );
}

