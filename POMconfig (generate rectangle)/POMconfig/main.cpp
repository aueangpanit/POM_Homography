#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>
#include <limits>
#include <numeric>
using namespace cv;
using namespace std;

// We need 4 corresponding 2D points(x,y) to calculate homography.
vector<Point2f> left_image;      // Stores 4 points(x,y) of the logo image. Here the four points are 4 corners of image.
vector<Point2f> right_image;    // stores 4 points that the user clicks(mouse left click) in the main image.

								// Image containers for main and logo image
Mat imageMain;
Mat imageLogo;

// Function to add main image and transformed logo image and show final output.
// Icon image replaces the pixels of main image in this implementation.
void showFinal(Mat src1, Mat src2, Point p11, Point p12, Point p21, Point p22)
{
	Mat gray, gray_inv, src1final, src2final;
	cvtColor(src2, gray, CV_BGR2GRAY);
	threshold(gray, gray, 0, 255, CV_THRESH_BINARY);
	//adaptiveThreshold(gray,gray,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,5,4);
	bitwise_not(gray, gray_inv);
	src1.copyTo(src1final, gray_inv);
	src2.copyTo(src2final, gray);
	Mat finalImage = src1final + src2final;
	rectangle(finalImage, p11, p22, (0, 255, 0), -1);

	namedWindow("output", WINDOW_AUTOSIZE);
	imshow("output", finalImage);
}

Point transformPoint(Mat H, double point[]) {
	double Z =	H.at<double>(2, 0) * point[0] + 
				H.at<double>(2, 1) * point[1] + 
				H.at<double>(2, 2) * point[2];
	int x = (int)((	H.at<double>(0, 0) * point[0] +
					H.at<double>(0, 1) * point[1] +
					H.at<double>(0, 2) * point[2]) / Z);
	int y = (int)((	H.at<double>(1, 0) * point[0] +
					H.at<double>(1, 1) * point[1] +
					H.at<double>(1, 2) * point[2]) / Z);
	Point tP(x, y);
	return tP;
}

void on_mouse(int e, int x, int y, int d, void *ptr)
{
	if (e == EVENT_LBUTTONDOWN)
	{
		// location (2 points which defines the width of the rectangle)
		double location1[3] = { x + 10, y - 20, 1 };
		double location2[3] = { x - 10, y + 20, 1 };
 		cout << "top location: " << x << ", " << y << endl;

		// Homography matrix
		double m1[3][3] = { { 0.07326423072601404, -1.23425226989803, 668.9999999999999 },
						   { 0.1007617627064437, 0.0542285795076401, 314 },
						   { -0.0007532626665634242, -0.0003505281198382014, 1 } };
		Mat H1 = Mat(3, 3, CV_64F, m1); // ground plane

		double m2[3][3] = { { 0.03621893511882955, -1.220173493264751, 669 },
							{ 0.04341408503409805, 0.06151861566769743, 255.0000000000001 },
							{ -0.0007863490696360756, -0.0003948695864289178, 1 } };
		Mat H2 = Mat(3, 3, CV_64F, m2); // head plane

		// calculate points transformations
		Point headP1(transformPoint(H2, location1));
		Point groundP1(transformPoint(H1, location1));

		Point headP2(transformPoint(H2, location2));
		Point groundP2(transformPoint(H1, location2));

		Mat logoWarped;
		// Warp the logo image to change its perspective
		warpPerspective(imageLogo, logoWarped, H1, imageMain.size());
		showFinal(imageMain, logoWarped, headP1, groundP1, headP2, groundP2);
	}
}


int main(int argc, char** argv)
{
	//  We need tow argumemts. "Main image" and "logo image"
	if (argc != 3)
	{
		std::cout << " Usage: error" << endl;
		return -1;
	}


	// Load images from arguments passed.
	imageMain = imread(argv[1], CV_LOAD_IMAGE_COLOR);
	imageLogo = imread(argv[2], CV_LOAD_IMAGE_COLOR);
	// Push the 4 corners of the logo image as the 4 points for correspondence to calculate homography.
	left_image.push_back(Point2f(float(0), float(0)));
	left_image.push_back(Point2f(float(0), float(imageLogo.rows)));
	left_image.push_back(Point2f(float(imageLogo.cols), float(imageLogo.rows)));
	left_image.push_back(Point2f(float(imageLogo.cols), float(0)));



	namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Display window", imageLogo);

	setMouseCallback("Display window", on_mouse, NULL);


	//  Press "Escape button" to exit
	while (1)
	{
		int key = cvWaitKey(10);
		if (key == 27) break;
	}


	return 0;
}