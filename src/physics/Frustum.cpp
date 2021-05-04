#include "Frustum.h"

void Frustum::calculateFrustum(const glm::mat4& view, const glm::mat4& projection) {
	std::array<float, 16> tmp;

	tmp[0] = view[0][0] * projection[0][0] + view[0][1] * projection[1][0] + view[0][2] * projection[2][0] + view[0][3] * projection[3][0];
	tmp[1] = view[0][0] * projection[0][1] + view[0][1] * projection[1][1] + view[0][2] * projection[2][1] + view[0][3] * projection[3][1];
	tmp[2] = view[0][0] * projection[0][2] + view[0][1] * projection[1][2] + view[0][2] * projection[2][2] + view[0][3] * projection[3][2];
	tmp[3] = view[0][0] * projection[0][3] + view[0][1] * projection[1][3] + view[0][2] * projection[2][3] + view[0][3] * projection[3][3];
	tmp[4] = view[1][0] * projection[0][0] + view[1][1] * projection[1][0] + view[1][2] * projection[2][0] + view[1][3] * projection[3][0];
	tmp[5] = view[1][0] * projection[0][1] + view[1][1] * projection[1][1] + view[1][2] * projection[2][1] + view[1][3] * projection[3][1];
	tmp[6] = view[1][0] * projection[0][2] + view[1][1] * projection[1][2] + view[1][2] * projection[2][2] + view[1][3] * projection[3][2];
	tmp[7] = view[1][0] * projection[0][3] + view[1][1] * projection[1][3] + view[1][2] * projection[2][3] + view[1][3] * projection[3][3];
	tmp[8] = view[2][0] * projection[0][0] + view[2][1] * projection[1][0] + view[2][2] * projection[2][0] + view[2][3] * projection[3][0];
	tmp[9] = view[2][0] * projection[0][1] + view[2][1] * projection[1][1] + view[2][2] * projection[2][1] + view[2][3] * projection[3][1];
	tmp[10] = view[2][0] * projection[0][2] + view[2][1] * projection[1][2] + view[2][2] * projection[2][2] + view[2][3] * projection[3][2];
	tmp[11] = view[2][0] * projection[0][3] + view[2][1] * projection[1][3] + view[2][2] * projection[2][3] + view[2][3] * projection[3][3];
	tmp[12] = view[3][0] * projection[0][0] + view[3][1] * projection[1][0] + view[3][2] * projection[2][0] + view[3][3] * projection[3][0];
	tmp[13] = view[3][0] * projection[0][1] + view[3][1] * projection[1][1] + view[3][2] * projection[2][1] + view[3][3] * projection[3][1];
	tmp[14] = view[3][0] * projection[0][2] + view[3][1] * projection[1][2] + view[3][2] * projection[2][2] + view[3][3] * projection[3][2];
	tmp[15] = view[3][0] * projection[0][3] + view[3][1] * projection[1][3] + view[3][2] * projection[2][3] + view[3][3] * projection[3][3];

	frustum[0].x = tmp[3] + tmp[0];
	frustum[0].y = tmp[7] + tmp[4];
	frustum[0].z = tmp[11] + tmp[8];
	frustum[0].w = tmp[15] + tmp[12];

	frustum[1].x = tmp[3] - tmp[0];
	frustum[1].y = tmp[7] - tmp[4];
	frustum[1].z = tmp[11] - tmp[8];
	frustum[1].w = tmp[15] - tmp[12];

	frustum[2].x = tmp[3] + tmp[1];
	frustum[2].y = tmp[7] + tmp[5];
	frustum[2].z = tmp[11] + tmp[9];
	frustum[2].w = tmp[15] + tmp[13];

	frustum[3].x = tmp[3] - tmp[1];
	frustum[3].y = tmp[7] - tmp[5];
	frustum[3].z = tmp[11] - tmp[9];
	frustum[3].w = tmp[15] - tmp[13];

	frustum[4].x = tmp[3] + tmp[2];
	frustum[4].y = tmp[7] + tmp[6];
	frustum[4].z = tmp[11] + tmp[10];
	frustum[4].w = tmp[15] + tmp[14];

	frustum[5].x = tmp[3] - tmp[2];
	frustum[5].y = tmp[7] - tmp[6];
	frustum[5].z = tmp[11] - tmp[10];
	frustum[5].w = tmp[15] - tmp[14];

	for (int i = 0; i < 6; i++) {
		float magnitude = std::sqrt(frustum[i].x * frustum[i].x + frustum[i].y * frustum[i].y + frustum[i].z * frustum[i].z);
		frustum[i].x /= magnitude;
		frustum[i].y /= magnitude;
		frustum[i].z /= magnitude;
		frustum[i].w /= magnitude;
	}
}

bool Frustum::collision(const AABB& other) {
	for (int i = 0; i < 6; i++) {
		if (((frustum[i].x * other.min.x + frustum[i].y * other.min.y + frustum[i].z * other.min.z + frustum[i].w) <= 0.0f)
			&& ((frustum[i].x * other.max.x + frustum[i].y * other.min.y + frustum[i].z * other.min.z + frustum[i].w) <= 0.0f)
			&& ((frustum[i].x * other.min.x + frustum[i].y * other.max.y + frustum[i].z * other.min.z + frustum[i].w) <= 0.0f)
			&& ((frustum[i].x * other.max.x + frustum[i].y * other.max.y + frustum[i].z * other.min.z + frustum[i].w) <= 0.0f)
			&& ((frustum[i].x * other.min.x + frustum[i].y * other.min.y + frustum[i].z * other.max.z + frustum[i].w) <= 0.0f)
			&& ((frustum[i].x * other.max.x + frustum[i].y * other.min.y + frustum[i].z * other.max.z + frustum[i].w) <= 0.0f)
			&& ((frustum[i].x * other.min.x + frustum[i].y * other.max.y + frustum[i].z * other.max.z + frustum[i].w) <= 0.0f)
			&& ((frustum[i].x * other.max.x + frustum[i].y * other.max.y + frustum[i].z * other.max.z + frustum[i].w) <= 0.0f)) {
			return false;
		}
	}

	return true;
}
