#include "shapes_app.hpp"

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
	ShapesApp::ShapesApp()
		: App(), mTheta(1.5f * PI), mPhi(0.1f * PI), mRadius(200.0f)
	{
		m_LastMousePos.first = 0;
		m_LastMousePos.second = 0;

		XMMATRIX I = XMMatrixIdentity();

		XMStoreFloat4x4(&mGridWorld, I);
		XMStoreFloat4x4(&mView, I);
		XMStoreFloat4x4(&mProj, I);

		XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
		XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
		XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

		XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
		XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
		XMStoreFloat4x4(&mCenterSphere, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

		for (int i = 0; i < 5; ++i)
		{
			XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
			XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

			XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
			XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
		}

	}
	void ShapesApp::Init()
	{
		effect_ = device_.CreateEffect(L"terrain_shader.fx");
		effectTechnique_ = effect_->GetTechniqueByName("ColorTech");

		worldMatrix_ = effect_->GetVariableByName("gWorldProjectView")->AsMatrix();

		CreateGeometryBuffers();
		CreateInputLayout();

		device_.Context()->IASetInputLayout(inputLayout_.Get());
		device_.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D11_RECT scissorsRect = { 100, 50, 950, 750 };
		device_.Context()->RSSetScissorRects(1, &scissorsRect);

		D3D11_RASTERIZER_DESC rastDesc{};
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_BACK;
		rastDesc.FrontCounterClockwise = false;
		rastDesc.ScissorEnable = true;

		ComPtr<ID3D11RasterizerState> rastState;
		DX::ThrowIfFailed(device_.Device()->CreateRasterizerState(&rastDesc, rastState.GetAddressOf()));

		device_.Context()->RSSetState(rastState.Get());

		UINT stride = sizeof(Vertex1);
		UINT offset = 0;
		ID3D11Buffer* const buffers[] = { vertexBuffer_.Get() };
		device_.Context()->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
		device_.Context()->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * PI,
			window_.AspectRatio(), 1.0f, 1000.0f);
		XMStoreFloat4x4(&mProj, P);
	}

	void ShapesApp::PollEvents()
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
	void ShapesApp::UpdateScene(float deltaTime)
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

	void ShapesApp::CreateGeometryBuffers()
	{
		using namespace utils;
		GeometryGenerator::MeshData box;
		GeometryGenerator::MeshData grid;
		GeometryGenerator::MeshData sphere;
		GeometryGenerator::MeshData cylinder;

		GeometryGenerator geoGen;
		geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
		geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
		geoGen.CreateGeosphere(0.5f, 3, sphere);
		//geoGen.CreateGeosphere(0.5f, 2, sphere);
		geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

		// Cache the vertex offsets to each object in the concatenated vertex buffer.
		mBoxVertexOffset = 0;
		mGridVertexOffset = box.Vertices.size();
		mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
		mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

		// Cache the index count of each object.
		mBoxIndexCount = box.Indices.size();
		mGridIndexCount = grid.Indices.size();
		mSphereIndexCount = sphere.Indices.size();
		mCylinderIndexCount = cylinder.Indices.size();

		// Cache the starting index for each object in the concatenated index buffer.
		mBoxIndexOffset = 0;
		mGridIndexOffset = mBoxIndexCount;
		mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
		mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

		UINT totalVertexCount =
			box.Vertices.size() +
			grid.Vertices.size() +
			sphere.Vertices.size() +
			cylinder.Vertices.size();

		UINT totalIndexCount =
			mBoxIndexCount +
			mGridIndexCount +
			mSphereIndexCount +
			mCylinderIndexCount;

		//
		// Extract the vertex elements we are interested in and pack the
		// vertices of all the meshes into one vertex buffer.
		//

		std::vector<Vertex1> vertices(totalVertexCount);

		XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

		UINT k = 0;
		for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
		{
			vertices[k].pos = box.Vertices[i].Position;
			vertices[k].color = black;
		}

		for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
		{
			vertices[k].pos = grid.Vertices[i].Position;
			vertices[k].color = black;
		}

		for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
		{
			vertices[k].pos = sphere.Vertices[i].Position;
			vertices[k].color = black;
		}

		for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
		{
			vertices[k].pos = cylinder.Vertices[i].Position;
			vertices[k].color = black;
		}

		D3D11_BUFFER_DESC vbd{};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex1) * totalVertexCount;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vinitData{};
		vinitData.pSysMem = &vertices[0];
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&vbd, &vinitData, vertexBuffer_.GetAddressOf()));

		//
		// Pack the indices of all the meshes into one index buffer.
		//

		std::vector<UINT> indices;
		indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
		indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
		indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
		indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

		D3D11_BUFFER_DESC ibd{};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData{};
		iinitData.pSysMem = &indices[0];
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&ibd, &iinitData, indexBuffer_.GetAddressOf()));
	}

	void ShapesApp::CreateInputLayout()
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
	void ShapesApp::DrawScene()
	{
		auto context = device_.Context();

		context->ClearRenderTargetView(device_.RenderTargetView(), reinterpret_cast<const float*>(&DirectX::Colors::MidnightBlue));
		context->ClearDepthStencilView(device_.DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX viewProj = view * proj;

		D3DX11_TECHNIQUE_DESC techDesc;
		effectTechnique_->GetDesc(&techDesc);
		for (uint32_t i = 0; i < techDesc.Passes; ++i)
		{
			XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
			XMMATRIX finalMatrix = world * viewProj;
			worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMatrix));
			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);


			world = XMLoadFloat4x4(&mBoxWorld);
			finalMatrix = world * viewProj;
			worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMatrix));
			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

			world = XMLoadFloat4x4(&mCenterSphere);
			finalMatrix = world * viewProj;
			worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMatrix));
			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);


			for (uint32_t j = 0; j < 10; ++j)
			{
				world = XMLoadFloat4x4(&mCylWorld[j]);
				finalMatrix = world * viewProj;
				worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMatrix));
				effectTechnique_->GetPassByIndex(i)->Apply(0, context);
				context->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
			}

			for (uint32_t j = 0; j < 10; ++j)
			{
				world = XMLoadFloat4x4(&mSphereWorld[j]);
				finalMatrix = world * viewProj;
				worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMatrix));
				effectTechnique_->GetPassByIndex(i)->Apply(0, context);
				context->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
			}
		}


		DX::ThrowIfFailed(device_.SwapChain()->Present(0, 0));
	}
}