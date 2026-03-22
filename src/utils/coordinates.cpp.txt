#include "coordinates.h"
//This turns x,y,z,w into one single number for the computer
int get4DIndex(int x, int y, int z, int w) {
    //Based on 4x4x4x4
    return x + (4 * y) + (16 * z) + (64 * w);
}