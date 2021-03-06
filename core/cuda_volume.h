//
// Created by 孙万捷 on 16/5/19.
//

#ifndef SUNVOLUMERENDER_VOLUME_H
#define SUNVOLUMERENDER_VOLUME_H

#include <cuda_runtime.h>

#define GLM_FORCE_INLINE
#include <glm/glm.hpp>

#include "geometry/cuda_bbox.h"

class cudaVolume
{
public:
    __host__ __device__ void Set(const cudaBBox& bbox, const glm::vec3& spacing, const cudaTextureObject_t& tex)
    {
        this->bbox = bbox;
        this->spacing = spacing;
        this->invSpacing = 1.f / spacing;
        this->tex = tex;
    }

    __host__ __device__ void SetClipPlane(const glm::vec2& x_clip, const glm::vec2& y_clip, const glm::vec2& z_clip)
    {
        this->x_clip = x_clip;
        this->y_clip = y_clip;
        this->z_clip = z_clip;
    }

    __host__ __device__ void SetXClipPlane(const glm::vec2& clip) {x_clip = clip;}
    __host__ __device__ void SetYClipPlane(const glm::vec2& clip) {y_clip = clip;}
    __host__ __device__ void SetZClipPlane(const glm::vec2& clip) {z_clip = clip;}

    __host__ __device__ void SetDensityScale(float s = 1.f) {densityScale = s;}

    __device__ float operator ()(const glm::vec3& pointInWorld) const
    {
        return GetIntensity(pointInWorld);
    }

    __device__ float operator ()(const glm::vec3& normalizedTexCoord, bool dummy) const
    {
        return GetIntensityNTC(normalizedTexCoord);
    }

    __device__ bool Intersect(const cudaRay& ray, float* tNear, float* tFar) const
    {
        return bbox.Intersect(ray, tNear, tFar, x_clip, y_clip, z_clip);
    }

    __device__ glm::vec3 Gradient_CentralDiff(const glm::vec3& pointInWorld) const
    {
        auto xdiff = GetIntensity(pointInWorld + glm::vec3(spacing.x, 0.f, 0.f)) - GetIntensity(pointInWorld - glm::vec3(spacing.x, 0.f, 0.f));
        auto ydiff = GetIntensity(pointInWorld + glm::vec3(0.f, spacing.y, 0.f)) - GetIntensity(pointInWorld - glm::vec3(0.f, spacing.y, 0.f));
        auto zdiff = GetIntensity(pointInWorld + glm::vec3(0.f, 0.f, spacing.z)) - GetIntensity(pointInWorld - glm::vec3(0.f, 0.f, spacing.z));

        return glm::vec3(xdiff, ydiff, zdiff) * 0.5f * invSpacing;
    }

    __device__ glm::vec3 NormalizedGradient(const glm::vec3& pointInWorld) const
    {
        return glm::normalize(Gradient_CentralDiff(pointInWorld));
    }

    __device__ bool IsInside(const glm::vec3& ptInWorld) const
    {
        return bbox.IsInside(ptInWorld);
    }

    __host__ __device__ glm::vec3 GetSize() const
    {
        return 1.f / bbox.invSize;
    }

    __host__ __device__ void SetInvMaxMagnitude(float invMag) {invMaxMagnitude = invMag;}

    __host__ __device__ float GetInvMaxMagnitude() const {return invMaxMagnitude;}

    __host__ __device__ void SetGradientFactor(float g) {gradientFactor = g;}

    __host__ __device__ float GetGradientFactor() const {return gradientFactor;}

private:
    __device__ glm::vec3 GetNormalizedTexCoord(const glm::vec3& pointInWorld) const
    {
        return (pointInWorld - bbox.vmin) * bbox.invSize;
    }

    __device__ float GetIntensity(const glm::vec3& pointInWorld) const
    {
#ifdef __CUDACC__
        auto texCoord = GetNormalizedTexCoord(pointInWorld);
        return tex3D<float>(tex, texCoord.x, texCoord.y, texCoord.z) * densityScale;
#else
        return 0.f;
#endif
    }

    __device__ float GetIntensityNTC(const glm::vec3& normalizedTexCoord) const
    {
#ifdef __CUDACC__
        return tex3D<float>(tex, normalizedTexCoord.x, normalizedTexCoord.y, normalizedTexCoord.z);
#else
        return 0.f;
#endif
    }

private:
    cudaBBox bbox;
    cudaTextureObject_t tex;
    float densityScale;
    float invMaxMagnitude;
    float gradientFactor;
    glm::vec3 spacing;
    glm::vec3 invSpacing;
    glm::vec2 x_clip;
    glm::vec2 y_clip;
    glm::vec2 z_clip;
};

struct VolumeSample
{
    glm::vec3 ptInWorld;
    glm::vec3 wo;
    float intensity;
    glm::vec3 gradient;
    float gradientMagnitude;
    glm::vec4 color_opacity;
};

#endif //SUNVOLUMERENDER_VOLUME_H
