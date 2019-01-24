#include <testsuites/testsuite.h>

class SegmentTests : public TestSuite
{
public:
	SegmentTests()
	{
		name = "SegmentTests";

		addTest("RainbowSpread", &SegmentTests::rainbowSpread );
		addTest("Flip90Rainbow", &SegmentTests::flip90Rainbow);
		addTest("Flip180Rainbow", &SegmentTests::flip180Rainbow);
		addTest("Flip270Rainbow", &SegmentTests::flip270Rainbow);
		addTest("WildRainbow", &SegmentTests::wildRainbow);
		addTest("Circles", &SegmentTests::circles);
		addTest("Elipses", &SegmentTests::elipses);
		addTest("LargeCircle", &SegmentTests::largeCircle);
		addTest("BrokenElipse", &SegmentTests::brokenElipse);

		addTest("SegBlendReplace", &SegmentTests::setBlendModeReplace, &SegmentTests::rainbowSpread, &SegmentTests::resetBlendMode);
		addTest("SegBlendAdd", &SegmentTests::setBlendModeAdd, &SegmentTests::rainbowSpread, &SegmentTests::resetBlendMode);
		addTest("SegBlendSubtract", &SegmentTests::setBlendModeSubtract, &SegmentTests::rainbowSpread, &SegmentTests::resetBlendMode);
		addTest("SegBlendMultiply", &SegmentTests::setBlendModeMultiply, &SegmentTests::rainbowSpread, &SegmentTests::resetBlendMode);
		addTest("SegBlendInvert", &SegmentTests::setBlendModeInvert, &SegmentTests::rainbowSpread, &SegmentTests::resetBlendMode);

	}


	bool init(GfxDevice * pDevice, const Rect& canvas)
	{
		// Generate rainbow segments

		for (int i = 0; i <= canvas.w; i++)
		{
			m_rainbowEdges[i][0] = i * 32;
			m_rainbowEdges[i][1] = i * 64;
			m_rainbowEdges[i][2] = i * 96;
			m_rainbowEdges[i][3] = i * 128;
			m_rainbowEdges[i][4] = i * 160;
			m_rainbowEdges[i][5] = i * 192;
			m_rainbowEdges[i][6] = i * 224;
		}

		return true;
	}


	bool	resetBlendMode(GfxDevice * pDevice, const Rect& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Blend);
	}

	bool	setBlendModeReplace(GfxDevice * pDevice, const Rect& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Replace);
	}

	bool	setBlendModeAdd(GfxDevice * pDevice, const Rect& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Add);
	}

	bool	setBlendModeSubtract(GfxDevice * pDevice, const Rect& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Subtract);
	}

	bool	setBlendModeMultiply(GfxDevice * pDevice, const Rect& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Multiply);
	}

	bool	setBlendModeInvert(GfxDevice * pDevice, const Rect& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Invert);
	}

	bool rainbowSpread(GfxDevice * pDevice, const Rect& canvas)
	{

		pDevice->drawSegments(canvas, 8, m_rainbowColors, m_nRainbowEdges, &m_rainbowEdges[0][0], 7);

		return true;
	}

	bool wildRainbow(GfxDevice * pDevice, const Rect& canvas)
	{
		Color colors[8] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue, Color::Indigo, Color::Violet, { 0,0,0,255 } };

		int edges[512 + 1][7];
		for (int i = 0; i <= canvas.w; i++)
		{

			if (i % 2 == 0)
			{
				for (int j = 0; j < 7; j++)
					edges[i][j] = 1024 + 1024 * j;
			}
			else
			{
				for (int j = 0; j < 7; j++)
					edges[i][j] = (canvas.h*256) + 1024 * j;
			}
		}

		pDevice->drawSegments(canvas, 8, colors, m_nRainbowEdges, &edges[0][0], 7);

		return true;
	}

	bool largeCircle(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->drawElipse(canvas, 20.f, Color::Aquamarine, 2.f, Color::White);
		return true;
	}

	bool circles(GfxDevice * pDevice, const Rect& canvas)
	{
		for (int y = 0; y < 10; y++)
		{
			for (int x = 0; x < 12; x++)
			{
				pDevice->drawElipse({ x*32.1f,y*26+y*y*1.5f+x*0.11f,y*15.01f+x*2.05f,y*15.01f+x*2.05f }, 4.f, Color::Aquamarine, 1.f, Color::White);
			}
		}

		return true;
	}

	bool elipses(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->drawElipse({ 0.f,0.f,canvas.w*3/4.f, canvas.w*3/4/3.f}, 20.f, Color::Aquamarine, 5.f, Color::White);
		pDevice->drawElipse({ canvas.w*3/4.f,0.f,canvas.w/4.f, canvas.h*1.f }, 20.f, Color::Aquamarine, 5.f, Color::White);

		return true;
	}


	bool brokenElipse(GfxDevice * pDevice, const Rect& canvas)
	{
		int x = 0;
		int y = 1;

		pDevice->drawElipse({ x*30.1f,y * 20 + y * y*1.5f + x * 0.11f,y*15.01f + x * 2.05f,y*10.01f + x * 2.05f }, 4.f, Color::Aquamarine, 1.f, Color::White);

		return true;
	}


	bool rainbow(GfxDevice * pDevice, const Rect& canvas)
	{
		int transform[2][2] = { 1,0,0,1 };

		pDevice->transformDrawSegments(canvas, 8, m_rainbowColors, m_nRainbowEdges, (int*)m_rainbowEdges, 7, transform);
		return true;
	}


	bool flip90Rainbow(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->flipDrawSegments(canvas, 8, m_rainbowColors, m_nRainbowEdges, (int*)m_rainbowEdges, 7, GfxFlip::Rot90);
		return true;
	}

	bool flip180Rainbow(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->flipDrawSegments(canvas, 8, m_rainbowColors, m_nRainbowEdges, (int*)m_rainbowEdges, 7, GfxFlip::Rot180);
		return true;
	}

	bool flip270Rainbow(GfxDevice * pDevice, const Rect& canvas)
	{
		pDevice->flipDrawSegments(canvas, 8, m_rainbowColors, m_nRainbowEdges, (int*)m_rainbowEdges, 7, GfxFlip::Rot270);
		return true;
	}


private:

	Color	m_rainbowColors[8] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue, Color::Indigo, Color::Violet, { 255,255,255,128 } };

	int		m_nRainbowEdges = 512 + 1;
	int		m_rainbowEdges[512 + 1][7];

};
 