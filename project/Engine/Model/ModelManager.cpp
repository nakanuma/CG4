#include "ModelManager.h"
#include <fstream>
#include <sstream>
#include <DirectXUtil.h>
#include <DirectXBase.h>

#include <SRVManager.h>

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
        modelData.vertices.resize(mesh->mNumVertices); // 最初に頂点数分のメモリを確保しておく
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
            aiVector3D& position = mesh->mVertices[vertexIndex];
            aiVector3D& normal = mesh->mNormals[vertexIndex];
            aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
            /// 右手系->左手系への変換
            modelData.vertices[vertexIndex].position = {-position.x, position.y, position.z, 1.0f};
            modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
            modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
        }
        // Indexの解析
        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            aiFace& face = mesh->mFaces[faceIndex];
            assert(face.mNumIndices == 3);

            for (uint32_t element = 0; element < face.mNumIndices; ++element) {
                uint32_t vertexIndex = face.mIndices[element];
                modelData.indices.push_back(vertexIndex);
            }
        }
        // SkinCluster構築用のデータ取得を追加
        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
            aiBone* bone = mesh->mBones[boneIndex];
            std::string jointName = bone->mName.C_Str();
            JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

            aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
            aiVector3D scale, translate;
            aiQuaternion rotate;
            bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
            Matrix bindPoseMatrix = Matrix::MakeAffine
            ({ scale.x, scale.y, scale.z }, { rotate.x, -rotate.y, -rotate.z, rotate.w }, { -translate.x, translate.y, translate.z });
            jointWeightData.inverseBindPoseMatrix = Matrix::Inverse(bindPoseMatrix);

            for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
                jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId });
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


    // indexResourceの作成
    modelData.indexResource = CreateBufferResource(DirectXBase::GetInstance()->GetDevice(), sizeof(uint32_t) * modelData.indices.size());

    // インデックスバッファビューを作成する
    modelData.indexBufferView.BufferLocation = modelData.indexResource->GetGPUVirtualAddress();
    modelData.indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * modelData.indices.size());
    modelData.indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    // インデックスリソースにデータを書き込む
    uint32_t* indexData = nullptr;
    modelData.indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
    std::memcpy(indexData, modelData.indices.data(), sizeof(uint32_t) * modelData.indices.size());

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

    aiVector3D scale, translate;
    aiQuaternion rotate;
    node->mTransformation.Decompose(scale, rotate, translate); // assimpの行列からSRTを抽出する関数を利用
    result.transform.scale = { scale.x, scale.y, scale.z }; // Scaleはそのまま
    result.transform.rotate = { rotate.x, -rotate.y, -rotate.z, rotate.w }; // x軸を反転、さらに回転方向が逆なので軸を反転させる
    result.transform.translate = { -translate.x, translate.y, translate.z }; // x軸を反転
    result.localMatrix = result.transform.MakeAffineMatrix();

    //aiMatrix4x4 aiLocalMatrix = node->mTransformation; // nodeのlocalMatrixを取得
    //aiLocalMatrix.Transpose(); // 列ベクトル形式を行ベクトル形式に転置
    //for (uint32_t i = 0; i < 4; ++i) { // 行列を結果にコピー
    //    for (uint32_t j = 0; j < 4; ++j) {
    //        result.localMatrix.r[i][j] = aiLocalMatrix[i][j];
    //    }
    //}
    result.name = node->mName.C_Str(); // Node名を格納
    result.children.resize(node->mNumChildren); // 子供の数だけ確保
    for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
        // 再帰的に読んで階層構造を作っていく
        result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
    }
    return result;
}

ModelManager::Animation ModelManager::LoadAnimation(const std::string& directoryPath, const std::string& filename)
{
    Animation animation; // 今回作るアニメーション
    Assimp::Importer importer;
    std::string filePath = directoryPath + "/" + filename;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
    assert(scene->mNumAnimations != 0); // アニメーションが無い
    aiAnimation* animationAssimp = scene->mAnimations[0]; // 最初のアニメーションだけ採用。
    animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond); // 時間の単位を秒に変換

    // assimpでは個々のNodeのAnimationをchannelと呼んでいるのでchannelを回してNodeAnimationの情報をとってくる
    for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
        aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
        NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
        // Translationを読み取る
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
            aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
            KeyframeFloat3 keyframe;
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
            keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; // 右手->左手
            nodeAnimation.translate.push_back(keyframe);
        }
        // Rotationを読み取る
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
            aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
            KeyframeQuaternion keyframe;
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
            aiQuaternion& rotate = keyAssimp.mValue;
            keyframe.value = { rotate.x, -rotate.y, -rotate.z, rotate.w }; // 右手->左手に変換するためにyとzを反転
            nodeAnimation.rotate.push_back(keyframe);
        }
        // Scaleを読み取る
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
            aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
            KeyframeFloat3 keyframe;
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
            keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
            nodeAnimation.scale.push_back(keyframe);
        }
    }
    // 解析完了
    return animation;
}

Float3 ModelManager::CalculateValue(const std::vector<KeyframeFloat3>& keyframes, float time)
{
    assert(!keyframes.empty()); // キーがないものは返す値がわからないのでダメ
    if (keyframes.size() == 1 || time <= keyframes[0].time) { // キーが1つか、時刻がキーフレーム前なら最初の軸とする
        return keyframes[0].value;
    }

    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // indexとnextINdexの2つのkeyframeを取得して範囲内に時刻があるかを判定
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内を補間する
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Float3::Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
    return (*keyframes.rbegin()).value;
} 

Quaternion ModelManager::CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time)
{
    assert(!keyframes.empty()); // キーがないものは返す値がわからないのでダメ
    if (keyframes.size() == 1 || time <= keyframes[0].time) { // キーが1つか、時刻がキーフレーム前なら最初の軸とする
        return keyframes[0].value;
    }

    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内を補間する
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Quaternion::Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
    return (*keyframes.rbegin()).value;
}

ModelManager::Skeleton ModelManager::CreateSkeleton(const Node& rootNode)
{
    Skeleton skeleton;
    skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

    // 名前とindexのマッピングを行いアクセスしやすくする
    for (const Joint& joint : skeleton.joints) {
        skeleton.jointMap.emplace(joint.name, joint.index);
    }

    // スケルトン作成時に更新しておく
    ModelManager::Update(skeleton);

    return skeleton;
}

int32_t ModelManager::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints)
{
    Joint joint;
    joint.name = node.name;
    joint.localMatrix = node.localMatrix;
    joint.skeletonSpaceMatrix = Matrix::Identity();
    joint.transform = node.transform;
    joint.index = int32_t(joints.size()); // 現在登録されてる数をIndexに
    joint.parent = parent;
    joints.push_back(joint); // SkeletonのJoint列に追加
    for (const Node& child : node.children) {
        // 子Jointを作成し、そのIndexを登録
        int32_t childIndex = CreateJoint(child, joint.index, joints);
        joints[joint.index].children.push_back(childIndex);
    }
    // 自身のIndexを返す
    return joint.index;
}

void ModelManager::Update(Skeleton& skeleton)
{
    // すべてのJointを更新
    for (Joint& joint : skeleton.joints) {
        joint.localMatrix = joint.transform.MakeAffineMatrix();
        if (joint.parent) { // 親がいれば親の行列を掛ける
            joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
        } else { // 親がいないのでlocalMatrixとskeletonSpaceMatrixは一致する
            joint.skeletonSpaceMatrix = joint.localMatrix;
        }
    }
}

void ModelManager::ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime)
{
    for (Joint& joint : skeleton.joints) {
        // 対象のJointのAnimationがあれば、他の適用を行う
        if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
            const NodeAnimation& rootNodeAnimation = (*it).second;
            joint.transform.translate = CalculateValue(rootNodeAnimation.translate, animationTime);
            joint.transform.rotate = CalculateValue(rootNodeAnimation.rotate, animationTime);
            joint.transform.scale = CalculateValue(rootNodeAnimation.scale, animationTime);
        }
    }
}

ModelManager::SkinCluster ModelManager::CreateSkinCluster(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Skeleton& skeleton, const ModelData& modelData)
{
    SkinCluster skinCluster;

    SRVManager* srvManager = SRVManager::GetInstance();

    // palette用のResourceを確保
    skinCluster.paletteResource = CreateBufferResource(device.Get(), sizeof(WellForGPU) * skeleton.joints.size());
    WellForGPU* mappedPalette = nullptr;
    skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
    skinCluster.mappedPalette = { mappedPalette, skeleton.joints.size() }; // spanを使ってアクセスするようにする
    skinCluster.paletteSrvHandle.first = srvManager->GetCPUDescriptorHandle(srvManager->GetIndex());
    skinCluster.paletteSrvHandle.second = srvManager->GetGPUDescriptorHandle(srvManager->GetIndex());
    SRVManager::GetInstance()->IncrementIndex(); // Indexを進める

    // palette用のsrvを作成
    D3D12_SHADER_RESOURCE_VIEW_DESC paletteSrvDesc{};
    paletteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    paletteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    paletteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    paletteSrvDesc.Buffer.FirstElement = 0;
    paletteSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    paletteSrvDesc.Buffer.NumElements = UINT(skeleton.joints.size());
    paletteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
    device->CreateShaderResourceView(skinCluster.paletteResource.Get(), &paletteSrvDesc, skinCluster.paletteSrvHandle.first);

    // influence用のResourceを確保
    skinCluster.influenceResource = CreateBufferResource(device.Get(), sizeof(VertexInfluence) * modelData.vertices.size());
    VertexInfluence* mappedInfluence = nullptr;
    skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
    std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size()); // 0埋め。weightを0にしておく
    skinCluster.mappedInfluence = { mappedInfluence, modelData.vertices.size() };

    // Influence用のVBVを作成
    skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
    skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
    skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

    // InverseBindPoseMatrixの保存領域を作成
    skinCluster.inverseBindPoseMatrices.resize(skeleton.joints.size());
    std::generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), []() {return Matrix::Identity(); }); // インスタンスを作成してIdentityを呼び出す

    // ModelDataのSkinCluster情報を解析してInfluenceの中身を埋める
    for (const auto& jointWeight : modelData.skinClusterData) { // ModelのSkinClusterの情報を解析
        auto it = skeleton.jointMap.find(jointWeight.first); // jointWeight.firstはjoint名なので、skeletonに対象となるjointが含まれているか判断
        if (it == skeleton.jointMap.end()) { // そんな名前のJointは存在しないため、次に回す
            continue;
        }
        // (*it).secondにはjointのindexが入っているので、該当のindexのinverseBindPoseMatrixを代入
        skinCluster.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
        for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
            auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex]; // 該当のvertexIndexのinfluence情報を参照しておく
            for (uint32_t index = 0; index < kNumMaxInfluence; ++index) { // 空いているところに入れる
                if (currentInfluence.weights[index] == 0.0f) { // weight == 0 が空いている状態なので、その場所にweightとjointのindexを代入
                    currentInfluence.weights[index] = vertexWeight.weight;
                    currentInfluence.jointIndices[index] = (*it).second;
                    break;
                }
            }
        }
    }

    return skinCluster;
}

void ModelManager::Update(SkinCluster& skinCluster, const Skeleton& skeleton)
{
    for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
        assert(jointIndex < skinCluster.inverseBindPoseMatrices.size());
        skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix =
            skinCluster.inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;
        skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
            Matrix::Transpose(Matrix::Inverse(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix));
    }
}
