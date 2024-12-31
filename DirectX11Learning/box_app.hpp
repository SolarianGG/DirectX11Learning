#pragma once

#include "app.hpp"

namespace lea {
	using Microsoft::WRL::ComPtr;
	using namespace lea::utils::light;
	class BoxApp : public App {

	public:
		BoxApp();

		virtual ~BoxApp() {}
	protected:
		ComPtr<ID3DX11Effect> mEffect_;
		ComPtr<ID3DX11EffectTechnique> mEffectTechnique_;

		ComPtr<ID3DX11EffectMatrixVariable> mWorldViewProj_;
		ComPtr<ID3DX11EffectMatrixVariable> mWorld_;
		ComPtr<ID3DX11EffectMatrixVariable> mWorldInverseTranspose_;
		ComPtr<ID3DX11EffectVariable> mMaterial_;
		ComPtr<ID3DX11EffectVariable> mPointLight_;
		ComPtr<ID3DX11EffectVariable> mDirectionalLight_;

		ComPtr<ID3DX11EffectVectorVariable> mEyePosition_;

		ComPtr<ID3D11Buffer> mVertexBuffer_;
		ComPtr<ID3D11Buffer> mIndexBuffer_;
		ComPtr<ID3D11InputLayout> mIputLayout_;
		ComPtr<ID3D11RasterizerState> mRastState_;

		XMFLOAT3 mEyePosition;

		Material mBoxMaterial;

		PointLight mPointLight;
		DirectionalLight mDirectionalLight;

		XMFLOAT4X4 mWorld;
		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;

		float m_Theta;
		float m_Phi;
		float m_Radius;

		UINT mBoxIndexCount = 0;

		std::pair<int, int> mLastMousePos;

		void Init() override;

		void PollEvents() override;

		void UpdateScene(float deltaTime) override;

		void DrawScene() override;

		void CreateGeometryBuffers();
		void CreateInputLayout();
	};
}