#include "triangle_app.hpp"

#include <algorithm>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include "lea_timer.hpp"
#include "lea_engine_utils.hpp"
#include "DXHelper.hpp"
using lea::utils::Vertex1;
using namespace DirectX;

constexpr auto PI = 3.14f;

lea::TriangleApp::TriangleApp()
	: App(), m_Theta(1.5f*PI), m_Phi(0.25f * PI), m_Radius(5.0f)
{
	m_LastMousePos.first = 0;
	m_LastMousePos.second = 0;

	XMMATRIX I = XMMatrixIdentity();

	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

void lea::TriangleApp::Init()
{
	effect_ = device_.CreateEffect(L"simple_shader.fx");
	effectTechnique_ = effect_->GetTechniqueByName("ColorTech");

	worldMatrix_ = effect_->GetVariableByName("gWorldProjectView")->AsMatrix();

	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateInputLayout();

	device_.Context()->IASetInputLayout(inputLayout_.Get());
	device_.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex1);
	UINT offset = 0;
	ID3D11Buffer* const buffers[]  = { vertexBuffer_.Get() };
	device_.Context()->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
	device_.Context()->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * PI,
		window_.AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);

}

void lea::TriangleApp::PollEvents()
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
			m_Phi = std::clamp(m_Phi, 0.1f, PI - 0.1f);
		}
		else if (event.key == LeaEvent::KeyRightMouse) {
			float dx = 0.005f * static_cast<float>(event.mouse_x - m_LastMousePos.first);
			float dy = 0.005f * static_cast<float>(event.mouse_y - m_LastMousePos.second);
			// Update the camera radius based on input.
			m_Radius += dx - dy;
			// Restrict the radius.
			m_Radius = std::clamp(m_Radius, 3.0f, 15.0f);
		}

		m_LastMousePos.first = event.mouse_x;
		m_LastMousePos.second = event.mouse_y;
	}
}

void lea::TriangleApp::UpdateScene(float deltaTime)
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

void lea::TriangleApp::DrawScene()
{
	auto context = device_.Context();

	context->ClearRenderTargetView(device_.RenderTargetView(), reinterpret_cast<const float*>(&DirectX::Colors::MidnightBlue));
	context->ClearDepthStencilView(device_.DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldFinal = world*view*proj;

	worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&worldFinal));
	auto time = TIMER.TotalTime();
	effect_->GetVariableByName("time")->AsScalar()->SetFloat(time);

	D3DX11_TECHNIQUE_DESC techDesc;
	effectTechnique_->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		effectTechnique_->GetPassByIndex(p)->Apply(0, context);

		context->DrawIndexed(36, 0, 0);
	}

	DX::ThrowIfFailed(device_.SwapChain()->Present(0, 0));

}

inline void lea::TriangleApp::CreateVertexBuffer()
{
	Vertex1 vertices[] =
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
	UINT arrSize = ARRAYSIZE(vertices);

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(Vertex1) * arrSize;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA subData{};
	subData.pSysMem = vertices;
	DX::ThrowIfFailed(device_.Device()->CreateBuffer(&bufferDesc, &subData, vertexBuffer_.GetAddressOf()));
}

void lea::TriangleApp::CreateIndexBuffer()
{
	UINT indexes[] = {
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

	UINT arrSize = ARRAYSIZE(indexes);

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(UINT) * arrSize;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA subData{};
	subData.pSysMem = indexes;
	DX::ThrowIfFailed(device_.Device()->CreateBuffer(&bufferDesc, &subData, indexBuffer_.GetAddressOf()));
}

void lea::TriangleApp::CreateInputLayout()
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
