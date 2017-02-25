#include <iostream>
#include "OpenNI.h"

int testni(int argc, char** argv)
{
// ��ʼ��OpenNI����
openni::OpenNI::initialize();

// ��������Device�豸�����õ���Kinect��
openni::Device devAnyDevice;
devAnyDevice.open(openni::ANY_DEVICE);

// �����������������
openni::VideoStream streamDepth;
streamDepth.create(devAnyDevice, openni::SENSOR_DEPTH);
streamDepth.start();

// ͬ���Ĵ������򿪲�ɫͼ��������
openni::VideoStream streamColor;
streamColor.create(devAnyDevice, openni::SENSOR_COLOR);
streamColor.start();

// ѭ����ȡ��������Ϣ��������VideoFrameRef��
openni::VideoFrameRef frameDepth;
openni::VideoFrameRef frameColor;
for (int i = 0; i < 1000; ++i)
{
// ��ȡ������
streamDepth.readFrame(&frameDepth);
streamColor.readFrame(&frameColor);

// ��ȡdata array
const openni::DepthPixel* pDepth
= (const openni::DepthPixel*)frameDepth.getData();
const openni::RGB888Pixel* pColor
= (const openni::RGB888Pixel*)frameColor.getData();

// ��ʾ�����Ϣ�Ͷ�Ӧ�Ĳ�ɫR��G��B��ֵ
int idx = frameDepth.getWidth() * (frameDepth.getHeight() + 1) / 2;
std::cout << pDepth[idx] << "( "
<< (int)pColor[idx].r << ","
<< (int)pColor[idx].g << ","
<< (int)pColor[idx].b << ")"
<< std::endl;
}

// �ر�������
streamDepth.destroy();
streamColor.destroy();

// �ر��豸
devAnyDevice.close();

// ���ر�OpenNI
openni::OpenNI::shutdown();

return 0;
}