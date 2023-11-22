#pragma once

#include <vector>

#include "D3D.h"
#include "SpriteBatch.h"
#include "Sprite.h"


class PlayMode
{
public:
	PlayMode(MyD3D& d3d);
	void Update(float dTime);
	void Render(float dTime, DirectX::SpriteBatch& batch);
private:
	const float SCROLL_SPEED = 10.f;
	static const int BGND_LAYERS = 4;
	MyD3D& mD3D;
	Sprite mPlayer;
	RECTF mPlayArea;
	std::vector<Sprite> mBgnd; //paralax layers
	
	void InitBgnd();
	void InitPlayer();
	void Render1(float dTime, DirectX::SpriteBatch& batch);
};


/*
Basic wrapper for a game
*/
class Game
{
public:
	enum class State { PLAY };
	State state = State::PLAY;
	Game(MyD3D& d3d);


	void Release();
	void Update(float dTime);
	void Render(float dTime);
private:
	MyD3D& mD3D;
	DirectX::SpriteBatch *mpSB = nullptr;
	//not much of a game, but this is it
	PlayMode mPMode;
};


