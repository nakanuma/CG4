#include "GamePlayScene.h"
#include "ImguiWrapper.h"
#include "DirectXBase.h"
#include "SRVManager.h"
#include "SpriteCommon.h"

void GamePlayScene::Initialize()
{
	DirectXBase* dxBase = DirectXBase::GetInstance();

	// カメラのインスタンスを生成
	camera = new Camera({ 0.0f, 0.0f, -10.0f }, { 0.0f, 0.0f, 0.0f }, 0.45f);
	Camera::Set(camera); // 現在のカメラをセット

	// SpriteCommonの生成と初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(DirectXBase::GetInstance());

	// TextureManagerの初期化
	TextureManager::Initialize(dxBase->GetDevice(), SRVManager::GetInstance());

	// SoundManagerの初期化
	soundManager = new SoundManager();
	soundManager->Initialize();

	///
	///	↓ ゲームシーン用
	///	
	
	// Texture読み込み
	uint32_t uvCheckerGH = TextureManager::Load("resources/Images/uvChecker.png", dxBase->GetDevice());
	
	// モデル読み込み
	model_ = ModelManager::LoadModelFile("resources/Models", "plane.gltf", dxBase->GetDevice());
	model_.material.textureHandle = uvCheckerGH;

	// 3Dオブジェクトの生成とモデル指定
	object_ = new Object3D();
	object_->model_ = &model_;
	object_->transform_.rotate = { 0.0f, 3.14f, 0.0f };

	// 音声読み込み
	soundData_ = soundManager->LoadWave("resources/Sounds/yay.wav");
}

void GamePlayScene::Finalize()
{
	// 音声データ開放
	soundManager->Unload(&soundData_);
	// SoundManager開放
	delete soundManager;

	// 3Dオブジェクト開放
	delete object_;

	// SpriteCommon開放
	delete spriteCommon;

	// カメラの開放
	delete camera;
}

void GamePlayScene::Update()
{
	// 3Dオブジェクトの更新
	object_->UpdateMatrix();
	/*object_->transform_.rotate.y += 0.001f;*/
}

void GamePlayScene::Draw()
{
	DirectXBase* dxBase = DirectXBase::GetInstance();
	SRVManager* srvManager = SRVManager::GetInstance();

	// 描画前処理
	dxBase->PreDraw();
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvManager->descriptorHeap.heap_.Get() };
	dxBase->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
	// ImGuiのフレーム開始処理
	ImguiWrapper::NewFrame();
	// カメラの定数バッファを設定
	Camera::TransferConstantBuffer();

	///
	///	↓ ここから3Dオブジェクトの描画コマンド
	/// 

	// オブジェクトの行列
	Matrix worldMatrix = object_->transform_.MakeAffineMatrix();
	Matrix viewMatrix = Camera::GetCurrent()->MakeViewMatrix();
	Matrix projectionMatrix = Camera::GetCurrent()->MakePerspectiveFovMatrix();
	// RootのMatrixを適用
	object_->wvpCB_.data_->WVP = model_.rootNode.localMatrix * worldMatrix * viewMatrix * projectionMatrix;
	object_->wvpCB_.data_->World = model_.rootNode.localMatrix * worldMatrix;

	// 3Dオブジェクト描画
	object_->Draw();

	///
	///	↑ ここまで3Dオブジェクトの描画コマンド
	/// 

	// Spriteの描画準備。全ての描画に共通のグラフィックスコマンドを積む
	spriteCommon->PreDraw();

	///
	/// ↓ ここからスプライトの描画コマンド
	/// 

	///
	/// ↑ ここまでスプライトの描画コマンド
	/// 

	ImGui::Begin("window");
	ImGui::DragFloat3("translate", &object_->transform_.translate.x, 0.01f);
	ImGui::DragFloat3("rotate", &object_->transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("scale", &object_->transform_.scale.x, 0.01f);
	ImGui::End();

	// ImGuiの内部コマンドを生成する
	ImguiWrapper::Render(dxBase->GetCommandList());
	// 描画後処理
	dxBase->PostDraw();
	// フレーム終了処理
	dxBase->EndFrame();
}
