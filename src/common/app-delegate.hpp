#ifndef _00_window_app_delegate_h
#define _00_window_app_delegate_h

#include "Metal/Metal.hpp"
#include "AppKit/AppKit.hpp"
#include "MetalKit/MetalKit.hpp"

#include "view-delegate.hpp"

/**
 * Delegate class: implements app functionality, receives notifications from the
 * NSApplication object
 */
class MyAppDelegate : public NS::ApplicationDelegate {
public:
  explicit MyAppDelegate(MyMTKViewDelegate *viewDelegate, const char *title = "Metal App");

  ~MyAppDelegate() override;

  static NS::Menu *createMenuBar();

  void applicationDidFinishLaunching(NS::Notification *notification) override;

  void applicationWillFinishLaunching(NS::Notification *notification) override;

  bool applicationShouldTerminateAfterLastWindowClosed(NS::Application *sender) override;

private:
  const char *m_title;
  NS::Window *m_window = nullptr;
  MTK::View *m_mtkView = nullptr;
  MTL::Device *m_device = nullptr;

  MyMTKViewDelegate *m_viewDelegate;
};

#endif
