#pragma once 

#include "app.hpp"
#include "waves.hpp"

namespace lea {


	class WavesApp : public App {


	public:
		WavesApp();
		virtual ~WavesApp() {}
	
	protected:
		ComPtr<ID3DX11Effect> effect_;
		ComPtr<ID3DX11EffectTechnique> effectTechnique_;
		ComPtr<ID3DX11EffectMatrixVariable> worldMatrix_;

		ComPtr<ID3D11Buffer> landVertexBuffer_;
		ComPtr<ID3D11Buffer> landIndexBuffer_;
		ComPtr<ID3D11Buffer> wavesVertexBuffer_;
		ComPtr<ID3D11Buffer> wavesIndexBuffer_;

		ComPtr<ID3D11RasterizerState> rastState_;


		ComPtr<ID3D11InputLayout> inputLayout_;

		Waves waves;

		XMFLOAT4X4 mGridWorld;
		XMFLOAT4X4 mGridWaves;

		UINT mGridIndexCount;

		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;

		float mTheta;
		float mPhi;
		float mRadius;
		std::pair<int, int> m_LastMousePos;
		void Init() override;

		void PollEvents() override;

		void UpdateScene(float deltaTime) override;
		void BuildLandGeometryBuffers();
		void BuildWavesGeometryBuffers();
		void CreateInputLayout();

		void DrawScene() override;

		float GetHeight(float x, float z) const;
	};
}