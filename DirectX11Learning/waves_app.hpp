#pragma once 

#include "app.hpp"
#include "waves.hpp"

#include "lea_engine_utils.hpp"

namespace lea {
	using namespace lea::utils::light;

	class WavesApp : public App {
	

	public:
		WavesApp();
		virtual ~WavesApp() {}
	
	protected:
		ComPtr<ID3DX11Effect> effect_;
		ComPtr<ID3DX11EffectTechnique> effectTechnique_;
		

		ComPtr<ID3D11Buffer> landVertexBuffer_;
		ComPtr<ID3D11Buffer> landIndexBuffer_;
		ComPtr<ID3D11Buffer> wavesVertexBuffer_;
		ComPtr<ID3D11Buffer> wavesIndexBuffer_;

		ComPtr<ID3D11InputLayout> inputLayout_;

		ComPtr<ID3D11ShaderResourceView> grassTexture_;
		ComPtr<ID3D11ShaderResourceView> wavesTexture_;

		Waves waves;

		XMFLOAT4X4 mGridWorld;
		XMFLOAT4X4 mGridWaves;

		XMFLOAT2 mWaterTexOffset;

		XMFLOAT4X4 mGrassTexTransform;
		XMFLOAT4X4 mWavesTexTransform;

		UINT mGridIndexCount;

		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;

		DirectionalLight mDirLight;
		PointLight mPointLight;
		SpotLight mSpotLight;
		Material mLandMat;
		Material mWavesMat;

		XMFLOAT3 mEyePosW;

		float mTheta;
		float mPhi;
		float mRadius;
		std::pair<int, int> m_LastMousePos;

		ComPtr<ID3DX11EffectMatrixVariable> mfxWorldViewProj;
		ComPtr<ID3DX11EffectMatrixVariable> mfxWorld;
		ComPtr<ID3DX11EffectMatrixVariable> mfxWorldInvTranspose;
		ComPtr<ID3DX11EffectVectorVariable> mfxEyePosW;
		ComPtr<ID3DX11EffectVariable> mfxDirLight;
		ComPtr<ID3DX11EffectVariable> mfxPointLight;
		ComPtr<ID3DX11EffectVariable> mfxSpotLight;
		ComPtr<ID3DX11EffectVariable> mfxMaterial;
		ComPtr<ID3DX11EffectMatrixVariable> mfxTexTransform;
		ComPtr<ID3DX11EffectShaderResourceVariable> mfxTexture;



		void Init() override;

		void PollEvents() override;

		void UpdateScene(float deltaTime) override;
		void BuildLandGeometryBuffers();
		void BuildWavesGeometryBuffers();
		void CreateInputLayout();

		void DrawScene() override;

		float GetHeight(float x, float z) const;
		XMFLOAT3 GetHillNormal(float x, float z) const;

		void InitFX();
		void LoadTextures();
	};
}