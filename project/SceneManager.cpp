#include "SceneManager.h"

SceneManager::~SceneManager()
{
	// 最後のシーンの終了と開放
	scene_->Finalize();
	delete scene_;
}

void SceneManager::Update()
{
	///
	///	シーン切り替え機構
	/// 
	
	// 次シーンの予約があるなら
	if (nextScene_) {
		// 旧シーンの終了
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}

		// シーン切り替え
		scene_ = nextScene_;
		nextScene_ = nullptr;
		// 次シーンを初期化する
		scene_->Initialize();
	}

	// 実行中シーンを更新する
	scene_->Update();
}

void SceneManager::Draw()
{
	// 実行中シーンの描画
	scene_->Draw();
}