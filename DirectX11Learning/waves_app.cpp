#include "waves_app.hpp"

#include <algorithm>
#include <vector>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include "lea_timer.hpp"
#include "lea_engine_utils.hpp"
#include "DXHelper.hpp"

using lea::utils::Vertex1;
using namespace DirectX;

namespace lea {
	WavesApp::WavesApp()
		: App(), mGridIndexCount(0), mTheta(1.5f * XM_PI), mPhi(0.1f * XM_PI), mRadius(200.0f)
	{
		m_LastMousePos.first = 0;
		m_LastMousePos.second = 0;

		XMMATRIX I = XMMatrixIdentity();

		XMStoreFloat4x4(&mGridWorld, I);
		XMStoreFloat4x4(&mGridWaves, I);
		XMStoreFloat4x4(&mView, I);
		XMStoreFloat4x4(&mProj, I);
	}
	void WavesApp::Init()
	{
		waves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

		effect_ = device_.CreateEffect(L"terrain_shader.fx");
		effectTechnique_ = effect_->GetTechniqueByName("ColorTech");
		worldMatrix_ = effect_->GetVariableByName("gWorldProjectView")->AsMatrix();

		CreateInputLayout();
		BuildLandGeometryBuffers();
		BuildWavesGeometryBuffers();

		device_.Context()->IASetInputLayout(inputLayout_.Get());
		device_.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D11_RASTERIZER_DESC rastDesc{};
		rastDesc.FillMode = D3D11_FILL_WIREFRAME;
		rastDesc.CullMode = D3D11_CULL_BACK;
		rastDesc.DepthClipEnable = true;
		rastDesc.FrontCounterClockwise = false;

		DX::ThrowIfFailed(device_.Device()->CreateRasterizerState(&rastDesc, rastState_.GetAddressOf()));

		XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * XM_PI,
			window_.AspectRatio(), 1.0f, 1000.0f);
		XMStoreFloat4x4(&mProj, P);
	}
	void WavesApp::PollEvents()
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
				mPhi = std::clamp(mPhi, 0.1f, XM_PI - 0.1f);
			}
			else if (event.key == LeaEvent::KeyRightMouse) {
				float dx = 0.2f * static_cast<float>(event.mouse_x - m_LastMousePos.first);
				float dy = 0.2f * static_cast<float>(event.mouse_y - m_LastMousePos.second);
				// Update the camera radius based on input.
				mRadius += dx - dy;
				// Restrict the radius.
				mRadius = std::clamp(mRadius, 50.0f, 500.0f);
			}
		}
		m_LastMousePos.first = event.mouse_x;
		m_LastMousePos.second = event.mouse_y;
	}
	void WavesApp::UpdateScene(float deltaTime)
	{
		// Convert Spherical to Cartesian coordinates.
		float x = mRadius * sinf(mPhi) * cosf(mTheta);
		float z = mRadius * sinf(mPhi) * sinf(mTheta);
		float y = mRadius * cosf(mPhi);

		// Build the view matrix.
		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&mView, V);

		//
		// Every quarter second, generate a random wave.
		//
		static float t_base = 0.0f;
		if ((TIMER.TotalTime() - t_base) >= 0.25f)
		{
			t_base += 0.25f;

			DWORD i = 5 + rand() % 190;
			DWORD j = 5 + rand() % 190;

			float r = lea::utils::MathHelper::RandF(1.0f, 2.0f);

			waves.Disturb(i, j, r);
		}

		waves.Update(deltaTime);

		//
		// Update the wave vertex buffer with the new solution.
		//

		D3D11_MAPPED_SUBRESOURCE mappedData;
		DX::ThrowIfFailed(device_.Context()->Map(wavesVertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

		Vertex1* v = reinterpret_cast<Vertex1*>(mappedData.pData);
		for (UINT i = 0; i < waves.VertexCount(); ++i)
		{
			v[i].pos = waves[i];
			v[i].color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		device_.Context()->Unmap(wavesVertexBuffer_.Get(), 0);


	}
	void WavesApp::BuildLandGeometryBuffers()
	{
		using namespace lea::utils;
		GeometryGenerator::MeshData grid;

		GeometryGenerator geoGen;

		geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

		mGridIndexCount = grid.Indices.size();

		//
		// Extract the vertex elements we are interested and apply the height function to
		// each vertex.  In addition, color the vertices based on their height so we have
		// sandy looking beaches, grassy low hills, and snow mountain peaks.
		//

		std::vector<Vertex1> vertices(grid.Vertices.size());
		for (size_t i = 0; i < grid.Vertices.size(); ++i)
		{
			XMFLOAT3 p = grid.Vertices[i].Position;

			p.y = GetHeight(p.x, p.z);

			vertices[i].pos = p;

			// Color the vertex based on its height.
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

		D3D11_BUFFER_DESC vbd{};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex1) * grid.Vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vinitData{};
		vinitData.pSysMem = vertices.data();
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&vbd, &vinitData, landVertexBuffer_.GetAddressOf()));

		//
		// Pack the indices of all the meshes into one index buffer.
		//

		D3D11_BUFFER_DESC ibd{};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * mGridIndexCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData{};
		iinitData.pSysMem = grid.Indices.data();
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&ibd, &iinitData, landIndexBuffer_.GetAddressOf()));

	}
	void WavesApp::BuildWavesGeometryBuffers()
	{
		D3D11_BUFFER_DESC vbd{};
		vbd.Usage = D3D11_USAGE_DYNAMIC;
		vbd.ByteWidth = sizeof(Vertex1) * waves.VertexCount();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vbd.MiscFlags = 0;
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&vbd, 0, wavesVertexBuffer_.GetAddressOf()));


		// Create the index buffer.  The index buffer is fixed, so we only 
		// need to create and set once.

		std::vector<UINT> indices(3 * waves.TriangleCount()); // 3 indices per face

		// Iterate over each quad.
		UINT m = waves.RowCount();
		UINT n = waves.ColumnCount();
		int k = 0;
		for (UINT i = 0; i < m - 1; ++i)
		{
			for (DWORD j = 0; j < n - 1; ++j)
			{
				indices[k] = i * n + j;
				indices[k + 1] = i * n + j + 1;
				indices[k + 2] = (i + 1) * n + j;

				indices[k + 3] = (i + 1) * n + j;
				indices[k + 4] = i * n + j + 1;
				indices[k + 5] = (i + 1) * n + j + 1;

				k += 6; // next quad
			}
		}

		D3D11_BUFFER_DESC ibd{};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * indices.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData{};
		iinitData.pSysMem = indices.data();
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&ibd, &iinitData, wavesIndexBuffer_.GetAddressOf()));
	}
	void WavesApp::CreateInputLayout()
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

	inline float WavesApp::GetHeight(float x, float z) const
	{
		return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
	}

	void WavesApp::DrawScene()
	{
		auto context = device_.Context();

		context->ClearRenderTargetView(device_.RenderTargetView(), reinterpret_cast<const float*>(&Colors::MidnightBlue));
		context->ClearDepthStencilView(device_.DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		UINT strides = sizeof(Vertex1);
		UINT offset = 0;

		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX viewProj = view * proj;

		D3DX11_TECHNIQUE_DESC techDesc;
		effectTechnique_->GetDesc(&techDesc);
		for (UINT i = 0; i < techDesc.Passes; ++i)
		{
			// grid drawing
			context->IASetVertexBuffers(0, 1, landVertexBuffer_.GetAddressOf(), &strides, &offset);
			context->IASetIndexBuffer(landIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

			XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
			XMMATRIX worldViewProj = world * viewProj;
			worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&worldViewProj));
			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(mGridIndexCount, 0, 0);

			// waves drawing

			context->RSSetState(rastState_.Get());


			context->IASetVertexBuffers(0, 1, wavesVertexBuffer_.GetAddressOf(), &strides, &offset);
			context->IASetIndexBuffer(wavesIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

			world = XMLoadFloat4x4(&mGridWaves);
			worldViewProj = world * viewProj;
			worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&worldViewProj));
			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(3*waves.TriangleCount(), 0, 0);


			context->RSSetState(0);
		}

		DX::ThrowIfFailed(device_.SwapChain()->Present(0, 0));
	}
	
}