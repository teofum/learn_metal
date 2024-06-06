#include "app-delegate.hpp"

MyAppDelegate::MyAppDelegate(MyMTKViewDelegate *viewDelegate, const char *title)
  : m_viewDelegate(viewDelegate), m_title(title) {
}

MyAppDelegate::~MyAppDelegate() {
  m_mtkView->release();
  m_window->release();
  m_device->release();

  delete m_viewDelegate;
}

NS::Menu *MyAppDelegate::createMenuBar() {
  using NS::StringEncoding::UTF8StringEncoding;

  // "root" object for the menubar
  NS::Menu *mainMenu = NS::Menu::alloc()->init();

  // Aplpication menu item
  NS::MenuItem *appMenuItem = NS::MenuItem::alloc()->init();
  mainMenu->addItem(appMenuItem);

  // "Appname" is a placeholder, the first menu always shows the app name
  NS::Menu *appMenu = NS::Menu::alloc()->init(NS::String::string("Appname", UTF8StringEncoding));
  appMenuItem->setSubmenu(appMenu);

  // Quit "appname" menu item
  NS::String *appName = NS::RunningApplication::currentApplication()->localizedName();
  NS::String *quitItemName = NS::String::string("Quit ", UTF8StringEncoding)->stringByAppendingString(appName);
  SEL quitCb = NS::MenuItem::registerActionCallback(
    "appQuit", [](void *, SEL, const NS::Object *sender) {
      auto app = NS::Application::sharedApplication();
      app->terminate(sender);
    }
  );
  NS::MenuItem *appQuitItem = appMenu->addItem(
    quitItemName,
    quitCb,
    NS::String::string("q", UTF8StringEncoding)
  );
  appQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);

  // Window menu item
  NS::MenuItem *windowMenuItem = NS::MenuItem::alloc()->init();
  mainMenu->addItem(windowMenuItem);

  NS::Menu *windowMenu = NS::Menu::alloc()->init(NS::String::string("Window", UTF8StringEncoding));
  windowMenuItem->setSubmenu(windowMenu);

  SEL closeWindowCb = NS::MenuItem::registerActionCallback(
    "windowClose", [](void *, SEL, const NS::Object *) {
      auto pApp = NS::Application::sharedApplication();
      pApp->windows()->object<NS::Window>(0)->close();
    }
  );
  NS::MenuItem *closeWindowItem = windowMenu->addItem(
    NS::String::string("Close Window", UTF8StringEncoding),
    closeWindowCb,
    NS::String::string("w", UTF8StringEncoding));
  closeWindowItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);

  // Release everything (mainMenu owns these references now?)
  appMenuItem->release();
  windowMenuItem->release();
  appMenu->release();
  windowMenu->release();

  return mainMenu->autorelease();
}

/**
 * Called just before the app launches (initialization about to complete)
 */
void MyAppDelegate::applicationWillFinishLaunching(NS::Notification *notification) {
  NS::Menu *menu = createMenuBar();

  // Get the app object and set the main menu
  auto *app = reinterpret_cast<NS::Application *>(notification->object());
  app->setMainMenu(menu);

  // Activation policy, basically the "type" of app
  // https://developer.apple.com/documentation/appkit/nsapplication/activationpolicy
  app->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

/**
 * Called just *after* the app launches
 * Creates the window
 */
void MyAppDelegate::applicationDidFinishLaunching(NS::Notification *notification) {
  // Initial window coordinates
  CGRect frame = {{100.0, 100.0},
                  {512.0, 512.0}};

  // Window type flags
  auto flags = NS::WindowStyleMaskClosable |
               NS::WindowStyleMaskTitled |
               NS::WindowStyleMaskResizable |
               NS::WindowStyleMaskMiniaturizable;

  // Create the window object
  m_window = NS::Window::alloc()->init(
    frame,
    flags,
    NS::BackingStoreBuffered, // Drawing mode, non buffered is deprecated
    false
  );

  // Get the Metal device
  m_device = MTL::CreateSystemDefaultDevice();

  // Create a MetalKit view
  m_mtkView = MTK::View::alloc()->init(frame, m_device);
  m_mtkView->setColorPixelFormat(MTL::PixelFormatBGRA8Unorm_sRGB);
  m_mtkView->setDepthStencilPixelFormat(MTL::PixelFormatDepth32Float);
  m_mtkView->setClearColor(MTL::ClearColor::Make(0.0, 0.0, 0.0, 1.0));

  // Pass view delegate to the MTK view
  m_viewDelegate->init(m_device, m_mtkView);
  m_mtkView->setDelegate(m_viewDelegate);

  // Pass the MTK view to the window
  m_window->setContentView(m_mtkView);
  m_window->setTitle(NS::String::string(m_title, NS::StringEncoding::UTF8StringEncoding));

  // Makes the window the "key" (main) window for the application and shows it
  m_window->makeKeyAndOrderFront(nullptr);

  // Focuses the window, even if the user switched to another app in the meantime
  // Deprecated because bad for UX, but keep it around for development because
  // we actually want the app to steal focus from the IDE
  auto *app = reinterpret_cast<NS::Application *>(notification->object());
  app->activateIgnoringOtherApps(true);
}

// Self explanatory
bool MyAppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application *sender) {
  return true;
}
