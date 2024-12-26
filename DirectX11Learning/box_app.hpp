#pragma once

#include "app.hpp"

namespace lea {
	using Microsoft::WRL::ComPtr;
	class BoxApp : public App {

	public:
		BoxApp();

		virtual ~BoxApp() {}
	protected:
		ComPtr<ID3DX11Effect> effect_;
		ComPtr<ID3DX11EffectTechnique> effectTechnique_;
		ComPtr<ID3DX11EffectMatrixVariable> worldMatrix_;
		//ComPtr<ID3DX11EffectMatrixVariable> effectTechnique_;

		ComPtr<ID3D11Buffer> vertexBuffer_;
		ComPtr<ID3D11Buffer> indexBuffer_;
		ComPtr<ID3D11InputLayout> inputLayout_;

		XMFLOAT4X4 mWorld;
		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;

		float m_Theta;
		float m_Phi;
		float m_Radius;

		std::pair<int, int> m_LastMousePos;

		void Init() override;

		void PollEvents() override;

		void UpdateScene(float deltaTime) override;

		void DrawScene() override;

		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateInputLayout();
	};
}