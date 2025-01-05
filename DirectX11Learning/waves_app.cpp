#include "waves_app.hpp"

#include <algorithm>
#include <vector>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include "lea_timer.hpp"
#include "lea_engine_utils.hpp"
#include "DXHelper.hpp"

using lea::utils::Vertex3;
using namespace DirectX;

namespace lea {
	WavesApp::WavesApp()
		: App(), mGridIndexCount(0), mTheta(1.5f * XM_PI), mPhi(0.1f * XM_PI), mRadius(200.0f), mWaterTexOffset(0.0f, 0.0f), renderOptions(ERenderTypes::LightAndTexturesAndFog)
	{
		m_LastMousePos.first = 0;
		m_LastMousePos.second = 0;

		XMMATRIX I = XMMatrixIdentity();

		XMStoreFloat4x4(&mLandWorld, I);
		XMStoreFloat4x4(&mWavesWorld, I);
		XMStoreFloat4x4(&mView, I);
		XMStoreFloat4x4(&mProj, I);
		XMStoreFloat4x4(&mWavesTexTransform, I);
		XMStoreFloat4x4(&mBoxTexTransform, I);

		XMMATRIX wavesOffset = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
		XMStoreFloat4x4(&mWavesWorld, wavesOffset);

		XMMATRIX boxScale = XMMatrixScaling(15.0f, 15.0f, 15.0f);
		XMMATRIX boxOffset = XMMatrixTranslation(8.0f, -3.0f, -15.0f);
		XMStoreFloat4x4(&mBoxWorld, boxScale * boxOffset);

		// Directional light.
		mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

		mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
		mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
		mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

		mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

		mLandMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		mLandMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

		mWavesMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		mWavesMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
		mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

		mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mBoxMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

		XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
		XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);
	}
	void WavesApp::Init()
	{
		waves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

		InitFX();
		LoadTextures();

		CreateInputLayout();
		BuildLandGeometryBuffers();
		BuildWavesGeometryBuffers();
		BuildCommonGeometryBuffers();

		device_.Context()->IASetInputLayout(inputLayout_.Get());
		device_.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CreateRasterizerStates();

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

		if (event.type == LeaEvent::TypePressed)
		{
			if (event.key == LeaEvent::KeyOne)
			{
				renderOptions = ERenderTypes::LightOnly;
			}
			if (event.key == LeaEvent::KeyThree)
			{
				renderOptions = ERenderTypes::LightAndTexturesAndFog;
			}
			if (event.key == LeaEvent::KeyTwo)
			{
				renderOptions = ERenderTypes::LightAndTextures;
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

		Vertex3* v = reinterpret_cast<Vertex3*>(mappedData.pData);
		for (UINT i = 0; i < waves.VertexCount(); ++i)
		{
			v[i].pos = waves[i];
			v[i].norm = waves.Normal(i);

			v[i].tex.x = 0.5f + waves[i].x / waves.Width();
			v[i].tex.y = 0.5f - waves[i].z / waves.Depth();
		}

		device_.Context()->Unmap(wavesVertexBuffer_.Get(), 0);

		XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
		mWaterTexOffset.y += 0.05f * deltaTime;
		mWaterTexOffset.x += 0.1f * deltaTime;
		XMMATRIX wavesTranslation = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

		XMStoreFloat4x4(&mWavesTexTransform, wavesScale * wavesTranslation);

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

		std::vector<Vertex3> vertices(grid.Vertices.size());
		for (size_t i = 0; i < grid.Vertices.size(); ++i)
		{
			XMFLOAT3 p = grid.Vertices[i].Position;

			p.y = GetHeight(p.x, p.z);

			vertices[i].pos = p;
			vertices[i].norm = GetHillNormal(p.x, p.z);
			vertices[i].tex = grid.Vertices[i].TexC;
		}

		D3D11_BUFFER_DESC vbd{};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex3) * grid.Vertices.size();
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
		vbd.ByteWidth = sizeof(Vertex3) * waves.VertexCount();
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
	void WavesApp::BuildCommonGeometryBuffers()
	{
		using lea::utils::GeometryGenerator;

		GeometryGenerator gen;

		GeometryGenerator::MeshData boxMesh;

		gen.CreateBox(1.f, 1.f, 1.f, boxMesh);

		mBoxIndexCount = boxMesh.Indices.size();

		std::vector<Vertex3> vertices(boxMesh.Vertices.size());

		for (UINT i = 0; i < boxMesh.Vertices.size(); ++i)
		{
			vertices[i].pos = boxMesh.Vertices[i].Position;
			vertices[i].norm = boxMesh.Vertices[i].Normal;
			vertices[i].tex = boxMesh.Vertices[i].TexC;
		}


		D3D11_BUFFER_DESC vbd{};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex3) * vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vinitData{};
		vinitData.pSysMem = vertices.data();
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&vbd, &vinitData, commonVertexBuffer_.GetAddressOf()));

		//
		// Pack the indices of all the meshes into one index buffer.
		//

		D3D11_BUFFER_DESC ibd{};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * mBoxIndexCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData{};
		iinitData.pSysMem = boxMesh.Indices.data();
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&ibd, &iinitData, commonIndexBuffer_.GetAddressOf()));
	}

	void WavesApp::CreateInputLayout()
	{
		D3D11_INPUT_ELEMENT_DESC inputs[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		D3DX11_PASS_DESC passDesc;
		effectTechniques_.at(ERenderTypes::LightOnly)->GetPassByIndex(0)->GetDesc(&passDesc);
		DX::ThrowIfFailed(
			device_.Device()->CreateInputLayout
			(inputs, ARRAYSIZE(inputs), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, inputLayout_.GetAddressOf()));
	}

	void WavesApp::CreateRasterizerStates()
	{
		D3D11_RASTERIZER_DESC rastDesc{};
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FrontCounterClockwise = false;
		rastDesc.DepthClipEnable = true;
		DX::ThrowIfFailed(device_.Device()->CreateRasterizerState(&rastDesc, mNoCullRS.GetAddressOf()));

		D3D11_BLEND_DESC transparentDesc{};
		transparentDesc.AlphaToCoverageEnable = false;
		transparentDesc.IndependentBlendEnable = false;

		transparentDesc.RenderTarget[0].BlendEnable = true;
		transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		DX::ThrowIfFailed(device_.Device()->CreateBlendState(&transparentDesc, mTransparentBS.GetAddressOf()));

		/*D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; 
		desc.DepthFunc = D3D11_COMPARISON_LESS; */


		// desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
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

	inline void WavesApp::InitFX()
	{
		effect_ = device_.CreateEffect(L"waves_blended.fx");
		effectTechniques_ = {
			{ ERenderTypes::LightOnly, effect_->GetTechniqueByName("Light3")},
			{ ERenderTypes::LightAndTextures, effect_->GetTechniqueByName("Light3TexAlphaClip")},
			{ ERenderTypes::LightAndTexturesAndFog, effect_->GetTechniqueByName("Light3TexAlphaClipFog")},
		};

		mfxWorldViewProj = effect_->GetVariableByName("gWorldProjectView")->AsMatrix();
		mfxWorld = effect_->GetVariableByName("gWorld")->AsMatrix();
		mfxWorldInvTranspose = effect_->GetVariableByName("gWorldInverseTranspose")->AsMatrix();
		mfxEyePosW = effect_->GetVariableByName("gEyePosW")->AsVector();
		mfxFogColor = effect_->GetVariableByName("gFogColor")->AsVector();
		mfxDirLights = effect_->GetVariableByName("gDirLights");
		mfxMaterial = effect_->GetVariableByName("gMaterial");
		mfxTexTransform = effect_->GetVariableByName("gTexTransform")->AsMatrix();
		mfxFogStart = effect_->GetVariableByName("gFogStart")->AsScalar();
		mfxFogRange = effect_->GetVariableByName("gFogRange")->AsScalar();


		mfxTexture = effect_->GetVariableByName("gDiffuseMap")->AsShaderResource();
	}

	inline void WavesApp::LoadTextures()
	{
		grassTexture_ = device_.CreateTexture(L"Textures/grass.dds");
		wavesTexture_ = device_.CreateTexture(L"Textures/water1.dds");
		boxTexture_ = device_.CreateTexture(L"Textures/WireFence.dds");
	}

	void WavesApp::DrawScene()
	{
		auto context = device_.Context();

		context->ClearRenderTargetView(device_.RenderTargetView(), reinterpret_cast<const float*>(&Colors::Silver));
		context->ClearDepthStencilView(device_.DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

		UINT strides = sizeof(Vertex3);
		UINT offset = 0;

		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX viewProj = view * proj;

		mfxDirLights->SetRawValue(mDirLights, 0, 3*sizeof(DirectionalLight));
		mfxEyePosW->SetRawValue(&mEyePosW, 0, sizeof(mEyePosW));
		mfxFogColor->SetFloatVector(reinterpret_cast<const float*>(&Colors::Silver));
		mfxFogStart->SetFloat(15.0f);
		mfxFogRange->SetFloat(175.0f);

		auto& effectTechnique_ = effectTechniques_[renderOptions];

		D3DX11_TECHNIQUE_DESC techDesc;
		effectTechnique_->GetDesc(&techDesc);
		for (UINT i = 0; i < techDesc.Passes; ++i)
		{
			// Box drawing
			context->IASetVertexBuffers(0, 1, commonVertexBuffer_.GetAddressOf(), &strides, &offset);
			context->IASetIndexBuffer(commonIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

			XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
			XMMATRIX worldInvTrans = lea::utils::MathHelper::InverseTranspose(world);
			XMMATRIX worldViewProj = world * viewProj;
			XMMATRIX texTransform = XMLoadFloat4x4(&mBoxTexTransform);

			mfxWorldViewProj->SetMatrix(reinterpret_cast<const float*>(&worldViewProj));
			mfxWorld->SetMatrix(reinterpret_cast<const float*>(&world));
			mfxWorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&worldInvTrans));
			mfxMaterial->SetRawValue(&mBoxMat, 0, sizeof(mBoxMat));
			mfxTexTransform->SetMatrix(reinterpret_cast<const float*>(&texTransform));
			mfxTexture->SetResource(boxTexture_.Get());

			context->RSSetState(mNoCullRS.Get());
			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(mBoxIndexCount, 0, 0);
			context->RSSetState(0);

			// grid drawing
			context->IASetVertexBuffers(0, 1, landVertexBuffer_.GetAddressOf(), &strides, &offset);
			context->IASetIndexBuffer(landIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

			world = XMLoadFloat4x4(&mLandWorld);
			worldInvTrans = lea::utils::MathHelper::InverseTranspose(world);
			worldViewProj = world * viewProj;
			texTransform = XMLoadFloat4x4(&mGrassTexTransform);

			mfxWorldViewProj->SetMatrix(reinterpret_cast<const float*>(&worldViewProj));
			mfxWorld->SetMatrix(reinterpret_cast<const float*>(&world));
			mfxWorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&worldInvTrans));
			mfxMaterial->SetRawValue(&mLandMat, 0, sizeof(mLandMat));
			mfxTexTransform->SetMatrix(reinterpret_cast<const float*>(&texTransform));
			mfxTexture->SetResource(grassTexture_.Get());

			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(mGridIndexCount, 0, 0);

			// waves drawing
			context->IASetVertexBuffers(0, 1, wavesVertexBuffer_.GetAddressOf(), &strides, &offset);
			context->IASetIndexBuffer(wavesIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

			world = XMLoadFloat4x4(&mWavesWorld);
			worldInvTrans = lea::utils::MathHelper::InverseTranspose(world);
			worldViewProj = world * viewProj;
			mfxWorldViewProj->SetMatrix(reinterpret_cast<const float*>(&worldViewProj));
			mfxWorld->SetMatrix(reinterpret_cast<const float*>(&world));
			mfxWorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&worldInvTrans));
			mfxMaterial->SetRawValue(&mWavesMat, 0, sizeof(mWavesMat));
			texTransform = XMLoadFloat4x4(&mWavesTexTransform);
			mfxTexTransform->SetMatrix(reinterpret_cast<const float*>(&texTransform));
			mfxTexture->SetResource(wavesTexture_.Get());
			
			context->OMSetBlendState(mTransparentBS.Get(), blendFactor, 0xFFFFFFFF);
			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(3*waves.TriangleCount(), 0, 0);
			context->OMSetBlendState(0, blendFactor, 0xFFFFFFFF);

		}

		DX::ThrowIfFailed(device_.SwapChain()->Present(0, 0));
	}
	
}