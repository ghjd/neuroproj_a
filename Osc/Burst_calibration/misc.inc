

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
  /*
  glutInitWindowPosition(LeftEdge + 970, Yrow0); 	A_layers[3].win[0] = glutCreateWindow("Window 1");
  glutInitWindowPosition(LeftEdge + 1270, Yrow0); 	A_layers[4].win[0] = glutCreateWindow("Window 1");
  glutInitWindowPosition(LeftEdge + 1570, Yrow0); 	A_layers[5].win[0] = glutCreateWindow("Window 1");
  glutInitWindowPosition(LeftEdge + 70, Yrow1); 	J_layer.win[0] = glutCreateWindow("Window 1");      
  glutInitWindowPosition(LeftEdge + 370, Yrow1); 	B_layers[1].win[0] = glutCreateWindow("Window 1");
  glutInitWindowPosition(LeftEdge + 670, Yrow1); 	B_layers[2].win[0] = glutCreateWindow("Window 1");
  glutInitWindowPosition(LeftEdge + 970, Yrow1); 	B_layers[3].win[0] = glutCreateWindow("Window 1");
  glutInitWindowPosition(LeftEdge + 1270, Yrow1); 	B_layers[4].win[0] = glutCreateWindow("Window 1");
  glutInitWindowPosition(LeftEdge + 1570, Yrow1); 	B_layers[5].win[0] = glutCreateWindow("Window 1");
  */
  glewInit();
}

void resetNetwork() {
  char fileName[100];
  int retCode;
  strcpy(fileName, "cell_params_layer_300.txt"); retCode = readCellParamsFromFile( fileName, &layer1_params );
  resetCells( I_cellLayer,  BIG, layer1_params );
  resetCells( A_cellLayer1, BIG, layer1_params );
  resetCells( A_cellLayer2, BIG, layer1_params );  
  /*
  resetCells( A_cellLayer3, BIG, layer1_params );  
  resetCells( A_cellLayer4, BIG, layer1_params );  
  resetCells( A_cellLayer5, BIG, layer1_params );  
  resetCells( J_cellLayer,  BIG, layer1_params );
  resetCells( B_cellLayer1, BIG, layer1_params );
  resetCells( B_cellLayer2, BIG, layer1_params );  
  resetCells( B_cellLayer3, BIG, layer1_params );  
  resetCells( B_cellLayer4, BIG, layer1_params );  
  resetCells( B_cellLayer5, BIG, layer1_params );  
  */

  strcpy(fileName, "syn_params.txt");       retCode = loadSynParams( fileName, &syn_params ) ;
  resetSnaps( syn_I_A1  , weights_a1,   BIG, syn_params, RADIUS, R_AE );
  resetSnaps( syn_A1_A2 , weights_a2,   BIG, syn_params, RADIUS, R_AE );
  /*
  resetSnaps( syn_A2_A3 , weights_a3,   BIG, syn_params, RADIUS, R_AE );
  resetSnaps( syn_A3_A4 , weights_a4,   BIG, syn_params, RADIUS, R_AE );
  resetSnaps( syn_A4_A5 , weights_a5,   BIG, syn_params, RADIUS, R_AE );
  resetSnaps( syn_I_B1  , weights_b1,   BIG, syn_params, RADIUS, R_CS );
  resetSnaps( syn_B1_B2 , weights_b2,   BIG, syn_params, RADIUS, R_CS );
  resetSnaps( syn_B2_B3 , weights_b3,   BIG, syn_params, RADIUS, R_CS );
  resetSnaps( syn_B3_B4 , weights_b4,   BIG, syn_params, RADIUS, R_CS );
  resetSnaps( syn_B4_B5 , weights_b5,   BIG, syn_params, RADIUS, R_CS );
  */

}

