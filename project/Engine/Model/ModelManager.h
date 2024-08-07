#pragma once
#include <vector>
#include <string>
#include <d3d12.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// MyClass
#include "MyMath.h"
#include "TextureManager.h"

class ModelManager
{
public:
	struct VertexData {
		Float4 position;
		Float2 texcoord;
		Float3 normal;
	};

	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureHandle;
	};

	struct Node {
		Matrix localMatrix;
		std::string name;
		std::vector<Node> children;
	};

	struct ModelData {
		std::vector<VertexData> vertices;
		MaterialData material;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		Node rootNode;
	};

	// Objファイルの読み込みを行う
	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename, ID3D12Device* device);
	// mtlファイルの読み込みを行う
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, ID3D12Device* device);
	// assimpのNodeから、Node構造体に変換
	static Node ReadNode(aiNode* node);
};

