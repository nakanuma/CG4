#pragma once
#include "BaseScene.h"
#include "Camera.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "ModelManager.h"
#include "Object3D.h"
#include "SoundManager.h"

// ゲームプレイシーン
class GamePlayScene : public BaseScene
{
public:
	// 初期化
	void Initialize() override;

	// 終了
	void Finalize() override;

	// 毎フレーム更新
	void Update() override;

	// 描画
	void Draw() override;

private:
	Camera* camera = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	SoundManager* soundManager = nullptr;

	///
	/// ↓ ゲームシーン用
	///

	// モデルデータ
	ModelManager::ModelData model_;
	ModelManager::ModelData sphereModel_;
	// アニメーション
	ModelManager::Animation animation_;
	// スケルトン
	ModelManager::Skeleton skeleton_;
	// スキンクラスター
	ModelManager::SkinCluster skinCluster_;
	// アニメーション時間
	float animationTime = 0.0f;
	// 3Dオブジェクト
	Object3D* object_;

	// 球を描画するための配列
	std::vector<Object3D*> jointSpheres_;

	// 音声データ
	SoundManager::SoundData soundData_;
};

