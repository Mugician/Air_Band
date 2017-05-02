#include <array>
#include <iostream>
#include <map>
#include <vector>

#include <stdlib.h> 
#include <time.h> 

#include <windows.h>
// OpenCV 头文件
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// NiTE 头文件
#include <OpenNI.h>
#include <NiTE.h>

#include "CFugueLib.h"

using namespace std;
using namespace openni;
using namespace nite;
using namespace cv;

static const int R = 2;
static const int G = 1;
static const int B = 0;

static const int Cb = 2;
static const int Cr = 1;
static const int Y = 0;

cv::RNG rng2(12345);
cv::Mat thinImage(const cv::Mat & src, cv::Mat& dst, const int maxIterations = -1);
void fix(cv::Mat& src, cv::Mat& dst);
void detectleft(int start_x, int start_y, int end_x, int end_y, cv::Mat& src, cv::Mat& dst);
void detectright(int start_x, int start_y, int end_x, int end_y, cv::Mat& src, cv::Mat& dst);
void handpath(cv::Mat& src, vector<vector<cv::Point2f>>& path, int start_x, int start_y, int end_x, int end_y);
int link(cv::Mat& src, cv::Mat& help, int point_x, int point_y, cv::Point2f* finger_list);
void music(Point2f* leftfinger_list, int leftfinger_count, int left_skip, Point2f* rightfinger_list, int rightfinger_count, int right_skip);

void music()
{
	clock_t start = clock();
	int nPortID = MIDI_MAPPER, nTimerRes = 5;

	unsigned int nOutPortCount = CFugue::GetMidiOutPortCount();
	if (nOutPortCount <= 0)
		exit(fprintf(stderr, "No MIDI Output Ports found !!"));
	if (nOutPortCount > 1)
	{
		_tprintf(_T("\n---------  The MIDI Out Port indices  --------\n\n"));
		/////////////////////////////////////////
		/// List the MIDI Ports
		for (unsigned int i = 0; i < nOutPortCount; ++i)
		{
			std::string portName = CFugue::GetMidiOutPortName(i);
			std::wcout << "\t" << i << "\t: " << portName.c_str() << "\n";
		}
		//////////////////////////////////////////
		/// Chose a Midi output port
		_tprintf(_T("\nChoose your MIDI Port ID for the play:"));
		_tscanf(_T("%d"), &nPortID);

	}
	CFugue::Player player;
	player.Play("F#8");
	cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
	start = clock();
	player.Play("F#8/0.25 F#8/0.25 F#8/0.25 F#8/0.25");
	cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
}
DWORD WINAPI Fun1Proc(LPVOID lpParameter)
{
	music();
	return 0;
}

int main(int argc, char **argv)
{
	// 初始化OpenNI
	OpenNI::initialize();

	// 打开Kinect设备
	Device  mDevice;
	mDevice.open(ANY_DEVICE);

	// 创建深度数据流
	VideoStream mDepthStream;
	mDepthStream.create(mDevice, SENSOR_DEPTH);

	// 设置VideoMode模式
	VideoMode mDepthMode;
	mDepthMode.setResolution(640, 480);
	mDepthMode.setFps(30);
	mDepthMode.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);
	mDepthStream.setVideoMode(mDepthMode);

	// 同样的设置彩色数据流
	VideoStream mColorStream;
	mColorStream.create(mDevice, SENSOR_COLOR);
	// 设置VideoMode模式
	VideoMode mColorMode;
	mColorMode.setResolution(640, 480);
	mColorMode.setFps(30);
	mColorMode.setPixelFormat(PIXEL_FORMAT_RGB888);
	mColorStream.setVideoMode(mColorMode);

	// 设置深度图像映射到彩色图像
	mDevice.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);

	// 初始化 NiTE
	if (NiTE::initialize() != nite::STATUS_OK)
	{
		cerr << "NiTE initial error" << endl;
		return -1;
	}
	// 创建用户跟踪器
	UserTracker mUserTracker;
	mUserTracker.create(&mDevice);

	// Control the smoothing factor of the skeleton joints. Factor should be between 0 (no smoothing at all) and 1 (no movement at all)
	mUserTracker.setSkeletonSmoothingFactor(0.1f);

	cv::namedWindow("Color Image", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("Binary Image", CV_WINDOW_AUTOSIZE);

	// 环境初始化后，开始获取深度数据流和彩色数据流
	mDepthStream.start();
	mColorStream.start();

	//指尖点列表的指针
	Point2f fingerlist1[10] = { Point2f() };
	Point2f fingerlist2[10] = { Point2f() };
	Point2f fingerlist3[10] = { Point2f() };
	Point2f fingerlist4[10] = { Point2f() };
	Point2f fingerlist5[10] = { Point2f() };
	Point2f* finger_list[5] = { fingerlist1, fingerlist2, fingerlist3, fingerlist4, fingerlist5 };

	int finger_count[5] = { 0 };
	int finger_list_tail = 0;

	while (true)
	{
		HANDLE hThread_1;
		if (finger_list[5][0].x - finger_list[0][0].x>10)
			hThread_1 = CreateThread(NULL, 0, Fun1Proc, NULL, 0, NULL);
		//手指
		Point2f leftfinger[5];
		Point2f rightfinger[5];
		//手掌心
		Point2f left_point2;
		Point2f right_point2;


		// 获得最大深度值
		int iMaxDepth = mDepthStream.getMaxPixelValue();

		// 读取User用户数据帧信息流
		UserTrackerFrameRef  mUserFrame;
		mUserTracker.readFrame(&mUserFrame);

		// 得到Users信息
		const nite::Array<UserData>& aUsers = mUserFrame.getUsers();
		for (int i = 0; i < aUsers.getSize(); ++i)
		{
			const UserData& rUser = aUsers[i];

			// 检查用户状态
			if (rUser.isNew())
			{
				// 开始对该用户的骨骼跟踪
				mUserTracker.startSkeletonTracking(rUser.getId());
			}

			if (rUser.isVisible())
			{
				// 得到用户骨骼数据
				const Skeleton& rSkeleton = rUser.getSkeleton();

				// 检查骨骼状态是否为“跟踪状态”
				if (rSkeleton.getState() == SKELETON_TRACKED)
				{
					SkeletonJoint left_hand = rSkeleton.getJoint(JOINT_LEFT_HAND);
					SkeletonJoint right_hand = rSkeleton.getJoint(JOINT_RIGHT_HAND);
					nite::Point3f left_point = left_hand.getPosition();
					nite::Point3f right_point = right_hand.getPosition();

					mUserTracker.convertJointCoordinatesToDepth(
						left_point.x, left_point.y, left_point.z,
						&(left_point2.x), &(left_point2.y));
					mUserTracker.convertJointCoordinatesToDepth(
						right_point.x, right_point.y, right_point.z,
						&(right_point2.x), &(right_point2.y));


					// 创建OpenCV::Mat，用于显示彩色数据图像
					cv::Mat cImageBGR;

					// 读取彩色数据帧信息流
					VideoFrameRef mColorFrame;
					mColorStream.readFrame(&mColorFrame);

					// 将彩色数据流转换为OpenCV格式，记得格式是：CV_8UC3（含R\G\B）
					const cv::Mat mImageRGB(mColorFrame.getHeight(), mColorFrame.getWidth(),
						CV_8UC3, (void*)mColorFrame.getData());

					// RGB ==> BGR
					cv::cvtColor(mImageRGB, cImageBGR, CV_RGB2BGR);

					openni::VideoFrameRef mDepthFrame = mUserFrame.getDepthFrame();
					// 将深度数据转换成OpenCV格式
					const cv::Mat mImageDepth(mDepthFrame.getHeight(), mDepthFrame.getWidth(), CV_16UC1, (void*)mDepthFrame.getData());
					// 为了让深度图像显示的更加明显一些，将CV_16UC1 ==> CV_8U格式
					cv::Mat mScaledDepth, mImageBGR, mBinaryImage, mBinaryImage_final, mBinaryImage_final2, mBinaryImage_final3, ycrcb, mBinaryImage_final4;
					mImageDepth.convertTo(mScaledDepth, CV_8U, 255.0 / 10000);
					mBinaryImage = cv::Mat::zeros(480, 640, CV_8UC3);
					mBinaryImage_final.create(480, 640, CV_8UC3);
					mBinaryImage_final2.create(480, 640, CV_8UC1);
					mBinaryImage_final3.create(480, 640, CV_8UC1);

					cvtColor(cImageBGR, ycrcb, CV_BGR2YCrCb);

					for (int j = 0; j < mImageDepth.rows; j++)
					{
						uchar* sptr = mScaledDepth.ptr<uchar>(j);
						uchar* bptr = mBinaryImage.ptr<uchar>(j);
						uchar* prgb = cImageBGR.ptr<uchar>(j);
						uchar* pycrcb = ycrcb.ptr<uchar>(j);

						for (int k = 0; k < mImageDepth.cols; k++)
						{
							if (j >= left_point2.y - 50 && j <= left_point2.y + 50
								&& k >= left_point2.x - 50 && k <= left_point2.x + 50)
							{
								if (bptr[3 * k] != 255)
									if (sptr[k] >= (uchar)mScaledDepth.ptr<uchar>(left_point2.y)[(int)left_point2.x] - 3
										&& sptr[k] <= (uchar)mScaledDepth.ptr<uchar>(left_point2.y)[(int)left_point2.x] + 3
										&& (pycrcb[3 * k + Cr] >= 133 && pycrcb[3 * k + Cr] <= 173 && pycrcb[3 * k + Cb] >= 77 && pycrcb[3 * k + Cb] <= 127))
									{
										bptr[3 * k] = 255;
										bptr[3 * k + 1] = 255;
										bptr[3 * k + 2] = 255;
									}
							}
							else if (j >= right_point2.y - 50 && j <= right_point2.y + 50
								&& k >= right_point2.x - 50 && k <= right_point2.x + 50)
							{
								if (bptr[3 * k] != 255)
									if (sptr[k] >= (uchar)mScaledDepth.ptr<uchar>(right_point2.y)[(int)right_point2.x] - 3
										&& sptr[k] <= (uchar)mScaledDepth.ptr<uchar>(right_point2.y)[(int)right_point2.x] + 3
										&& (pycrcb[3 * k + Cr] >= 133 && pycrcb[3 * k + Cr] <= 173 && pycrcb[3 * k + Cb] >= 77 && pycrcb[3 * k + Cb] <= 127))
									{
										bptr[3 * k] = 255;
										bptr[3 * k + 1] = 255;
										bptr[3 * k + 2] = 255;
									}
							}
						}
					}

					// 根据指定的核类型，核大小，锚点生成核
					cv::Mat element = cv::getStructuringElement(2, cv::Size(3, 3), cv::Point(1, 1));
					cv::Mat element2 = cv::getStructuringElement(2, cv::Size(5, 5), cv::Point(2, 2));

					// 形态学处理
					erode(mBinaryImage, mBinaryImage_final, element2);
					cv::dilate(mBinaryImage_final, mBinaryImage, element);
					erode(mBinaryImage, mBinaryImage_final, element);
					cv::dilate(mBinaryImage_final, mBinaryImage, element);

					//检测指尖3(细化)
					cvtColor(mBinaryImage, mBinaryImage_final2, CV_RGB2GRAY);
					cv::threshold(mBinaryImage_final2, mBinaryImage_final2, 128, 1, cv::THRESH_BINARY);
					thinImage(mBinaryImage_final2, mBinaryImage_final3);

					Mat help = Mat::zeros(mBinaryImage_final3.rows, mBinaryImage_final3.cols, CV_8UC1);//定位图初始黑色
					Mat dst = Mat::zeros(mBinaryImage_final3.rows, mBinaryImage_final3.cols, CV_8UC1);

					for (int i = left_point2.y - 50; i < left_point2.y; i++)
					{
						uchar* src_ptr = mBinaryImage_final3.ptr<uchar>(i);
						uchar* help_ptr = help.ptr<uchar>(i);
						uchar* dst_ptr = dst.ptr<uchar>(i);
						for (int j = left_point2.x - 50; j < left_point2.x + 50; j++)
						{
							if (help_ptr[j]<128)//对于未遍历的
							{
								if (src_ptr[j] > 200)//遇到细化图
								{
									//help_ptr[j] = 255;//定位图置白色
									finger_count[finger_list_tail] = link(mBinaryImage_final3, help, j, i, finger_list[finger_list_tail]);
									if (finger_count[finger_list_tail]>0)
									{
										finger_list_tail++;
									}
								}
								else//非细化图
								{
									help_ptr[j] = 128;//定位图置灰色
								}
							}
						}
					}

					for (int i = right_point2.y - 50; i < right_point2.y + 50; i++)
					{
						uchar* src_ptr = mBinaryImage_final3.ptr<uchar>(i);
						uchar* help_ptr = help.ptr<uchar>(i);
						uchar* dst_ptr = dst.ptr<uchar>(i);
						for (int j = right_point2.x - 50; j < right_point2.x + 50; j++)
						{
							if (help_ptr[j]<128)//对于未遍历的
							{
								if (src_ptr[j] > 200)//遇到细化图
								{
									//help_ptr[j] = 255;//定位图置白色
									finger_count[finger_list_tail] = link(mBinaryImage_final3, help, j, i, finger_list[finger_list_tail]);
									if (finger_count[finger_list_tail]>0)
									{
										finger_list_tail++;
									}
								}
								else//非细化图
								{
									help_ptr[j] = 128;//定位图置灰色
								}
							}
						}
					}
					////////////////指尖位置//////////////////

					if (finger_list_tail == 1)
					{

					}
					else if (finger_list_tail == 2)
					{
						int left_min = 0;
						int right_max = 0;
						//y从小到大排序
						if (finger_count[0] > 1)
						{
							for (int i = finger_count[0] - 1; i >= 1; i--)
							{
								for (int j = finger_count[0] - 1; j >= finger_count[0] - i; j--)
								{
									if (finger_list[0][j].y < finger_list[0][j - 1].y)
									{
										finger_list[5][j - 1] = finger_list[0][j - 1];
										finger_list[5][j] = finger_list[0][j];

										Point2f temp = finger_list[0][j - 1];
										finger_list[0][j - 1] = finger_list[0][j];
										finger_list[0][j] = temp;

									}
								}
							}

							for (int i = 1; i < finger_count[0]; i++)
							{
								if (finger_list[0][i].x < finger_list[0][i - 1].x)
								{
									left_min = i;
								}
							}
						}
						if (finger_count[1] > 1)
						{
							for (int i = finger_count[1] - 1; i >= 1; i--)
							{
								for (int j = finger_count[1] - 1; j >= finger_count[1] - i; j--)
								{
									if (finger_list[1][j].y < finger_list[1][j - 1].y)
									{
										finger_list[6][j - 1] = finger_list[1][j - 1];
										finger_list[6][j] = finger_list[1][j];

										Point2f temp = finger_list[1][j - 1];
										finger_list[1][j - 1] = finger_list[1][j];
										finger_list[1][j] = temp;
									}
								}
							}

							for (int i = 1; i < finger_count[1]; i++)
							{
								if (finger_list[1][i].x > finger_list[1][i - 1].x)
								{
									right_max = i;
								}
							}
						}
						//music(finger_list[0], finger_count[0],left_min, finger_list[1], finger_count[1],right_max);

						for (int i = 0; i < finger_list_tail; i++)
						{
							for (int j = 0; j < finger_count[i]; j++)
							{
								if (i == 0 && j != left_min)
								{
									cout << finger_list[i][j].x << "," << finger_list[i][j].y << endl;
									circle(mBinaryImage_final3, finger_list[i][j], 5, Scalar(255, 0, 0));
								}
								if (i == 1 && j != right_max)
								{
									cout << finger_list[i][j].x << "," << finger_list[i][j].y << endl;
									circle(mBinaryImage_final3, finger_list[i][j], 5, Scalar(255, 0, 0));
								}
							}
						}
					}

					imshow("Binary Image", mBinaryImage_final3);
					//////////////////////////////////////////

					// 显示image
					//cv::imshow("Depth Image", mImageBGR);
					cv::imshow("Color Image", cImageBGR);
					//cv::imshow("Binary Image", mBinaryImage);
					//cv::imshow("Binary Image2", mBinaryImage_final2);
					mUserFrame.release();

					// 按键“q”退出循环
					if (cv::waitKey(1) == 'q')
						break;
				}
			}
		}
		CloseHandle(hThread_1);
	}


	mUserTracker.destroy();
	mColorStream.destroy();
	NiTE::shutdown();
	OpenNI::shutdown();

	return 0;
}

cv::Mat thinImage(const cv::Mat & src, cv::Mat& dst, const int maxIterations)
{
	assert(src.type() == CV_8UC1);
	int width = src.cols;
	int height = src.rows;
	src.copyTo(dst);
	int count = 0;  //记录迭代次数  
	while (true)
	{
		count++;
		if (maxIterations != -1 && count > maxIterations) //限制次数并且迭代次数到达  
			break;
		std::vector<uchar *> mFlag; //用于标记需要删除的点  
		//对点标记  
		for (int i = 0; i < height; ++i)
		{
			uchar * p = dst.ptr<uchar>(i);
			for (int j = 0; j < width; ++j)
			{
				//如果满足四个条件，进行标记  
				//  p9 p2 p3  
				//  p8 p1 p4  
				//  p7 p6 p5  
				uchar p1 = p[j];
				if (p1 != 1) continue;
				uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
				uchar p8 = (j == 0) ? 0 : *(p + j - 1);
				uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
				uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
				uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
				uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
				uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
				uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
				{
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;

					if (ap == 1 && p2 * p4 * p6 == 0 && p4 * p6 * p8 == 0)
					{
						//标记  
						mFlag.push_back(p + j);
					}
				}
			}
		}

		//将标记的点删除  
		for (std::vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
		{
			**i = 0;
		}

		//直到没有点满足，算法结束  
		if (mFlag.empty())
		{
			break;
		}
		else
		{
			mFlag.clear();//将mFlag清空  
		}

		//对点标记  
		for (int i = 0; i < height; ++i)
		{
			uchar * p = dst.ptr<uchar>(i);
			for (int j = 0; j < width; ++j)
			{
				//如果满足四个条件，进行标记  
				//  p9 p2 p3  
				//  p8 p1 p4  
				//  p7 p6 p5  
				uchar p1 = p[j];
				if (p1 != 1) continue;
				uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
				uchar p8 = (j == 0) ? 0 : *(p + j - 1);
				uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
				uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
				uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
				uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
				uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
				uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);

				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
				{
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;

					if (ap == 1 && p2 * p4 * p8 == 0 && p2 * p6 * p8 == 0)
					{
						//标记  
						mFlag.push_back(p + j);
					}
				}
			}
		}

		//将标记的点删除  
		for (std::vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
		{
			**i = 0;
		}

		//直到没有点满足，算法结束  
		if (mFlag.empty())
		{
			break;
		}
		else
		{
			mFlag.clear();//将mFlag清空  
		}
	}
	dst = dst * 255;
	return dst;
}

void fix(cv::Mat& src, cv::Mat& dst)
{
	for (int i = 1; i < src.rows - 1; i++)
	{
		for (int j = 1; j < src.cols - 1; j++)
		{
			int flag_w = 0;
			if (src.ptr<uchar>(i - 1)[3 * (j - 1)] >= 220)
			{
				flag_w++;
			}
			if (src.ptr<uchar>(i - 1)[3 * (j)] >= 220)
			{
				flag_w++;
			}
			if (src.ptr<uchar>(i - 1)[3 * (j + 1)] >= 220)
			{
				flag_w++;
			}
			if (src.ptr<uchar>(i + 1)[3 * (j - 1)] >= 220)
			{
				flag_w++;
			}
			if (src.ptr<uchar>(i + 1)[3 * (j)] >= 220)
			{
				flag_w++;
			}
			if (src.ptr<uchar>(i + 1)[3 * (j + 1)] >= 220)
			{
				flag_w++;
			}
			if (src.ptr<uchar>(i)[3 * (j - 1)] >= 220)
			{
				flag_w++;
			}
			if (src.ptr<uchar>(i)[3 * (j + 1)] >= 220)
			{
				flag_w++;
			}
			if (flag_w >= 4)
			{
				dst.ptr<uchar>(i)[3 * j] = 255;
				dst.ptr<uchar>(i)[3 * j + 1] = 225;
				dst.ptr<uchar>(i)[3 * j + 2] = 225;
			}


			int flag_b = 0;
			if (src.ptr<uchar>(i - 1)[3 * (j - 1)] <= 50)
			{
				flag_b++;
			}
			if (src.ptr<uchar>(i - 1)[3 * (j)] <= 50)
			{
				flag_b++;
			}
			if (src.ptr<uchar>(i - 1)[3 * (j + 1)] <= 50)
			{
				flag_b++;
			}
			if (src.ptr<uchar>(i + 1)[3 * (j - 1)] <= 50)
			{
				flag_b++;
			}
			if (src.ptr<uchar>(i + 1)[3 * (j)] <= 50)
			{
				flag_b++;
			}
			if (src.ptr<uchar>(i + 1)[3 * (j + 1)] <= 50)
			{
				flag_b++;
			}
			if (src.ptr<uchar>(i)[3 * (j - 1)] <= 50)
			{
				flag_b++;
			}
			if (src.ptr<uchar>(i)[3 * (j + 1)] <= 50)
			{
				flag_b++;
			}
			if (flag_b >= 4)
			{
				dst.ptr<uchar>(i)[3 * j] == 0;
				dst.ptr<uchar>(i)[3 * j + 1] == 0;
				dst.ptr<uchar>(i)[3 * j + 2] == 0;
			}
		}
	}
}

void detectleft(int start_x, int start_y, int end_x, int end_y, cv::Mat& src, cv::Mat& dst)
{
	vector<vector<cv::Point2f>> fingerPointList;
	vector<vector<cv::Point2f>> fingerValleyList;
	cv::Point2f first_finger;
	cv::Point2f second_finger;
	cv::Point2f third_finger;
	cv::Point2f forth_finger;
	cv::Point2f fifth_finger;

	//cout << src.cols << "," << src.rows << endl;
	for (int s = start_x; s >= end_x; s--)
	{
		vector<cv::Point2f> colPointUpList;//每列的第一个区域手部点/1
		vector<cv::Point2f> colPointDownList;//每列的第一个区域非手部点/3
		cv::Point2f previousPoint(start_x, start_y);
		for (int t = start_y; t < end_y; t++)
		{
			//从上至下扫描,从右至左移动
			if (src.ptr<uchar>(t)[3 * s] >= 200)//手
			{
				//cout << "hand !" << endl;
				//waitKey(3000);
				if (colPointUpList.size()>0 && previousPoint.y == t - 1)//有前一个且与之相连/2
				{
					previousPoint.x = s;
					previousPoint.y = t;
					continue;
				}
				else//第一个遇到的区域手点/1
				{
					previousPoint.x = s;
					previousPoint.y = t;
					//cout << s << "," << t << endl;
					colPointUpList.push_back(cv::Point2f(s, t));
				}
			}
			else//非手
			{
				//cout << "background !" << endl;
				if (colPointUpList.size() > 0 && previousPoint.y == t - 1)//有区域手部点且与之相连/3
				{
					colPointDownList.push_back(cv::Point2f(s, t));
				}
				else//4
				{
					continue;
				}
			}
		}
		//cout << colPointUpList << endl;
		//cout << colPointDownList << endl << endl;
		//一列扫描完
		if (colPointUpList.size() == 1)//最外的手指或手掌部分
		{
			//cout << "one" << endl;
			if (second_finger.x != 0)
			{
				break;
			}
			first_finger.x = colPointUpList[0].x;
			first_finger.y = colPointUpList[0].y;
			//cout << "first " << first_finger.x << "," << first_finger.y << endl;
		}
		else if (colPointUpList.size() == 2)//出现第二根
		{
			//cout << "two" << endl;
			if (third_finger.x != 0)
			{
				break;
			}
			if (colPointDownList[0].y < first_finger.y)
			{
				second_finger.x = first_finger.x;
				second_finger.y = first_finger.y;
				first_finger.x = colPointUpList[0].x;
				first_finger.y = colPointUpList[0].y;
			}
			else
			{
				second_finger.x = colPointUpList[1].x;
				second_finger.y = colPointUpList[1].y;
			}
			//cout << "first" << first_finger.x << "," << first_finger.y << endl;
			//cout << "second" << second_finger.x << "," << second_finger.y << endl;
		}
		else if (colPointUpList.size() == 3)//出现第三根
		{
			//cout << "three" << endl;
			if (forth_finger.x != 0)
			{
				break;
			}
			if (colPointDownList[0].y<first_finger.y)
			{
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = first_finger.x;
				second_finger.y = first_finger.y;
				first_finger.x = colPointUpList[0].x;
				first_finger.y = colPointUpList[0].y;
			}
			else if (colPointDownList[1].y < second_finger.y)
			{
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = colPointUpList[1].x;
				second_finger.y = colPointUpList[1].y;
			}
			else
			{
				third_finger.x = colPointUpList[2].x;
				third_finger.y = colPointUpList[2].y;
			}
			//cout << "first" << first_finger.x << "," << first_finger.y << endl;
			//cout << "second" << second_finger.x << "," << second_finger.y << endl;
			//cout << "third" << third_finger.x << "," << third_finger.y << endl;
		}
		else if (colPointUpList.size() == 4)//出现第四根
		{
			//cout << "four" << endl;
			if (colPointDownList[0].y<first_finger.y)
			{
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = first_finger.x;
				second_finger.y = first_finger.y;
				first_finger.x = colPointUpList[0].x;
				first_finger.y = colPointUpList[0].y;
			}
			else if (colPointDownList[1].y < second_finger.y)
			{
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = colPointUpList[1].x;
				second_finger.y = colPointUpList[1].y;
			}
			else if (colPointDownList[2].y < third_finger.y)
			{
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = colPointUpList[2].x;
				third_finger.y = colPointUpList[2].y;
			}
			else
			{
				forth_finger.x = colPointUpList[3].x;
				forth_finger.y = colPointUpList[3].y;
			}
			//cout << "first" << first_finger.x << "," << first_finger.y << endl;
			//cout << "second" << second_finger.x << "," << second_finger.y << endl;
			//cout << "third" << third_finger.x << "," << third_finger.y << endl;
			//cout << "forth" << forth_finger.x << "," << forth_finger.y << endl;
		}
		else if (colPointUpList.size() == 5)//出现第五根
		{
			//cout << "five" << endl;
			if (colPointDownList[0].y<first_finger.y)
			{
				fifth_finger.x = forth_finger.x;
				fifth_finger.y = forth_finger.y;
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = first_finger.x;
				second_finger.y = first_finger.y;
				first_finger.x = colPointUpList[0].x;
				first_finger.y = colPointUpList[0].y;
			}
			else if (colPointDownList[1].y < second_finger.y)
			{
				fifth_finger.x = forth_finger.x;
				fifth_finger.y = forth_finger.y;
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = colPointUpList[1].x;
				second_finger.y = colPointUpList[1].y;
			}
			else if (colPointDownList[2].y < third_finger.y)
			{
				fifth_finger.x = forth_finger.x;
				fifth_finger.y = forth_finger.y;
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = colPointUpList[2].x;
				third_finger.y = colPointUpList[2].y;
			}
			else if (colPointDownList[3].y < third_finger.y)
			{
				fifth_finger.x = forth_finger.x;
				fifth_finger.y = forth_finger.y;
				forth_finger.x = colPointUpList[3].x;
				forth_finger.y = colPointUpList[3].y;
			}
			else
			{
				fifth_finger.x = colPointUpList[4].x;
				fifth_finger.y = colPointUpList[4].y;
			}
			//cout << "first" << first_finger.x << "," << first_finger.y << endl;
			//cout << "second" << second_finger.x << "," << second_finger.y << endl;
			//cout << "third" << third_finger.x << "," << third_finger.y << endl;
			//cout << "forth" << forth_finger.x << "," << forth_finger.y << endl;
			//cout << "fifth" << fifth_finger.x << "," << fifth_finger.y << endl;
			break;
		}
		else
		{
			cout << colPointUpList.size() << endl;
		}
	}

	/**********************************************************/

	cv::circle(dst, first_finger, 5, cv::Scalar(255, 0, 0));
	cv::circle(dst, second_finger, 5, cv::Scalar(0, 255, 0));
	cv::circle(dst, third_finger, 5, cv::Scalar(0, 0, 255));
	cv::circle(dst, forth_finger, 5, cv::Scalar(255, 0, 255));
	cv::circle(dst, fifth_finger, 5, cv::Scalar(0, 255, 255));
}

void detectright(int start_x, int start_y, int end_x, int end_y, cv::Mat& src, cv::Mat& dst)
{
	vector<vector<cv::Point2f>> fingerPointList;
	vector<vector<cv::Point2f>> fingerValleyList;
	cv::Point2f first_finger;
	cv::Point2f second_finger;
	cv::Point2f third_finger;
	cv::Point2f forth_finger;
	cv::Point2f fifth_finger;

	//cout << src.cols << "," << src.rows << endl;
	for (int s = start_x; s <= end_x; s++)
	{
		vector<cv::Point2f> colPointUpList;//每列的第一个区域手部点/1
		vector<cv::Point2f> colPointDownList;//每列的第一个区域非手部点/3
		cv::Point2f previousPoint(start_x, start_y);
		for (int t = start_y; t < end_y; t++)
		{
			//从上至下扫描,从右至左移动
			if (src.ptr<uchar>(t)[3 * s] >= 200)//手
			{
				//cout << "hand !" << endl;
				//waitKey(3000);
				if (colPointUpList.size() > 0 && previousPoint.y == t - 1)//有前一个且与之相连/2
				{
					previousPoint.x = s;
					previousPoint.y = t;
					continue;
				}
				else//第一个遇到的区域手点/1
				{
					previousPoint.x = s;
					previousPoint.y = t;
					//cout << s << "," << t << endl;
					colPointUpList.push_back(cv::Point2f(s, t));
				}
			}
			else//非手
			{
				//cout << "background !" << endl;
				if (colPointUpList.size() > 0 && previousPoint.y == t - 1)//有区域手部点且与之相连/3
				{
					colPointDownList.push_back(cv::Point2f(s, t));
				}
				else//4
				{
					continue;
				}
			}
		}
		//cout << colPointUpList << endl;
		//cout << colPointDownList << endl << endl;
		//一列扫描完
		if (colPointUpList.size() == 1)//最外的手指或手掌部分
		{
			//cout << "one" << endl;
			if (second_finger.x != 0)
			{
				break;
			}
			first_finger.x = colPointUpList[0].x;
			first_finger.y = colPointUpList[0].y;
			//cout << "first " << first_finger.x << "," << first_finger.y << endl;
		}
		else if (colPointUpList.size() == 2)//出现第二根
		{
			//cout << "two" << endl;
			if (third_finger.x != 0)
			{
				break;
			}
			if (colPointDownList[0].y < first_finger.y)
			{
				second_finger.x = first_finger.x;
				second_finger.y = first_finger.y;
				first_finger.x = colPointUpList[0].x;
				first_finger.y = colPointUpList[0].y;
			}
			else
			{
				second_finger.x = colPointUpList[1].x;
				second_finger.y = colPointUpList[1].y;
			}
			//cout << "first" << first_finger.x << "," << first_finger.y << endl;
			//cout << "second" << second_finger.x << "," << second_finger.y << endl;
		}
		else if (colPointUpList.size() == 3)//出现第三根
		{
			//cout << "three" << endl;
			if (forth_finger.x != 0)
			{
				break;
			}
			if (colPointDownList[0].y < first_finger.y)
			{
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = first_finger.x;
				second_finger.y = first_finger.y;
				first_finger.x = colPointUpList[0].x;
				first_finger.y = colPointUpList[0].y;
			}
			else if (colPointDownList[1].y < second_finger.y)
			{
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = colPointUpList[1].x;
				second_finger.y = colPointUpList[1].y;
			}
			else
			{
				third_finger.x = colPointUpList[2].x;
				third_finger.y = colPointUpList[2].y;
			}
			//cout << "first" << first_finger.x << "," << first_finger.y << endl;
			//cout << "second" << second_finger.x << "," << second_finger.y << endl;
			//cout << "third" << third_finger.x << "," << third_finger.y << endl;
		}
		else if (colPointUpList.size() == 4)//出现第四根
		{
			//cout << "four" << endl;
			if (colPointDownList[0].y < first_finger.y)
			{
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = first_finger.x;
				second_finger.y = first_finger.y;
				first_finger.x = colPointUpList[0].x;
				first_finger.y = colPointUpList[0].y;
			}
			else if (colPointDownList[1].y < second_finger.y)
			{
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = colPointUpList[1].x;
				second_finger.y = colPointUpList[1].y;
			}
			else if (colPointDownList[2].y < third_finger.y)
			{
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = colPointUpList[2].x;
				third_finger.y = colPointUpList[2].y;
			}
			else
			{
				forth_finger.x = colPointUpList[3].x;
				forth_finger.y = colPointUpList[3].y;
			}
			//cout << "first" << first_finger.x << "," << first_finger.y << endl;
			//cout << "second" << second_finger.x << "," << second_finger.y << endl;
			//cout << "third" << third_finger.x << "," << third_finger.y << endl;
			//cout << "forth" << forth_finger.x << "," << forth_finger.y << endl;
		}
		else if (colPointUpList.size() == 5)//出现第五根
		{
			//cout << "five" << endl;
			if (colPointDownList[0].y < first_finger.y)
			{
				fifth_finger.x = forth_finger.x;
				fifth_finger.y = forth_finger.y;
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = first_finger.x;
				second_finger.y = first_finger.y;
				first_finger.x = colPointUpList[0].x;
				first_finger.y = colPointUpList[0].y;
			}
			else if (colPointDownList[1].y < second_finger.y)
			{
				fifth_finger.x = forth_finger.x;
				fifth_finger.y = forth_finger.y;
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = second_finger.x;
				third_finger.y = second_finger.y;
				second_finger.x = colPointUpList[1].x;
				second_finger.y = colPointUpList[1].y;
			}
			else if (colPointDownList[2].y < third_finger.y)
			{
				fifth_finger.x = forth_finger.x;
				fifth_finger.y = forth_finger.y;
				forth_finger.x = third_finger.x;
				forth_finger.y = third_finger.y;
				third_finger.x = colPointUpList[2].x;
				third_finger.y = colPointUpList[2].y;
			}
			else if (colPointDownList[3].y < third_finger.y)
			{
				fifth_finger.x = forth_finger.x;
				fifth_finger.y = forth_finger.y;
				forth_finger.x = colPointUpList[3].x;
				forth_finger.y = colPointUpList[3].y;
			}
			else
			{
				fifth_finger.x = colPointUpList[4].x;
				fifth_finger.y = colPointUpList[4].y;
			}
			//cout << "first" << first_finger.x << "," << first_finger.y << endl;
			//cout << "second" << second_finger.x << "," << second_finger.y << endl;
			//cout << "third" << third_finger.x << "," << third_finger.y << endl;
			//cout << "forth" << forth_finger.x << "," << forth_finger.y << endl;
			//cout << "fifth" << fifth_finger.x << "," << fifth_finger.y << endl;
			break;
		}
		else
		{
			cout << colPointUpList.size() << endl;
		}
	}
	/**********************************************************/

	cv::circle(dst, first_finger, 5, cv::Scalar(255, 0, 0));
	cv::circle(dst, second_finger, 5, cv::Scalar(0, 255, 0));
	cv::circle(dst, third_finger, 5, cv::Scalar(0, 0, 255));
	cv::circle(dst, forth_finger, 5, cv::Scalar(255, 0, 255));
	cv::circle(dst, fifth_finger, 5, cv::Scalar(0, 255, 255));
}

int link(Mat& src, Mat& help, int point_x, int point_y, Point2f* finger_list)
{
	//候选指尖位置
	//Point2f* finger_candidate=new Point2f[10];
	int finger_count = 0;

	//待遍历线点压栈
	int stack_x[100];
	int stack_y[100];
	int stack_top = -1;
	//连通图中点的个数，过小候选指尖位置都放弃
	int plot_counter = 0;

	//邻域遍历次序
	int x_op[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	int y_op[] = { 0, 1, 1, 1, 0, -1, -1, -1 };

	//初始化，将该点压栈
	stack_x[0] = point_x;
	stack_y[0] = point_y;
	stack_top = 0;

	//栈中无点则结束一个连通图的遍历
	while (stack_top >= 0)
	{
		//遍历点坐标
		int x = stack_x[stack_top];
		int y = stack_y[stack_top];
		stack_top--;
		//邻域中线点个数
		int partial_counter = 0;

		//连通点计数器加一
		plot_counter++;

		help.ptr<uchar>(y)[x] = 255;//置255

		//访问邻域
		for (int i = 0; i < 8; i++)
		{
			uchar* help_ptr = help.ptr<uchar>(y + y_op[i]);

			if (help_ptr[x + x_op[i]] < 128)//未访问过
			{
				help_ptr[x + x_op[i]] = 128;//置128
				if (src.ptr<uchar>(y + y_op[i])[x + x_op[i]] > 128)//如果是线点
				{
					//压栈
					stack_top++;
					stack_x[stack_top] = x + x_op[i];
					stack_y[stack_top] = y + y_op[i];
					//邻域计数器加一
					partial_counter++;
				}
			}
			else//已访问过
			{
				if (src.ptr<uchar>(y + y_op[i])[x + x_op[i]] > 128)//如果是线点
				{
					//邻域计数器加一
					partial_counter++;
				}
			}
		}

		if (partial_counter == 1)//邻域内只有一个线点，则该点为候选指尖
		{
			finger_list[finger_count].x = x;//finger_candidate[finger_count].x = x;
			finger_list[finger_count].y = y;//finger_candidate[finger_count].y = y;
			finger_count++;
		}
	}

	//连通图中点的个数过小则放弃所有候选指尖
	if (plot_counter < 100)//返回NULL
	{
		return -1;
	}
	else//返回指尖列表
	{
		return finger_count;
	}

}

void OnParseTrace(const CFugue::CParser*, CFugue::CParser::TraceEventHandlerArgs* pEvArgs)
{
	std::wcout << "\n\t" << pEvArgs->szTraceMsg;
}

void OnParseError(const CFugue::CParser*, CFugue::CParser::ErrorEventHandlerArgs* pEvArgs)
{
	std::wcerr << "\n\t Error --> " << pEvArgs->szErrMsg;
	if (pEvArgs->szToken)
	{
		std::wcerr << "\t Token: " << pEvArgs->szToken;
	}
}

void music(Point2f* leftfinger_list, int leftfinger_count, int left_skip, Point2f* rightfinger_list, int rightfinger_count, int right_skip)
{
	int nPortID = MIDI_MAPPER, nTimerRes = 5;

	unsigned int nOutPortCount = CFugue::GetMidiOutPortCount();
	if (nOutPortCount <= 0)
		exit(fprintf(stderr, "No MIDI Output Ports found !!"));
	if (nOutPortCount > 1)
	{
		_tprintf(_T("\n---------  The MIDI Out Port indices  --------\n\n"));
		/////////////////////////////////////////
		/// List the MIDI Ports
		for (unsigned int i = 0; i < nOutPortCount; ++i)
		{
			std::string portName = CFugue::GetMidiOutPortName(i);
			std::wcout << "\t" << i << "\t: " << portName.c_str() << "\n";
		}
		//////////////////////////////////////////
		/// Chose a Midi output port
		_tprintf(_T("\nChoose your MIDI Port ID for the play:"));
		_tscanf(_T("%d"), &nPortID);

	}
	CFugue::Player player;
	player.Play("CT");
	player.Play("GT");
}
