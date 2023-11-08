#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <d3d11.h>

#include "D3DUtil.h"

//we only ever want one unique texture to be loaded
//it can then be shared between any meshes that need it
class TexCache
{
public:
	//associate a file name with a texture resource
	struct Data
	{
		Data() {}
		/*constructor to setup a texture
		fName - filename
		p - pointer to the loaded texture resource ready to use
		_dim - width and height in pixels
		*/
		Data(const std::string& fName, ID3D11ShaderResourceView*p, const DirectX::SimpleMath::Vector2& _dim)
			: fileName(fName), pTex(p), dim(_dim) 
		{}
		std::string fileName;
		ID3D11ShaderResourceView* pTex = nullptr;
		DirectX::SimpleMath::Vector2 dim;
	};

	//tidy up at the end
	void Release();
	/*if this texture is new load it in, otherwise find it and return a handle
	pDevice - d3d handle
	fileName - texture to load
	texName - optional, a friendly name for the texture the game can later use to refer to it
	appendPath - optional, are we just specifying a file and want the path to the data folder adding 
				or are we specifying the path and file name together
	*/
	ID3D11ShaderResourceView* LoadTexture(ID3D11Device*pDevice, const std::string& fileName, const std::string& texName="", bool appendPath=true);
	//usually we just have specify a texture file name to load, but they're all usually in a sub folder
	//so this needs setting at the start or if that folder changes
	void SetAssetPath(const std::string& path) {
		mAssetPath = path;
	}
	//pull out a texture by nickname = fast
	Data& Get(const std::string& texName) {
		return mCache.at(texName);
	}
	//slowly find a texture by handle
	const Data& Get(ID3D11ShaderResourceView *pTex);

private:
	//get width and height
	DirectX::SimpleMath::Vector2 GetDimensions(ID3D11ShaderResourceView* pTex);
	//array of texture data
	typedef std::unordered_map<std::string, Data> MyMap;
	MyMap mCache;

	//some data sub folder with all the textures in
	std::string mAssetPath;
};


