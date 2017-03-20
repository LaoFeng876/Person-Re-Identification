#pragma once

#include <opencv2\opencv.hpp>

using namespace cv;

Mat* CreatMask(const Mat img);
Mat* GetRGBFeature(const Mat img, Mat* mask);
Mat* GetHSVFeature(const Mat img, Mat* mask);
Mat* GetYCbCrFeature(const Mat img, Mat* mask);
Mat* GetLabFeature(const Mat img, Mat* mask);
void Matcpy(Mat src, Mat dst, int& num);
Mat GetFeatureVector(Mat img);
Mat GetFeatureVector_half(Mat img);

