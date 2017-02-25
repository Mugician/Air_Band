#include <windows.h>
#include <iostream> 
#include <NuiApi.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int testsdk(int argc, char *argv[])
{
	Mat image;
	image.create(480, 640, CV_8UC3);

	//1、初始化NUI 
	HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
	if (FAILED(hr))
	{
		cout << "NuiInitialize failed" << endl;
		return hr;
	}

	//2、定义事件句柄 
	//创建读取下一帧的信号事件句柄，控制KINECT是否可以开始读取下一帧数据
	HANDLE nextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE colorStreamHandle = NULL; //保存图像数据流的句柄，用以提取数据 

	//3、打开KINECT设备的彩色图信息通道，并用colorStreamHandle保存该流的句柄，以便于以后读取
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,
		0, 2, nextColorFrameEvent, &colorStreamHandle);
	if (FAILED(hr))//判断是否提取正确 
	{
		cout << "Could not open color image stream video" << endl;
		NuiShutdown();
		return hr;
	}
	namedWindow("colorImage", CV_WINDOW_AUTOSIZE);

	//4、开始读取彩色图数据 
	while (1)
	{
		const NUI_IMAGE_FRAME * pImageFrame = NULL;

		//4.1、无限等待新的数据，等到后返回
		if (WaitForSingleObject(nextColorFrameEvent, INFINITE) == 0)
		{
			//4.2、从刚才打开数据流的流句柄中得到该帧数据，读取到的数据地址存于pImageFrame
			hr = NuiImageStreamGetNextFrame(colorStreamHandle, 0, &pImageFrame);
			if (FAILED(hr))
			{
				cout << "Could not get color image" << endl;
				NuiShutdown();
				return -1;
			}

			INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
			NUI_LOCKED_RECT LockedRect;

			//4.3、提取数据帧到LockedRect，它包括两个数据对象：pitch每行字节数，pBits第一个字节地址
			//并锁定数据，这样当我们读数据的时候，kinect就不会去修改它
			pTexture->LockRect(0, &LockedRect, NULL, 0);
			//4.4、确认获得的数据是否有效
			if (LockedRect.Pitch != 0)
			{
				//4.5、将数据转换为OpenCV的Mat格式
				for (int i = 0; i<image.rows; i++)
				{
					uchar *ptr = image.ptr<uchar>(i);  //第i行的指针

					//每个字节代表一个颜色信息，直接使用uchar
					uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
					for (int j = 0; j<image.cols; j++)
					{
						ptr[3 * j] = pBuffer[4 * j];  //内部数据是4个字节，0-1-2是BGR，第4个现在未使用 
						ptr[3 * j + 1] = pBuffer[4 * j + 1];
						ptr[3 * j + 2] = pBuffer[4 * j + 2];
					}
				}
				imshow("colorImage", image); //显示图像 
			}
			else
			{
				cout << "Buffer length of received texture is bogus\r\n" << endl;
			}

			//5、这帧已经处理完了，所以将其解锁
			pTexture->UnlockRect(0);
			//6、释放本帧数据，准备迎接下一帧 
			NuiImageStreamReleaseFrame(colorStreamHandle, pImageFrame);
		}
		if (cvWaitKey(20) == 27)
			break;
	}
	//7、关闭NUI链接 
	NuiShutdown();
	return 0;
}