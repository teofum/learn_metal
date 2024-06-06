#include <AppKit/AppKit.hpp>

#include <iostream>
#include <cassert>

#include <app-delegate.hpp>
#include <utils.hpp>

#include "vertex.hpp"

/**
 * Renderer class
 */
class HelloTriangleViewDelegate : public MyMTKViewDelegate {
private:
  MTL::RenderPipelineState *m_pso = nullptr;
  MTL::Buffer *m_vertexBuffer = nullptr;
  uint2 m_viewportSize = {0, 0};

  static constexpr const Vertex m_vertexData[] = {
    {{100,  -100}, {1, 0, 0, 1}},
    {{-100, -100}, {0, 1, 0, 1}},
    {{0,    100},  {0, 0, 1, 1}},
  };

  void buildBuffers() {
    size_t bufferSize = 3 * sizeof(Vertex);
    m_vertexBuffer = m_device->newBuffer(bufferSize, MTL::ResourceStorageModeManaged);

    memcpy(m_vertexBuffer->contents(), m_vertexData, bufferSize);
    m_vertexBuffer->didModifyRange(NS::Range::Make(0, m_vertexBuffer->length()));
  }

  void buildShaders() {
    NS::Error *error = nullptr;
    MTL::Library *lib = m_device->newLibrary(nsStr("02-hello-3d.metallib"), &error);
    if (!lib) {
      std::cerr << error->localizedDescription()->utf8String() << "\n";
      assert(false);
    }

    MTL::Function *vertexFunction = lib->newFunction(nsStr("vertexShader"));
    MTL::Function *fragmentFunction = lib->newFunction(nsStr("fragmentShader"));

    MTL::RenderPipelineDescriptor *desc = MTL::RenderPipelineDescriptor::alloc()->init();
    desc->setVertexFunction(vertexFunction);
    desc->setFragmentFunction(fragmentFunction);
    desc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

    m_pso = m_device->newRenderPipelineState(desc, &error);
    if (!m_pso) {
      std::cerr << error->localizedDescription()->utf8String() << "\n";
      assert(false);
    }

    vertexFunction->release();
    fragmentFunction->release();
    desc->release();
    lib->release();
  }

public:
  void init(MTL::Device *device, MTK::View *view) override {
    MyMTKViewDelegate::init(device, view);

    m_viewportSize.x = static_cast<uint>(view->drawableSize().width);
    m_viewportSize.y = static_cast<uint>(view->drawableSize().height);
    buildBuffers();
    buildShaders();
  }

  ~HelloTriangleViewDelegate() override {
    m_pso->release();
    m_vertexBuffer->release();
  }

  void drawInMTKView(MTK::View *view) override {
    {
      NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

      MTL::CommandBuffer *cmd = m_commandQueue->commandBuffer();
      MTL::RenderPassDescriptor *rpd = view->currentRenderPassDescriptor();
      MTL::RenderCommandEncoder *enc = cmd->renderCommandEncoder(rpd);

      enc->setViewport({0.0, 0.0, (double) m_viewportSize.x, (double) m_viewportSize.y, 0.0, 1.0});
      enc->setRenderPipelineState(m_pso);
      enc->setVertexBuffer(m_vertexBuffer, 0, 0);
      enc->setVertexBytes(&m_viewportSize, sizeof(m_viewportSize), 1);

      enc->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 3);

      enc->endEncoding();

      cmd->presentDrawable(view->currentDrawable());
      cmd->commit();

      pool->release();
    }
  }

  void drawableSizeWillChange(MTK::View *view, CGSize size) override {
    m_viewportSize.x = static_cast<uint>(size.width);
    m_viewportSize.y = static_cast<uint>(size.height);
  }
};

int main() {
  // Autorelease pool used for reference counting
  // https://developer.apple.com/documentation/foundation/nsautoreleasepool
  NS::AutoreleasePool *autoreleasePool = NS::AutoreleasePool::alloc()->init();

  MyAppDelegate del(new HelloTriangleViewDelegate(), "01 - Hello Triangle");

  // NSApplication object managed the main event loop and delegates
  // https://developer.apple.com/documentation/appkit/nsapplication
  NS::Application *sharedApplication = NS::Application::sharedApplication();
  sharedApplication->setDelegate(&del);
  sharedApplication->run();

  autoreleasePool->release();
  return 0;
}
