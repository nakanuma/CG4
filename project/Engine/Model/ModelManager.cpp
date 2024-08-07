#include "ModelManager.h"
#include <fstream>
#include <sstream>
#include <DirectXUtil.h>
#include <DirectXBase.h>

ModelManager::ModelData ModelManager::LoadModelFile(const std::string& directoryPath, const std::string& filename, ID3D12Device* device)
{
    // 1. 中で必要となる変数の宣言
    ModelData modelData; // 構築するModelData
    std::vector<Float4> positions; // 位置
    std::vector<Float3> normals; // 法線
    std::vector<Float2> texcoords; // テクスチャ座標
    std::string line; // ファイルから読んだ1行を格納するもの

    // 2. ファイルを開く
    Assimp::Importer importer;
    std::string filePath = directoryPath + "/" + filename;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    assert(scene->HasMeshes()); // メッシュがないのは対応しない

    // RootNodeを読む
    modelData.rootNode = ReadNode(scene->mRootNode);

    // 3. 実際にファイルを読み、ModelDataを構築していく
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        assert(mesh->HasNormals()); // 法線がないMeshは今回は非対応
        assert(mesh->HasTextureCoords(0)); // TexcoordがないMeshは今回非対応
        // ここからMeshの中身（Face）の解析を行っていく
        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            aiFace& face = mesh->mFaces[faceIndex];
            assert(face.mNumIndices == 3); // 三角形のみサポート
            // ここからFaceの中身（Vertex）の解析を行っていく
            for (uint32_t element = 0; element < face.mNumIndices; ++element) {
                uint32_t vertexIndex = face.mIndices[element];
                aiVector3D& position = mesh->mVertices[vertexIndex];
                aiVector3D& normal = mesh->mNormals[vertexIndex];
                aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
                VertexData vertexData;
                vertexData.position = { position.x, position.y, position.z, 1.0f };
                vertexData.normal = { normal.x, normal.y, normal.z };
                vertexData.texcoord = { texcoord.x, texcoord.y };
                // aiProcess_MakeLeftHandedはz*=-1で、右手->左手に変換するので手動で対処
                vertexData.position.x *= -1.0f;
                vertexData.normal.x *= -1.0f;
                modelData.vertices.push_back(vertexData);
            }
        }
    }

    for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
        aiMaterial* material = scene->mMaterials[materialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
            aiString textureFilePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
            modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
        }
    }

    // vertexResourceの作成
    modelData.vertexResource = CreateBufferResource(DirectXBase::GetInstance()->GetDevice(), sizeof(VertexData) * modelData.vertices.size());

    // 頂点バッファビューを作成する
    modelData.vertexBufferView;
    // リソースの先頭のアドレスから使う
    modelData.vertexBufferView.BufferLocation = modelData.vertexResource->GetGPUVirtualAddress();
    // 使用するリソースのサイズは頂点のサイズ
    modelData.vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
    // 1頂点あたりのサイズ
    modelData.vertexBufferView.StrideInBytes = sizeof(VertexData);


    // 頂点リソースにデータを書き込む
    VertexData* vertexData = nullptr;
    // 書き込むためのアドレスを取得
    modelData.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    // 頂点データをリソースにコピー
    std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

    // 4. ModelDataを返す
    return modelData;
}

ModelManager::MaterialData ModelManager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, ID3D12Device* device)
{
    // 1. 中で必要となる変数の宣言
    MaterialData materialData; // 構築するMaterialData
    std::string line; // ファイルから読んだ1行を格納するもの

    // 2. ファイルを開く
    std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める

    // 3. 実際にファイルを読み、MaterialDataを構築していく
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        // identifierに応じた処理
        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            // 連結してファイルパスにする
            materialData.textureFilePath = directoryPath + "/" + textureFilename;
            // 画像を読み込む
            materialData.textureHandle = TextureManager::Load(materialData.textureFilePath, device);
        }
    }

    // 4. MaterialDataを返す
    return materialData;
}

ModelManager::Node ModelManager::ReadNode(aiNode* node)
{
    Node result;
    aiMatrix4x4 aiLocalMatrix = node->mTransformation; // nodeのlocalMatrixを取得
    aiLocalMatrix.Transpose(); // 列ベクトル形式を行ベクトル形式に転置
    for (uint32_t i = 0; i < 4; ++i) { // 行列を結果にコピー
        for (uint32_t j = 0; j < 4; ++j) {
            result.localMatrix.r[i][j] = aiLocalMatrix[i][j];
        }
    }
    result.name = node->mName.C_Str(); // Node名を格納
    result.children.resize(node->mNumChildren); // 子供の数だけ確保
    for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
        // 再帰的に読んで階層構造を作っていく
        result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
    }
    return result;
}
