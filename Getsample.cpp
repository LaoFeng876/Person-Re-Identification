#include<iostream>  
#include<opencv2\core\core.hpp>  
#include<opencv2\highgui\highgui.hpp>  
#include<opencv\cv.h>
#include<ctime>  
#include "getFeatureVector.h"
#include "readPersonImg.h"
#include "GetWebCamImage.h"

const int maxComponents = 75;//�����������ɷ���

using namespace std;
using namespace cv;

cv::Mat image;



#define MIN_AREA 1000
IplImage *resutlImg=0;
int RectX=0;
int RectY=0;
int Width=1;
int Height=1; 
bool haveRect=0;//��־λ���ڱ���Ƿ�����������ڻ�����
//const string VIPerDIR = "E:\\���ѧԺ\\SRTP\\VIPeR\\VIPeR";



IplImage* PedestrainScan(IplImage *src)//��������ͷ�������ص��ȡ������ͼ�е�λ��
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

//������ƴ��
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
	//�ļ����
	long   hFile   =   0;
	//�ļ���Ϣ
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
			//������ļ����������ļ���,����֮
			//�������,�����б�
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
	//ʹ����������ͷ��Ϊ����Դ
	//---------------------------------------
	// ��ʼ��
	NET_DVR_Init();
	//��������ʱ��������ʱ��
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);


	//---------------------------------------
	// ע���豸
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
	//�����쳣��Ϣ�ص�����
	NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

	//---------------------------------------
	//����Ԥ�������ûص�������
	LONG lRealPlayHandle;
	LONG lRealPlayHandle2;

	NET_DVR_PREVIEWINFO struPlayInfo = { 0};
	struPlayInfo.hPlayWnd = NULL;         //��ҪSDK����ʱ�����Ϊ��Чֵ����ȡ��������ʱ����Ϊ��
	struPlayInfo.lChannel = 1;           //Ԥ��ͨ����
	struPlayInfo.dwStreamType = 0;       //0-��������1-��������2-����3��3-����4���Դ�����
	struPlayInfo.dwLinkMode = 0;         //0- TCP��ʽ��1- UDP��ʽ��2- �ಥ��ʽ��3- RTP��ʽ��4-RTP/RTSP��5-RSTP/HTTP

	NET_DVR_PREVIEWINFO struPlayInfo2 = { 0 };
	struPlayInfo2.hPlayWnd = NULL;         //��ҪSDK����ʱ�����Ϊ��Чֵ����ȡ��������ʱ����Ϊ��
	struPlayInfo2.lChannel = 1;           //Ԥ��ͨ����
	struPlayInfo2.dwStreamType = 0;       //0-��������1-��������2-����3��3-����4���Դ�����
	struPlayInfo2.dwLinkMode = 0;         //0- TCP��ʽ��1- UDP��ʽ��2- �ಥ��ʽ��3- RTP��ʽ��4-RTP/RTSP��5-RSTP/HTTP
	

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
	//����JPEGͼ������  
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
	cout<<"***********************������ʶ��ϵͳ***********************\n";
	cout<<"�����������¼�񱣴�·��\n";
	string VideoDir,VIPerDIR,TargetDir;

	cin>>VideoDir;
	cout<<"��������󱣴�·��\n";
	cin>>VIPerDIR;
	cout<<"������Ŀ������ͼƬ�ļ���·��\n";
	cin>>TargetDir;
	//����ʶ���ò����Լ�������Ƶor����ͷ
	VideoCapture capture(VideoDir);
	//VideoCapture cap(0); 

	Mat tempframe, currentframe, previousframe,resultframe;
	Mat frame,Result;
	int framenum = 0,count;
	CvMemStorage * storage = cvCreateMemStorage(0);    
    CvSeq * contour = 0;  
	
	bool jump_flag=1;//��֡����λ,0����,1�ǲ���
	bool detect=0;
	

	//������֡�õĲ���
	int n = 1, m = 1;

	int detctNum;//�����������

	char *cstr = new char[1];

	//��ȡĿ������
	//IplImage *targetimg=cvLoadImage("target.jpg");
	//Mat target(targetimg,0);
	//Mat	targetFeature=GetFeatureVector_half(target);

	string targetDIR =TargetDir;
	std::vector<Mat> target = ReadPersonImg(targetDIR.c_str());
	Mat targetFeature = GetTrainSet(target);
	int targetNum=targetFeature.cols;


	//char * filePath = "E:\\���ѧԺ\\SRTP\\��ʷ�汾\\PedestrianDetection_ver1.9\\PedestrianDetection\\Target";
	vector<string> files;
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

	//��ȡ��·���µ�����jpg�ļ�
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



	//��ȡ�����õ�����
	Mat cam_a;

	string featureDIR = VIPerDIR + "\\feature.xml";
	FileStorage fs(featureDIR, FileStorage::READ);
	fs["cam_a"] >> cam_a;
	cam_a=cam_a.colRange(0,maxComponents-targetNum-1);

	
	//��ȡM
	Mat M;
	string MDIR = VIPerDIR + "\\M.xml";
	FileStorage Mfs(MDIR, FileStorage::READ);
	Mfs["M"] >> M;

	Mat resultFeature,result;


	clock_t start,finish;  
	double fps;
    char string[10];  // ���ڴ��֡�ʵ��ַ���
    double t = 0;
	
	//��ȡһ֡����
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
			cvtColor(tempframe, currentframe, CV_BGR2GRAY);//ת��Ϊ��ͨ���Ҷ�ͼ����ʱcurrentFrame�Ѿ�����tempFrame������ 
			absdiff(currentframe,previousframe,currentframe);//���������ֵ  
			threshold(currentframe, currentframe, 20, 255.0, CV_THRESH_BINARY);
			dilate(currentframe, currentframe,Mat());//����
			erode(currentframe, currentframe,Mat());//��ʴ

			//��ʾͼ��
			IplImage* iplimg= &IplImage(currentframe);
			count=countNonZero(currentframe);	
			if(count>=1000)
			{
				//cout<<"img has changed"<<endl;
				resutlImg=PedestrainScan(iplimg);

				resultFeature=GetFeatureVector_half(resultframe);

				cam_a.copyTo(result);//�������õ�����������������
				result=mergeCols(result,targetFeature);//�����ڶ���ΪĿ�������
				result=mergeCols(result,resultFeature);//������һ����Ϊ��ǰ��⵽����������
				
				//string ResultDIR = DATADIR + "\\result.xml";
				//FileStorage fs_result(ResultDIR, FileStorage::WRITE);
				//fs_result << "result" << result;
				//fs_result.release();

				PCA pca(result, Mat(), CV_PCA_DATA_AS_COL, maxComponents);//����pca���ݽṹ
				result= pca.project(result);//result�������н�δ��ɵ�ͼƬ��Ϣ
				
				Mat Fintarget,Finresult;
				Fintarget=result.colRange(maxComponents-1-targetNum,maxComponents-1).clone();//��ȡ��ά֮�������Ŀ����������
				Finresult=result.colRange(maxComponents-1,maxComponents).clone();//��ȡ��ά֮��ĵ�ǰ���������

				int *similarity=new int[targetNum];
				for(int i=0;i<targetNum;i++)//���μ��ÿ��ͼƬ�����бȶ�
				{
					Mat temp_target;
					temp_target=Fintarget.colRange(i,i+1);
					similarity[i] = calcSimilarity(temp_target,Finresult, M);
					tempframe.copyTo(showframe); //�˴�Ӧ�ÿ������Ǹ�ֵ����Ϊ��ֵΪǳ����������ֻ�޸�����һ���ᵼ������ͬʱ���޸�
					resultframe=tempframe(Rect( RectX,RectY,Width,Height));
					rectangle(showframe,cvPoint(RectX,RectY),cvPoint(RectX+Width,RectY+Height),CV_RGB(255,255,255),1.5,CV_AA,0);
				}

				detect=0;//�Ƚ�detect����Ϊ0��ֱ�����ּ���Ŀ������Ž�����Ϊ1
				for(int m=0;m<targetNum;m++)
				{
					if(similarity[m]<=20)
					{
						cout<<"Target Ditected";
						detctNum=m;
						putText( showframe, targetName[m], Point( 30,30),CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255) );
						rectangle(showframe,cvPoint(0,0),cvPoint(tempframe.cols ,tempframe.rows),CV_RGB(255,0,0),5,CV_AA,0);
						//����������һ��ͼƬ���������ǲ����к�����Ϊ��ѭ���Ĺ����оͱ���ס��
						detect=1;
					}
				}

			}
			else
			{
				imshow("moving area", currentframe);//currentframeΪ��ֵͼ��tempframeΪ��ǰ����ͷ���ͼ�� 
				tempframe.copyTo(showframe); //�˴�Ӧ�ÿ������Ǹ�ֵ����Ϊ��ֵΪǳ����������ֻ�޸�����һ���ᵼ������ͬʱ���޸�
				resultframe=tempframe(Rect( RectX,RectY,Width,Height));
				rectangle(showframe,cvPoint(RectX,RectY),cvPoint(RectX+Width,RectY+Height),CV_RGB(255,255,255),1.5,CV_AA,0);
			}

			finish=clock();  //��������ʱ��
			cout << finish-start   << "/" << CLOCKS_PER_SEC  << " (s) "<< endl;  
		}

		if(jump_flag==0)
			{
				start=clock();  
				cvtColor(tempframe, currentframe, CV_BGR2GRAY);//ת��Ϊ��ͨ���Ҷ�ͼ����ʱcurrentFrame�Ѿ�����tempFrame������ 
				absdiff(currentframe,previousframe,currentframe);//���������ֵ  
				threshold(currentframe, currentframe, 20, 255.0, CV_THRESH_BINARY);
				dilate(currentframe, currentframe,Mat());//����
				erode(currentframe, currentframe,Mat());//��ʴ

				imshow("����仯����", currentframe);//currentframeΪ��ֵͼ��tempframeΪ��ǰ����ͷ���ͼ�� 
				tempframe.copyTo(showframe); //�˴�Ӧ�ÿ������Ǹ�ֵ����Ϊ��ֵΪǳ����������ֻ�޸�����һ���ᵼ������ͬʱ���޸�
				if(detect==1)
				{
					putText( showframe, targetName[detctNum], Point( 30,30),CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255) );
					//resultframe=tempframe(Rect( RectX,RectY,Width,Height));
					rectangle(showframe,cvPoint(0,0),cvPoint(tempframe.cols ,tempframe.rows),CV_RGB(255,0,0),5,CV_AA,0);
				}
				cout<<"Pass Frame";
				finish=clock();  //��������ʱ��
				cout << finish-start   << "/" << CLOCKS_PER_SEC  << " (s) "<< endl;  

				t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
				fps = 1.0 / t;


				sprintf(string, "%.2f", fps);      // ֡�ʱ�����λС��
				std::string fpsString("FPS:");
				fpsString += string;                    // ��"FPS:"�����֡����ֵ�ַ���
				// ��֡����Ϣд�����֡��
				putText(showframe, // ͼ�����
				        fpsString,                  // string����������
				        cv::Point(5, 20),           // �������꣬�����½�Ϊԭ��
				        cv::FONT_HERSHEY_SIMPLEX,   // ��������
				        0.5, // �����С
				        cv::Scalar(0, 0, 0));       // ������ɫ


				imshow("�������", showframe);
				//imshow("Result imagine", resultframe);
			}
		else if(framenum!=1)
			{
				t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
				fps = 1.0 / t;


				sprintf(string, "%.2f", fps);      // ֡�ʱ�����λС��
				std::string fpsString("FPS:");
				fpsString += string;                    // ��"FPS:"�����֡����ֵ�ַ���
				// ��֡����Ϣд�����֡��
				putText(showframe, // ͼ�����
				        fpsString,                  // string����������
				        cv::Point(5, 20),           // �������꣬�����½�Ϊԭ��
				        cv::FONT_HERSHEY_SIMPLEX,   // ��������
				        0.5, // �����С
				        cv::Scalar(0, 0, 0));       // ������ɫ
				imshow("�������", showframe);
				imshow("�ƶ�����", resultframe);
			}
			
			jump_flag=!jump_flag;
	
		//�ѵ�ǰ֡������Ϊ��һ�δ����ǰһ֡  
		cvtColor(tempframe, previousframe, CV_BGR2GRAY);
		waitKey(33);
	}//end while  
	
}


