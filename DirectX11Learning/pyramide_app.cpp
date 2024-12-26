#include "pyramide_app.hpp"

#include <algorithm>
#include <random>
#include <ctime>

#include <DirectXColors.h>

#include "lea_timer.hpp"


#include "DXHelper.hpp"

using namespace DirectX;
using lea::utils::Vertex1;

namespace lea {
	PyramideApp::PyramideApp()
		: App(), m_Theta(1.5f * XM_PI), m_Phi(0.25f * XM_PI), m_Radius(5.0f)
	{
		m_LastMousePos.first = 0;
		m_LastMousePos.second = 0;

		auto I = XMMatrixIdentity();
		XMStoreFloat4x4(&mView, I);
		XMStoreFloat4x4(&mProj, I);
	}
	void PyramideApp::Init()
	{
		std::srand(std::time(nullptr));
		mEffect_ = device_.CreateEffect(L"pyramide_shader.fx");
		mTech_ = mEffect_->GetTechniqueByName("PyramideTech");

		mWorldMatrix_ = mEffect_->GetVariableByName("gWorldMatrix")->AsMatrix();
		mTime_ = mEffect_->GetVariableByName("gTime")->AsScalar();

		CreateInputLayout();
		CreateGeometryBuffers();
		PlacePyramidInTheWorld(16);

		auto context = device_.Context();
		context->IASetInputLayout(mInputLayout_.Get());

		UINT strides = sizeof(Vertex1);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, mVertexBuffer_.GetAddressOf(), &strides, &offset);

		context->IASetIndexBuffer(mIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
		auto proj = XMMatrixPerspectiveFovLH(0.25f * XM_PI, window_.AspectRatio(), 1.f, 100.f);
		XMStoreFloat4x4(&mProj, proj);
	}
	void PyramideApp::PlacePyramidInTheWorld(uint32_t PyramideCount)
	{
		constexpr float LO = -10.f;
		constexpr float HI = 10.f;
		mPyramideTransforms.reserve(PyramideCount);
		for (UINT i = 0; i < PyramideCount; ++i)
		{
			float tx = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
			float ty = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
			float tz = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
			XMFLOAT4X4 tMat;
			XMStoreFloat4x4(&tMat, XMMatrixTranslation(tx, ty, tz));
			mPyramideTransforms.push_back(tMat);
		}
	}

	void PyramideApp::CreateInputLayout()
	{
		D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		D3DX11_PASS_DESC passDesc;
		mTech_->GetPassByIndex(0)->GetDesc(&passDesc);

		DX::ThrowIfFailed(
		device_.Device()->CreateInputLayout(vertexLayout, ARRAYSIZE(vertexLayout), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, mInputLayout_.GetAddressOf()));
	}
	void PyramideApp::CreateGeometryBuffers()
	{
		XMFLOAT4 greenColor = XMFLOAT4(0.f, 1.f, 0.f, 1.f);
		XMFLOAT4 redColor = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
		Vertex1 pyramidVertexes[] = {
			{XMFLOAT3(0.f, 0.5f, 0.f), redColor },
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), greenColor },
			{XMFLOAT3(-0.5f, -0.5f, 0.5f), greenColor },
			{XMFLOAT3(0.5f, -0.5f, -0.5f),greenColor },
			{XMFLOAT3(0.5f, -0.5f, 0.5f), greenColor },
		};

		UINT pyramidIndexes[] = {
			2, 0, 1,
			1, 0, 3,
			3, 0, 4,
			4, 0, 2,

			2, 1, 4,
			1, 3, 4

		};

		Vertex1 boxVertexes[] =
		{
		{ XMFLOAT3(-.5f, -.5f, -.5f),  XMFLOAT4(1.f, 0.f, 0.f, 1.f)},
		{ XMFLOAT3(-.5f, +.5f, -.5f), XMFLOAT4(1.f, 1.f, 0.f, 1.f) },
		{ XMFLOAT3(+.5f, +.5f, -.5f), XMFLOAT4(1.f, 0.f, 1.f, 1.f) },
		{ XMFLOAT3(+.5f, -.5f, -.5f), XMFLOAT4(1.f, 1.f, 0.f, 1.f) },
		{ XMFLOAT3(-.5f, -.5f, +.5f), XMFLOAT4(1.f, 0.f, 1.f, 1.f) },
		{ XMFLOAT3(-.5f, +.5f, +.5f), XMFLOAT4(1.f, 1.f, 1.f, 1.f) },
		{ XMFLOAT3(+.5f, +.5f, +.5f), XMFLOAT4(1.f, 0.5f, 0.3f, 1.f) },
		{ XMFLOAT3(+.5f, -.5f, +.5f), XMFLOAT4(0.3f, 0.6f, 0.2f, 1.f) }
		};

		UINT boxIndexes[] = {
			// front face
			0, 1, 2,
			0, 2, 3,
			// back face
			4, 6, 5,
			4, 7, 6,
			// left face
			4, 5, 1,
			4, 1, 0,
			// right face
			3, 2, 6,
			3, 6, 7,
			// top face
		   1, 5, 6,
			1, 6, 2,
			// bottom face
			4, 0, 3,
			4, 3, 7
		};

		mPyramidIndexCount = ARRAYSIZE(pyramidIndexes);
		mBoxIndexCount = ARRAYSIZE(boxIndexes);

		mBoxVertexOffset = ARRAYSIZE(pyramidVertexes);
		mBoxIndexOffset = ARRAYSIZE(pyramidIndexes);

		UINT allVertexCount = ARRAYSIZE(pyramidVertexes) + ARRAYSIZE(boxVertexes);
		std::vector<Vertex1> allVertexes;
		allVertexes.reserve(allVertexCount);

		for (UINT i = 0; i < ARRAYSIZE(pyramidVertexes); ++i)
		{
			allVertexes.push_back(pyramidVertexes[i]);
		}

		for (UINT i = 0; i < ARRAYSIZE(boxVertexes); ++i)
		{
			allVertexes.push_back(boxVertexes[i]);
		}

		D3D11_BUFFER_DESC vertexBufferDesc{};
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = sizeof(Vertex1) * allVertexes.size();
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubresourceDesc{};
		vertexSubresourceDesc.pSysMem = allVertexes.data();
		
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&vertexBufferDesc, &vertexSubresourceDesc, mVertexBuffer_.GetAddressOf()));

		std::vector<UINT> allIndexes;
		
		allIndexes.reserve(mPyramidIndexCount + mBoxIndexCount);

		for (UINT i = 0; i < mPyramidIndexCount; ++i)
		{
			allIndexes.push_back(pyramidIndexes[i]);
		}

		for (UINT i = 0; i < mBoxIndexCount; ++i)
		{
			allIndexes.push_back(boxIndexes[i]);
		}
		
		D3D11_BUFFER_DESC indexBufferDesc{};
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = sizeof(UINT) * allIndexes.size();
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubresourceDesc{};
		indexSubresourceDesc.pSysMem = allIndexes.data();

		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&indexBufferDesc, &indexSubresourceDesc, mIndexBuffer_.GetAddressOf()));
	}
	
	void PyramideApp::PollEvents()
	{
		LeaEvent event = window_.GetCurrentEvent();

		if (event.type == LeaEvent::TypeMouseDown) {
			m_LastMousePos.first = event.mouse_x;
			m_LastMousePos.second = event.mouse_y;
		}
		else if (event.type == LeaEvent::TypeMouseMotion) {
			if (event.key == LeaEvent::KeyLeftMouse) {
				float dx = XMConvertToRadians(
					0.25f * static_cast<float>(event.mouse_x - m_LastMousePos.first));
				float dy = XMConvertToRadians(
					0.25f * static_cast<float>(event.mouse_y - m_LastMousePos.second));
				// Update angles based on input to orbit camera around box.
				m_Theta += dx;
				m_Phi += dy;
				// Restrict the angle mPhi.
				m_Phi = std::clamp(m_Phi, 0.1f, XM_PI - 0.1f);
			}
			else if (event.key == LeaEvent::KeyRightMouse) {
				float dx = 0.005f * static_cast<float>(event.mouse_x - m_LastMousePos.first);
				float dy = 0.005f * static_cast<float>(event.mouse_y - m_LastMousePos.second);
				// Update the camera radius based on input.
				m_Radius += dx - dy;
				// Restrict the radius.
				m_Radius = std::clamp(m_Radius, 3.0f, 15.0f);
			}
		}
		m_LastMousePos.first = event.mouse_x;
		m_LastMousePos.second = event.mouse_y;
	}
	void PyramideApp::UpdateScene(float delta)
	{
		float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
		float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
		float y = m_Radius * cosf(m_Phi);
		// Build the view matrix.
		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&mView, V);
	}
	void PyramideApp::DrawScene()
	{
		auto context = device_.Context();

		context->ClearRenderTargetView(device_.RenderTargetView(), reinterpret_cast<const float*>(&DirectX::Colors::MidnightBlue));
		context->ClearDepthStencilView(device_.DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.f);

		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX viewProj = view * proj;

		mTime_->SetFloat(TIMER.TotalTime());
		// mTech_->GetPassByIndex(0)->Apply(0, context);
		
		D3DX11_TECHNIQUE_DESC techDesc;
		mTech_->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			for (UINT i = 0; i < mPyramideTransforms.size(); ++i)
			{
				XMMATRIX currentPyramidTransform = XMLoadFloat4x4(&mPyramideTransforms[i]);
				XMMATRIX finalMat = currentPyramidTransform * viewProj;
				mWorldMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMat));
				mTech_->GetPassByIndex(p)->Apply(0, context);
				if (i % 2 == 0)
				{
					context->DrawIndexed(mPyramidIndexCount, mPyramidIndexOffset, mPyramidVertexOffset);
				}
				else {
					context->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
				}
			}
		}

		DX::ThrowIfFailed(device_.SwapChain()->Present(0, 0));
	}
}