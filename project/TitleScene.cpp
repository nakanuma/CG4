#include "TitleScene.h" 
#include "ImguiWrapper.h"
#include "DirectXBase.h"
#include "SRVManager.h"
#include "SpriteCommon.h"
#include "SceneManager.h"
//#include "GamePlayScene.h"

void TitleScene::Initialize()
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

	// Inputの初期化
	input = Input::GetInstance();

	///
	///	↓ ゲームシーン用
	///	

	// Texture読み込み
	uint32_t titleGH = TextureManager::Load("resources/Images/title.png", dxBase->GetDevice());

	// スプライトの生成と初期化
	sprite_ = new Sprite();
	sprite_->Initialize(spriteCommon, titleGH);
	sprite_->SetSize({ 500.0f, 500.0f });

	// モデル読み込み
	model_ = ModelManager::LoadModelFile("resources/Models", "plane.obj", dxBase->GetDevice());

	// 3Dオブジェクトの生成とモデル指定
	object_ = new Object3D();
	object_->model_ = &model_;
	object_->transform_.rotate = { 0.0f, 3.14f, 0.0f };
}

void TitleScene::Finalize()
{
	// SoundManager開放
	delete soundManager;

	// 3Dオブジェクト開放
	delete object_;

	// Sprite開放
	delete sprite_;

	// SpriteCommon開放
	delete spriteCommon;

	// カメラの開放
	delete camera;
}

void TitleScene::Update()
{
	// スプライトの更新
	sprite_->Update();

	// 3Dオブジェクトの更新
	object_->UpdateMatrix();
	object_->transform_.rotate.y += 0.001f;

	///
	///	シーン切り替え
	/// 

	// ENTERキーを押したら
	if (input->TriggerKey(DIK_RETURN)) {
		//// ゲームプレイシーン（次シーンを生成）
		//BaseScene* scene = new GamePlayScene();
		//// シーン切り替え依頼
		//SceneManager::GetInstance()->SetNextScene(scene);

		// シーン切り替え
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}
}

void TitleScene::Draw()
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

	object_->Draw();

	///
	///	↑ ここまで3Dオブジェクトの描画コマンド
	/// 

	// Spriteの描画準備。全ての描画に共通のグラフィックスコマンドを積む
	spriteCommon->PreDraw();

	///
	/// ↓ ここからスプライトの描画コマンド
	/// 

	// スプライトの描画
	sprite_->Draw();

	///
	/// ↑ ここまでスプライトの描画コマンド
	/// 

	ImGui::Begin("window");

	ImGui::Text("Trigger ENTER key to GamePlayScene");

	ImGui::End();

	// ImGuiの内部コマンドを生成する
	ImguiWrapper::Render(dxBase->GetCommandList());
	// 描画後処理
	dxBase->PostDraw();
	// フレーム終了処理
	dxBase->EndFrame();
}
