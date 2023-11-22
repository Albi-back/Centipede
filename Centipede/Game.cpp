#include "Game.h"
#include "WindowUtils.h"
#include "CommonStates.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;



Game::Game(MyD3D& d3d)
	: mPMode(d3d), mD3D(d3d), mpSB(nullptr)
{

	mpSB = new SpriteBatch(&mD3D.GetDeviceCtx());
}


//any memory or resources we made need releasing at the end
void Game::Release()
{
	delete mpSB;
	mpSB = nullptr;
}

//called over and over, use it to update game logic
void Game::Update(float dTime)
{
	
	switch (state)
	{
	case State::PLAY:
		mPMode.Update(dTime);
	}
}

//called over and over, use it to render things
void Game::Render(float dTime)
{
	mD3D.BeginRender(Colours::Black);


	CommonStates dxstate(&mD3D.GetDevice());
	mpSB->Begin(SpriteSortMode_Deferred, dxstate.NonPremultiplied(), &mD3D.GetWrapSampler());

	switch (state)
	{
	case State::PLAY:
		mPMode.Render(dTime, *mpSB);
	}

	mpSB->End();

	mD3D.EndRender();
}


PlayMode::PlayMode(MyD3D & d3d)
	:mD3D(d3d),mPlayer(d3d)
{
	InitBgnd();
	InitPlayer();
}

void PlayMode::Update(float dTime)
{
	int i = 0;
	for (auto& s : mBgnd)
		s.Scroll(-0,dTime * (i++) * SCROLL_SPEED);
}
RECTF rect;
void PlayMode::Render(float dTime, DirectX::SpriteBatch & batch) 
{
	
	for (auto& s : mBgnd)
		s.Draw(batch);
	mPlayer.Draw(batch);
}
	
	
	//Sprite spr(mD3D);
	//Sprite spr2(mD3D);
	//spr.SetTex(*mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), "backgroundlayers/background_1.dds", "bgnd", true));
	//spr.SetScale(Vector2(2,2));
	//spr.Draw(batch);
	//spr.SetTex(*mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), "backgroundlayers/background_2.dds", "bgnd1", true));
	//float scroll = GetClock() * 50.f;
	//rect = RECTF{ 0,-scroll, 512, 512 - scroll };
	//spr.SetTexRect(rect);
	//spr.Draw(batch);
	//spr.SetTex(*mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), "backgroundlayers/background_3.dds", "bgnd2", true));
	//float scroll1 = GetClock() * 100.f;
	//rect = RECTF{ 0,-scroll1, 512, 512 - scroll1 };
	//spr.SetTexRect(rect);
	//spr.Draw(batch);
	//spr.SetTex(*mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), "backgroundlayers/background_4.dds", "bgnd3", true));
	//float scroll2 = GetClock() * 150.f;
	//rect = RECTF{ 0,-scroll2, 512, 512 - scroll2 };
	//spr.SetTexRect(rect);
	//spr.Draw(batch);
	//spr2.SetTex(*mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), "backgroundlayers/spaceship.dds", "player", true));
	//spr2.SetScale(Vector2(2, 2));
	//spr2.mPos=Vector2(224, 756);
	//
	//spr2.Draw(batch);

void PlayMode::InitBgnd()
{
	//a sprite for each layer
	assert(mBgnd.empty());
	mBgnd.insert(mBgnd.begin(), BGND_LAYERS, Sprite(mD3D));


	
	pair<string, string> files[BGND_LAYERS]{
		{ "bgnd0","backgroundlayers/background_1.dds" },
		{ "bgnd1","backgroundlayers/background_2.dds" },
		{ "bgnd2","backgroundlayers/background_3.dds" },
		{ "bgnd3","backgroundlayers/background_4.dds" },
		
		
	};
	int i = 0;
	for (auto& f : files)
	{
		//set each texture layer
		ID3D11ShaderResourceView* p = mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), f.second, f.first);
		if (!p)
			assert(false);
		mBgnd[i++].SetTex(*p);
	}


}
void PlayMode::InitPlayer()
{
	ID3D11ShaderResourceView* p = mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), "spaceship.dds");
	mPlayer.SetTex(*p);
	mPlayer.SetScale(Vector2(3.f, 3.f));
	
	mPlayer.origin = mPlayer.GetTexData().dim / 2.f;
	//mPlayer.rotation = PI / 2.f;
	int w, h;
	WinUtil::Get().GetClientExtents(w, h);
	mPlayArea.left = mPlayer.GetScreenSize().x * 0.6f;
	mPlayArea.top = mPlayer.GetScreenSize().y * 0.6f;
	mPlayArea.right = w - mPlayArea.left;
	mPlayArea.bottom = h * 0.75f;
	mPlayer.mPos = Vector2(mPlayArea.left + mPlayer.GetScreenSize().x * 2.f, (mPlayArea.bottom - mPlayArea.top)*1.3f);
}



