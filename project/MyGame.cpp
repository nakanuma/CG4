#include "MyGame.h"
#include "SceneFactory.h"

void MyGame::Initialize()
{
	///
	/// 基底クラスの初期化処理
	/// 
	
	Framework::Initialize();

	///
	/// ゲーム固有の初期化
	///

	//// 最初のシーンの生成
	//BaseScene* scene = new TitleScene();
	//scene->Initialize();
	//// シーンマネージャに最初のシーンをセット
	//SceneManager::GetInstance()->SetNextScene(scene);

	// シーンファクトリーを生成し、マネージャにセット
	sceneFactory_ = new SceneFactory();
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory_);
	// シーンマネージャに最初のシーンをセット
	SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
}

void MyGame::Finalize()
{
	///
	/// ゲーム固有の終了処理
	///

	///
	/// 基底クラスの終了処理
	/// 
	
	Framework::Finalize();
}

void MyGame::Update()
{
	///
	/// 基底クラスの更新処理
	/// 
	
	Framework::Update();

	///
	/// ゲーム固有の更新処理
	///
	
}

void MyGame::Draw()
{
	// シーンの描画処理
	SceneManager::GetInstance()->Draw();
}
