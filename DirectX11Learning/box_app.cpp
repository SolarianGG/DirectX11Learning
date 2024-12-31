#include "box_app.hpp"

#include <algorithm>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include "lea_timer.hpp"
#include "lea_engine_utils.hpp"
#include "DXHelper.hpp"
using lea::utils::Vertex2;
using namespace DirectX;

lea::BoxApp::BoxApp()
	: App(), m_Theta(1.5f * XM_PI), m_Phi(0.25f * XM_PI), m_Radius(5.0f)
{
	mLastMousePos.first = 0;
	mLastMousePos.second = 0;

	XMMATRIX I = XMMatrixIdentity();

	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	mBoxMaterial.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f);
	mBoxMaterial.Diffuse = XMFLOAT4(0.f, 0.8f, 0.2f, 1.f);
	mBoxMaterial.Specular = XMFLOAT4(1.f, 1.f, 1.f, 256.f);

	mPointLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f);
	mPointLight.Diffuse = XMFLOAT4(1.f, 0.8f, 0.6f, 1.f);
	mPointLight.Specular = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	mPointLight.Position = XMFLOAT3(0.f, 3.f, 0.f);
	mPointLight.Range = 20.f;
	mPointLight.Att = XMFLOAT3(0.f, 0.1f, 0.01f);



	mEyePosition = XMFLOAT3(0.f, 0.f, 0.f);

	mDirectionalLight.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.f);
	mDirectionalLight.Diffuse = XMFLOAT4(0.8f, 0.0f, 1.0f, 1.f);
	mDirectionalLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.f);
	mDirectionalLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
	
}
void lea::BoxApp::Init()
{
	mEffect_ = device_.CreateEffect(L"box_light.fx");
	mEffectTechnique_ = mEffect_->GetTechniqueByName("LightTech");

	mWorldViewProj_ = mEffect_->GetVariableByName("gWorldProjectView")->AsMatrix();
	mWorld_ = mEffect_->GetVariableByName("gWorld")->AsMatrix();
	mWorldInverseTranspose_ = mEffect_->GetVariableByName("gWorldInverseTranspose")->AsMatrix();

	mEyePosition_ = mEffect_->GetVariableByName("gEyePosW")->AsVector();

	mMaterial_ = mEffect_->GetVariableByName("gMat");
	mPointLight_ = mEffect_->GetVariableByName("gPointLight");
	mDirectionalLight_ = mEffect_->GetVariableByName("gDirectionalLight");

	CreateGeometryBuffers();
	CreateInputLayout();

	device_.Context()->IASetInputLayout(mIputLayout_.Get());
	device_.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;

	DX::ThrowIfFailed(device_.Device()->CreateRasterizerState(&rasterizerDesc, mRastState_.GetAddressOf()));

	UINT stride = sizeof(Vertex2);
	UINT offset1 = 0;
	device_.Context()->IASetVertexBuffers(0, 1, mVertexBuffer_.GetAddressOf(), &stride, &offset1);
	device_.Context()->IASetIndexBuffer(mIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * XM_PI,
		window_.AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void lea::BoxApp::PollEvents()
{
	LeaEvent event = window_.GetCurrentEvent();

	if (event.type == LeaEvent::TypeMouseDown) {
		mLastMousePos.first = event.mouse_x;
		mLastMousePos.second = event.mouse_y;
	}
	else if (event.type == LeaEvent::TypeMouseMotion) {
		if (event.key == LeaEvent::KeyLeftMouse) {
			float dx = XMConvertToRadians(
				0.25f * static_cast<float>(event.mouse_x - mLastMousePos.first));
			float dy = XMConvertToRadians(
				0.25f * static_cast<float>(event.mouse_y - mLastMousePos.second));
			// Update angles based on input to orbit camera around box.
			m_Theta += dx;
			m_Phi += dy;
			// Restrict the angle mPhi.
			m_Phi = std::clamp(m_Phi, 0.1f, XM_PI - 0.1f);
		}
		else if (event.key == LeaEvent::KeyRightMouse) {
			float dx = 0.005f * static_cast<float>(event.mouse_x - mLastMousePos.first);
			float dy = 0.005f * static_cast<float>(event.mouse_y - mLastMousePos.second);
			// Update the camera radius based on input.
			m_Radius += dx - dy;
			// Restrict the radius.
			m_Radius = std::clamp(m_Radius, 3.0f, 15.0f);
		}

		mLastMousePos.first = event.mouse_x;
		mLastMousePos.second = event.mouse_y;
	}
}

void lea::BoxApp::UpdateScene(float deltaTime)
{
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);
	mEyePosition = XMFLOAT3(x, y, z);
	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
	
	mPointLight.Position.x = 5.0f * cosf(XM_2PI / 5.0f * TIMER.TotalTime());
	mPointLight.Position.z = 5.0f * sinf(XM_2PI / 5.0f * TIMER.TotalTime());
}

void lea::BoxApp::DrawScene()
{
	auto context = device_.Context();

	context->ClearRenderTargetView(device_.RenderTargetView(), reinterpret_cast<const float*>(&DirectX::Colors::MidnightBlue));
	context->ClearDepthStencilView(device_.DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldFinal = world*view*proj;

	XMVECTOR det = XMMatrixDeterminant(world);
	XMMATRIX worldInverseTranspose = XMMatrixTranspose(XMMatrixInverse(&det, world));
	
	mWorldViewProj_->SetMatrix(reinterpret_cast<const float*>(&worldFinal));
	mWorld_->SetMatrix(reinterpret_cast<const float*>(&world));
	mWorldInverseTranspose_->SetMatrix(reinterpret_cast<const float*>(&worldInverseTranspose));
	mMaterial_->SetRawValue(reinterpret_cast<const void*>(&mBoxMaterial), 0, sizeof(mBoxMaterial));
	mPointLight_->SetRawValue(reinterpret_cast<const void*>(&mPointLight), 0, sizeof(mPointLight));
	mDirectionalLight_->SetRawValue(reinterpret_cast<const void*>(&mDirectionalLight), 0, sizeof(mDirectionalLight));

	mEyePosition_->SetFloatVector(reinterpret_cast<const float*>(&mEyePosition));
		
	D3DX11_TECHNIQUE_DESC techDesc;
	mEffectTechnique_->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		device_.Context()->RSSetState(mRastState_.Get());

		mEffectTechnique_->GetPassByIndex(p)->Apply(0, context);

		context->DrawIndexed(mBoxIndexCount, 0, 0);
		device_.Context()->RSSetState(0);
	}

	DX::ThrowIfFailed(device_.SwapChain()->Present(0, 0));

}

void lea::BoxApp::CreateGeometryBuffers()
{
	Vertex2 vertices[] =
	{
		{XMFLOAT3(-.5f, -.5f, -.5f), XMFLOAT3(0.f, 0.f, 0.f)},
		{XMFLOAT3(-.5f, +.5f, -.5f), XMFLOAT3(0.f, 0.f, 0.f)},
		{XMFLOAT3(+.5f, +.5f, -.5f), XMFLOAT3(0.f, 0.f, 0.f)},
		{XMFLOAT3(+.5f, -.5f, -.5f), XMFLOAT3(0.f, 0.f, 0.f)},
		{XMFLOAT3(-.5f, -.5f, +.5f), XMFLOAT3(0.f, 0.f, 0.f)},
		{XMFLOAT3(-.5f, +.5f, +.5f), XMFLOAT3(0.f, 0.f, 0.f)},
		{XMFLOAT3(+.5f, +.5f, +.5f), XMFLOAT3(0.f, 0.f, 0.f)},
		{XMFLOAT3(+.5f, -.5f, +.5f), XMFLOAT3(0.f, 0.f, 0.f)}
	};
	UINT verticesSize = ARRAYSIZE(vertices);

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

	mBoxIndexCount = ARRAYSIZE(indexes);

	for (UINT i = 0; i < mBoxIndexCount / 3; ++i)
	{
		UINT i0 = indexes[i * 3 + 0];
		UINT i1 = indexes[i * 3 + 1];
		UINT i2 = indexes[i * 3 + 2];

		Vertex2& v0 = vertices[i0];
		Vertex2& v1 = vertices[i1];
		Vertex2& v2 = vertices[i2];

		XMVECTOR v0Pos = XMLoadFloat3(&v0.pos);
		XMVECTOR v1Pos = XMLoadFloat3(&v1.pos);
		XMVECTOR v2Pos = XMLoadFloat3(&v2.pos);

		XMVECTOR e0 = v1Pos - v0Pos;
		XMVECTOR e1 = v2Pos - v0Pos;
		XMVECTOR faceNormalVec = XMVector3Cross(e0, e1);
		
		XMVECTOR v0Norm = XMLoadFloat3(&v0.norm);
		XMVECTOR v1Norm = XMLoadFloat3(&v1.norm);
		XMVECTOR v2Norm = XMLoadFloat3(&v2.norm);

		v0Norm += faceNormalVec;
		v1Norm += faceNormalVec;
		v2Norm += faceNormalVec;

		XMStoreFloat3(&v0.norm, v0Norm);
		XMStoreFloat3(&v1.norm, v1Norm);
		XMStoreFloat3(&v2.norm, v2Norm);
	}

	for (UINT i = 0; i < verticesSize; ++i)
	{
		XMVECTOR normVec = XMLoadFloat3(&vertices[i].norm);
		XMStoreFloat3(&vertices[i].norm, XMVector3Normalize(normVec));
	}


	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = sizeof(Vertex2) * verticesSize;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubData{};
	vertexSubData.pSysMem = vertices;
	DX::ThrowIfFailed(device_.Device()->CreateBuffer(&vertexBufferDesc, &vertexSubData, mVertexBuffer_.GetAddressOf()));


	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(UINT) * mBoxIndexCount;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexSubData{};
	indexSubData.pSysMem = indexes;
	DX::ThrowIfFailed(device_.Device()->CreateBuffer(&indexBufferDesc, &indexSubData, mIndexBuffer_.GetAddressOf()));
}

void lea::BoxApp::CreateInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC inputs[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	D3DX11_PASS_DESC passDesc;
	mEffectTechnique_->GetPassByIndex(0)->GetDesc(&passDesc);
	DX::ThrowIfFailed(
		device_.Device()->CreateInputLayout
		(inputs, ARRAYSIZE(inputs), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, mIputLayout_.GetAddressOf()));

}
