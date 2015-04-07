#include "opencv_helper.h"
#include <fstream>
using namespace std;
double OpencvHelper::get_avg(vector<double> list)
{
	double sum = 0;
	for (int i = 0; i != list.size(); ++ i)
	{
		sum += list[i];	
	}
	return sum / list.size();
}
double OpencvHelper::get_delta(vector<double> list, double avg)
{
	double temp;
	double sum = 0;
	for (int i = 0; i != list.size(); ++ i)
	{
		temp = list[i] - avg;
		temp *= temp;
		sum += temp;
	}
	return sum / list.size();
}
void OpencvHelper::CheckAll(IplImage *image, double T)
{

	Th.clear();
	Tl.clear();
	double temp = 0;
	uchar *data = (uchar *)image->imageData;	
	int step = image->widthStep / sizeof(uchar);
	for (int i = 0; i != image->height; ++ i)
	{
		for (int j = 0; j != image->width; ++ j)
		{	
			temp = data[i * step + j];
			if (temp >= T) Th.push_back(temp);
			else Tl.push_back(temp);
		}
	}
}
double OpencvHelper::get_T(IplImage *image)
{
	double T = 200;
	while (T >= 150)	
	{
		CheckAll(image, T);
		double uh = get_avg(Th);
		double ul = get_avg(Tl);
		T = (uh + ul) / 2;
	}
	return T;
}
void OpencvHelper::pic_edge(IplImage *image, IplImage *result)
{
	IplImage *grey, *edge;
	grey = cvCreateImage(cvGetSize(image), image->depth, 1);
	cvCvtColor(image, grey, CV_BGR2GRAY);
	edge = cvCreateImage(cvGetSize(image), image->depth, 1);
	double T = get_T(grey);
	CheckAll(grey, T);
	double ul = get_avg(Tl);
	double dl = get_delta(Tl, ul);
	dl = sqrt(dl);
	cvCanny(grey, edge, ul - 0.3 * dl, ul + dl, 3);

	cvCopy(edge, result, NULL);

	//cvNamedWindow("edge", CV_WINDOW_AUTOSIZE);
	//cvShowImage("edge",edge);
	//cvWaitKey(0);
	cvReleaseImage(&grey);
	cvReleaseImage(&edge);
	//cvDestroyWindow("edge");
	//
}