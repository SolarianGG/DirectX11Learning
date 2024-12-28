#pragma once

#include <cinttypes>

#include "DirectXMath.h"

using namespace DirectX;

using UINT = uint32_t;

namespace lea {
	class Waves {
	public:
		Waves();
		Waves(const Waves& other) = delete;
		Waves& operator=(const Waves& other) = delete;
		Waves(Waves&& other) noexcept = delete;
		Waves& operator=(Waves&& other) noexcept = delete;
		~Waves();

		UINT RowCount()const;
		UINT ColumnCount()const;
		UINT VertexCount()const;
		UINT TriangleCount()const;

		// Returns the solution at the ith grid point.
		const XMFLOAT3& operator[](int i)const { return mCurrSolution[i]; }

		void Init(UINT m, UINT n, float dx, float dt, float speed, float damping);
		void Update(float dt);
		void Disturb(UINT i, UINT j, float magnitude);

	private:
		UINT mNumRows;
		UINT mNumCols;

		UINT mVertexCount;
		UINT mTriangleCount;

		// Simulation constants we can precompute.
		float mK1;
		float mK2;
		float mK3;

		float mTimeStep;
		float mSpatialStep;

		XMFLOAT3* mPrevSolution;
		XMFLOAT3* mCurrSolution;

	};
}