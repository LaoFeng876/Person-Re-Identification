#pragma once
#include "readPersonImg.h"

std::vector<Mat> ReadPersonImg(const char* directory) {
	//构造类对象
	CStatDir statdir;

	Mat person;
	std::vector<Mat> feature;


	//设置要遍历的目录
	if (!statdir.SetInitDir(directory))
	{
		puts("目录不存在!");
		return feature;
	}

	//开始遍历

	std::vector<std::string>file_vec = statdir.BeginBrowseFilenames("*.*");



	for (std::vector<std::string>::const_iterator it = file_vec.begin(); it < file_vec.end(); ++it) {
		person = imread(*it,1);
		feature.push_back(GetFeatureVector_half(person));
	}
	printf("文件总数: %d\n", file_vec.size());
	return feature;
}


Mat GetTrainSet(std::vector<Mat> feature) {
	Mat trainSet(feature[0].rows,int(feature.size()),CV_32FC1);
	Mat dstMat;
	for (int idx = 0; idx < feature.size(); idx = idx + 1) {
		dstMat = trainSet.col(idx);
		feature[idx].copyTo(dstMat);
	}
	return trainSet;
}
