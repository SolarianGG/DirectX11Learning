#pragma once

#include <DirectXMath.h>

namespace lea {
	using namespace DirectX;

	namespace utils {
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
	}
}