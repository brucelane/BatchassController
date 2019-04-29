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
// UI
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include "VDUI.h"
#define IM_ARRAYSIZE(_ARR)			((int)(sizeof(_ARR)/sizeof(*_ARR)))

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace videodromm;

class BatchassControllerApp : public App {

public:
	BatchassControllerApp();
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
};


BatchassControllerApp::BatchassControllerApp()
	: mSpoutOut("BatchassController", app::getWindowSize())
{
	// Settings
	mVDSettings = VDSettings::create("BatchassController");
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
	mRenderWindowTimer = 0.0f;
	//timeline().apply(&mRenderWindowTimer, 1.0f, 2.0f).finishFn([&] { positionRenderWindow(); });

}
void BatchassControllerApp::resizeWindow()
{
	mVDUI->resize();
}
void BatchassControllerApp::positionRenderWindow() {
	mVDSettings->mRenderPosXY = ivec2(mVDSettings->mRenderX, mVDSettings->mRenderY);//20141214 was 0
	setWindowPos(mVDSettings->mRenderX, mVDSettings->mRenderY);
	setWindowSize(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight);
}
void BatchassControllerApp::setUIVisibility(bool visible)
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
void BatchassControllerApp::fileDrop(FileDropEvent event)
{
	mVDSession->fileDrop(event);
}
void BatchassControllerApp::update()
{
	mVDSession->setFloatUniformValueByIndex(mVDSettings->IFPS, getAverageFps());
	mVDSession->update();
}
void BatchassControllerApp::cleanup()
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
void BatchassControllerApp::mouseMove(MouseEvent event)
{
	if (!mVDSession->handleMouseMove(event)) {
		// let your application perform its mouseMove handling here
	}
}
void BatchassControllerApp::mouseDown(MouseEvent event)
{
	if (!mVDSession->handleMouseDown(event)) {
		// let your application perform its mouseDown handling here
		if (event.isRightDown()) { 
		}
	}
}
void BatchassControllerApp::mouseDrag(MouseEvent event)
{
	if (!mVDSession->handleMouseDrag(event)) {
		// let your application perform its mouseDrag handling here
	}	
}
void BatchassControllerApp::mouseUp(MouseEvent event)
{
	if (!mVDSession->handleMouseUp(event)) {
		// let your application perform its mouseUp handling here
	}
}

void BatchassControllerApp::keyDown(KeyEvent event)
{
	if (!mVDSession->handleKeyDown(event)) {
		switch (event.getCode()) {
		case KeyEvent::KEY_ESCAPE:
			// quit the application
			quit();
			break;
		case KeyEvent::KEY_h:
			// mouse cursor and ui visibility
			mVDSettings->mCursorVisible = !mVDSettings->mCursorVisible;
			setUIVisibility(mVDSettings->mCursorVisible);
			break;
		}
	}
}
void BatchassControllerApp::keyUp(KeyEvent event)
{
	if (!mVDSession->handleKeyUp(event)) {
	}
}

void BatchassControllerApp::draw()
{
	gl::clear(Color::black());
	if (mFadeInDelay) {
		mVDSettings->iAlpha = 0.0f;
		if (getElapsedFrames() > mVDSession->getFadeInDelay()) {
			mFadeInDelay = false;
			timeline().apply(&mVDSettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}

	//gl::setMatricesWindow(toPixels(getWindowSize()),false);
	gl::setMatricesWindow(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight, false);
	gl::draw(mVDSession->getMixTexture(), getWindowBounds());

	// Spout Send
	mSpoutOut.sendViewport();
	mVDUI->Run("UI", (int)getAverageFps());
	if (mVDUI->isReady()) {
	}
	getWindow()->setTitle(mVDSettings->sFps + " fps VDUI");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(1280, 720);
}

CINDER_APP(BatchassControllerApp, RendererGl, prepareSettings)
