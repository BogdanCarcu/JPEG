// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <iostream>
#include <fstream>

void testOpenImage()
{
	char fname[MAX_PATH];
	while(openFileDlg(fname))
	{
		Mat src;
		src = imread(fname);
		imshow("image",src);
		waitKey();
	}
}

void testOpenImagesFld()
{
	char folderName[MAX_PATH];
	if (openFolderDlg(folderName)==0)
		return;
	char fname[MAX_PATH];
	FileGetter fg(folderName,"bmp");
	while(fg.getNextAbsFile(fname))
	{
		Mat src;
		src = imread(fname);
		imshow(fg.getFoundFileName(),src);
		if (waitKey()==27) //ESC pressed
			break;
	}
}


void testResize()
{
	char fname[MAX_PATH];
	while(openFileDlg(fname))
	{
		Mat src;
		src = imread(fname);
		Mat dst1,dst2;
		//without interpolation
		resizeImg(src,dst1,320,false);
		//with interpolation
		resizeImg(src,dst2,320,true);
		imshow("input image",src);
		imshow("resized image (without interpolation)",dst1);
		imshow("resized image (with interpolation)",dst2);
		waitKey();
	}
}


void testVideoSequence()
{
	VideoCapture cap("Videos/rubic.avi"); // off-line video from file
	//VideoCapture cap(0);	// live video from web cam
	if (!cap.isOpened()) {
		printf("Cannot open video capture device.\n");
		waitKey(0);
		return;
	}
		
	Mat edges;
	Mat frame;
	char c;

	while (cap.read(frame))
	{
		Mat grayFrame;
		cvtColor(frame, grayFrame, CV_BGR2GRAY);
		imshow("source", frame);
		imshow("gray", grayFrame);
		c = cvWaitKey(0);  // waits a key press to advance to the next frame
		if (c == 27) {
			// press ESC to exit
			printf("ESC pressed - capture finished\n"); 
			break;  //ESC pressed
		};
	}
}


void testSnap()
{
	VideoCapture cap(0); // open the deafult camera (i.e. the built in web cam)
	if (!cap.isOpened()) // openenig the video device failed
	{
		printf("Cannot open video capture device.\n");
		return;
	}

	Mat frame;
	char numberStr[256];
	char fileName[256];
	
	// video resolution
	Size capS = Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CV_CAP_PROP_FRAME_HEIGHT));

	// Display window
	const char* WIN_SRC = "Src"; //window for the source frame
	namedWindow(WIN_SRC, CV_WINDOW_AUTOSIZE);
	cvMoveWindow(WIN_SRC, 0, 0);

	const char* WIN_DST = "Snapped"; //window for showing the snapped frame
	namedWindow(WIN_DST, CV_WINDOW_AUTOSIZE);
	cvMoveWindow(WIN_DST, capS.width + 10, 0);

	char c;
	int frameNum = -1;
	int frameCount = 0;

	for (;;)
	{
		cap >> frame; // get a new frame from camera
		if (frame.empty())
		{
			printf("End of the video file\n");
			break;
		}

		++frameNum;
		
		imshow(WIN_SRC, frame);

		c = cvWaitKey(10);  // waits a key press to advance to the next frame
		if (c == 27) {
			// press ESC to exit
			printf("ESC pressed - capture finished");
			break;  //ESC pressed
		}
		if (c == 115){ //'s' pressed - snapp the image to a file
			frameCount++;
			fileName[0] = NULL;
			sprintf(numberStr, "%d", frameCount);
			strcat(fileName, "Images/A");
			strcat(fileName, numberStr);
			strcat(fileName, ".bmp");
			bool bSuccess = imwrite(fileName, frame);
			if (!bSuccess) 
			{
				printf("Error writing the snapped image\n");
			}
			else
				imshow(WIN_DST, frame);
		}
	}

}

void MyCallBackFunc(int event, int x, int y, int flags, void* param)
{
	//More examples: http://opencvexamples.blogspot.com/2014/01/detect-mouse-clicks-and-moves-on-image.html
	Mat* src = (Mat*)param;
	if (event == CV_EVENT_LBUTTONDOWN)
		{
			printf("Pos(x,y): %d,%d  Color(RGB): %d,%d,%d\n",
				x, y,
				(int)(*src).at<Vec3b>(y, x)[2],
				(int)(*src).at<Vec3b>(y, x)[1],
				(int)(*src).at<Vec3b>(y, x)[0]);
		}
}

void testMouseClick()
{
	Mat src;
	// Read image from file 
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		src = imread(fname);
		//Create a window
		namedWindow("My Window", 1);

		//set the callback function for any mouse event
		setMouseCallback("My Window", MyCallBackFunc, &src);

		//show the image
		imshow("My Window", src);

		// Wait until user press some key
		waitKey(0);
	}
}

/* Histogram display function - display a histogram using bars (simlilar to L3 / PI)
Input:
name - destination (output) window name
hist - pointer to the vector containing the histogram values
hist_cols - no. of bins (elements) in the histogram = histogram image width
hist_height - height of the histogram image
Call example:
showHistogram ("MyHist", hist_dir, 255, 200);
*/
void showHistogram(const string& name, int* hist, const int  hist_cols, const int hist_height)
{
	Mat imgHist(hist_height, hist_cols, CV_8UC3, CV_RGB(255, 255, 255)); // constructs a white image

	//computes histogram maximum
	int max_hist = 0;
	for (int i = 0; i<hist_cols; i++)
	if (hist[i] > max_hist)
		max_hist = hist[i];
	double scale = 1.0;
	scale = (double)hist_height / max_hist;
	int baseline = hist_height - 1;

	for (int x = 0; x < hist_cols; x++) {
		Point p1 = Point(x, baseline);
		Point p2 = Point(x, baseline - cvRound(hist[x] * scale));
		line(imgHist, p1, p2, CV_RGB(255, 0, 255)); // histogram bins colored in magenta
	}

	imshow(name, imgHist);
}

// JPEG COMPRESSION

static int Luminance[8][8] = { { 16, 11, 10, 16, 24, 40, 51, 61 },
								{ 12, 12, 14, 19, 26, 58, 60, 55 },
								{ 14, 13, 16, 24, 40, 57, 69, 56 },
								{ 14, 17, 22, 29, 51, 87, 80, 62 },
								{ 18, 22, 37, 56, 68, 109, 103, 77 },
								{ 24, 35, 55, 64, 81, 104, 113, 92 },
								{ 49, 64, 78, 87, 103, 121, 120, 101 },
								{ 72, 92, 95, 98, 112, 100, 103, 99 } };

static int test[8][8] = { { 52, 55, 61, 66, 70, 61, 64, 73 }, { 64, 59, 55, 90, 109, 85, 69, 72 },
{ 62, 59, 68, 113, 144, 104, 66, 73 }, { 63, 58, 71, 122, 154, 106, 70, 69 }, { 67, 61, 68, 104, 126, 88, 68, 70 },
{ 79, 65, 60, 70, 77, 68, 58, 75 }, { 85, 71, 64, 59, 55, 61, 65, 83 }, { 87, 79, 69, 68, 65, 76, 78, 94 } };

Mat DCT(Mat bl) {

	float ci, cj;
	Mat res(8, 8, CV_32FC1);
	Mat block;
	bl.convertTo(block, CV_32FC1);

	for (int i = 0; i < block.rows; ++i)
		for (int j = 0; j < block.cols; ++j) {

			if (i == 0)
				ci = (float)sqrt(1 / 8.0);
			else
				ci = (float)sqrt(2 / 8.0);

			if (j == 0)
				cj = (float)sqrt(1 / 8.0);
			else
				cj = (float)sqrt(2 / 8.0);
			
			float S = 0;

			for (int x = 0; x < 8; ++x)
				for (int y = 0; y < 8; ++y) {

					S += (float) block.at<float>(x, y) * cos((PI / 8.0) * (x + 0.5) * i) * cos((PI / 8.0) * (y + 0.5) * j);
				}

			res.at<float>(i, j) = (float)ci * cj * S;
		}

	return res;
}

Mat IDCT(Mat bl) {

	float ci, cj;
	Mat res(8, 8, CV_32FC1);
	Mat block;
	bl.convertTo(block, CV_32FC1);

	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j) {

			float S = 0;

			for (int x = 0; x < 8; ++x)
				for (int y = 0; y < 8; ++y) {

					if (x == 0)
						ci = (float)sqrt(1 / 8.0);
					else
						ci = (float)sqrt(2 / 8.0);

					if (y == 0)
						cj = (float)sqrt(1 / 8.0);
					else
						cj = (float)sqrt(2 / 8.0);

					S += ci * cj * (float)block.at<float>(x, y) * cos((PI / 8.0) * (i + 0.5) * x) * cos((PI / 8.0) * (j + 0.5) * y);
				}

			res.at<float>(i, j) = S + 128;
		}

	return res;


}

// Utility function to traverse matrix
// in zig-zag form
int* zigZagMatrix(Mat img, int n, int m)
{

	static int result[64];
	int t = 0;

	for (int i = 0; i < n + m - 1; i++) {
		if (i % 2 == 1) {
			// down left
			int x = i < n ? 0 : i - n + 1;
			int y = i < n ? i : n - 1;
			while (x < m && y >= 0) {
				result[t++] = img.at<int>(x++, y--);
			}
		}
		else {
			// up right
			int x = i < m ? i : m - 1;
			int y = i < m ? 0 : i - m + 1;
			while (x >= 0 && y < n) {
				result[t++] = img.at<int>(x--, y++);
			}
		}
	}
	return result;
}

Mat zigZagVector(std::vector<int> arr, int n, int m)
{
	Mat_<int> result(n, m);
	int t = 0;

	for (int i = 0; i < n + m - 1; i++) {
		if (i % 2 == 1) {
			// down left
			int x = i < n ? 0 : i - n + 1;
			int y = i < n ? i : n - 1;
			while (x < m && y >= 0) {
				result.at<int>(x++, y--) = arr.at(t++);
			}
		}
		else {
			// up right
			int x = i < m ? i : m - 1;
			int y = i < m ? 0 : i - m + 1;
			while (x >= 0 && y < n) {
				result.at<int>(x--, y++) = arr.at(t++);
			}
		}
	}
	return result;
}

/* Returns the Run Length Encoded string for the
source string src */
std::vector<int> rle(int *src)
{
	int c = 1;
	std::vector<int> result;

	for (int i = 0; i < 63; ++i) {

		if (src[i] == src[i + 1])
			c++;
		else {

			result.push_back(src[i]);
			result.push_back(c);
			c = 1;
		}

	}

	result.push_back(src[63]);
	result.push_back(c);

	return result;
}

//crop an image from top left
Mat crop(Mat img, int h, int w) {

	Mat_<Vec3b> res(h, w);

	for (int i = 0; i < h; ++i)
		for (int j = 0; j < w; ++j) {
			res.at<Vec3b>(i, j)[0] = img.at<Vec3b>(i, j)[0];
			res.at<Vec3b>(i, j)[1] = img.at<Vec3b>(i, j)[1];
			res.at<Vec3b>(i, j)[2] = img.at<Vec3b>(i, j)[2];
		}

	return res;
}

void jpeg_compression(Mat img, int &garbage_h, int &garbage_w) {

	Mat YCbCr_mat(img.rows, img.cols, CV_8UC3);
		
	//convert to YCbCr color space
	cvtColor(img, YCbCr_mat, COLOR_BGR2YCrCb);

	//open file for writing
	std::fstream myfile;
	myfile.open("compressed.txt");

	// pick every channel
	Mat ycbcr[3];
	split(YCbCr_mat, ycbcr);

	for (int current_channel = 0; current_channel < 3; ++current_channel) {
	
		if (myfile.is_open())
		{
			myfile << "Channel\n";
		}
		else std::cout << "Unable to open file";

		Mat channel = ycbcr[current_channel];

		// test if x an y coordinates are a multiple of 8
		int xPadding = 0, yPadding = 0;

		if (channel.rows % 8 != 0) {
			xPadding = 1;

			while ((img.rows + xPadding) % 8 != 0)
				xPadding++;

		}

		if (channel.cols % 8 != 0) {
			yPadding = 1;

			while ((img.cols + yPadding) % 8 != 0)
				yPadding++;

		}

		//update padded height and width for future use
		garbage_h = xPadding;
		garbage_w = yPadding;

		// compute a new channel matrix with padded values (last pixel is repeated)
		Mat new_channel(img.rows + xPadding, img.cols + yPadding, CV_8UC1);
		uchar last_pixel = 0;
	
		for (int i = 0; i < new_channel.rows; ++i)
			for (int j = 0; j < new_channel.cols; ++j) {

				if (i < channel.rows && j < channel.cols)
					new_channel.at<uchar>(i, j) = channel.at<uchar>(i, j);
				else
					new_channel.at<uchar>(i, j) = last_pixel;

				last_pixel = new_channel.at<uchar>(i, j);
			}
	
		// declare a vector of MCUs
		std::vector<Mat> MCUs;

		// split into 8 x 8 tiles and iterate
		for (int i = 0; i < new_channel.rows - 7; i += 8)
			for (int j = 0; j < new_channel.cols - 7; j += 8) {

				Mat MCU(8, 8, CV_8UC1);

				for (int m = 0; m < 8; ++m)
					for (int n = 0; n < 8; ++n)
						MCU.at<uchar>(m, n) = new_channel.at<uchar>(i + m, j + n);

				MCUs.push_back(MCU);
			}

		// apply transformations on the resulted blocks
		int number_of_blocks = MCUs.size();

		if (myfile.is_open())
		{
			myfile << number_of_blocks << "\n";
		}
		else std::cout << "Unable to open file";

		for (int b = 0; b < number_of_blocks; ++b) {

			Mat new_block(8, 8, CV_32SC1);

			int aux[8][8];

			for (int i = 0; i < 8; ++i)
				for (int j = 0; j < 8; ++j) {

					aux[i][j] = (int)MCUs.at(b).at<uchar>(i, j);
					new_block.at<int>(i, j) = aux[i][j] - 128;
				}

			Mat dct_block;
			dct_block = DCT(new_block);

			Mat quantized_block(8, 8, CV_32SC1);
			for (int i = 0; i < 8; ++i)
				for (int j = 0; j < 8; ++j) {

					quantized_block.at<int>(i, j) = round(nearbyint(dct_block.at<float>(i, j)) / Luminance[i][j]);

				}

			int* zig_zag = zigZagMatrix(quantized_block, 8, 8);
			std::vector<int> arr = rle(zig_zag);

			if (myfile.is_open())
			{
				myfile << arr.size() << " ";
				for (int i = 0; i < arr.size(); ++i)
					myfile << arr.at(i) << " ";
				myfile << "\n";

			}
			else std::cout << "Unable to open file";

		}

	}

	myfile.close();

}

void jpeg_decompression(int h, int w, int garbage_h, int garbage_w) {

	//open file for reading
	std::ifstream myfile("compressed.txt");
	vector<Mat> channels;

	//read file -> try to rebuild channels
	for (int channel = 0; channel < 3; ++channel) {
		
		char line[8];
		myfile >> line;

		int number_of_blocks;
		myfile >> number_of_blocks;

		std::vector<Mat> blocks;

		for (int i = 0; i < number_of_blocks; ++i) {

				int n;
				myfile >> n;
				std::vector<int> block_arr;

				for (int j = 0; j < n / 2; j ++) {

					int el, occurrence;
					myfile >> el;
					myfile >> occurrence;

					for (int k = 0; k < occurrence; ++k)
						block_arr.push_back(el);
		
				}

				Mat block = zigZagVector(block_arr, 8, 8);	
				Mat_<int> dequantized_block(8, 8);

				for (int r = 0; r < 8; ++r)
					for (int c = 0; c < 8; ++c)
						dequantized_block.at<int>(r, c) = block.at<int>(r, c) * Luminance[r][c];
				
				Mat inversed_block = IDCT(dequantized_block);

				Mat final_block;
				inversed_block.convertTo(final_block, CV_8UC1);

				blocks.push_back(final_block);
			}

		Mat_<uchar> current_channel(h + garbage_h, w + garbage_w);
		int current_block = 0;

		for (int i = 0; i < h + garbage_h - 7;  i += 8)
			for (int j = 0; j < w + garbage_w - 7; j += 8) {

				for (int bi = 0; bi < 8; ++bi)
					for (int bj = 0; bj < 8; ++bj) {
						
						current_channel.at<uchar>(i + bi, j + bj) = blocks.at(current_block).at<uchar>(bi, bj);

					}
				
				current_block++;
			}

		
		channels.push_back(current_channel);
	}
	
	Mat final_image;
	Mat bgr_image;
	Mat cropped_image;
	merge(channels, final_image);

	//convert to BGR color space
	cvtColor(final_image, bgr_image, COLOR_YCrCb2BGR);
	
	//remove the garbage pixels
	cropped_image = crop(bgr_image, h, w);

	myfile.close();
	imshow("Decompressed image", cropped_image);
	waitKey(0);
	imwrite("Lena.jpeg", cropped_image);

}

int main()
{
	int op;
	do
	{
		system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf(" 1 - Open image\n");
		printf(" 2 - Open BMP images from folder\n");
		printf(" 3 - Resize image\n");
		printf(" 4 - Process video\n");
		printf(" 5 - Snap frame from live video\n");
		printf(" 6 - Mouse callback demo\n");
		printf(" 7 - JPEG compression\n");
		printf(" 8 - JPEG decompression\n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d",&op);
		switch (op)
		{
			case 1:
				testOpenImage();
				break;
			case 2:
				testOpenImagesFld();
				break;
			case 3:
				testResize();
				break;
			case 4:
				testVideoSequence();
				break;
			case 5:
				testSnap();
				break;
			case 6:
				testMouseClick();
				break;
			case 7: {
				int garbage_h, garbage_w;
				Mat img = imread("Images/Lena_24bits.bmp", CV_LOAD_IMAGE_COLOR);
				jpeg_compression(img, garbage_h, garbage_w); 
				
			} break;
			case 8: {
				int garbage_h, garbage_w;
				Mat img = imread("Images/Lena_24bits.bmp", CV_LOAD_IMAGE_COLOR);
				jpeg_compression(img, garbage_h, garbage_w);
				jpeg_decompression(img.rows, img.cols, garbage_h, garbage_w);

			} break;

		}
	}
	while (op!=0);
	return 0;
}