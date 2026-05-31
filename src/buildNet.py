#!/usr/bin/env python3
import os,sys
import subprocess
import glob
from os import path
import json

with open('network_definition.json', 'r') as f:
    # json.load reads the entire file and deserializes it
    all_lists = json.load(f)

cellLayers = all_lists[0]
synapseList = all_lists[1]

########################################################################
###########             declarations                 ###################
###########                   &                      ###################
###########                 data                     ###################
########################################################################

timeString = "\"%2.3f Seconds\", SimTime"
leftMargin = 50

r = open('resets.inc','w')
w = open('windows.inc','w')
d = open('declarations.inc','w')
m = open('mallocs.inc','w')
g1 = open('glut1.inc','w')
g2 = open('glut2.inc','w')
g3 = open('glut3.inc','w')
g4 = open('glut4.inc','w')
cl = open('cell_launches.inc','w')
sl = open('syn_launches.inc','w')


#a list of the cell layers
#windows will be opened in the order of the list.
#all layers of a particular name must have the same size, but can have different params, and must have unique locs

#make a list of the layer groups, count them, and declare them

layerGroups = {} #dictionary of layer-name/value pairs of all layer-names and the count of their occurrances in cellLayers
cSwitchList = [] #list of cell-switches in cellLayers
sSwitchList = [] #list of synapse-switches for synapse groups
cParamList = []  #list of cell-params in cellLayers
sParamList = []  #list of syn-params in synapseList

########################################################################
###########                functions                 ###################
########################################################################

#generate synapse launch statements
def printSynapseLaunches( synapseList ) :
    for i, line in enumerate( synapseList ) :
        name = line["name"]
        print (f"synapseKernelLauncher_a (syn_stream[{i}], {name}_currents, {line["src"]},  {line["dest"]}, {name}, {name}_weights, {line["size"]}, {line["ratio"]}, {line["switch"]}, {line["radius"]});", file=sl)

#generate synapse declarations
def printSynapseDeclarations( synapseList ) :
    for line in synapseList:
        name = line["name"]
        print (f"synapse_data_type *{name};", file=d)
        print (f"float *{name}_currents;", file=d)
        print (f"float *{name}_weights;", file=d)
        foundIt = 0
        for switch in sSwitchList :
            if switch == line["switch"] :
                foundIt = 1
        if foundIt == 0 :
            sSwitchList.append(line["switch"])
            print (f"syn_switchesStruct {line["switch"]};", file=d)

#generate synapse mallocs
def printSynapseMallocs( synapseList ) :
    print("\n\n",file=m)
    for line in synapseList:
        name = line["name"]
        size = line["size"]
        radius = line["radius"]
        #note that weight-sharing parameter WS=0 requires size^2*(2*RADIUS+1)^2 weights, WS=1 requires (2*RADIUS+1)^2 weights. 
        weight_count = f"(2*{radius}+1)*(2*{radius}+1)"
        if ( line["ws"] ) == 0 :
            weight_count = f"{size}*{size}*(2*{radius}+1)*(2*{radius}+1)"

        print (f"cudaMalloc((void **)&{name}, {size}*{size}*sizeof( synapse_data_type ));", file=m)   #one per cell
        print (f"cudaMalloc((void **)&{name}_currents, {size}*{size}*(2*{radius}+1)*(2*{radius}+1)*sizeof( float ));", file=m)   #one per synapse (#cells * fanout)
        print (f"cudaMalloc((void **)&{name}_weights, {weight_count}*sizeof( float ));", file=m)

#generate synapse resets
def printSynapseResets( synapseList ) :
    for line in synapseList:
        foundIt = 0
        for paramFileName in sParamList :
            if paramFileName == line["params"] :
                foundIt = 1
        if foundIt == 0 :
            sParamList.append(line["params"])
            print (f"synParamStruct {line["params"]};", file=d)
            print (f"strcpy(fileName, \"{line["params"]}.txt\"); retCode = loadSynParams( fileName, &{line["params"]} );", file=r)
        name = line["name"]
        size = line["size"]
        rf = line["rf"]
        radius = line["radius"]
        print (f"{line["params"]}.ws = {line["ws"]};", file=r)
        print (f"resetSnaps( {name}, {name}_weights, {size}, {line["params"]}, {radius}, {rf});", file=r)

#scan the cellLayer dictionary list and create a list of key/values uniquely specifying the layer names and their count
def findLayerGroups( cellLayerList ) :
    i = 0
    for i, layer in enumerate(cellLayerList):
        foundIt = 0;
        for name in layerGroups :
            if name == layer["name"] :
                #this layer group has already been encountered
                foundIt = 1
            layerGroups[name] = layerGroups[name] + 1
        if foundIt == 0 :
                #this layer hasn't been seen yet.  Add it to the layerGroups dictionary
                name = layer["name"]
                layerGroups[name] = 1
    return layerGroups

#scan the cellLayer dictionary and create a list of cell switches needing to be declared
def findCellSwitchList( cellLayerList ) :
    for layer in cellLayerList:
        foundIt = 0;
        for name in cSwitchList :
            if name == layer["switch"] :
                #this layer group has already been encountered
                foundIt = 1
        if foundIt == 0 :
                #this layer hasn't been seen yet.  Add it to the layerGroups dictionary
                cSwitchList.append(  layer["switch"] )
    return cSwitchList

#scan the cellLayer dictionary and create a list of cell params needing to be declared
def findCellParamList( cellLayerList ) :
    for layer in cellLayerList:
        foundIt = 0;
        for name in cParamList :
            if name == layer["params"] :
                #this layer group has already been encountered
                foundIt = 1
        if foundIt == 0 :
                #this layer hasn't been seen yet.  Add it to the layerGroups dictionary
                cParamList.append(  layer["params"] )
    return cParamList


#create the include files containing glut boilerplate for maintaining & refreshing the cell-array Vm heat mapts, etc
def printGlutIncFiles ( layerGoups ) :
    displayFunc = "display"  #the first timed displa gets called, use this.  Every other time, use display_nul
    for name in layerGroups :
        count = 0
        #find all the layers with this name
        for i, layer in enumerate(cellLayers) :
            if ( layer["name"]  == name ) :
                size = layer["size"]
                params = layer["params"]
                nameIndex = f"{name}_{count}"
                txt = f"resetCells( {nameIndex}, {layer["size"]}, {layer["params"]});"
                print (txt, file=r)
                #txt = f"cudaMalloc((void **)\&{nameIndex}, {size}*{size}*sizeof( lif_data_type ));"
                txt = f"cudaMalloc((void **)&{nameIndex}, {size}*{size}*sizeof( lif_data_type ));"
                print (txt, file=m)
                nameIndex = f"{name}[{count}]"
                txt = f"glutSetWindow( {nameIndex}.win[0]); glutKeyboardFunc(keyboard); initPixelBuffer( &{nameIndex}.pbo[0], &{nameIndex}.tex[0], "
                txt = txt + f"&{nameIndex}.cuda_pbo_resource[0], {size} );"
                txt = txt + f" gluOrtho2D(0, {size}, {size}, 0);  glutMouseFunc(mouse); glutMotionFunc(mouseDrag); glutDisplayFunc({displayFunc});"
                displayFunc = "display_nul"
                print (txt, file=g4)
                if layer["title"] == "timeString" :
                    layer["title"] = timeString
                txt = f"glutSetWindow( {nameIndex}.win[0]); sprintf(title, {layer["title"]}); draw_texture( {size}, {size}); glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers();"
                #txt = f"glutSetWindow( {nameIndex}.win[0]); sprintf(title, {layer["title"]}); draw_texture( {size}, {size}); glutSetWindowTitle(title); glutPostRedisplay(); glutSwapBuffers();"
                print (txt, file=g3)
                txt = f"cudaGraphicsUnmapResources(1, &{nameIndex}.cuda_pbo_resource[0], 0);"
                print (txt, file=g2)
                txt = f"cudaGraphicsMapResources(1, &{nameIndex}.cuda_pbo_resource[0], 0); cudaGraphicsResourceGetMappedPointer((void **)&{nameIndex}.vm,  NULL, {nameIndex}.cuda_pbo_resource[0]);"
                print (txt, file=g1)
                txt = f"cellKernelLauncher_aelif(cell_stream[{i}], {nameIndex}.vm,   {name}_{count}, {size},  {layer["switch"]},  {layer["params"]} );"
                print (txt, file=cl)
                #window locations
                x = layer["loc"]["x"] + leftMargin
                y = layer["loc"]["y"]
                txt1 = f"glutInitWindowSize({size}, {size});"
                txt2 = f"glutInitWindowPosition({x}, {y});    {nameIndex}.win[0] = glutCreateWindow(\"W{i}\");"
                print (txt1, txt2, file=w)
                count = count + 1
    g1.close( )
    g2.close( )
    g3.close( )
    g4.close( )
    cl.close( )

#create the include file containing declarations related to cell layers and their control files
def printDeclarations ( layerGroups ) :
    for name in cSwitchList :
        print (f"cell_switchesStruct {name};", file=d)
    for name in cParamList :
        print (f"cellParamStruct {name};", file=d)
        print (f"strcpy(fileName, \"{name}.txt\"); retCode = readCellParamsFromFile( fileName, &{name} );", file=r)
    for name in layerGroups :
        count = layerGroups[name] 
        print (f"layer {name}[{count}];", file=d)
    for name in layerGroups :
        count = layerGroups[name] 
        for i in range( count ) :
            layerName = f"{name}_{i}"
            print (f"lif_data_type *{layerName};", file=d)

########################################################################
###########                  main                    ###################
########################################################################

layerGroups = findLayerGroups( cellLayers )
cSwitchList = findCellSwitchList( cellLayers ) 
cParamList = findCellParamList( cellLayers ) 
printDeclarations ( layerGroups )
printGlutIncFiles ( layerGroups )

printSynapseDeclarations( synapseList )
printSynapseMallocs( synapseList )
printSynapseResets( synapseList )
printSynapseLaunches( synapseList )

#os.system("cat windows.inc > misc.inc" )
#os.system("cat resets.inc >> misc.inc" )
