#pragma once

#include "app.hpp"

#include <vector>

#include <DirectXMath.h>

#include "lea_engine_utils.hpp"

namespace lea {
	
	using Microsoft::WRL::ComPtr;

	class PyramideApp : public App {

	public:
		PyramideApp();
		virtual ~PyramideApp() {}
	protected:
		ComPtr<ID3DX11Effect> mEffect_;
		ComPtr<ID3DX11EffectTechnique> mTech_;
		ComPtr<ID3DX11EffectMatrixVariable> mWorldMatrix_;
		ComPtr<ID3DX11EffectScalarVariable> mTime_;
		

		ComPtr<ID3D11Buffer> mVertexBuffer_;
		ComPtr<ID3D11Buffer> mIndexBuffer_;
		ComPtr<ID3D11InputLayout> mInputLayout_;

		std::vector<XMFLOAT4X4> mPyramideTransforms;
		XMFLOAT4X4 mProj;
		XMFLOAT4X4 mView;

		float m_Theta;
		float m_Phi;
		float m_Radius;

		std::pair<int, int> m_LastMousePos;

		UINT mPyramidIndexCount = 0;
		UINT mBoxIndexCount = 0;

		UINT mPyramidVertexOffset = 0;
		UINT mBoxVertexOffset = 0;

		UINT mPyramidIndexOffset = 0;
		UINT mBoxIndexOffset = 0;

		void PlacePyramidInTheWorld(uint32_t PyramideCount);
		void CreateInputLayout();
		void CreateGeometryBuffers();

		void Init() override;
		void PollEvents() override;
		void UpdateScene(float delta) override;
		void DrawScene() override;
	};
}