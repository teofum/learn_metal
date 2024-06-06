#ifndef _00_window_view_delegate_h
#define _00_window_view_delegate_h

#include "Metal/Metal.hpp"
#include "AppKit/AppKit.hpp"
#include "MetalKit/MetalKit.hpp"

class MyMTKViewDelegate : public MTK::ViewDelegate {
public:
  MyMTKViewDelegate();

  ~MyMTKViewDelegate() override;

  virtual void init(MTL::Device *device, MTK::View *view);

protected:
  MTL::Device *m_device = nullptr;
  MTL::CommandQueue *m_commandQueue = nullptr;
  MTK::View *m_view = nullptr;
};

#endif
