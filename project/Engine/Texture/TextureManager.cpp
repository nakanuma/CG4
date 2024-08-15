#include "TextureManager.h"
#include <cassert>

void TextureManager::Initialize(ID3D12Device* device, SRVManager* srvManager)
{
	/*GetInstance().srvHeap_.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);*/
	GetInstance().srvManager = srvManager;
}

int TextureManager::Load(const std::string& filePath, ID3D12Device* device)
{
	// 読み込み済みテクスチャを検索
	auto& instance = GetInstance();
	if (instance.textureDatas.contains(filePath)) {
		// テクスチャが既に読み込まれている場合、その srvIndex を返す
		return instance.textureDatas[filePath].srvIndex;
	}

	// テクスチャ枚数上限チェック
	assert(instance.srvManager->Allocate());

	// Textureを読んで転送する
	DirectX::ScratchImage mipImages = GetInstance().LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	// メタデータの配列に保存
	GetInstance().texMetadata[GetInstance().srvManager->GetIndex()] = mipImages.GetMetadata();

	// リソースの配列に保存
	Microsoft::WRL::ComPtr<ID3D12Resource>& targetResource = GetInstance().texResources[GetInstance().srvManager->GetIndex()];
	targetResource = TextureManager::CreateTextureResource(device, metadata);

	TextureManager::GetInstance().UploadTextureData(targetResource.Get(), mipImages);

	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	// metadataからcubemapかどうかを取得して分岐する
	if (metadata.IsCubemap()) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0; // unionがTextureCubeになったが、内部パラメータの意味はTexture2dと変わらない
		srvDesc.TextureCube.MipLevels = UINT_MAX;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	} else { // 2Dテクスチャの設定を行う
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; 
		srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	}

	// テクスチャデータを追加して書き込む
	TextureData& textureData = GetInstance().textureDatas[filePath];
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = targetResource;
	textureData.srvIndex = GetInstance().srvManager->GetIndex();
	textureData.srvHandleCPU = GetInstance().srvManager->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = GetInstance().srvManager->GetGPUDescriptorHandle(textureData.srvIndex);

	// SRVの生成
	device->CreateShaderResourceView(targetResource.Get(), &srvDesc, GetInstance().srvManager->GetCPUDescriptorHandle(GetInstance().srvManager->GetIndex()));
	// 実際に返すのはインクリメントする前の値なので1引いて返す
	return GetInstance().srvManager->Allocate();
}

TextureManager& TextureManager::GetInstance()
{
	static TextureManager instance;

	return instance;
}

void TextureManager::SetDescriptorTable(UINT rootParamIndex, ID3D12GraphicsCommandList* commandList, uint32_t textureHandle)
{
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex, GetInstance().srvManager->descriptorHeap.GetGPUHandle(textureHandle));
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureHandle)
{
	return GetInstance().texMetadata[textureHandle];
}

DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filePath)
{
	HRESULT result = S_FALSE;

	// テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	if (filePathW.ends_with(L".dds")) { // .ddsで終わっていたらddsとみなす
		result = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else { // .dds以外のファイルの場合
		result = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(result));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	if (DirectX::IsCompressed(image.GetMetadata().format)) { // 圧縮フォーマットかどうかを調べる
		mipImages = std::move(image); // 圧縮フォーマットならそのまま使うのでmoveする
	} else {
		result = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImages);
	}
	assert(SUCCEEDED(result));

	// ミップマップ付きのデータを返す
	return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata)
{
	HRESULT result = S_FALSE;

	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Textureの幅
	resourceDesc.Height = UINT(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行き or 配列Textureの配列数
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM; // 細かい設定を行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; // プロセッサの近くに配置

	// Resourceを生成する
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	result = device->CreateCommittedResource(
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定
		&resourceDesc, // Resourceの設定
		D3D12_RESOURCE_STATE_GENERIC_READ, // 初回のResourceState
		nullptr, // Clear最適値
		IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(result));

	return std::move(resource);
}

void TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages)
{
	HRESULT result = S_FALSE;

	// Meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	// 全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		// MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		// Textureに転送
		result = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr, // 全領域へコピー
			img->pixels, // 元データアドレス
			UINT(img->rowPitch), // 1ラインサイズ
			UINT(img->slicePitch) // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}
}
