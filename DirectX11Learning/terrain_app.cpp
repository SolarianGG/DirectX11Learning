#include "terrain_app.hpp"

#include <algorithm>
#include <vector>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include "lea_timer.hpp"
#include "lea_engine_utils.hpp"
#include "DXHelper.hpp"
using lea::utils::Vertex1;
using namespace DirectX;

constexpr auto PI = 3.14f;

namespace lea {
	TerrainApp::TerrainApp() 
		: App(), mTheta(1.5f * PI), mPhi(0.1f * PI), mRadius(200.0f)
	{
		m_LastMousePos.first = 0;
		m_LastMousePos.second = 0;

		XMMATRIX I = XMMatrixIdentity();

		XMStoreFloat4x4(&mGridWorld, I);
		XMStoreFloat4x4(&mView, I);
		XMStoreFloat4x4(&mProj, I);
	}
	void TerrainApp::Init()
	{
		effect_ = device_.CreateEffect(L"terrain_shader.fx");
		effectTechnique_ = effect_->GetTechniqueByName("ColorTech");

		worldMatrix_ = effect_->GetVariableByName("gWorldProjectView")->AsMatrix();

		CreateGeometryBuffers();
		CreateInputLayout();

		device_.Context()->IASetInputLayout(inputLayout_.Get());
		device_.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT stride = sizeof(Vertex1);
		UINT offset = 0;
		ID3D11Buffer* const buffers[] = { vertexBuffer_.Get() };
		device_.Context()->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
		device_.Context()->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * PI,
			window_.AspectRatio(), 1.0f, 1000.0f);
		XMStoreFloat4x4(&mProj, P);
	}

	void TerrainApp::PollEvents()
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
				mTheta += dx;
				mPhi += dy;
				// Restrict the angle mPhi.
				mPhi = std::clamp(mPhi, 0.1f, PI - 0.1f);
			}
			else if (event.key == LeaEvent::KeyRightMouse) {
				float dx = 0.005f * static_cast<float>(event.mouse_x - m_LastMousePos.first);
				float dy = 0.005f * static_cast<float>(event.mouse_y - m_LastMousePos.second);
				// Update the camera radius based on input.
				mRadius += dx - dy;
				// Restrict the radius.
				mRadius = std::clamp(mRadius, 3.0f, 15.0f);
			}
		}
		m_LastMousePos.first = event.mouse_x;
		m_LastMousePos.second = event.mouse_y;
	}

	void TerrainApp::UpdateScene(float deltaTime)
	{
		float x = mRadius * sinf(mPhi) * cosf(mTheta);
		float z = mRadius * sinf(mPhi) * sinf(mTheta);
		float y = mRadius * cosf(mPhi);
		// Build the view matrix.
		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&mView, V);
	}

	void TerrainApp::DrawScene()
	{
		auto context = device_.Context();

		context->ClearRenderTargetView(device_.RenderTargetView(), reinterpret_cast<const float*>(&DirectX::Colors::MidnightBlue));
		context->ClearDepthStencilView(device_.DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX worldFinal = world * view * proj;
		
		D3DX11_TECHNIQUE_DESC techDesc;
		effectTechnique_->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&worldFinal));
			effectTechnique_->GetPassByIndex(p)->Apply(0, context);

			context->DrawIndexed(mGridIndexCount, 0, 0);
		}

		DX::ThrowIfFailed(device_.SwapChain()->Present(0, 0));
	}

	void TerrainApp::CreateGeometryBuffers()
	{
		utils::GeometryGenerator::MeshData grid;

		utils::GeometryGenerator geoGen;

		geoGen.CreateGrid(160.f, 160.f, 50, 50, grid);

		std::vector<Vertex1> vertices(grid.Vertices.size());
		for (uint32_t i = 0; i < grid.Vertices.size(); ++i)
		{
			XMFLOAT3 p = grid.Vertices[i].Position;

			p.y = GetHeight(p.x, p.z);

			vertices[i].pos = p;

			if (p.y < -10.0f)
			{
				// Sandy beach color.
				vertices[i].color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
			}
			else if (p.y < 5.0f)
			{
				// Light yellow-green.
				vertices[i].color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
			}
			else if (p.y < 12.0f)
			{
				// Dark yellow-green.
				vertices[i].color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
			}
			else if (p.y < 20.0f)
			{
				// Dark brown.
				vertices[i].color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
			}
			else
			{
				// White snow.
				vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}

		}
		D3D11_BUFFER_DESC vertexBufferDesc{};
		vertexBufferDesc.ByteWidth = sizeof(Vertex1) * static_cast<uint32_t>(vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubData{};
		vertexSubData.pSysMem = vertices.data();

		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&vertexBufferDesc, &vertexSubData, vertexBuffer_.GetAddressOf()));

		mGridIndexCount = static_cast<uint32_t>(grid.Indices.size());
		D3D11_BUFFER_DESC indexBufferDesc{};
		indexBufferDesc.ByteWidth = sizeof(uint32_t) * mGridIndexCount;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubData{};
		indexSubData.pSysMem = grid.Indices.data();

		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&indexBufferDesc, &indexSubData, indexBuffer_.GetAddressOf()));
	}

	void TerrainApp::CreateInputLayout()
	{
		D3D11_INPUT_ELEMENT_DESC inputs[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		D3DX11_PASS_DESC passDesc;
		effectTechnique_->GetPassByIndex(0)->GetDesc(&passDesc);
		DX::ThrowIfFailed(
			device_.Device()->CreateInputLayout
			(inputs, ARRAYSIZE(inputs), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, inputLayout_.GetAddressOf()));
	}

	inline float TerrainApp::GetHeight(float x, float z) const
	{
		return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
	}


}

