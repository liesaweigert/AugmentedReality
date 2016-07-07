//
//  DrawObject.h
//  AR_Project
//
//  Created by Verena Dogeanu on 07/07/16.
//  Copyright © 2016 Verena Dogeanu. All rights reserved.
//

#ifndef ObjectParser_h
#define ObjectParser_h
#define GLFW_INCLUDE_GLU
#include <math.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <fstream>

using namespace std;
using namespace Eigen;

const string AGUMON = "/Users/verenuschka/Documents/University/Master/Semester2_SoSe'16/AR/AR_Project/assets/Agumon.obj";
const string BAKEMON = "/Users/verenuschka/Documents/University/Master/Semester2_SoSe'16/AR/AR_Project/assets/Bakemon.obj";
const string BIYOMON = "/Users/verenuschka/Documents/University/Master/Semester2_SoSe'16/AR/AR_Project/assets/Biyomon.obj";
const string BULBASAUR = "/Users/verenuschka/Documents/University/Master/Semester2_SoSe'16/AR/AR_Project/assets/BR_Bulbasaur";
const string LUGIA = "/Users/verenuschka/Documents/University/Master/Semester2_SoSe'16/AR/AR_Project/assets/BR_Lugia";
const string CHARMANDER = "/Users/verenuschka/Documents/University/Master/Semester2_SoSe'16/AR/AR_Project/assets/Charmander";

struct Index
{
    unsigned int vertexIdx_1;
    unsigned int vertexIdx_2;
    unsigned int vertexIdx_3;
    unsigned int texIdx_1;
    unsigned int texIdx_2;
    unsigned int texIdx_3;
};

void drawObject(string oname);
void loadObject(char *fname);
void renderObject(vector<Index> idx, vector<Vector3f> v, vector<Vector3f> n, vector<Vector2f> uv);

#endif /* ObjectParser_h */
