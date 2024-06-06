#include "AppKit/AppKit.hpp"

#include "app-delegate.hpp"

/**
 * Renderer class
 */
class WindowViewDelegate : public MyMTKViewDelegate {
public:
  void drawInMTKView(MTK::View *view) override {
    {
      NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

      // Stores rendering commands
      MTL::CommandBuffer *cmd = m_commandQueue->commandBuffer();

      // Defines render targets (framebuffers) among other things
      // This is the view render pass, equivalent to a default framebuffer
      MTL::RenderPassDescriptor *rpd = view->currentRenderPassDescriptor();

      // Encondes rendering commands to a render pass descriptor
      // We pass no commands so this only clears the buffer (with clear color)
      MTL::RenderCommandEncoder *enc = cmd->renderCommandEncoder(rpd);
      enc->endEncoding();

      // A drawable is a RT that can be drawn to the screen, MTK creates one for us
      // (the default drawable, which uses the default RT)
      cmd->presentDrawable(view->currentDrawable());
      cmd->commit(); // Commit the render commands to the GPU

      pool->release();
    }
  }
};

int main() {
  // Autorelease pool used for reference counting
  // https://developer.apple.com/documentation/foundation/nsautoreleasepool
  NS::AutoreleasePool *autoreleasePool = NS::AutoreleasePool::alloc()->init();
  
  MyAppDelegate del(new WindowViewDelegate());

  // NSApplication object managed the main event loop and delegates
  // https://developer.apple.com/documentation/appkit/nsapplication
  NS::Application *sharedApplication = NS::Application::sharedApplication();
  sharedApplication->setDelegate(&del);
  sharedApplication->run();

  autoreleasePool->release();
  return 0;
}
