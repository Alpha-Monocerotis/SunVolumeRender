//
// Created by 孙万捷 on 16/3/4.
//

#ifndef SUNVOLUMERENDER_PATHTRACER_H
#define SUNVOLUMERENDER_PATHTRACER_H

#define GLM_FORCE_INLINE
#include <glm/glm.hpp>

#include "core/cuda_camera.h"
#include "core/geometry/cuda_bbox.h"
#include "core/cuda_transfer_function.h"
#include "core/render_parameters.h"

// kernel
extern "C" void render_pathtracer(glm::u8vec4* img, const RenderParams& renderParams);

// setup functions
extern "C" void setup_volume(const cudaVolume& vol);
extern "C" void setup_transferfunction(const cudaTransferFunction& tf);
extern "C" void setup_camera(const cudaCamera& cam);
extern "C" void setup_env_lights(const cudaEnvironmentLight& light);
extern "C" void setup_area_lights(cudaAreaLight* lights, uint32_t n);

#endif //SUNVOLUMERENDER_PATHTRACER_H
