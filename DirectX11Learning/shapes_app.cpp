#include "shapes_app.hpp"

#include <algorithm>
#include <vector>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include <fstream>

#include "lea_timer.hpp"
#include "lea_engine_utils.hpp"

#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl2.h"

#include "DXHelper.hpp"
using lea::utils::Vertex3;
using namespace DirectX;

namespace lea {
	ShapesApp::ShapesApp()
		: App(), mTheta(1.5f * XM_PI), mPhi(0.1f * XM_PI), mRadius(200.0f)
	{
		m_LastMousePos.first = 0;
		m_LastMousePos.second = 0;

		XMMATRIX I = XMMatrixIdentity();

		XMStoreFloat4x4(&mGridWorld, I);
		XMStoreFloat4x4(&mView, I);
		XMStoreFloat4x4(&mProj, I);
		XMStoreFloat4x4(&mBoxTexTransform, I);
		XMStoreFloat4x4(&mCylinderTexTransform, I);
		XMStoreFloat4x4(&mSphereTexTransform, I);
		XMStoreFloat4x4(&mSkullTexTransform, I);

		XMStoreFloat4x4(&mFloorTexTransform, I * XMMatrixScaling(8.f, 8.f, 8.f));

		XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
		XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
		XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

		XMMATRIX skullScale = XMMatrixScaling(.5f, .5f, .5f);
		XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
		XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

		constexpr auto numOfSphAndCylInLine = 5;
		for (int i = 0; i < numOfSphAndCylInLine; ++i)
		{
			XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
			XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

			XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
			XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
		}


		dirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		dirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		dirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		dirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

		boxMat.Ambient = XMFLOAT4(0.4f, 0.1f, 0.1f, 1.0f);
		boxMat.Diffuse = XMFLOAT4(0.8f, 0.2f, 0.2f, 1.0f);
		boxMat.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 32.0f);

		gridMat.Ambient = XMFLOAT4(0.166f, 0.166f, 0.166f, 1.0f);
		gridMat.Diffuse = XMFLOAT4(0.33f, 0.33f, 0.33f, 1.0f);
		gridMat.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 2.0f);

		sphereMat.Ambient = XMFLOAT4(0.1f, 0.1f, 0.4f, 1.0f);
		sphereMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.8f, 1.0f);
		sphereMat.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 256.0f);

		cylinderMat.Ambient = XMFLOAT4(0.1f, 0.4f, 0.1f, 1.0f);
		cylinderMat.Diffuse = XMFLOAT4(0.2f, 0.8f, 0.2f, 1.0f);
		cylinderMat.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 24.0f);

		skullMat.Ambient = XMFLOAT4(0.35f, 0.35f, 0.1f, 1.0f);
		skullMat.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.2f, 1.0f);
		skullMat.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 32.0f);

	}
	ShapesApp::~ShapesApp()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}
	void ShapesApp::Init()
	{
		InitFX();
		LoadTextures();

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
		rastDesc.ScissorEnable = false; // SET TO TRUE FOR SCISSORS 

		ComPtr<ID3D11RasterizerState> rastState;
		DX::ThrowIfFailed(device_.Device()->CreateRasterizerState(&rastDesc, rastState.GetAddressOf()));

		device_.Context()->RSSetState(rastState.Get());

		UINT stride = sizeof(Vertex3);
		UINT offset = 0;
		device_.Context()->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
		device_.Context()->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * XM_PI,
			window_.AspectRatio(), 1.0f, 1000.0f);
		XMStoreFloat4x4(&mProj, P);


		// ImGUI

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		     
		
		ImGui_ImplSDL2_InitForD3D(window_.SDL_WINDOW());
		ImGui_ImplDX11_Init(device_.Device(), device_.Context());
	}

	void ShapesApp::PollEvents()
	{
		auto e = window_.SDL_EVENT();
		ImGui_ImplSDL2_ProcessEvent(&e);
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse || io.WantCaptureKeyboard) return;

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
		eyePos = XMFLOAT3(x, y, z);
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

		auto [skullVertices, skullIndexes] = ScanModel(L"Models/skull.txt");

		// Cache the vertex offsets to each object in the concatenated vertex buffer.
		mBoxVertexOffset = 0;
		mGridVertexOffset = box.Vertices.size();
		mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
		mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();
		mSkullVertexOffset = mCylinderVertexOffset + cylinder.Vertices.size();

		// Cache the index count of each object.
		mBoxIndexCount = box.Indices.size();
		mGridIndexCount = grid.Indices.size();
		mSphereIndexCount = sphere.Indices.size();
		mCylinderIndexCount = cylinder.Indices.size();
		mSkullIndexCount = skullIndexes.size();

		// Cache the starting index for each object in the concatenated index buffer.
		mBoxIndexOffset = 0;
		mGridIndexOffset = mBoxIndexCount;
		mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
		mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;
		mSkullIndexOffset = mCylinderIndexOffset + mCylinderIndexCount;

		UINT totalVertexCount =
			box.Vertices.size() +
			grid.Vertices.size() +
			sphere.Vertices.size() +
			cylinder.Vertices.size() +
			skullVertices.size();

		UINT totalIndexCount =
			mBoxIndexCount +
			mGridIndexCount +
			mSphereIndexCount +
			mCylinderIndexCount +
			mSkullIndexCount;

		//
		// Extract the vertex elements we are interested in and pack the
		// vertices of all the meshes into one vertex buffer.
		//

		std::vector<Vertex3> vertices(totalVertexCount);

		UINT k = 0;
		for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
		{
			vertices[k].pos = box.Vertices[i].Position;
			vertices[k].norm = box.Vertices[i].Normal;
			vertices[k].tex = box.Vertices[i].TexC;
		}

		for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
		{
			vertices[k].pos = grid.Vertices[i].Position;
			vertices[k].norm = grid.Vertices[i].Normal;
			vertices[k].tex = grid.Vertices[i].TexC;
		}

		for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
		{
			vertices[k].pos = sphere.Vertices[i].Position;
			vertices[k].norm = sphere.Vertices[i].Normal;
			vertices[k].tex = sphere.Vertices[i].TexC;
		}

		for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
		{
			vertices[k].pos = cylinder.Vertices[i].Position;
			vertices[k].norm = cylinder.Vertices[i].Normal;
			vertices[k].tex = cylinder.Vertices[i].TexC;
		}

		for (size_t i = 0; i < skullVertices.size(); ++i, ++k)
		{
			vertices[k] = std::move(skullVertices[i]);
		}

		D3D11_BUFFER_DESC vbd{};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex3) * totalVertexCount;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vinitData{};
		vinitData.pSysMem = vertices.data();
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&vbd, &vinitData, vertexBuffer_.GetAddressOf()));

		//
		// Pack the indices of all the meshes into one index buffer.
		//

		std::vector<UINT> indices;
		indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
		indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
		indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
		indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());
		indices.insert(indices.end(), skullIndexes.begin(), skullIndexes.end());

		D3D11_BUFFER_DESC ibd{};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData{};
		iinitData.pSysMem = indices.data();
		DX::ThrowIfFailed(device_.Device()->CreateBuffer(&ibd, &iinitData, indexBuffer_.GetAddressOf()));
	}

	void ShapesApp::CreateInputLayout()
	{
		D3D11_INPUT_ELEMENT_DESC inputs[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		D3DX11_PASS_DESC passDesc;
		effectTechnique_->GetPassByIndex(0)->GetDesc(&passDesc);
		DX::ThrowIfFailed(
			device_.Device()->CreateInputLayout
			(inputs, ARRAYSIZE(inputs), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, inputLayout_.GetAddressOf()));
	}
	void ShapesApp::DrawScene()
	{
		DrawGUI();
		auto context = device_.Context();
		context->ClearRenderTargetView(device_.RenderTargetView(), reinterpret_cast<const float*>(&DirectX::Colors::MidnightBlue));
		context->ClearDepthStencilView(device_.DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX viewProj = view * proj;

		mEyePosW_->SetFloatVector(reinterpret_cast<const float*>(&eyePos));
		mDirectionalLight_->SetRawValue(&dirLight, 0, sizeof(dirLight));

		D3DX11_TECHNIQUE_DESC techDesc;
		effectTechnique_->GetDesc(&techDesc);
		for (uint32_t i = 0; i < techDesc.Passes; ++i)
		{
			XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
			XMMATRIX finalMatrix = world * viewProj;
			
			XMMATRIX inverseTranspose = lea::utils::MathHelper::InverseTranspose(world);

			worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&world));
			worldInverseTransposeMatrix_->SetMatrix(reinterpret_cast<const float*>(&inverseTranspose));
			worldViewProjectionMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMatrix));
			mShapeMaterial_->SetRawValue(&gridMat, 0, sizeof(gridMat));
			mEffectTexture_->SetResource(mFloorTexture_.Get());

			XMMATRIX texTransform = XMLoadFloat4x4(&mFloorTexTransform);
			textureTransform_->SetMatrix(reinterpret_cast<const float*>(&texTransform));
			

			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);


			world = XMLoadFloat4x4(&mBoxWorld);
			finalMatrix = world * viewProj;

			inverseTranspose = lea::utils::MathHelper::InverseTranspose(world);

			worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&world));
			worldInverseTransposeMatrix_->SetMatrix(reinterpret_cast<const float*>(&inverseTranspose));
			worldViewProjectionMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMatrix));
			mShapeMaterial_->SetRawValue(&boxMat, 0, sizeof(boxMat));
			mEffectTexture_->SetResource(mBoxTexture_.Get());

			texTransform = XMLoadFloat4x4(&mBoxTexTransform);
			textureTransform_->SetMatrix(reinterpret_cast<const float*>(&texTransform));

			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

			world = XMLoadFloat4x4(&mSkullWorld);
			finalMatrix = world * viewProj;

			inverseTranspose = lea::utils::MathHelper::InverseTranspose(world);

			worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&world));
			worldInverseTransposeMatrix_->SetMatrix(reinterpret_cast<const float*>(&inverseTranspose));
			worldViewProjectionMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMatrix));
			mShapeMaterial_->SetRawValue(&skullMat, 0, sizeof(skullMat));
			mEffectTexture_->SetResource(mSkullTexture_.Get());

			texTransform = XMLoadFloat4x4(&mSkullTexTransform);
			textureTransform_->SetMatrix(reinterpret_cast<const float*>(&texTransform));

			effectTechnique_->GetPassByIndex(i)->Apply(0, context);
			context->DrawIndexed(mSkullIndexCount, mSkullIndexOffset, mSkullVertexOffset);

			mShapeMaterial_->SetRawValue(&cylinderMat, 0, sizeof(cylinderMat));
			mEffectTexture_->SetResource(mCylinderTexture_.Get());
			texTransform = XMLoadFloat4x4(&mCylinderTexTransform);
			textureTransform_->SetMatrix(reinterpret_cast<const float*>(&texTransform));
			for (uint32_t j = 0; j < 10; ++j)
			{
				world = XMLoadFloat4x4(&mCylWorld[j]);
				finalMatrix = world * viewProj;

				inverseTranspose = lea::utils::MathHelper::InverseTranspose(world);

				worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&world));
				worldInverseTransposeMatrix_->SetMatrix(reinterpret_cast<const float*>(&inverseTranspose));
				worldViewProjectionMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMatrix));

				effectTechnique_->GetPassByIndex(i)->Apply(0, context);
				context->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
			}

			mShapeMaterial_->SetRawValue(&sphereMat, 0, sizeof(sphereMat));
			mEffectTexture_->SetResource(mSphereTexture_.Get());
			texTransform = XMLoadFloat4x4(&mSphereTexTransform);
			textureTransform_->SetMatrix(reinterpret_cast<const float*>(&texTransform));
			for (uint32_t j = 0; j < 10; ++j)
			{
				world = XMLoadFloat4x4(&mSphereWorld[j]);
				finalMatrix = world * viewProj;

				inverseTranspose = lea::utils::MathHelper::InverseTranspose(world);

				worldMatrix_->SetMatrix(reinterpret_cast<const float*>(&world));
				worldInverseTransposeMatrix_->SetMatrix(reinterpret_cast<const float*>(&inverseTranspose));
				worldViewProjectionMatrix_->SetMatrix(reinterpret_cast<const float*>(&finalMatrix));

				effectTechnique_->GetPassByIndex(i)->Apply(0, context);
				context->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
			}
		}

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		DX::ThrowIfFailed(device_.SwapChain()->Present(0, 0));
	}
	std::pair<std::vector<Vertex3>, std::vector<UINT>> ShapesApp::ScanModel(std::wstring_view file_name)
	{
		std::ifstream fin(file_name.data());

		if (!fin)
		{
			MessageBox(0, L"Models not found.", 0, 0);
			throw std::runtime_error("Failed to load model in memory");
		}

		UINT vcount = 0;
		UINT tcount = 0;
		std::string ignore;

		fin >> ignore >> vcount;
		fin >> ignore >> tcount;
		fin >> ignore >> ignore >> ignore >> ignore;

		std::vector<Vertex3> vertices(vcount);
		for (UINT i = 0; i < vcount; ++i)
		{
			fin >> vertices[i].pos.x >> vertices[i].pos.y >> vertices[i].pos.z
				>> vertices[i].norm.x >> vertices[i].norm.y >> vertices[i].norm.z;
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

		for (auto& vertex : vertices) {
			float length = std::sqrt(vertex.pos.x * vertex.pos.x + vertex.pos.y * vertex.pos.y + vertex.pos.z * vertex.pos.z);
			float u = 0.5f + std::atan2(vertex.pos.z, vertex.pos.x) / (2.0f * XM_PI);
			float v = 0.5f - std::asin(vertex.pos.y / length) / XM_PI;

			vertex.tex = XMFLOAT2(u, v);
		}


		return { vertices, indices };
	}


	void ShapesApp::DrawGUI()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();


		// Example of ImGUI window
		ImGui::Begin("ColorPickingWindow");

		ImGui::DragFloat3("Light Direction", reinterpret_cast<float*>(&dirLight.Direction), 0.01f, -1.f, 1.f);

		ImGui::Text("Light colors: ");
		ImGui::ColorEdit3("Light Ambient Color", reinterpret_cast<float*>(&dirLight.Ambient));
		ImGui::ColorEdit3("Light Diffuse Color", reinterpret_cast<float*>(&dirLight.Diffuse));
		ImGui::ColorEdit3("Light Specular Color", reinterpret_cast<float*>(&dirLight.Specular));

		ImGui::Text("Cylinder colors: ");
		ImGui::ColorEdit3("Cylinder Ambient Color", reinterpret_cast<float*>(&cylinderMat.Ambient));
		ImGui::ColorEdit3("Cylinder Diffuse Color", reinterpret_cast<float*>(&cylinderMat.Diffuse));
		ImGui::ColorEdit3("Cylinder Specular Color", reinterpret_cast<float*>(&cylinderMat.Specular));
		ImGui::DragFloat("Cylinder specular", &cylinderMat.Specular.w, 0.0f, 512.0f);

		if (ImGui::Button("TurnRed"))
		{
			dirLight.Diffuse = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
		}

		ImGui::Text("Grid colors: ");
		ImGui::ColorEdit3("Grid Ambient Color", reinterpret_cast<float*>(&gridMat.Ambient));
		ImGui::ColorEdit3("Grid Diffuse Color", reinterpret_cast<float*>(&gridMat.Diffuse));
		ImGui::ColorEdit3("Grid Specular Color", reinterpret_cast<float*>(&gridMat.Specular));
		ImGui::DragFloat("Grid specular", &gridMat.Specular.w, 0.0f, 512.0f);

		ImGui::Text("Sphere colors: ");
		ImGui::ColorEdit3("Sphere Ambient Color", reinterpret_cast<float*>(&sphereMat.Ambient));
		ImGui::ColorEdit3("Sphere Diffuse Color", reinterpret_cast<float*>(&sphereMat.Diffuse));
		ImGui::ColorEdit3("Sphere Specular Color", reinterpret_cast<float*>(&sphereMat.Specular));
		ImGui::DragFloat("Sphere specular", &sphereMat.Specular.w, 0.0f, 512.0f);

		ImGui::Text("Box colors: ");
		ImGui::ColorEdit3("Box Ambient Color", reinterpret_cast<float*>(&boxMat.Ambient));
		ImGui::ColorEdit3("Box Diffuse Color", reinterpret_cast<float*>(&boxMat.Diffuse));
		ImGui::ColorEdit3("Box Specular Color", reinterpret_cast<float*>(&boxMat.Specular));
		ImGui::DragFloat("Box specular", &boxMat.Specular.w, 0.0f, 512.0f);

		ImGui::Text("Skull colors: ");
		ImGui::ColorEdit3("Skull Ambient Color", reinterpret_cast<float*>(&skullMat.Ambient));
		ImGui::ColorEdit3("Skull Diffuse Color", reinterpret_cast<float*>(&skullMat.Diffuse));
		ImGui::ColorEdit3("Skull Specular Color", reinterpret_cast<float*>(&skullMat.Specular));
		ImGui::DragFloat("Skull specular", &skullMat.Specular.w, 0.0f, 512.0f);


		ImGui::End();
	}
	void ShapesApp::LoadTextures()
	{
		mFloorTexture_ = device_.CreateTexture(L"Textures/floor.dds");
		mSphereTexture_ = device_.CreateTexture(L"Textures/stone.dds");
		mCylinderTexture_ = device_.CreateTexture(L"Textures/bricks.dds");
		mBoxTexture_ = device_.CreateTexture(L"Textures/WoodCrate01.dds");
		mSkullTexture_ = device_.CreateTexture(L"Textures/bone.jpg");
	}
	void ShapesApp::InitFX()
	{
		effect_ = device_.CreateEffect(L"shapes_light_tex.fx");
		effectTechnique_ = effect_->GetTechniqueByName("TextLightTech");

		worldMatrix_ = effect_->GetVariableByName("gWorld")->AsMatrix();
		worldViewProjectionMatrix_ = effect_->GetVariableByName("gWorldProjectView")->AsMatrix();
		worldInverseTransposeMatrix_ = effect_->GetVariableByName("gWorldInverseTranspose")->AsMatrix();
		textureTransform_ = effect_->GetVariableByName("gTexTransform")->AsMatrix();

		mShapeMaterial_ = effect_->GetVariableByName("gMat");
		mDirectionalLight_ = effect_->GetVariableByName("gDirectionalLight");

		mEyePosW_ = effect_->GetVariableByName("gEyePosW")->AsVector();
		mEffectTexture_ = effect_->GetVariableByName("gDiffuseMap")->AsShaderResource();
	}
}