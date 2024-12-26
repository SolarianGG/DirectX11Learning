#pragma once

#include <vector>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
using UINT = uint32_t;

namespace lea {
	using namespace DirectX;

	namespace utils {
		struct Vertex0 {
			XMFLOAT3 pos;
			DirectX::PackedVector::XMCOLOR color;
		};

		struct Vertex1 {
			XMFLOAT3 pos;
			XMFLOAT4 color;
		};

		struct Vertex2 {
			XMFLOAT3 pos;
			XMFLOAT3 normal;
			XMFLOAT2 tex0;
			XMFLOAT2 tex1;
		};

		class GeometryGenerator {

		public:

			struct Vertex
			{
				Vertex() {}
				Vertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& t, const DirectX::XMFLOAT2& uv)
					: Position(p), Normal(n), TangentU(t), TexC(uv) {
				}
				Vertex(
					float px, float py, float pz,
					float nx, float ny, float nz,
					float tx, float ty, float tz,
					float u, float v)
					: Position(px, py, pz), Normal(nx, ny, nz),
					TangentU(tx, ty, tz), TexC(u, v) {
				}

				DirectX::XMFLOAT3 Position;
				DirectX::XMFLOAT3 Normal;
				DirectX::XMFLOAT3 TangentU;
				DirectX::XMFLOAT2 TexC;
			};
			struct MeshData
			{
				std::vector<Vertex> Vertices;
				std::vector<uint32_t> Indices;
			};

			void CreateBox(float width, float height, float depth, MeshData& meshData);
			void CreateGeosphere(float radius, UINT numSubdivisions, MeshData& meshData);
			void Subdivide(MeshData& meshData);
			void CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData);
			void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
			
			void CreateGrid(float width, float depth, uint32_t m, uint32_t n, MeshData& meshData);
		private:
			void BuildCylinderTopCap(float bottomRadius, float topRadius, float height,
				UINT sliceCount, UINT stackCount, MeshData& meshData);
			void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
				UINT sliceCount, UINT stackCount, MeshData& meshData);
		};
	}
}