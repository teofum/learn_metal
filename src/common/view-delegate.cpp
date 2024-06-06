#include "view-delegate.hpp"

MyMTKViewDelegate::MyMTKViewDelegate()
  : MTK::ViewDelegate() {
}

MyMTKViewDelegate::~MyMTKViewDelegate() {
  m_commandQueue->release();
  m_device->release();
}

void MyMTKViewDelegate::init(MTL::Device *device, MTK::View *view) {
  m_device = device->retain();
  m_commandQueue = m_device->newCommandQueue();
  m_view = view;
}
