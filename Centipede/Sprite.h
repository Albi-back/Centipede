#pragma once
#include "SpriteBatch.h"
#include "D3D.h"

//handy rectangle definer
struct RECTF
{
	float left, top, right, bottom;
	operator RECT() {
		return RECT{ (int)left,(int)top,(int)right,(int)bottom };
	}
};

/*
Wrap up sprite rendering and movement
*/
class Sprite
{
private:
	ID3D11ShaderResourceView *mpTex;
	MyD3D& mD3D;
	RECTF mTexRect;
	DirectX::SimpleMath::Vector2 scale;
	const TexCache::Data *mpTexData;

public:
	DirectX::SimpleMath::Vector2 mPos;
	DirectX::SimpleMath::Vector2 mVel;
	float depth;
	DirectX::SimpleMath::Vector4 colour;
	float rotation;
	DirectX::SimpleMath::Vector2 origin;

	Sprite(MyD3D& d3d)
		:mPos(0, 0), mVel(0, 0),
		depth(0), mTexRect{ 0,0,0,0 }, colour(1, 1, 1, 1),
		rotation(0), scale(2.f, 4.5f), origin(0, 0), mpTex(nullptr),
		mD3D(d3d), mpTexData(nullptr)
	{}
	Sprite(const Sprite& rhs)
		:mD3D(rhs.mD3D)
	{
		(*this) = rhs;
	}
	Sprite& operator=(const Sprite& rhs);
	
	//sprite is drawn using batch parameter
	void Draw(DirectX::SpriteBatch& batch);
	//change texture, optional rectf can isolate part of the texture
	void SetTex(ID3D11ShaderResourceView& tex, const RECTF& texRect = RECTF{ 0,0,0,0 });
	//change which part later
	void SetTexRect(const RECTF& texRect);
	void Scroll(float x, float y);
	//getters
	const TexCache::Data& GetTexData() const {
		assert(mpTexData);
		return *mpTexData;
	}
	ID3D11ShaderResourceView& GetTex() {
		assert(mpTex);
		return *mpTex;
	}
	void SetScale(const DirectX::SimpleMath::Vector2& s) {
		scale = s;
	}
	const DirectX::SimpleMath::Vector2& GetScale() const {
		return scale;
	}
	DirectX::SimpleMath::Vector2 GetScreenSize() const {
		assert(mpTexData);
		return scale * mpTexData->dim;
	}
};


