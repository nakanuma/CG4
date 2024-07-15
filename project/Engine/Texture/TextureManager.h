#pragma once
#include <d3d12.h>
#include "externals/DirectXTex/DirectXTex.h"
#include "StringUtil.h"
#include "Logger.h"
#include "DescriptorHeap.h"
#include <array>

class TextureManager final
{
public:
	static void Initialize(ID3D12Device* device);

	static int Load(const std::string& filePath, ID3D12Device* device);

	static TextureManager& GetInstance();

	static void SetDescriptorTable(UINT rootParamIndex, ID3D12GraphicsCommandList* commandList, uint32_t textureHandle);

	static const DirectX::TexMetadata& GetMetaData(uint32_t textureHandle);

	DescriptorHeap srvHeap_;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;
private:
	// TextureデータをCPUで読む
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);
	// DirectX12のTextureResourceを作る
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);
	// TextureResourceにデータを転送する
	static void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

	static const uint32_t kMaxTextureValue_ = 128;
	uint32_t index_ = 1;

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kMaxTextureValue_> texResources;

	std::array<DirectX::TexMetadata, kMaxTextureValue_> texMetadata;
};

