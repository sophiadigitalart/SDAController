#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

// Settings
#include "VDSettings.h"
// Session
#include "VDSession.h"
// Log
#include "VDLog.h"
// Spout
#include "CiSpoutOut.h"
#include "CiSpoutIn.h"
// UI
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include "VDUI.h"
#define IM_ARRAYSIZE(_ARR)			((int)(sizeof(_ARR)/sizeof(*_ARR)))

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace VideoDromm;

class VDControllerApp : public App {

public:
	VDControllerApp();
	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;
	void fileDrop(FileDropEvent event) override;
	void update() override;
	void draw() override;
	void cleanup() override;
	void setUIVisibility(bool visible);
private:
	// Settings
	VDSettingsRef					mVDSettings;
	// Session
	VDSessionRef					mVDSession;
	// Log
	VDLogRef						mVDLog;
	// UI
	VDUIRef						mVDUI;
	// handle resizing for imgui
	void							resizeWindow();
	// imgui
	float							color[4];
	float							backcolor[4];
	int								playheadPositions[12];
	int								speeds[12];

	float							f = 0.0f;
	char							buf[64];
	unsigned int					i, j;

	bool							mouseGlobal;

	string							mError;
	// fbo
	bool							mIsShutDown;
	Anim<float>						mRenderWindowTimer;
	void							positionRenderWindow();
	bool							mFadeInDelay;
	SpoutOut 						mSpoutOut;
	SpoutIn							mSpoutIn;
	bool							mFlipV;
	bool							mFlipH;
	int								xLeft, xRight, yLeft, yRight;
	int								margin, tWidth, tHeight;
	int								mode;
};


VDControllerApp::VDControllerApp()
	: mSpoutOut("VDController", app::getWindowSize())
{
	// Settings
	mVDSettings = VDSettings::create("Controller");
	// Session
	mVDSession = VDSession::create(mVDSettings);
	//mVDSettings->mCursorVisible = true;
	setUIVisibility(mVDSettings->mCursorVisible);
	mVDSession->getWindowsResolution();

	mouseGlobal = false;
	mFadeInDelay = true;
	// UI
	mVDUI = VDUI::create(mVDSettings, mVDSession);
	// windows
	mIsShutDown = false;
	mFlipV = false;
	mFlipH = true;
	xLeft = 0;
	xRight = mVDSettings->mRenderWidth;
	yLeft = 0;
	yRight = mVDSettings->mRenderHeight;
	margin = 20;
	tWidth = mVDSettings->mFboWidth / 2;
	tHeight = mVDSettings->mFboHeight / 2;
	mRenderWindowTimer = 0.0f;
	mode = 0;
	//timeline().apply(&mRenderWindowTimer, 1.0f, 2.0f).finishFn([&] { positionRenderWindow(); });
	mVDSession->setBoolUniformValueByIndex(mVDSettings->IFLIPH, true);
	mVDSession->setBoolUniformValueByIndex(mVDSettings->IFLIPV, false);
}
void VDControllerApp::resizeWindow()
{
	mVDUI->resize();
}
void VDControllerApp::positionRenderWindow() {
	mVDSettings->mRenderPosXY = ivec2(mVDSettings->mRenderX, mVDSettings->mRenderY);//20141214 was 0
	setWindowPos(mVDSettings->mRenderX, mVDSettings->mRenderY);
	setWindowSize(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight);
}
void VDControllerApp::setUIVisibility(bool visible)
{
	if (visible)
	{
		showCursor();
	}
	else
	{
		hideCursor();
	}
}
void VDControllerApp::fileDrop(FileDropEvent event)
{
	mVDSession->fileDrop(event);
}
void VDControllerApp::update()
{
	mVDSession->setFloatUniformValueByIndex(mVDSettings->IFPS, getAverageFps());
	mVDSession->update();
	if(mSpoutIn.getSize() != app::getWindowSize()) {
		//app::setWindowSize(mSpoutIn.getSize());
	}
}
void VDControllerApp::cleanup()
{
	if (!mIsShutDown)
	{
		mIsShutDown = true;
		CI_LOG_V("shutdown");
		// save settings
		mVDSettings->save();
		mVDSession->save();
		quit();
	}
}
void VDControllerApp::mouseMove(MouseEvent event)
{
	if (!mVDSession->handleMouseMove(event)) {
		// let your application perform its mouseMove handling here
	}
}
void VDControllerApp::mouseDown(MouseEvent event)
{
	if (event.isRightDown()) { // Select a sender
		// SpoutPanel.exe must be in the executable path
		mSpoutIn.getSpoutReceiver().SelectSenderPanel(); // DirectX 11 by default
	}
	if (!mVDSession->handleMouseDown(event)) {
		// let your application perform its mouseDown handling here
		if (event.isRightDown()) { 
		}
	}
}
void VDControllerApp::mouseDrag(MouseEvent event)
{
	if (!mVDSession->handleMouseDrag(event)) {
		// let your application perform its mouseDrag handling here
	}	
}
void VDControllerApp::mouseUp(MouseEvent event)
{
	if (!mVDSession->handleMouseUp(event)) {
		// let your application perform its mouseUp handling here
	}
}

void VDControllerApp::keyDown(KeyEvent event)
{
#if defined( CINDER_COCOA )
	bool isModDown = event.isMetaDown();
#else // windows
	bool isModDown = event.isControlDown();
#endif
	CI_LOG_V("main keydown: " + toString(event.getCode()) + " ctrl: " + toString(isModDown));
	if (isModDown) {
	}
	if (!mVDSession->handleKeyDown(event)) {
		switch (event.getCode()) {
		case KeyEvent::KEY_h:
			// mouse cursor and ui visibility
			mVDSettings->mCursorVisible = !mVDSettings->mCursorVisible;
			setUIVisibility(mVDSettings->mCursorVisible);
			break;
	
		default:
			CI_LOG_V("main keydown: " + toString(event.getCode()));
			break;
		}
	}
}
void VDControllerApp::keyUp(KeyEvent event)
{
	if (!mVDSession->handleKeyUp(event)) {
		switch (event.getCode()) {
		
		default:
			CI_LOG_V("main keyup: " + toString(event.getCode()));
			break;
		}
	}
}

void VDControllerApp::draw()
{
	gl::clear(Color::black());
	if (mFadeInDelay) {
		mVDSettings->iAlpha = 0.0f;
		if (getElapsedFrames() > mVDSession->getFadeInDelay()) {
			mFadeInDelay = false;
			timeline().apply(&mVDSettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}
	
	// 20190215 gl::setMatricesWindow(toPixels(getWindowSize()), false);
	gl::setMatricesWindow(mVDSettings->mFboWidth, mVDSettings->mFboHeight, false);
	mode = mVDSession->getMode();
	switch (mode)
	{
	case 1:
		gl::draw(mVDSession->getMixTexture(), getWindowBounds());
		break;
	case 2:
		gl::draw(mVDSession->getRenderTexture(), getWindowBounds());
		break;
	case 3:
		gl::draw(mVDSession->getHydraTexture(), getWindowBounds());
		break;
	case 4:
		gl::draw(mVDSession->getFboTexture(0), getWindowBounds());
		break;
	case 5:
		gl::draw(mVDSession->getFboTexture(1), getWindowBounds());
		break;
	case 6:
		gl::draw(mVDSession->getFboTexture(2), getWindowBounds());
		break;
	case 7:
		gl::draw(mVDSession->getFboTexture(3), getWindowBounds());
		break;
	default:
		gl::draw(mVDSession->getMixetteTexture(), getWindowBounds());

		break;
	}
	// 20190215 gl::setMatricesWindow(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight, mVDSession->isFlipV());
	
	//Rectf rectangle = Rectf(mVDSettings->mxLeft, mVDSettings->myLeft, mVDSettings->mxRight, mVDSettings->myRight);
	//gl::draw(mVDSession->getMixTexture(), rectangle);
	//gl::drawString("xRight: " + std::to_string(mVDSettings->myRight), vec2(toPixels(400), toPixels(300)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
	// 20190215 gl::setMatricesWindow(mVDSettings->mMainWindowWidth, mVDSettings->mMainWindowHeight, false);
	//gl::draw(mVDSession->getMixTexture(), getWindowBounds());
	
	
	// Spout Send
	//mSpoutOut.sendViewport();
	mSpoutOut.sendTexture(mVDSession->getMixetteTexture());
	gl::draw(mVDSession->getMixetteTexture(), Rectf(0, 0, tWidth, tHeight));
	gl::drawString("Mixette", vec2(toPixels(xLeft), toPixels(tHeight)), Color(1, 1, 1), Font("Verdana", toPixels(16)));
	// flipH MODE_IMAGE = 1
	gl::draw(mVDSession->getRenderTexture(), Rectf(tWidth * 2 + margin, 0, tWidth + margin, tHeight));
	gl::drawString("Render", vec2(toPixels(xLeft + tWidth + margin), toPixels(tHeight)), Color(1, 1, 1), Font("Verdana", toPixels(16)));
	// flipV MODE_MIX = 0
	gl::draw(mVDSession->getMixTexture(), Rectf(0, tHeight * 2 + margin, tWidth, tHeight + margin));
	gl::drawString("Mix", vec2(toPixels(xLeft), toPixels(tHeight * 2 + margin)), Color(1, 1, 1), Font("Verdana", toPixels(16)));
	// show the FBO color texture 
	gl::draw(mVDSession->getHydraTexture(), Rectf(tWidth + margin, tHeight + margin, tWidth * 2 + margin, tHeight * 2 + margin));
	gl::drawString("Hydra", vec2(toPixels(xLeft + tWidth + margin), toPixels(tHeight * 2 + margin)), Color(1, 1, 1), Font("Verdana", toPixels(16)));


	gl::drawString("irx: " + std::to_string(mVDSession->getFloatUniformValueByName("iResolutionX"))
		+ " iry: " + std::to_string(mVDSession->getFloatUniformValueByName("iResolutionY"))
		+ " fw: " + std::to_string(mVDSettings->mFboWidth)
		+ " fh: " + std::to_string(mVDSettings->mFboHeight),
		vec2(xLeft, getWindowHeight() - toPixels(30)), Color(1, 1, 1),
		Font("Verdana", toPixels(24)));


	mVDUI->Run("UI", (int)getAverageFps());
	if (mVDUI->isReady()) {
	}
	getWindow()->setTitle(mVDSettings->sFps + " fps VDController");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(1280, 720);
	//settings->setWindowSize(1920, 1080);
#ifdef _DEBUG
	//settings->setConsoleWindowEnabled();
#else
	//settings->setConsoleWindowEnabled();
#endif  // _DEBUG
}

CINDER_APP(VDControllerApp, RendererGl, prepareSettings)
