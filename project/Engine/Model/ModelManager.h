#pragma once
#include <vector>
#include <string>
#include <d3d12.h>
#include <map>

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

	///
	/// Animation
	/// 
	
	struct KeyframeFloat3 {
		Float3 value; // キーフレームの値
		float time; // キーフレームの時刻
	};
	struct KeyframeQuaternion {
		Quaternion value; // キーフレームの値
		float time; // キーフレームの時刻
	};

	struct NodeAnimation {
		std::vector<KeyframeFloat3> translate;
		std::vector<KeyframeQuaternion> rotate;
		std::vector<KeyframeFloat3> scale;
	};

	struct Animation {
		float duration; // アニメーション全体の尺（単位は秒）
		// NodeAnimationの集合。Node名でひけるようにしておく
		std::map<std::string, NodeAnimation> nodeAnimations;
	};

	// Objファイルの読み込みを行う
	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename, ID3D12Device* device);
	// mtlファイルの読み込みを行う
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, ID3D12Device* device);
	// assimpのNodeから、Node構造体に変換
	static Node ReadNode(aiNode* node);

	// Animation読み込み
	static Animation LoadAnimation(const std::string& directoryPath, const std::string& filename);
	// 任意の時刻の値を取得する
	static Float3 CalculateValue(const std::vector<KeyframeFloat3>& keyframes, float time);
	static Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);
};

