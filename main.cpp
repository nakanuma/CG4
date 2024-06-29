#include <Windows.h>
#include <cstdint>
#include <assert.h>
#define DIRECTINPUT_VERSON 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
 
// MyClass 
#include "MyWindow.h"
#include "Logger.h"
#include "StringUtil.h"
#include "DirectXBase.h"
#include "DirectXUtil.h"
#include "MyMath.h"
#include "Camera.h"
#include "DescriptorHeap.h"
#include "ImguiWrapper.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ConstBuffer.h"
#include "Object3D.h"
#include "OutlinedObject.h"
#include "Emitter.h"

struct DirectionalLight {
	Float4 color; // ライトの色
	Float3 direction; // ライトの向き
	float intensity; // 輝度
};

enum BlendMode {
	kBlendModeNormal,
	kBlendModeNone,
	kBlendModeAdd,
	kBlendModeSubtract,
	kBlendModeMultiply,
	kBlendModeScreen
};

const char* BlendModeNames[6] = {
	"kBlendModeNormal",
	"kBlendModeNone",
	"kBlendModeAdd",
	"kBlendModeSubtract",
	"kBlendModeMultiply",
	"kBlendModeScreen"
};

/////////////////// ↓入力デバイス関連↓ ///////////////////

char keys[256] = { 0 };
char preKeys[256] = { 0 };

// キーが押された場合を判定
bool isKeyPressed(char key) {
	return keys[key] && !preKeys[key];
}

// キーが離された場合を判定
bool isKeyReleased(char key) {
	return !keys[key] && preKeys[key];
}

/////////////////// ↑入力デバイス関連↑ ///////////////////

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker::GetInstance();
	DirectXBase* dxBase = nullptr;

	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);
	 
	// ウィンドウの生成
	Window::Create(L"CG2WindowClass", 1280, 720);

	// DirectX初期化処理
	dxBase = DirectXBase::GetInstance();
	dxBase->Initialize();

	/////////////////// ↓入力デバイス初期化処理↓ ///////////////////

	// DirectInputの初期化
	IDirectInput8* directInput = nullptr;
	HRESULT result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの生成
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));
	
	// 入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard); // 標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard->SetCooperativeLevel(
		Window::GetHandle(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	/////////////////// ↑入力デバイス初期化処理↑ ///////////////////

	// TextureManagerの初期化（srvHeapの生成）
	TextureManager::Initialize(dxBase->GetDevice());

	// ImGuiの初期化
	ImguiWrapper::Initialize(dxBase->GetDevice(), dxBase->GetSwapChainDesc().BufferCount, dxBase->GetRtvDesc().Format, TextureManager::GetInstance().srvHeap_.heap_.Get());

	///
	///	↓ ここから3Dオブジェクトの設定
	/// 

	// モデル読み込み
	ModelData planeModel = ModelManager::LoadObjFile("resources/Models", "plane.obj", dxBase->GetDevice());

	// 平面オブジェクトの生成
	Object3D plane;
	// モデルを指定
	plane.model_ = &planeModel;
	// 初期回転角を設定
	plane.transform_.rotate = { 0.0f, 3.1f, 0.0f };

	///
	///	↑ ここまで3Dオブジェクトの設定
	/// 

	///
	///	↓ ここからスプライトの設定
	/// 

	// Sprite用のリソースを作る
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceSprite = CreateBufferResource(dxBase->GetDevice(), sizeof(VertexData) * 4);

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点4つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	// 頂点データを設定
	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	// 1枚目の三角形
	// 左下
	vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[0].normal = { 0.0f, 0.0f, -1.0f };
	// 左上
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	vertexDataSprite[1].normal = { 0.0f, 0.0f, -1.0f };
	// 右下
	vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f };
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[2].normal = { 0.0f, 0.0f, -1.0f };
	// 右上
	vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f };
	vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
	vertexDataSprite[3].normal = { 0.0f, 0.0f, -1.0f };


	// 頂点インデックスの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(dxBase->GetDevice(), sizeof(uint32_t) * 6);

	// IndexBufferViewの作成
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	// インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;


	// Sprite用のTransformationMatrix用のリソースを作る。Matrix 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(dxBase->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	// 単位行列を書き込んでおく
	transformationMatrixDataSprite->WVP = Matrix::Identity();

	// Sprite用のマテリアルのリソースを作る
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceSprite = CreateBufferResource(dxBase->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	// 輝度を白に設定
	materialDataSprite->color = Float4(1.0f, 1.0f, 1.0f, 1.0f);
	// SpriteはLightingしないのでfalseを設定する
	materialDataSprite->enableLighting = false;
	// UVTransform行列を単位行列で初期化
	materialDataSprite->uvTransform = Matrix::Identity();


	// CPUで動かす用のTransformを作る
	Transform transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f}, {0.0f, 0.0f, 0.0f} };

	///
	///	↑ ここまでスプライトの設定
	/// 

	///
	/// ↓ ここから光源の設定
	/// 

	// 平行光源のResourceを作成
	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource = CreateBufferResource(dxBase->GetDevice(), sizeof(DirectionalLight));
	// データを書き込む
	DirectionalLight* directionalLightData = nullptr;
	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// デフォルト値を書き込む
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	///
	/// ↑ ここまで光源の設定
	/// 

	// カメラのインスタンスを生成
	Camera camera{ {0.0f, 0.0f, -10.0f}, {0.0f, 0.0f, 0.0f}, 0.45f };
	Camera::Set(&camera);

	// Textureを読み込む
	uint32_t uvCheckerGH = TextureManager::Load("resources/Images/uvChecker.png", dxBase->GetDevice());

	// UVTransform用の変数を用意
	Transform uvTransformSprite{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	// 選択されたブレンドモードを保存する変数
	static BlendMode selectedBlendMode = kBlendModeNormal;

	// ウィンドウの×ボタンが押されるまでループ
	while (!Window::ProcessMessage()) {
		/////////////////// ↓入力デバイス更新処理↓ ///////////////////

		// キーボード情報の取得開始
		keyboard->Acquire();
		// 全キーの入力状態を取得する
		BYTE key[256] = {};
		keyboard->GetDeviceState(sizeof(key), key);

		// preKeysにkeysをコピー
		memcpy(preKeys, keys, sizeof(keys));
		// keysに最新のキー状態をコピー
		memcpy(keys, key, sizeof(keys));

		/////////////////// ↑入力デバイス更新処理↑ ///////////////////

		// フレーム開始処理
		dxBase->BeginFrame();
		// 描画前処理
		dxBase->PreDraw();

		// 描画用のDescriptorHeapの設定
		ID3D12DescriptorHeap* descriptorHeaps[] = { TextureManager::GetInstance().srvHeap_.heap_.Get() };
		dxBase->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);

		// ImGuiのフレーム開始処理
		ImguiWrapper::NewFrame();

		///
		///	更新処理
		/// 

		//////////////////////////////////////////////////////

		// 平面オブジェクトの行列更新
		plane.UpdateMatrix();

		// Sprite用のWorldViewProjectionMatrixを作る
		Matrix worldMatrixSprite = transformSprite.MakeAffineMatrix();
		Matrix viewMatrixSprite = Matrix::Identity();
		Matrix projectionMatrixSprite = Matrix::Orthographic(static_cast<float>(Window::GetWidth()), static_cast<float>(Window::GetHeight()), 0.0f, 1000.0f);
		Matrix worldViewProjectionMatrixSprite = worldMatrixSprite * viewMatrixSprite * projectionMatrixSprite;
		transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
		transformationMatrixDataSprite->World = worldMatrixSprite;


		// UVTransform用の行列を生成する
		Matrix uvTransformMatrix = Matrix::Scaling(uvTransformSprite.scale);
		uvTransformMatrix = uvTransformMatrix * Matrix::RotationZ(uvTransformSprite.rotate.z);
		uvTransformMatrix = uvTransformMatrix * Matrix::Translation(uvTransformSprite.translate);
		materialDataSprite->uvTransform = uvTransformMatrix;

		// キー入力でplaneを移動
		if (key[DIK_W]) {
			plane.transform_.translate.y += 0.01f;
		}
		if (key[DIK_S]) {
			plane.transform_.translate.y -= 0.01f;
		}

		if (isKeyPressed(DIK_A)) { // 押された瞬間
			plane.transform_.translate.x -= 0.1f;
		}
		if (isKeyReleased(DIK_D)) { // 離された瞬間
			plane.transform_.translate.x += 0.1f;
		}

		// ImGui
		ImGui::Begin("Settings");
		ImGui::DragFloat3("translate", &plane.transform_.translate.x, 0.01f);
		ImGui::DragFloat3("rotate", &plane.transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("scale", &plane.transform_.scale.x, 0.01f);
		ImGui::ColorEdit4("color", &plane.materialCB_.data_->color.x);
		ImGui::DragFloat("Intensity", &directionalLightData->intensity, 0.01f);
		if (ImGui::BeginCombo("Blend", BlendModeNames[selectedBlendMode])) {
			for (int n = 0; n < 6; n++) {
				const bool isSelected = (selectedBlendMode == n);
				if (ImGui::Selectable(BlendModeNames[n], isSelected))
					selectedBlendMode = static_cast<BlendMode>(n);
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		
		ImGui::NewLine();
		ImGui::Text("Push [W] / [S] : up / down");
		ImGui::Text("Pressed [A] : move left");
		ImGui::Text("Released [D] : move right");

		ImGui::End();

		//////////////////////////////////////////////////////

		///
		///	描画処理
		/// 

		// 平行光源の情報の定数バッファのセット
		dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		// カメラの定数バッファを設定
		Camera::TransferConstantBuffer();

		///
		/// ↓ ここから3Dオブジェクトの描画コマンド
		/// 

		// 選択されたブレンドモードに変更
		switch (selectedBlendMode) {
		case kBlendModeNormal:
			dxBase->GetCommandList()->SetPipelineState(dxBase->GetPipelineState());
			break;
		case kBlendModeNone:
			dxBase->GetCommandList()->SetPipelineState(dxBase->GetPipelineStateBlendModeNone());
			break;
		case kBlendModeAdd:
			dxBase->GetCommandList()->SetPipelineState(dxBase->GetPipelineStateBlendModeAdd());
			break;
		case kBlendModeSubtract:
			dxBase->GetCommandList()->SetPipelineState(dxBase->GetPipelineStateBlendModeSubtract());
			break;
		case kBlendModeMultiply:
			dxBase->GetCommandList()->SetPipelineState(dxBase->GetPipelineStateBlendModeMultiply());
			break;
		case kBlendModeScreen:
			dxBase->GetCommandList()->SetPipelineState(dxBase->GetPipelineStateBlendModeScreen());
			break;
		}
		// 平面オブジェクトの描画
		plane.Draw();

		///
		/// ↑ ここまで3Dオブジェクトの描画コマンド
		/// 

		///
		/// ↓ ここからスプライトの描画コマンド
		/// 

		// VBVを設定
		dxBase->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		// IBVを設定
		dxBase->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);
		// マテリアルCBufferの場所を設定
		dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		// TransformatinMatrixCBufferの場所を設定
		dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
		// SRVのDescriptorTableの先頭を設定
		TextureManager::SetDescriptorTable(2, dxBase->GetCommandList(), uvCheckerGH);
		// 描画（DrawCall/ドローコール）6個のインデックスを使用し1つのインスタンスを描画
		/*dxBase->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);*/

		///
		/// ↑ ここまでスプライトの描画コマンド
		/// 

		// ImGuiの内部コマンドを生成する
		ImguiWrapper::Render(dxBase->GetCommandList());
		// 描画後処理
		dxBase->PostDraw();
		// フレーム終了処理
		dxBase->EndFrame();
	}
	// ImGuiの終了処理
	ImguiWrapper::Finalize();

	// COMの終了処理
	CoUninitialize();

	return 0;
}