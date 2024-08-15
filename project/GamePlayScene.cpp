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
	uint32_t whiteGH = TextureManager::Load("resources/Images/white.png", dxBase->GetDevice());

	cubeMapDDS_ = TextureManager::Load("resources/Images/StandardCubeMap.dds", dxBase->GetDevice());
	
	// モデル読み込み
	model_ = ModelManager::LoadModelFile("resources/Models/human", "sneakWalk.gltf", dxBase->GetDevice());
	model_.material.textureHandle = whiteGH;

	sphereModel_ = ModelManager::LoadModelFile("resources/Models", "sphere.obj", dxBase->GetDevice());
	sphereModel_.material.textureHandle = whiteGH;

	// アニメーション読み込み
	animation_ = ModelManager::LoadAnimation("resources/Models/human", "sneakWalk.gltf");
	// スケルトン作成
	skeleton_ = ModelManager::CreateSkeleton(model_.rootNode);
	// スキンクラスター作成
	skinCluster_ = ModelManager::CreateSkinCluster(dxBase->GetDevice(), skeleton_, model_);

	// 3Dオブジェクトの生成とモデル指定
	object_ = new Object3D();
	object_->model_ = &model_;
	object_->transform_.rotate = { 0.0f, 3.14f, 0.0f };
	object_->transform_.translate = { 0.0f, -1.0f, -4.0f };

	// Jointの数に応じて球のObject3Dを作成し、配列に追加
	for (size_t i = 0; i < skeleton_.joints.size(); ++i) {
		Object3D* sphere = new Object3D();
		sphere->model_ = &sphereModel_;
		sphere->transform_.scale = { 1.f, 1.f, 1.f }; // サイズを指定
		jointSpheres_.push_back(sphere);
	} 

	///
	/// SkyBox用データの設定
	/// 
	
	// 頂点リソース作成
	vertexResource_ = CreateBufferResource(dxBase->GetDevice(), sizeof(ModelManager::VertexData) * vertexNum_);

	// 頂点バッファビュー作成
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(ModelManager::VertexData) * vertexNum_;
	vertexBufferView_.StrideInBytes = sizeof(ModelManager::VertexData);

	// リソースのアドレスを取得してデータを書き込む
	ModelManager::VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 右面。描画インデックスは[0, 1, 2][2, 1, 3]で内側を向く
	vertexData[0].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[1].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[2].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[3].position = { 1.0f, -1.0f, -1.0f, 1.0f };
	// 左面。描画インデックスは[4, 5, 6][6, 5, 7]
	vertexData[4].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[5].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[6].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[7].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	// 前面。描画インデックスは[8, 9, 10][10, 9, 11]
	vertexData[8].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[9].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[10].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[11].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	// 後面。描画インデックスは[12, 13, 14][14, 13, 15]
	vertexData[12].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[13].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[14].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[15].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	// 上面。描画インデックスは[16, 17, 18][18, 17, 19]
	vertexData[16].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[17].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[18].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[19].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	// 下面。描画インデックスは[20, 21, 22][22, 21, 23]
	vertexData[20].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[21].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[22].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[23].position = { 1.0f, -1.0f, -1.0f, 1.0f };

	// マテリアル用のリソース作成
	materialResource_ = CreateBufferResource(dxBase->GetDevice(), sizeof(Object3D::Material));
	Object3D::Material* materialData = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = Float4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = Matrix::Identity();

	// WVP用のリソース作成
	wvpResource_ = CreateBufferResource(dxBase->GetDevice(), sizeof(Object3D::TransformationMatrix));
	Object3D::TransformationMatrix* wvpData = nullptr;
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	wvpData->WVP = Matrix::Identity();
	wvpData->World = Matrix::Identity();
	wvpData->WorldInverseTranspose = Matrix::Identity();
}

void GamePlayScene::Finalize()
{
	// 球オブジェクト開放
	for (Object3D* sphere : jointSpheres_) {
		delete sphere;
	}
	jointSpheres_.clear();

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

	animationTime += 1.0f / 60.0f; // 時刻を進める
	animationTime = std::fmod(animationTime, animation_.duration); // 最後までいったら最初からリピート再生

	// アニメーションの更新を行って、骨ごとのLocal情報を更新する
	ModelManager::ApplyAnimation(skeleton_, animation_, animationTime);
	// 現在の骨ごとのLocal情報を基にSkeletonSpaceの情報を更新する
	ModelManager::Update(skeleton_);
	// SkeletonSpaceの情報を基に、SkinClusterのMatrixPaletteを更新する
	ModelManager::Update(skinCluster_, skeleton_);
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
	object_->wvpCB_.data_->WVP = worldMatrix * viewMatrix * projectionMatrix;
	object_->wvpCB_.data_->World = worldMatrix;

	// 3Dオブジェクト描画
	object_->Draw(skinCluster_);

	///
	///	Jointの位置に球を描画
	/// 

	//for (size_t i = 0; i < jointSpheres_.size(); ++i) {
	//	const auto& joint = skeleton_.joints[i];

	//	// Jointのワールド座標を計算
	//	Matrix jointWorldMatrix = joint.skeletonSpaceMatrix * worldMatrix;

	//	// 球のスケーリング行列を適用
	//	Matrix sphereScaleMatrix = Matrix::Scaling(jointSpheres_[i]->transform_.scale);
	//	Matrix sphereWorldMatrix = sphereScaleMatrix * jointWorldMatrix;

	//	// 球を描画
	//	jointSpheres_[i]->wvpCB_.data_->WVP = sphereWorldMatrix * viewMatrix * projectionMatrix;
	//	jointSpheres_[i]->wvpCB_.data_->World = sphereWorldMatrix;
	//	jointSpheres_[i]->Draw();
	//}

	// Skyboxの描画
	dxBase->GetCommandList()->SetPipelineState(dxBase->GetPipelineStateSkybox());

	dxBase->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	TextureManager::SetDescriptorTable(2, dxBase->GetCommandList(), cubeMapDDS_);
	dxBase->GetCommandList()->DrawInstanced(vertexNum_, 1, 0, 0);

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

	ImGui::Text("object");
	ImGui::DragFloat3("object.translate", &object_->transform_.translate.x, 0.01f);
	ImGui::DragFloat3("object.rotate", &object_->transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("object.scale", &object_->transform_.scale.x, 0.01f);

	ImGui::Text("camera");
	ImGui::DragFloat3("camera.translate", &camera->transform.translate.x, 0.01f);
	ImGui::DragFloat3("camera.rotate", &camera->transform.rotate.x, 0.01f);
	ImGui::DragFloat3("camera.scale", &camera->transform.scale.x, 0.01f);

	ImGui::End();


	ImGui::Begin("Joint");

	// Jointの位置を表示
	for (size_t i = 0; i < skeleton_.joints.size(); ++i) {
		const auto& joint = skeleton_.joints[i];
		Float3 position = Float3(
			joint.skeletonSpaceMatrix.r[3][0],
			joint.skeletonSpaceMatrix.r[3][1],
			joint.skeletonSpaceMatrix.r[3][2]
			);

		ImGui::Text("Joint %zu: (%.3f, %.3f, %.3f)", i, position.x, position.y, position.z);
	}

	ImGui::End();


	// ImGuiの内部コマンドを生成する
	ImguiWrapper::Render(dxBase->GetCommandList());
	// 描画後処理
	dxBase->PostDraw();
	// フレーム終了処理
	dxBase->EndFrame();
}
