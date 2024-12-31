#include "waves_app.hpp"

#include <algorithm>
#include <vector>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include "lea_timer.hpp"
#include "lea_engine_utils.hpp"
#include "DXHelper.hpp"

using lea::utils::Vertex2;
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

		XMMATRIX wavesOffset = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
		XMStoreFloat4x4(&mGridWaves, wavesOffset);

		// Directional light.
		mDirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		mDirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		mDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		mDirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

		// Point light--position is changed every frame to animate in UpdateScene function.
		mPointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		mPointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
		mPointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
		mPointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
		mPointLight.Range = 25.0f;

		// Spot light--position and direction changed every frame to animate in UpdateScene function.
		mSpotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		mSpotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		mSpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mSpotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
		mSpotLight.Spot = 96.0f;
		mSpotLight.Range = 10000.0f;

		mLandMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		mLandMat.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

		mWavesMat.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
		mWavesMat.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
		mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);

	}
	void WavesApp::Init()
	{
		waves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

		effect_ = device_.CreateEffect(L"simple_light.fx");
		effectTechnique_ = effect_->GetTechniqueByName("LightTech");

		mfxWorldViewProj = effect_->GetVariableByName("gWorldViewProj")->AsMatrix();
		mfxWorld = effect_->GetVariableByName("gWorld")->AsMatrix();
		mfxWorldInvTranspose = effect_->GetVariableByName("gWorldInvTranspose")->AsMatrix();
		mfxEyePosW = effect_->GetVariableByName("gEyePosW")->AsVector();
		mfxDirLight = effect_->GetVariableByName("gDirLight");
		mfxPointLight = effect_->GetVariableByName("gPointLight");
		mfxSpotLight = effect_->GetVariableByName("gSpotLight");
		mfxMaterial = effect_->GetVariableByName("gMaterial");

		CreateInputLayout();
		BuildLandGeometryBuffers();
		BuildWavesGeometryBuffers();

		device_.Context()->IASetInputLayout(inputLayout_.Get());
		device_.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
		else if (event.type = LeaEvent::TypePressed)
		{
			if (event.key == LeaEvent::KeyRight)
			{
				mSpotLight.Spot += 1.0f;
			}
			else if (event.key == LeaEvent::KeyLeft)
			{
				mSpotLight.Spot -= 1.0f;
			}
		}
		m_LastMousePos.first = event.mouse_x;
		m_LastMousePos.second = event.mouse_y;
	}
	void WavesApp::UpdateScene(float deltaTime)
	{
		using namespace lea::utils;
		// Convert Spherical to Cartesian coordinates.
		float x = mRadius * sinf(mPhi) * cosf(mTheta);
		float z = mRadius * sinf(mPhi) * sinf(mTheta);
		float y = mRadius * cosf(mPhi);

		mEyePosW = XMFLOAT3(x, y, z);

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

			DWORD i = 5 + rand() % (waves.RowCount() - 10);
			DWORD j = 5 + rand() % (waves.ColumnCount() - 10);

			float r = MathHelper::RandF(1.0f, 2.0f);

			waves.Disturb(i, j, r);
		}

		waves.Update(deltaTime);

		//
		// Update the wave vertex buffer with the new solution.
		//

		D3D11_MAPPED_SUBRESOURCE mappedData;
		DX::ThrowIfFailed(device_.Context()->Map(wavesVertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

		Vertex2* v = reinterpret_cast<Vertex2*>(mappedData.pData);
		for (UINT i = 0; i < waves.VertexCount(); ++i)
		{
			v[i].pos = waves[i];
			v[i].norm = waves.Normal(i);
		}

		device_.Context()->Unmap(wavesVertexBuffer_.Get(), 0);

		//
		// Animate the lights.
		//

		// Circle light over the land surface.
		mPointLight.Position.x = 70.0f * cosf(0.2f * TIMER.TotalTime());
		mPointLight.Position.z = 70.0f * sinf(0.2f * TIMER.TotalTime());
		mPointLight.Position.y = MathHelper::Max(GetHeight(mPointLight.Position.x,
			mPointLight.Position.z), -3.0f) + 10.0f;


		// The spotlight takes on the camera position and is aimed in the
		// same direction the camera is looking.  In this way, it looks
		// like we are holding a flashlight.
		mSpotLight.Position = mEyePosW;
		XMStoreFloat3(&mSpotLight.Direction, XMVector3Normalize(target - pos));

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

		std::vector<Vertex2> vertices(grid.Vertices.size());
		for (size_t i = 0; i < grid.Vertices.size(); ++i)
		{
			XMFLOAT3 p = grid.Vertices[i].Position;

			p.y = GetHeight(p.x, p.z);

			vertices[i].pos = p;
			vertices[i].norm = GetHillNormal(p.x, p.z);
		}

		D3D11_BUFFER_DESC vbd{};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex2) * grid.Vertices.size();
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
		vbd.ByteWidth = sizeof(Vertex2) * waves.VertexCount();
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
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
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

	XMFLOAT3 WavesApp::GetHillNormal(float x, float z) const
	{
		XMFLOAT3 n(
			-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
			1.0f,
			-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

		XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
		XMStoreFloat3(&n, unitNormal);

		return n;
	}

	void WavesApp::DrawScene()
	{
		auto context = device_.Context();

		context->ClearRenderTargetView(device_.RenderTargetView(), reinterpret_cast<const float*>(&Colors::MidnightBlue));
		context->ClearDepthStencilView(device_.DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		UINT strides = sizeof(Vertex2);
		UINT offset = 0;

		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX viewProj = view * proj;

		mfxDirLight->SetRawValue(&mDirLight, 0, sizeof(mDirLight));
		mfxPointLight->SetRawValue(&mPointLight, 0, sizeof(mPointLight));
		mfxSpotLight->SetRawValue(&mSpotLight, 0, sizeof(mSpotLight));
		mfxEyePosW->SetRawValue(&mEyePosW, 0, sizeof(mEyePosW));

		D3DX11_TECHNIQUE_DESC techDesc;
		effectTechnique_->GetDesc(&techDesc);
		for (UINT i = 0; i < techDesc.Passes; ++i)
		{
			// grid drawing
			context->IASetVertexBuffers(0, 1, landVertexBuffer_.GetAddressOf(), &strides, &offset);
			context->IASetIndexBuffer(landIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

			XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
			XMMATRIX worldInvTrans = lea::utils::MathHelper::InverseTranspose(world);
			XMMATRIX worldViewProj = world * viewProj;

			mfxWorldViewProj->SetMatrix(reinterpret_cast<const float*>(&worldViewProj));
			mfxWorld->SetMatrix(reinterpret_cast<const float*>(&world));
			mfxWorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&worldInvTrans));
			mfxMaterial->SetRawValue(&mLandMat, 0, sizeof(mLandMat));

			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(mGridIndexCount, 0, 0);

			// waves drawing
			context->IASetVertexBuffers(0, 1, wavesVertexBuffer_.GetAddressOf(), &strides, &offset);
			context->IASetIndexBuffer(wavesIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

			world = XMLoadFloat4x4(&mGridWaves);
			worldInvTrans = lea::utils::MathHelper::InverseTranspose(world);
			worldViewProj = world * viewProj;
			mfxWorldViewProj->SetMatrix(reinterpret_cast<const float*>(&worldViewProj));
			mfxWorld->SetMatrix(reinterpret_cast<const float*>(&world));
			mfxWorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&worldInvTrans));
			mfxMaterial->SetRawValue(&mWavesMat, 0, sizeof(mWavesMat));
			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(3*waves.TriangleCount(), 0, 0);


		}

		DX::ThrowIfFailed(device_.SwapChain()->Present(0, 0));
	}
	
}