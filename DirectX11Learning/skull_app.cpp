#include "skull_app.hpp"

#include <algorithm>
#include <fstream>
#include <vector>

#include <DirectXColors.h>

#include "DXHelper.hpp"
#include "lea_engine_utils.hpp"

using namespace DirectX;
using lea::utils::Vertex1;

namespace lea {
	SkullApp::SkullApp()
		: App(), m_Theta(1.5f * XM_PI), m_Phi(0.25f * XM_PI), m_Radius(5.0f)
	{
		m_LastMousePos.first = 0;
		m_LastMousePos.second = 0;

		auto I = XMMatrixIdentity();
		XMStoreFloat4x4(&mView, I);
		XMStoreFloat4x4(&mProj, I);
		XMMATRIX T = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
		XMStoreFloat4x4(&mWorld, T);
	}
	void SkullApp::Init()
	{
		effect_ = device_.CreateEffect(L"terrain_shader.fx");
		effectTechnique_ = effect_->GetTechniqueByName("ColorTech");
		worldMatrix_ = effect_->GetVariableByName("gWorldProjectView")->AsMatrix();

		CreateInputLayout();
		BuildGeometryBuffers();

		auto context = device_.Context();
		context->IASetInputLayout(inputLayout_.Get());

		UINT strides = sizeof(Vertex1);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &strides, &offset);

		context->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		auto proj = XMMatrixPerspectiveFovLH(0.25f * XM_PI, window_.AspectRatio(), 1.f, 100.f);
		XMStoreFloat4x4(&mProj, proj);
	}
	void SkullApp::UpdateScene(float deltaTime)
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
	void SkullApp::PollEvents()
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
	void SkullApp::DrawScene()
	{
		auto context = device_.Context();
		context->ClearRenderTargetView(device_.RenderTargetView(), reinterpret_cast<const float*>(&Colors::MidnightBlue));
		context->ClearDepthStencilView(device_.DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		XMMATRIX model = XMLoadFloat4x4(&mWorld);
		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX finalTransform = model * view * proj;
		worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalTransform));

		D3DX11_TECHNIQUE_DESC techDesc;
		effectTechnique_->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			effectTechnique_->GetPassByIndex(p)->Apply(0, context);
			
			context->DrawIndexed(mSkullIndexCount, 0, 0);
		}

		DX::ThrowIfFailed(device_.SwapChain()->Present(0, 0));
	}
	void SkullApp::BuildGeometryBuffers()
	{
		std::ifstream fin("Models/skull.txt");

		if (!fin)
		{
			MessageBox(0, L"Models/skull.txt not found.", 0, 0);
			return;
		}

		UINT vcount = 0;
		UINT tcount = 0;
		std::string ignore;

		fin >> ignore >> vcount;
		fin >> ignore >> tcount;
		fin >> ignore >> ignore >> ignore >> ignore;

		float nx, ny, nz;
		XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

		std::vector<Vertex1> vertices(vcount);
		for (UINT i = 0; i < vcount; ++i)
		{
			fin >> vertices[i].pos.x >> vertices[i].pos.y >> vertices[i].pos.z;

			vertices[i].color = black;

			// Normal not used in this demo.
			fin >> nx >> ny >> nz;
		}

		fin >> ignore;
		fin >> ignore;
		fin >> ignore;

		mSkullIndexCount = 3 * tcount;
		std::vector<UINT> indices(mSkullIndexCount);
		for (UINT i = 0; i < tcount; ++i)
		{
			fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
		}

		fin.close();

		D3D11_BUFFER_DESC vbd{};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex1) * vcount;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vinitData{};
		vinitData.pSysMem = vertices.data();
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&vbd, &vinitData, vertexBuffer_.GetAddressOf()));

		//
		// Pack the indices of all the meshes into one index buffer.
		//

		D3D11_BUFFER_DESC ibd{};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData{};
		iinitData.pSysMem = indices.data();
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&ibd, &iinitData, indexBuffer_.GetAddressOf()));
	}
	void SkullApp::CreateInputLayout()
	{
		D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		D3DX11_PASS_DESC passDesc;
		effectTechnique_->GetPassByIndex(0)->GetDesc(&passDesc);

		DX::ThrowIfFailed(
			device_.Device()->CreateInputLayout(vertexLayout, ARRAYSIZE(vertexLayout), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, inputLayout_.GetAddressOf()));
	}
}