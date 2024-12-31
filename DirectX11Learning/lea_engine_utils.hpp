#pragma once



#include <vector>

#include <cstring>

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
			XMFLOAT3 norm;
		};

		struct Vertex3 {
			XMFLOAT3 pos;
			XMFLOAT3 normal;
			XMFLOAT2 tex0;
			XMFLOAT2 tex1;
		};

		class GeometryGenerator {

		public:

			struct Vertex
			{
				Vertex() { std::memset(this, 0, sizeof(*this)); }
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

		class MathHelper {
		public:
			static float RandF()
			{
				return (float)(rand()) / (float)RAND_MAX;
			}

			// Returns random float in [a, b).
			static float RandF(float a, float b)
			{
				return a + RandF() * (b - a);
			}

			template<typename T>
			static T Max(const T& a, const T& b)
			{
				return a > b ? a : b;
			}

			template<typename T>
			static T Min(const T& a, const T& b)
			{
				return a < b ? a : b;
			}

			template<typename T>
			static T Lerp(const T& a, const T& b, float t)
			{
				return a + (b - a) * t;
			}

			template<typename T>
			static T Clamp(const T& x, const T& low, const T& high)
			{
				return x < low ? low : (x > high ? high : x);
			}

			static float AngleFromXY(float x, float y)
			{
				constexpr float Pi = 3.14f;
				float theta = 0.0f;

				// Quadrant I or IV
				if (x >= 0.0f)
				{
					// If x = 0, then atanf(y/x) = +pi/2 if y > 0
					//                atanf(y/x) = -pi/2 if y < 0
					theta = atanf(y / x); // in [-pi/2, +pi/2]

					if (theta < 0.0f)
						theta += 2.0f * Pi; // in [0, 2*pi).
				}

				// Quadrant II or III
				else
					theta = atanf(y / x) + Pi; // in [0, 2*pi).

				return theta;
			}

			static XMMATRIX InverseTranspose(CXMMATRIX M)
			{
				XMMATRIX A = M;
				A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMVECTOR det = XMMatrixDeterminant(A);
				return XMMatrixTranspose(XMMatrixInverse(&det, A));
			}

			static XMVECTOR RandUnitVec3()
			{
				XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
				XMVECTOR Zero = XMVectorZero();

				// Keep trying until we get a point on/in the hemisphere.
				while (true)
				{
					// Generate random point in the cube [-1,1]^3.
					XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), 0.0f);

					// Ignore points outside the unit sphere in order to get an even distribution 
					// over the unit sphere.  Otherwise points will clump more on the sphere near 
					// the corners of the cube.

					if (XMVector3Greater(XMVector3LengthSq(v), One))
						continue;

					return XMVector3Normalize(v);
				}
			}
		};

		namespace light {
			struct Material {
				Material() { std::memset(this, 0, sizeof(*this)); }

				XMFLOAT4 Ambient;
				XMFLOAT4 Diffuse;
				XMFLOAT4 Specular; // w = SpecPower
				XMFLOAT4 Reflect;
			};

			struct DirectionalLight
			{
				DirectionalLight() { std::memset(this, 0, sizeof(*this)); }

				XMFLOAT4 Ambient;
				XMFLOAT4 Diffuse;
				XMFLOAT4 Specular;
				XMFLOAT3 Direction;
				float Pad; // Pad the last float so we can
				// array of lights if we wanted.
			};

			struct PointLight
			{
				PointLight() { std::memset(this, 0, sizeof(*this)); }
				XMFLOAT4 Ambient;
				XMFLOAT4 Diffuse;
				XMFLOAT4 Specular;
				// Packed into 4D vector: (Position, Range)
				XMFLOAT3 Position;
				float Range;
				// Packed into 4D vector: (A0, A1, A2, Pad)
				XMFLOAT3 Att;
				float Pad; // Pad the last float so we can set an
				// array of lights if we wanted.
			};

			struct SpotLight
			{
				SpotLight() { std::memset(this, 0, sizeof(*this)); }
				XMFLOAT4 Ambient;
				XMFLOAT4 Diffuse;
				XMFLOAT4 Specular;
				// Packed into 4D vector: (Position, Range)
				XMFLOAT3 Position;
				float Range;
				// Packed into 4D vector: (Direction,Spot)
				XMFLOAT3 Direction;
				float Spot;
				// Packed into 4D vector: (Att, Pad)
				XMFLOAT3 Att;
				float Pad; // Pad the last float so we can set an
				// array of lights if we wanted.
			};
		}
	}
}