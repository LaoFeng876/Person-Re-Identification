#pragma once
#include <iostream>
#include <string>
#include "CBrowseDir.h"
#include "getFeatureVector.h"

std::vector<Mat> ReadPersonImg(const char* directory);
Mat GetTrainSet(std::vector<Mat> feature);
