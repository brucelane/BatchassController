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
	bool							mFlipV;
	bool							mFlipH;
	int								xLeft, xRight, yLeft, yRight;
	int								margin, tWidth, tHeight;
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
	mFlipV = false;
	mFlipH = true;
	xLeft = 0;
	xRight = mVDSettings->mRenderWidth;
	yLeft = 0;
	yRight = mVDSettings->mRenderHeight;
	margin = 20;
	tWidth = mVDSettings->mFboWidth / 2;
	tHeight = mVDSettings->mFboHeight / 2;
	mVDSession->setMode(1);
	mRenderWindowTimer = 0.0f;
	//timeline().apply(&mRenderWindowTimer, 1.0f, 2.0f).finishFn([&] { positionRenderWindow(); });

}
void BatchassControllerApp::resizeWindow()
{
	mVDUI->resize();
}
void BatchassControllerApp::positionRenderWindow() {
	mVDSettings->mRenderPosXY = ivec2(mVDSettings->mRenderX, mVDSettings->mRenderY);
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
	gl::setMatricesWindow(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight, true);
	//gl::draw(mVDSession->getMixTexture(), getWindowBounds());
	if (mVDSession->getMode() == 0) gl::draw(mVDSession->getMixetteTexture(), getWindowBounds());
	if (mVDSession->getMode() == 1) gl::draw(mVDSession->getMixTexture(), getWindowBounds());
	if (mVDSession->getMode() == 2) gl::draw(mVDSession->getRenderTexture(), getWindowBounds());
	if (mVDSession->getMode() == 3) gl::draw(mVDSession->getHydraTexture(), getWindowBounds());
	if (mVDSession->getMode() == 4) gl::draw(mVDSession->getFboTexture(0), getWindowBounds());
	if (mVDSession->getMode() == 5) gl::draw(mVDSession->getFboTexture(1), getWindowBounds());
	if (mVDSession->getMode() == 6) gl::draw(mVDSession->getFboTexture(2), getWindowBounds());
	if (mVDSession->getMode() == 7) gl::draw(mVDSession->getFboTexture(3), getWindowBounds());
	if (mVDSession->getMode() == 8) gl::draw(mVDSession->getFboTexture(4), getWindowBounds());
	// Spout Send
	mSpoutOut.sendViewport();

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
	getWindow()->setTitle(mVDSettings->sFps + " Ctrl");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(1280, 720);
}

CINDER_APP(BatchassControllerApp, RendererGl, prepareSettings)
