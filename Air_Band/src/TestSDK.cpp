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

	//1����ʼ��NUI 
	HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
	if (FAILED(hr))
	{
		cout << "NuiInitialize failed" << endl;
		return hr;
	}

	//2�������¼���� 
	//������ȡ��һ֡���ź��¼����������KINECT�Ƿ���Կ�ʼ��ȡ��һ֡����
	HANDLE nextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE colorStreamHandle = NULL; //����ͼ���������ľ����������ȡ���� 

	//3����KINECT�豸�Ĳ�ɫͼ��Ϣͨ��������colorStreamHandle��������ľ�����Ա����Ժ��ȡ
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,
		0, 2, nextColorFrameEvent, &colorStreamHandle);
	if (FAILED(hr))//�ж��Ƿ���ȡ��ȷ 
	{
		cout << "Could not open color image stream video" << endl;
		NuiShutdown();
		return hr;
	}
	namedWindow("colorImage", CV_WINDOW_AUTOSIZE);

	//4����ʼ��ȡ��ɫͼ���� 
	while (1)
	{
		const NUI_IMAGE_FRAME * pImageFrame = NULL;

		//4.1�����޵ȴ��µ����ݣ��ȵ��󷵻�
		if (WaitForSingleObject(nextColorFrameEvent, INFINITE) == 0)
		{
			//4.2���ӸղŴ���������������еõ���֡���ݣ���ȡ�������ݵ�ַ����pImageFrame
			hr = NuiImageStreamGetNextFrame(colorStreamHandle, 0, &pImageFrame);
			if (FAILED(hr))
			{
				cout << "Could not get color image" << endl;
				NuiShutdown();
				return -1;
			}

			INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
			NUI_LOCKED_RECT LockedRect;

			//4.3����ȡ����֡��LockedRect���������������ݶ���pitchÿ���ֽ�����pBits��һ���ֽڵ�ַ
			//���������ݣ����������Ƕ����ݵ�ʱ��kinect�Ͳ���ȥ�޸���
			pTexture->LockRect(0, &LockedRect, NULL, 0);
			//4.4��ȷ�ϻ�õ������Ƿ���Ч
			if (LockedRect.Pitch != 0)
			{
				//4.5��������ת��ΪOpenCV��Mat��ʽ
				for (int i = 0; i<image.rows; i++)
				{
					uchar *ptr = image.ptr<uchar>(i);  //��i�е�ָ��

					//ÿ���ֽڴ���һ����ɫ��Ϣ��ֱ��ʹ��uchar
					uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
					for (int j = 0; j<image.cols; j++)
					{
						ptr[3 * j] = pBuffer[4 * j];  //�ڲ�������4���ֽڣ�0-1-2��BGR����4������δʹ�� 
						ptr[3 * j + 1] = pBuffer[4 * j + 1];
						ptr[3 * j + 2] = pBuffer[4 * j + 2];
					}
				}
				imshow("colorImage", image); //��ʾͼ�� 
			}
			else
			{
				cout << "Buffer length of received texture is bogus\r\n" << endl;
			}

			//5����֡�Ѿ��������ˣ����Խ������
			pTexture->UnlockRect(0);
			//6���ͷű�֡���ݣ�׼��ӭ����һ֡ 
			NuiImageStreamReleaseFrame(colorStreamHandle, pImageFrame);
		}
		if (cvWaitKey(20) == 27)
			break;
	}
	//7���ر�NUI���� 
	NuiShutdown();
	return 0;
}