#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "math.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <sstream>
#include <time.h>
#include <fstream>
#include <opencv2/core/core.hpp>    
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/opencv.hpp"

using namespace std;


int main()
{
	int c = 0;
	ofstream fout;
	fout.open("data.txt");				// test for depth value
	const string NAME = "out.avi";

	CvSeq* a = 0;
    CvCapture* capture = cvCaptureFromCAM(0);
	if(!cvQueryFrame(capture))
	{ 
		cout<<"Video capture failed, please check the camera."<<endl;
	}
	else
	{
		cout<<"Video camera capture status: OK"<<endl;
		}

	// Initialize
    CvSize sz = cvGetSize(cvQueryFrame( capture));
	CvVideoWriter *videoWriter =   cvCreateVideoWriter("C:\\Users\\LumiG\\Desktop\\out.mpg",CV_FOURCC('P','I','M','1'),20,sz,1);
	 
	IplImage* src = cvCreateImage( sz, 8, 3 );
	IplImage* hsv_image = cvCreateImage( sz, 8, 3);
	IplImage* hsv_mask = cvCreateImage( sz, 8, 1);
	IplImage* hsv_edge = cvCreateImage( sz, 8, 1);
	
	CvScalar  hsv_min = cvScalar(0, 30, 80, 0);
	CvScalar  hsv_max = cvScalar(20, 150, 255, 0);
	// Store
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvMemStorage* areastorage = cvCreateMemStorage(0);
	CvMemStorage* minStorage = cvCreateMemStorage(0);
	CvMemStorage* dftStorage = cvCreateMemStorage(0);
	CvSeq* contours = NULL;
	//
	cvNamedWindow( "src",1);
	cvNamedWindow( "hsv-msk",1);
	//////

	// Capture
	while( c != 27)
	{
		src = cvQueryFrame( capture);

		cvCvtColor(src, hsv_image, CV_BGR2HSV);
		cvInRangeS (hsv_image, hsv_min, hsv_max, hsv_mask);
		cvSmooth( hsv_mask, hsv_mask, CV_MEDIAN, 27, 0, 0, 0 );
		cvCanny(hsv_mask, hsv_edge, 1, 3, 5);
		cvFindContours( hsv_mask, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
			CvSeq* contours2 = NULL;
			double result = 0, result2 = 0;
		
		
		// deconstruct contours
		while(contours)
		{
			// find the biggest contour
			result = fabs( cvContourArea( contours, CV_WHOLE_SEQ ) );
			if ( result > result2) {result2 = result; contours2 = contours;};
			contours  =  contours->h_next;
		}



		if ( contours2 )
		{
			CvRect rect = cvBoundingRect( contours2, 0 );
			int checkcxt = cvCheckContourConvexity( contours2 );
			cvRectangle( src, cvPoint(rect.x, rect.y + rect.height), cvPoint(rect.x + rect.width, rect.y), CV_RGB(200, 0, 200), 1, 8, 0 ); //darw a rectangle
			CvSeq* hull = cvConvexHull2( contours2, 0, CV_CLOCKWISE, 0 );
			CvSeq* defect = cvConvexityDefects( contours2, hull, dftStorage );
			

			//detect gestures
			if( defect->total >=26 ) {cout << " Palm " << endl;}
			else{ cout << " Fist " << endl;}
			cout << "defet: " << defect->total << endl;
			CvBox2D box = cvMinAreaRect2( contours2, minStorage );


			//draw the fingers
			for(;defect;defect = defect->h_next) 
			{ 
				int nomdef = defect->total;
				if(nomdef == 0)  
					continue; 
				CvConvexityDefect* defectArray = (CvConvexityDefect*)malloc(sizeof(CvConvexityDefect)*nomdef);     
				cvCvtSeqToArray (defect, defectArray, CV_WHOLE_SEQ);

				for(int i=0; i<nomdef;i++)
				{ 
					fout<<defectArray[i].depth<<endl;
					if(defectArray[i].depth < 50)	{continue;}
					cvCircle( src, *(defectArray[i].end), 5, CV_RGB(255,0,0), -1, 8,0);  
					cvCircle( src, *(defectArray[i].start), 5, CV_RGB(0,0,255), -1, 8,0); 
					cvCircle( src, *(defectArray[i].depth_point), 5, CV_RGB(0,255,255), -1, 8,0);        
				}			
				free(defectArray);
			}


			
			
		}
			

		cvDrawContours( src, contours2,  CV_RGB( 0, 200, 0), CV_RGB( 100, 100, 0), 1,2);
//		cvWriteFrame(videoWriter,src);
		cvShowImage( "src", src);
		cvShowImage( "hsv-msk", hsv_mask); hsv_mask->origin = 1;
		

		c = cvWaitKey( 10);
	}
	//////
	cvReleaseCapture( &capture);
	cvReleaseVideoWriter(&videoWriter);
	cvDestroyAllWindows();

}