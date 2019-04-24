#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

// Settings
#include "SDASettings.h"
// Session
#include "SDASession.h"
// Log
#include "SDALog.h"
// Spout
#include "CiSpoutOut.h"
#include "CiSpoutIn.h"
// UI
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include "SDAUI.h"
#define IM_ARRAYSIZE(_ARR)			((int)(sizeof(_ARR)/sizeof(*_ARR)))

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace SophiaDigitalArt;

class SDAControllerApp : public App {

public:
	SDAControllerApp();
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
	SDASettingsRef					mSDASettings;
	// Session
	SDASessionRef					mSDASession;
	// Log
	SDALogRef						mSDALog;
	// UI
	SDAUIRef						mSDAUI;
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
};


SDAControllerApp::SDAControllerApp()
	: mSpoutOut("SDAController", app::getWindowSize())
{
	// Settings
	mSDASettings = SDASettings::create("Controller");
	// Session
	mSDASession = SDASession::create(mSDASettings);
	//mSDASettings->mCursorVisible = true;
	setUIVisibility(mSDASettings->mCursorVisible);
	mSDASession->getWindowsResolution();

	mouseGlobal = false;
	mFadeInDelay = true;
	// UI
	mSDAUI = SDAUI::create(mSDASettings, mSDASession);
	// windows
	mIsShutDown = false;
	mFlipV = false;
	mFlipH = true;
	xLeft = 0;
	xRight = mSDASettings->mRenderWidth;
	yLeft = 0;
	yRight = mSDASettings->mRenderHeight;
	margin = 20;
	tWidth = mSDASettings->mFboWidth / 2;
	tHeight = mSDASettings->mFboHeight / 2;
	mRenderWindowTimer = 0.0f;
	//timeline().apply(&mRenderWindowTimer, 1.0f, 2.0f).finishFn([&] { positionRenderWindow(); });

}
void SDAControllerApp::resizeWindow()
{
	mSDAUI->resize();
}
void SDAControllerApp::positionRenderWindow() {
	mSDASettings->mRenderPosXY = ivec2(mSDASettings->mRenderX, mSDASettings->mRenderY);//20141214 was 0
	setWindowPos(mSDASettings->mRenderX, mSDASettings->mRenderY);
	setWindowSize(mSDASettings->mRenderWidth, mSDASettings->mRenderHeight);
}
void SDAControllerApp::setUIVisibility(bool visible)
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
void SDAControllerApp::fileDrop(FileDropEvent event)
{
	mSDASession->fileDrop(event);
}
void SDAControllerApp::update()
{
	mSDASession->setFloatUniformValueByIndex(mSDASettings->IFPS, getAverageFps());
	mSDASession->update();
	if(mSpoutIn.getSize() != app::getWindowSize()) {
		//app::setWindowSize(mSpoutIn.getSize());
	}
}
void SDAControllerApp::cleanup()
{
	if (!mIsShutDown)
	{
		mIsShutDown = true;
		CI_LOG_V("shutdown");
		// save settings
		mSDASettings->save();
		mSDASession->save();
		quit();
	}
}
void SDAControllerApp::mouseMove(MouseEvent event)
{
	if (!mSDASession->handleMouseMove(event)) {
		// let your application perform its mouseMove handling here
	}
}
void SDAControllerApp::mouseDown(MouseEvent event)
{
	if (event.isRightDown()) { // Select a sender
		// SpoutPanel.exe must be in the executable path
		mSpoutIn.getSpoutReceiver().SelectSenderPanel(); // DirectX 11 by default
	}
	if (!mSDASession->handleMouseDown(event)) {
		// let your application perform its mouseDown handling here
		if (event.isRightDown()) { 
		}
	}
}
void SDAControllerApp::mouseDrag(MouseEvent event)
{
	if (!mSDASession->handleMouseDrag(event)) {
		// let your application perform its mouseDrag handling here
	}	
}
void SDAControllerApp::mouseUp(MouseEvent event)
{
	if (!mSDASession->handleMouseUp(event)) {
		// let your application perform its mouseUp handling here
	}
}

void SDAControllerApp::keyDown(KeyEvent event)
{
#if defined( CINDER_COCOA )
	bool isModDown = event.isMetaDown();
#else // windows
	bool isModDown = event.isControlDown();
#endif
	CI_LOG_V("main keydown: " + toString(event.getCode()) + " ctrl: " + toString(isModDown));
	if (isModDown) {
	}
	if (!mSDASession->handleKeyDown(event)) {
		switch (event.getCode()) {
		case KeyEvent::KEY_h:
			// mouse cursor and ui visibility
			mSDASettings->mCursorVisible = !mSDASettings->mCursorVisible;
			setUIVisibility(mSDASettings->mCursorVisible);
			break;
	
		default:
			CI_LOG_V("main keydown: " + toString(event.getCode()));
			break;
		}
	}
}
void SDAControllerApp::keyUp(KeyEvent event)
{
	if (!mSDASession->handleKeyUp(event)) {
		switch (event.getCode()) {
		
		default:
			CI_LOG_V("main keyup: " + toString(event.getCode()));
			break;
		}
	}
}

void SDAControllerApp::draw()
{
	gl::clear(Color::black());
	if (mFadeInDelay) {
		mSDASettings->iAlpha = 0.0f;
		if (getElapsedFrames() > mSDASession->getFadeInDelay()) {
			mFadeInDelay = false;
			timeline().apply(&mSDASettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}
	
	// 20190215 gl::setMatricesWindow(toPixels(getWindowSize()), false);
	gl::setMatricesWindow(mSDASettings->mFboWidth, mSDASettings->mFboHeight, false);
	// 20190215 gl::setMatricesWindow(mSDASettings->mRenderWidth, mSDASettings->mRenderHeight, mSDASession->isFlipV());
	
	//Rectf rectangle = Rectf(mSDASettings->mxLeft, mSDASettings->myLeft, mSDASettings->mxRight, mSDASettings->myRight);
	//gl::draw(mSDASession->getMixTexture(), rectangle);
	//gl::drawString("xRight: " + std::to_string(mSDASettings->myRight), vec2(toPixels(400), toPixels(300)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
	// 20190215 gl::setMatricesWindow(mSDASettings->mMainWindowWidth, mSDASettings->mMainWindowHeight, false);
	gl::draw(mSDASession->getMixTexture(), getWindowBounds());
	
	
	// Spout Send
	mSpoutOut.sendViewport();

	// original
	gl::draw(mSDASession->getHydraTexture(), Rectf(0, 0, tWidth, tHeight));
	gl::drawString("Original", vec2(toPixels(0), toPixels(tHeight)), Color(1, 1, 1), Font("Verdana", toPixels(16)));
	// flipH
	gl::draw(mSDASession->getHydraTexture(), Rectf(tWidth * 2 + margin, 0, tWidth + margin, tHeight));
	gl::drawString("FlipH", vec2(toPixels(tWidth + margin), toPixels(tHeight)), Color(1, 1, 1), Font("Verdana", toPixels(16)));
	// flipV
	gl::draw(mSDASession->getHydraTexture(), Rectf(0, tHeight * 2 + margin, tWidth, tHeight + margin));
	gl::drawString("FlipV", vec2(toPixels(0), toPixels(tHeight * 2 + margin)), Color(1, 1, 1), Font("Verdana", toPixels(16)));

	// show the FBO color texture 
	gl::draw(mSDASession->getMixTexture(), Rectf(tWidth + margin, tHeight + margin, tWidth * 2 + margin, tHeight * 2 + margin));
	gl::drawString("Shader", vec2(toPixels(tWidth + margin), toPixels(tHeight * 2 + margin)), Color(1, 1, 1), Font("Verdana", toPixels(16)));



	mSDAUI->Run("UI", (int)getAverageFps());
	if (mSDAUI->isReady()) {
	}
	getWindow()->setTitle(mSDASettings->sFps + " fps SDAController");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(1280, 720);
	//settings->setWindowSize(1920, 1080);
#ifdef _DEBUG
	settings->setConsoleWindowEnabled();
#else
	settings->setConsoleWindowEnabled();
#endif  // _DEBUG
}

CINDER_APP(SDAControllerApp, RendererGl, prepareSettings)
