#include <testsuites/testsuite.h>

class PatchBlitTests : public TestSuite
{
public:
	PatchBlitTests()
	{
		name = "PatchBlitTests";

		addTest("Blit", &PatchBlitTests::setSplash, &PatchBlitTests::blit, &PatchBlitTests::dummy);
		addTest("BlitAlphaOnly", &PatchBlitTests::setAlphaOnly, &PatchBlitTests::blit, &PatchBlitTests::dummy);

		addTest("FlipBlit90", &PatchBlitTests::setSplash, &PatchBlitTests::flipBlit90, &PatchBlitTests::dummy);
		addTest("FlipBlit180", &PatchBlitTests::setSplash, &PatchBlitTests::flipBlit180, &PatchBlitTests::dummy);
		addTest("FlipBlit270", &PatchBlitTests::setSplash, &PatchBlitTests::flipBlit270, &PatchBlitTests::dummy);

		addTest("StretchBlit", &PatchBlitTests::setSplash, &PatchBlitTests::stretchBlit, &PatchBlitTests::dummy);

		addTest("RotScaleBlit", &PatchBlitTests::setClockFace, &PatchBlitTests::rotScaleBlit, &PatchBlitTests::dummy);
	}

	bool init(GfxDevice * pDevice, const Rect& canvas)
	{
		m_pClockFace = FileUtil::loadSurface("../resources/clockface_2500.png", pDevice->surfaceFactory());
		if (!m_pClockFace)
			return false;
		m_pClockFace->setScaleMode(ScaleMode::Interpolate);

		m_pSplash = FileUtil::loadSurface("../resources/splash.png", pDevice->surfaceFactory());
		if (!m_pSplash)
			return false;

		m_pAlphaOnly = pDevice->surfaceFactory()->createSurface({ 400,400 }, PixelFormat::A8);

		Rect	copySource = Rect(m_pClockFace->size()).center({ 400,400 });
		m_pAlphaOnly->copyFrom(m_pClockFace, copySource, { 0,0 });

		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const Rect& canvas)
	{
		m_pClockFace = nullptr;
		m_pSplash = nullptr;
		return true;
	}


	bool dummy(GfxDevice * pDevice, const Rect& canvas)
	{
		return true;
	}

	bool setClockFace(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->setBlitSource(m_pClockFace);
		return true;
	}

	bool setSplash(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->setBlitSource(m_pSplash);
		return true;
	}

	bool setAlphaOnly(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->setBlitSource(m_pAlphaOnly);
		return true;
	}


	bool	blit(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->blit(canvas);
		return true;
	}

	bool	flipBlit90(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->flipBlit(canvas, GfxFlip::Rot90);
		return true;
	}

	bool	flipBlit180(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->flipBlit(canvas, GfxFlip::Rot180);
		return true;
	}

	bool	flipBlit270(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->flipBlit(canvas, GfxFlip::Rot270);
		return true;
	}


	bool	rotScaleBlit(GfxDevice * pDevice, const Rect& canvas)
	{
		CoordF center = { m_pClockFace->size().w / 2.f, m_pClockFace->size().h / 2.f };
		pDevice->rotScaleBlit(canvas, center, rot, scale);
		return true;
	}

	bool	stretchBlit(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->stretchBlit(canvas, Rect( 0,0, m_pSplash->size() ) );
		return true;
	}

private:
	Surface_p	m_pClockFace;
	Surface_p	m_pSplash;
	Surface_p	m_pAlphaOnly;

	float rot = 35;
	float scale = 1.3f;
};