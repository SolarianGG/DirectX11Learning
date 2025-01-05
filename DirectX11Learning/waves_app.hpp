#pragma once 

#include "app.hpp"
#include "waves.hpp"

#include <unordered_map>

#include "lea_engine_utils.hpp"

namespace lea {
	using namespace lea::utils::light;

	class WavesApp : public App {
		enum class ERenderTypes {
			LightOnly,
			LightAndTextures,
			LightAndTexturesAndFog,
		};

	public:
		WavesApp();
		virtual ~WavesApp() {}
	
	protected:
		ComPtr<ID3DX11Effect> effect_;
		std::unordered_map<ERenderTypes, ComPtr<ID3DX11EffectTechnique>> effectTechniques_;

		ComPtr<ID3D11Buffer> landVertexBuffer_;
		ComPtr<ID3D11Buffer> landIndexBuffer_;
		ComPtr<ID3D11Buffer> wavesVertexBuffer_;
		ComPtr<ID3D11Buffer> wavesIndexBuffer_;

		ComPtr<ID3D11Buffer> commonVertexBuffer_;
		ComPtr<ID3D11Buffer> commonIndexBuffer_;

		ComPtr<ID3D11InputLayout> inputLayout_;

		ComPtr<ID3D11ShaderResourceView> grassTexture_;
		ComPtr<ID3D11ShaderResourceView> wavesTexture_;
		ComPtr<ID3D11ShaderResourceView> boxTexture_;

		Waves waves;

		XMFLOAT4X4 mLandWorld;
		XMFLOAT4X4 mWavesWorld;
		XMFLOAT4X4 mBoxWorld;

		XMFLOAT2 mWaterTexOffset;

		XMFLOAT4X4 mGrassTexTransform;
		XMFLOAT4X4 mWavesTexTransform;
		XMFLOAT4X4 mBoxTexTransform;

		UINT mGridIndexCount = 0;

		UINT mBoxIndexCount = 0;

		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;

		DirectionalLight mDirLights[3];

		Material mLandMat;
		Material mWavesMat;
		Material mBoxMat;

		XMFLOAT3 mEyePosW;

		ERenderTypes renderOptions;

		float mTheta;
		float mPhi;
		float mRadius;
		std::pair<int, int> m_LastMousePos;

		ComPtr<ID3DX11EffectMatrixVariable> mfxWorldViewProj;
		ComPtr<ID3DX11EffectMatrixVariable> mfxWorld;
		ComPtr<ID3DX11EffectMatrixVariable> mfxWorldInvTranspose;
		ComPtr<ID3DX11EffectVectorVariable> mfxEyePosW;
		ComPtr<ID3DX11EffectVectorVariable> mfxFogColor;
		ComPtr<ID3DX11EffectScalarVariable> mfxFogStart;
		ComPtr<ID3DX11EffectScalarVariable> mfxFogRange;
		ComPtr<ID3DX11EffectVariable> mfxDirLights;
		ComPtr<ID3DX11EffectVariable> mfxMaterial;
		ComPtr<ID3DX11EffectMatrixVariable> mfxTexTransform;
		ComPtr<ID3DX11EffectShaderResourceVariable> mfxTexture;

		ComPtr<ID3D11RasterizerState> mNoCullRS;
		ComPtr<ID3D11BlendState> mTransparentBS;

		void Init() override;

		void PollEvents() override;

		void UpdateScene(float deltaTime) override;
		void BuildLandGeometryBuffers();
		void BuildWavesGeometryBuffers();
		void BuildCommonGeometryBuffers();
		void CreateInputLayout();
		void CreateRasterizerStates();

		void DrawScene() override;

		float GetHeight(float x, float z) const;
		XMFLOAT3 GetHillNormal(float x, float z) const;

		void InitFX();
		void LoadTextures();
	};
}