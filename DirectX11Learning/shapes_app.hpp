#pragma once

#include "app.hpp"

namespace lea{
	using Microsoft::WRL::ComPtr;
	using namespace lea::utils::light;
	class ShapesApp : public App {
	public:
		ShapesApp();
		virtual ~ShapesApp();

	protected:
		ComPtr<ID3DX11Effect> effect_;
		ComPtr<ID3DX11EffectTechnique> effectTechnique_;

		ComPtr<ID3DX11EffectMatrixVariable> worldMatrix_;
		ComPtr<ID3DX11EffectMatrixVariable> worldViewProjectionMatrix_;
		ComPtr<ID3DX11EffectMatrixVariable> worldInverseTransposeMatrix_;
		ComPtr<ID3DX11EffectMatrixVariable> textureTransform_;

		ComPtr<ID3DX11EffectVariable> mShapeMaterial_;
		ComPtr<ID3DX11EffectVariable> mDirectionalLight_;

		ComPtr<ID3DX11EffectShaderResourceVariable> mEffectTexture_;

		ComPtr<ID3DX11EffectVectorVariable> mEyePosW_;

		ComPtr<ID3D11Buffer> vertexBuffer_;
		ComPtr<ID3D11Buffer> indexBuffer_;
		ComPtr<ID3D11InputLayout> inputLayout_;

		// Define transformations from local spaces to world space.
		XMFLOAT4X4 mSphereWorld[10];
		XMFLOAT4X4 mCylWorld[10];
		XMFLOAT4X4 mBoxWorld;
		XMFLOAT4X4 mGridWorld;
		XMFLOAT4X4 mSkullWorld;

		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;

		XMFLOAT4X4 mBoxTexTransform;
		XMFLOAT4X4 mCylinderTexTransform;
		XMFLOAT4X4 mSphereTexTransform;
		XMFLOAT4X4 mFloorTexTransform;
		XMFLOAT4X4 mSkullTexTransform;

		int mBoxVertexOffset;
		int mGridVertexOffset;
		int mSphereVertexOffset;
		int mCylinderVertexOffset;
		int mSkullVertexOffset;

		UINT mBoxIndexOffset;
		UINT mGridIndexOffset;
		UINT mSphereIndexOffset;
		UINT mCylinderIndexOffset;
		UINT mSkullIndexOffset;

		UINT mBoxIndexCount;
		UINT mGridIndexCount;
		UINT mSphereIndexCount;
		UINT mCylinderIndexCount;
		UINT mSkullIndexCount;

		ComPtr<ID3D11ShaderResourceView> mSkullTexture_;
		ComPtr<ID3D11ShaderResourceView> mFloorTexture_;
		ComPtr<ID3D11ShaderResourceView> mBoxTexture_;
		ComPtr<ID3D11ShaderResourceView> mCylinderTexture_;
		ComPtr<ID3D11ShaderResourceView> mSphereTexture_;

		XMFLOAT3 eyePos;

		DirectionalLight dirLight;

		Material boxMat;
		Material sphereMat;
		Material gridMat;
		Material cylinderMat;
		Material skullMat;

		float mTheta;
		float mPhi;
		float mRadius;
		std::pair<int, int> m_LastMousePos;
		void Init() override;

		void PollEvents() override;

		void UpdateScene(float deltaTime) override;
		void CreateGeometryBuffers();
		void CreateInputLayout();

		void DrawScene() override;

		std::pair<std::vector<lea::utils::Vertex3>, std::vector<UINT>> ScanModel(std::wstring_view file_name);
		void DrawGUI();

		void LoadTextures();
		void InitFX();
	};
}