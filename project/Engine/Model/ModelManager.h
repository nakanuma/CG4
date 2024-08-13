#pragma once
#include <vector>
#include <string>
#include <d3d12.h>
#include <map>
#include <optional>
#include <span>

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
		QuaternionTransform transform;
		Matrix localMatrix;
		std::string name;
		std::vector<Node> children;
	};

	struct VertexWeightData {
		float weight;
		uint32_t vertexIndex;
	};

	struct JointWeightData {
		Matrix inverseBindPoseMatrix;
		std::vector<VertexWeightData> vertexWeights;
	};

	struct ModelData {
		std::map<std::string, JointWeightData> skinClusterData;
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
		MaterialData material;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
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


	///
	///	Skeleton
	/// 
	
	struct Joint {
		QuaternionTransform transform; // Transform情報
		Matrix localMatrix; // localMatrix
		Matrix skeletonSpaceMatrix; // sletetonSpaceでの変換行列
		std::string name; // 名前
		std::vector<int32_t> children; // 子JointのIndexのリスト。いなければ空
		int32_t index; // 自身のIndex
		std::optional<int32_t> parent; // 親JointのIndex。いなければnull
	};

	struct Skeleton {
		int32_t root; // RootJointのIndex
		std::map<std::string, int32_t> jointMap; // Joint名とIndexとの辞書
		std::vector<Joint> joints; // 所属しているジョイント
	};

	///
	/// Skinning
	///

	static const uint32_t kNumMaxInfluence = 4; // 最大4Jointの影響を受ける
	struct VertexInfluence {
		std::array<float, kNumMaxInfluence> weights;
		std::array<int32_t, kNumMaxInfluence> jointIndices;
	};

	struct WellForGPU {
		Matrix skeletonSpaceMatrix; // 位置用
		Matrix skeletonSpaceInverseTransposeMatrix; // 法線用
	};

	struct SkinCluster {
		std::vector<Matrix> inverseBindPoseMatrices;
		Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
		D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
		std::span<VertexInfluence> mappedInfluence;
		Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
		std::span<WellForGPU> mappedPalette;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
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


	// ModelDataのNodeの階層構造からSkeletonを作成
	static Skeleton CreateSkeleton(const Node& rootNode);
	// NodeからJointを作成
	static int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);
	// Skeletonの更新
	static void Update(Skeleton& skeleton);
	// Skeletonに対してAnimationの適用
	static void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);

	// SkinClusterの生成
	static SkinCluster CreateSkinCluster(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Skeleton& skeleton, const ModelData& modelData);
	// SkinClusterの更新
	static void Update(SkinCluster& skinCluster, const Skeleton& skeleton);
};

