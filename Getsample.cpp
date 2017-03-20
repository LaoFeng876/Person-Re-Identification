#include<iostream>  
#include<opencv2\core\core.hpp>  
#include<opencv2\highgui\highgui.hpp>  
#include<opencv\cv.h>
#include<ctime>  
#include "getFeatureVector.h"
#include "readPersonImg.h"
#include "GetWebCamImage.h"

const int maxComponents = 75;//保留最大的主成分数

using namespace std;
using namespace cv;

cv::Mat image;



#define MIN_AREA 1000
IplImage *resutlImg=0;
int RectX=0;
int RectY=0;
int Width=1;
int Height=1; 
bool haveRect=0;//标志位用于标记是否有人物出现在画面中
//const string VIPerDIR = "E:\\软件学院\\SRTP\\VIPeR\\VIPeR";



IplImage* PedestrainScan(IplImage *src)//利用摄像头不动的特点获取行人在图中的位置
{
	CvMemStorage * storage = cvCreateMemStorage(0);  
	CvSeq * contour = 0;
	IplImage *img=src;
	cvFindContours(img, storage, &contour, sizeof(CvContour),CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	for(;contour!=0;contour=contour->h_next)
	{
		CvRect rect=((CvContour *)contour)->rect;
		if (rect.width*rect.height>MIN_AREA)
		{
			cvRectangle(img,cvPoint(rect.x,rect.y),cvPoint(rect.x+rect.width,rect.y+rect.height),CV_RGB(255,255,255),1.5,CV_AA,0);
			RectX=rect.x;
			RectY=rect.y;
			Width=rect.width;
			Height=rect.height;
			haveRect=1;
		}
		else
			haveRect=0;
	}
	cvShowImage("moving area",img);
	return src;
}       

//矩阵按列拼接
Mat mergeCols(Mat A, Mat B)
{
	assert(A.rows == B.rows&&A.type() == B.type());
	int totalCols = A.cols + B.cols;

	Mat mergedDescriptors(A.rows, totalCols, A.type());
	Mat submat = mergedDescriptors.colRange(0, A.cols);
	A.copyTo(submat);
	submat = mergedDescriptors.colRange(A.cols, totalCols);
	B.copyTo(submat);
	return mergedDescriptors;
}

float calcSimilarity(Mat x, Mat y, Mat M) {
	Mat m = (x - y).t()*M*(x - y);
	//cout << m << endl;
	return m.at<float>(0, 0);
}

void getFiles( string path, string exd, vector<string>& files )
{
	//文件句柄
	long   hFile   =   0;
	//文件信息
	struct _finddata_t fileinfo;
	string pathName, exdName;

	if (0 != strcmp(exd.c_str(), ""))
	{
		exdName = "\\*." + exd;
	}
	else
	{
		exdName = "\\*";
	}
	
	if((hFile = _findfirst(pathName.assign(path).append(exdName).c_str(),&fileinfo)) !=  -1)
	{
		do
		{
			//如果是文件夹中仍有文件夹,迭代之
			//如果不是,加入列表
			if((fileinfo.attrib &  _A_SUBDIR))
			{
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)
					getFiles( pathName.assign(path).append("\\").append(fileinfo.name), exd, files );
			}
			else
			{
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)
					files.push_back(pathName.assign(path).append("\\").append(fileinfo.name));
			}
		}while(_findnext(hFile, &fileinfo)  == 0);
		_findclose(hFile);
	}
}


void main(int argc,char * argv())
{
	/*
	//使用网络摄像头作为数据源
	//---------------------------------------
	// 初始化
	NET_DVR_Init();
	//设置连接时间与重连时间
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);


	//---------------------------------------
	// 注册设备
	LONG lUserID;
	LONG lUserID2;
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;
	lUserID = NET_DVR_Login_V30("10.250.94.156", 8000, "admin", "12345", &struDeviceInfo);
	lUserID2 = NET_DVR_Login_V30("10.250.94.157", 8000, "admin", "12345", &struDeviceInfo);
	if (lUserID < 0)
	{
		printf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return;
	}
	if (lUserID2 < 0)
	{
		printf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return;
	}

	//---------------------------------------
	//设置异常消息回调函数
	NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

	//---------------------------------------
	//启动预览并设置回调数据流
	LONG lRealPlayHandle;
	LONG lRealPlayHandle2;

	NET_DVR_PREVIEWINFO struPlayInfo = { 0};
	struPlayInfo.hPlayWnd = NULL;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
	struPlayInfo.lChannel = 1;           //预览通道号
	struPlayInfo.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
	struPlayInfo.dwLinkMode = 0;         //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP

	NET_DVR_PREVIEWINFO struPlayInfo2 = { 0 };
	struPlayInfo2.hPlayWnd = NULL;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
	struPlayInfo2.lChannel = 1;           //预览通道号
	struPlayInfo2.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
	struPlayInfo2.dwLinkMode = 0;         //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
	

	lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, fRealDataCallBack, NULL);
	lRealPlayHandle2 = NET_DVR_RealPlay_V40(lUserID2, &struPlayInfo2, fRealDataCallBack, NULL);

	if (lRealPlayHandle < 0)
	{
		printf("NET_DVR_RealPlay_V40 error\n");
		printf("%d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return;
	}
	//waitKey();

	if (lRealPlayHandle2 < 0)
	{
		printf("NET_DVR_RealPlay_V40 error\n");
		printf("%d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID2);
		NET_DVR_Cleanup();
		return;
	}
	//waitKey();

	IplImage* frame;
	//定义JPEG图像质量  
	LPNET_DVR_JPEGPARA JpegPara = new NET_DVR_JPEGPARA;
	JpegPara->wPicQuality = 0;
	JpegPara->wPicSize = 9;

	char * Jpeg = new char[200 * 1024];
	char * Jpeg2 = new char[200 * 1024];
	DWORD len = 200 * 1024;
	LPDWORD Ret = 0;

	if (!NET_DVR_SetCapturePictureMode(BMP_MODE))
	{
		cout << "Set Capture Picture Mode error!" << endl;
		cout << "The error code is " << NET_DVR_GetLastError() << endl;
	}

	//bool capture = NET_DVR_CaptureJPEGPicture(lUserID,1,JpegPara,"1111");  
	vector<char>data(200 * 1024);
	vector<char>data2(200 * 1024);
	string str = "./image/";

	
	Mat tempframe, currentframe, previousframe,resultframe;
	Mat Result;
	int framenum = 0,count;
	*/
	cout<<"***********************行人再识别系统***********************\n";
	cout<<"请输入待测监控录像保存路径\n";
	string VideoDir,VIPerDIR,TargetDir;

	cin>>VideoDir;
	cout<<"请输入矩阵保存路径\n";
	cin>>VIPerDIR;
	cout<<"请输入目标人物图片文件夹路径\n";
	cin>>TargetDir;
	//行人识别用参数以及读入视频or摄像头
	VideoCapture capture(VideoDir);
	//VideoCapture cap(0); 

	Mat tempframe, currentframe, previousframe,resultframe;
	Mat frame,Result;
	int framenum = 0,count;
	CvMemStorage * storage = cvCreateMemStorage(0);    
    CvSeq * contour = 0;  
	
	bool jump_flag=1;//跳帧符号位,0是跳,1是不跳
	bool detect=0;
	

	//保存结果帧用的参数
	int n = 1, m = 1;

	int detctNum;//用于输出人名

	char *cstr = new char[1];

	//读取目标特征
	//IplImage *targetimg=cvLoadImage("target.jpg");
	//Mat target(targetimg,0);
	//Mat	targetFeature=GetFeatureVector_half(target);

	string targetDIR =TargetDir;
	std::vector<Mat> target = ReadPersonImg(targetDIR.c_str());
	Mat targetFeature = GetTrainSet(target);
	int targetNum=targetFeature.cols;


	//char * filePath = "E:\\软件学院\\SRTP\\历史版本\\PedestrianDetection_ver1.9\\PedestrianDetection\\Target";
	vector<string> files;
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

	//获取该路径下的所有jpg文件
	getFiles(TargetDir, "jpg", files);

	int size = files.size();
	string targetName[100];
	for (int i = 0;i < size;i++)
	{
		cout<<files[i].c_str()<<endl;
		_splitpath( files[i].c_str(), drive, dir, fname, ext );
		 printf("Drive:%s\n file name: %s\n file type: %s\n",drive,fname,ext);
		 targetName[i]=fname;
	}



	//读取凑数用的样本
	Mat cam_a;

	string featureDIR = VIPerDIR + "\\feature.xml";
	FileStorage fs(featureDIR, FileStorage::READ);
	fs["cam_a"] >> cam_a;
	cam_a=cam_a.colRange(0,maxComponents-targetNum-1);

	
	//读取M
	Mat M;
	string MDIR = VIPerDIR + "\\M.xml";
	FileStorage Mfs(MDIR, FileStorage::READ);
	Mfs["M"] >> M;

	Mat resultFeature,result;


	clock_t start,finish;  
	double fps;
    char string[10];  // 用于存放帧率的字符串
    double t = 0;
	
	//读取一帧处理
	while (true)
	{
		t = (double)cv::getTickCount();
		if(!capture.isOpened())
		{
			break;
		}
	
		capture >> frame;
		tempframe = frame;
		framenum++; 
		Mat currentframe1,currentframe2, currentframe3, currentframe4,showframe;
		if (framenum == 1)
		{
			cvtColor(tempframe, previousframe, CV_BGR2GRAY);
			tempframe.copyTo(resultframe);
		}
		if (framenum >= 2&& n <= 1 && jump_flag)
		{
			start=clock();  
			cvtColor(tempframe, currentframe, CV_BGR2GRAY);//转化为单通道灰度图，此时currentFrame已经存了tempFrame的内容 
			absdiff(currentframe,previousframe,currentframe);//做差求绝对值  
			threshold(currentframe, currentframe, 20, 255.0, CV_THRESH_BINARY);
			dilate(currentframe, currentframe,Mat());//膨胀
			erode(currentframe, currentframe,Mat());//腐蚀

			//显示图像
			IplImage* iplimg= &IplImage(currentframe);
			count=countNonZero(currentframe);	
			if(count>=1000)
			{
				//cout<<"img has changed"<<endl;
				resutlImg=PedestrainScan(iplimg);

				resultFeature=GetFeatureVector_half(resultframe);

				cam_a.copyTo(result);//将凑数用的人物的特征存入矩阵
				result=mergeCols(result,targetFeature);//倒数第二列为目标的特征
				result=mergeCols(result,resultFeature);//倒数第一列列为当前监测到的行人特征
				
				//string ResultDIR = DATADIR + "\\result.xml";
				//FileStorage fs_result(ResultDIR, FileStorage::WRITE);
				//fs_result << "result" << result;
				//fs_result.release();

				PCA pca(result, Mat(), CV_PCA_DATA_AS_COL, maxComponents);//构造pca数据结构
				result= pca.project(result);//result保存所有将未完成的图片信息
				
				Mat Fintarget,Finresult;
				Fintarget=result.colRange(maxComponents-1-targetNum,maxComponents-1).clone();//获取降维之后的所有目标人物特征
				Finresult=result.colRange(maxComponents-1,maxComponents).clone();//获取降维之后的当前人物的特征

				int *similarity=new int[targetNum];
				for(int i=0;i<targetNum;i++)//依次检查每张图片并进行比对
				{
					Mat temp_target;
					temp_target=Fintarget.colRange(i,i+1);
					similarity[i] = calcSimilarity(temp_target,Finresult, M);
					tempframe.copyTo(showframe); //此处应用拷贝而非赋值，因为赋值为浅拷贝，两者只修改其中一个会导致两者同时被修改
					resultframe=tempframe(Rect( RectX,RectY,Width,Height));
					rectangle(showframe,cvPoint(RectX,RectY),cvPoint(RectX+Width,RectY+Height),CV_RGB(255,255,255),1.5,CV_AA,0);
				}

				detect=0;//先将detect设置为0，直到发现检测出目标人物才将其置为1
				for(int m=0;m<targetNum;m++)
				{
					if(similarity[m]<=20)
					{
						cout<<"Target Ditected";
						detctNum=m;
						putText( showframe, targetName[m], Point( 30,30),CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255) );
						rectangle(showframe,cvPoint(0,0),cvPoint(tempframe.cols ,tempframe.rows),CV_RGB(255,0,0),5,CV_AA,0);
						//如果不是最后一张图片被检测出来是不会有红框的因为在循环的过程中就被盖住了
						detect=1;
					}
				}

			}
			else
			{
				imshow("moving area", currentframe);//currentframe为二值图像，tempframe为当前摄像头里的图像 
				tempframe.copyTo(showframe); //此处应用拷贝而非赋值，因为赋值为浅拷贝，两者只修改其中一个会导致两者同时被修改
				resultframe=tempframe(Rect( RectX,RectY,Width,Height));
				rectangle(showframe,cvPoint(RectX,RectY),cvPoint(RectX+Width,RectY+Height),CV_RGB(255,255,255),1.5,CV_AA,0);
			}

			finish=clock();  //测试运行时间
			cout << finish-start   << "/" << CLOCKS_PER_SEC  << " (s) "<< endl;  
		}

		if(jump_flag==0)
			{
				start=clock();  
				cvtColor(tempframe, currentframe, CV_BGR2GRAY);//转化为单通道灰度图，此时currentFrame已经存了tempFrame的内容 
				absdiff(currentframe,previousframe,currentframe);//做差求绝对值  
				threshold(currentframe, currentframe, 20, 255.0, CV_THRESH_BINARY);
				dilate(currentframe, currentframe,Mat());//膨胀
				erode(currentframe, currentframe,Mat());//腐蚀

				imshow("画面变化区域", currentframe);//currentframe为二值图像，tempframe为当前摄像头里的图像 
				tempframe.copyTo(showframe); //此处应用拷贝而非赋值，因为赋值为浅拷贝，两者只修改其中一个会导致两者同时被修改
				if(detect==1)
				{
					putText( showframe, targetName[detctNum], Point( 30,30),CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255) );
					//resultframe=tempframe(Rect( RectX,RectY,Width,Height));
					rectangle(showframe,cvPoint(0,0),cvPoint(tempframe.cols ,tempframe.rows),CV_RGB(255,0,0),5,CV_AA,0);
				}
				cout<<"Pass Frame";
				finish=clock();  //测试运行时间
				cout << finish-start   << "/" << CLOCKS_PER_SEC  << " (s) "<< endl;  

				t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
				fps = 1.0 / t;


				sprintf(string, "%.2f", fps);      // 帧率保留两位小数
				std::string fpsString("FPS:");
				fpsString += string;                    // 在"FPS:"后加入帧率数值字符串
				// 将帧率信息写在输出帧上
				putText(showframe, // 图像矩阵
				        fpsString,                  // string型文字内容
				        cv::Point(5, 20),           // 文字坐标，以左下角为原点
				        cv::FONT_HERSHEY_SIMPLEX,   // 字体类型
				        0.5, // 字体大小
				        cv::Scalar(0, 0, 0));       // 字体颜色


				imshow("输出画面", showframe);
				//imshow("Result imagine", resultframe);
			}
		else if(framenum!=1)
			{
				t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
				fps = 1.0 / t;


				sprintf(string, "%.2f", fps);      // 帧率保留两位小数
				std::string fpsString("FPS:");
				fpsString += string;                    // 在"FPS:"后加入帧率数值字符串
				// 将帧率信息写在输出帧上
				putText(showframe, // 图像矩阵
				        fpsString,                  // string型文字内容
				        cv::Point(5, 20),           // 文字坐标，以左下角为原点
				        cv::FONT_HERSHEY_SIMPLEX,   // 字体类型
				        0.5, // 字体大小
				        cv::Scalar(0, 0, 0));       // 字体颜色
				imshow("输出画面", showframe);
				imshow("移动物体", resultframe);
			}
			
			jump_flag=!jump_flag;
	
		//把当前帧保存作为下一次处理的前一帧  
		cvtColor(tempframe, previousframe, CV_BGR2GRAY);
		waitKey(33);
	}//end while  
	
}


