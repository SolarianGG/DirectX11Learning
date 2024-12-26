#pragma once

#include "app.hpp"

namespace lea{
	using Microsoft::WRL::ComPtr;

	class TerrainApp : public App {

	public:
		TerrainApp();

		virtual ~TerrainApp() {}
	protected:
		ComPtr<ID3DX11Effect> effect_;
		ComPtr<ID3DX11EffectTechnique> effectTechnique_;
		ComPtr<ID3DX11EffectMatrixVariable> worldMatrix_;

		ComPtr<ID3D11Buffer> vertexBuffer_;
		ComPtr<ID3D11Buffer> indexBuffer_;
		ComPtr<ID3D11InputLayout> inputLayout_;

		XMFLOAT4X4 mGridWorld;

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

		void DrawScene() override;

		void CreateGeometryBuffers();
		void CreateInputLayout();

		float GetHeight(float x, float z) const;
	};
}