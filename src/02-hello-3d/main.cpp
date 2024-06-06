#include <AppKit/AppKit.hpp>

#include <iostream>
#include <cassert>
#include <cmath>
#include <numbers>

#include <app-delegate.hpp>
#include <utils.hpp>

#include "shader-defs.hpp"
#include "matrices.hpp"

/**
 * Renderer class
 */
class HelloTriangleViewDelegate : public MyMTKViewDelegate {
private:
  MTL::RenderPipelineState *m_pso = nullptr;
  MTL::DepthStencilState *m_dsso = nullptr;
  MTL::Buffer *m_vertexBuffer = nullptr;
  MTL::Buffer *m_indexBuffer = nullptr;
  uint2 m_viewportSize = {0, 0};

  MTL::Buffer *m_constantsBuffer = nullptr;
  size_t m_constantsSize = 0, m_constantsStride = 0, m_constantsOffset = 0;

  static constexpr size_t m_maxFramesInFlight = 3;
  size_t m_frameIdx = 0;
  dispatch_semaphore_t m_frameSemaphore = nullptr;

  std::chrono::time_point<std::chrono::steady_clock> m_startTime;

  float3 m_cameraPos = {0.0f, 0.0f, 5.0f};
  float m_fov = 45.0f;
  float m_aspect = 1.0;

  static constexpr const Vertex m_vertexData[] = {
    {{1,  1,  -1}, {1, 1, 0, 1}},
    {{1,  -1, -1}, {1, 0, 0, 1}},
    {{1,  1,  1},  {1, 1, 1, 1}},
    {{1,  -1, 1},  {1, 0, 1, 1}},
    {{-1, 1,  -1}, {0, 1, 0, 1}},
    {{-1, -1, -1}, {0, 0, 0, 1}},
    {{-1, 1,  1},  {0, 1, 1, 1}},
    {{-1, -1, 1},  {0, 0, 1, 1}},
  };
  static constexpr size_t m_vertexCount = sizeof(m_vertexData) / sizeof(Vertex);

  static constexpr const unsigned m_indexData[] = {
    4, 2, 0, 2, 7, 3,
    6, 5, 7, 1, 7, 5,
    0, 3, 1, 4, 1, 5,
    4, 6, 2, 2, 6, 7,
    6, 4, 5, 1, 3, 7,
    0, 2, 3, 4, 0, 1,
  };
  static constexpr size_t m_indexCount = sizeof(m_indexData) / sizeof(unsigned);

  void buildBuffers() {
    /*
     * Build the vertex buffer
     */
    size_t vertexBufferSize = m_vertexCount * sizeof(Vertex);
    m_vertexBuffer = m_device->newBuffer(vertexBufferSize, MTL::ResourceStorageModeManaged);

    memcpy(m_vertexBuffer->contents(), m_vertexData, vertexBufferSize);
    m_vertexBuffer->didModifyRange(NS::Range::Make(0, m_vertexBuffer->length()));

    /*
     * Build the index buffer
     */
    size_t indexBufferSize = m_indexCount * sizeof(unsigned);
    m_indexBuffer = m_device->newBuffer(indexBufferSize, MTL::ResourceStorageModeShared);

    memcpy(m_indexBuffer->contents(), m_indexData, indexBufferSize);
    m_indexBuffer->didModifyRange(NS::Range::Make(0, m_indexBuffer->length()));

    /*
     * Build the constants buffer
     */
    m_constantsSize = sizeof(Transforms);
    m_constantsStride = ((m_constantsSize - 1) / 256 + 1) * 256;
    m_constantsOffset = 0;

    m_constantsBuffer = m_device->newBuffer(m_constantsStride * m_maxFramesInFlight, MTL::ResourceStorageModeShared);
  }

  void buildShaders() {
    /*
     * Load the shader library, then load the shader functions
     */
    NS::Error *error = nullptr;
    MTL::Library *lib = m_device->newLibrary(nsStr("02-hello-3d.metallib"), &error);
    if (!lib) {
      std::cerr << error->localizedDescription()->utf8String() << "\n";
      assert(false);
    }

    MTL::Function *vertexFunction = lib->newFunction(nsStr("vertexShader"));
    MTL::Function *fragmentFunction = lib->newFunction(nsStr("fragmentShader"));

    /*
     * Set up a render pipeline descriptor (parameter object)
     * Set the vertex and fragment funcs and color attachment format (match view)
     */
    auto desc = MTL::RenderPipelineDescriptor::alloc()->init();
    desc->setVertexFunction(vertexFunction);
    desc->setFragmentFunction(fragmentFunction);
    desc->colorAttachments()->object(0)->setPixelFormat(m_view->colorPixelFormat());

    /*
     * Set up a vertex attribute descriptor, this tells Metal where each attribute
     * is located
     * TODO: this can be encapsulated in a less verbose API
     */
    auto *vertexDesc = MTL::VertexDescriptor::alloc()->init();

    auto positionAttribDesc = MTL::VertexAttributeDescriptor::alloc()->init();
    positionAttribDesc->setFormat(MTL::VertexFormatFloat3);
    positionAttribDesc->setOffset(offsetof(Vertex, position));
    positionAttribDesc->setBufferIndex(0);

    auto colorAttribDesc = MTL::VertexAttributeDescriptor::alloc()->init();
    colorAttribDesc->setFormat(MTL::VertexFormatFloat4);
    colorAttribDesc->setOffset(offsetof(Vertex, color));
    colorAttribDesc->setBufferIndex(0);

    vertexDesc->attributes()->setObject(positionAttribDesc, 0);
    vertexDesc->attributes()->setObject(colorAttribDesc, 1);

    auto vertexLayout = MTL::VertexBufferLayoutDescriptor::alloc()->init();
    vertexLayout->setStride(sizeof(Vertex));
    vertexDesc->layouts()->setObject(vertexLayout, 0);

    desc->setVertexDescriptor(vertexDesc);

    /*
     * Get the pipeline state object
     */
    m_pso = m_device->newRenderPipelineState(desc, &error);
    if (!m_pso) {
      std::cerr << error->localizedDescription()->utf8String() << "\n";
      assert(false);
    }

    /*
     * Set up the depth/stencil buffer
     */
    auto depthStencilDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthStencilDesc->setDepthWriteEnabled(true);
    depthStencilDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    m_dsso = m_device->newDepthStencilState(depthStencilDesc);

    depthStencilDesc->release();
    vertexFunction->release();
    fragmentFunction->release();
    desc->release();
    lib->release();
  }

  void updateConstants() {
    float time = getElapsedSeconds();
    float angle = std::fmod(time * 0.5f, 2.0f * std::numbers::pi_v<float>);

    Transforms transforms;
    transforms.model = mat::rotation(angle, float3{0.5, 1.0, 0.0});
    transforms.view = mat::translation(-m_cameraPos);
    transforms.projection = mat::projection(m_fov, m_aspect, 0.1f, 100.0f);

    m_constantsOffset = (m_frameIdx % m_maxFramesInFlight) * m_constantsStride;
    void *bufferWrite = (char *) m_constantsBuffer->contents() + m_constantsOffset;
    memcpy(bufferWrite, &transforms, m_constantsSize);
  }

  float getElapsedSeconds() {
    auto now = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::duration<float>>(now - m_startTime).count();
  }

public:
  HelloTriangleViewDelegate() {
    m_frameSemaphore = dispatch_semaphore_create(m_maxFramesInFlight);
  }

  void init(MTL::Device *device, MTK::View *view) override {
    MyMTKViewDelegate::init(device, view);

    m_viewportSize.x = static_cast<uint>(view->drawableSize().width);
    m_viewportSize.y = static_cast<uint>(view->drawableSize().height);
    m_aspect = static_cast<float>(view->drawableSize().width) / static_cast<float>(view->drawableSize().height);

    buildBuffers();
    buildShaders();

    m_startTime = std::chrono::high_resolution_clock::now();
  }

  ~HelloTriangleViewDelegate() override {
    m_pso->release();
    m_vertexBuffer->release();
  }

  void drawInMTKView(MTK::View *view) override {
    {
      NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

      dispatch_semaphore_wait(m_frameSemaphore, 100);
      updateConstants();

      MTL::CommandBuffer *cmd = m_commandQueue->commandBuffer();
      MTL::RenderPassDescriptor *rpd = view->currentRenderPassDescriptor();
      MTL::RenderCommandEncoder *enc = cmd->renderCommandEncoder(rpd);

      enc->setDepthStencilState(m_dsso);
      enc->setFrontFacingWinding(MTL::WindingCounterClockwise);
      enc->setCullMode(MTL::CullModeBack);

      enc->setViewport({0.0, 0.0, (double) m_viewportSize.x, (double) m_viewportSize.y, 0.0, 1.0});
      enc->setRenderPipelineState(m_pso);
      enc->setVertexBuffer(m_vertexBuffer, 0, 0);
      enc->setVertexBuffer(m_constantsBuffer, m_constantsOffset, 1);

      enc->drawIndexedPrimitives(
        MTL::PrimitiveTypeTriangle,
        m_indexCount,
        MTL::IndexTypeUInt32,
        m_indexBuffer,
        0
      );

      enc->endEncoding();

      cmd->presentDrawable(view->currentDrawable());
      cmd->addCompletedHandler(
        [this](MTL::CommandBuffer *cmd) {
          dispatch_semaphore_signal(this->m_frameSemaphore);
        }
      );
      cmd->commit();

      m_frameIdx++;

      pool->release();
    }
  }

  void drawableSizeWillChange(MTK::View *view, CGSize size) override {
    m_viewportSize.x = static_cast<uint>(size.width);
    m_viewportSize.y = static_cast<uint>(size.height);

    m_aspect = static_cast<float>(size.width) / static_cast<float>(size.height);
  }
};

int main() {
  // Autorelease pool used for reference counting
  // https://developer.apple.com/documentation/foundation/nsautoreleasepool
  NS::AutoreleasePool *autoreleasePool = NS::AutoreleasePool::alloc()->init();

  MyAppDelegate del(new HelloTriangleViewDelegate(), "02 - Hello 3D");

  // NSApplication object managed the main event loop and delegates
  // https://developer.apple.com/documentation/appkit/nsapplication
  NS::Application *sharedApplication = NS::Application::sharedApplication();
  sharedApplication->setDelegate(&del);
  sharedApplication->run();

  autoreleasePool->release();
  return 0;
}
