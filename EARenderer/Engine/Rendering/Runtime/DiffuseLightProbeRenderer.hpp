//
//  DiffuseLightProbeRenderer.hpp
//  EARenderer
//
//  Created by Pavlo Muratov on 01.10.2018.
//  Copyright © 2018 MPO. All rights reserved.
//

#ifndef DiffuseLightProbeRenderer_hpp
#define DiffuseLightProbeRenderer_hpp

#include "Scene.hpp"
#include "DiffuseLightProbeData.hpp"
#include "GLSLGridLightProbeRendering.hpp"
#include "RenderingSettings.hpp"

#include <memory>

namespace EARenderer {

    class DiffuseLightProbeRenderer {
    private:
        const Scene *mScene;
        std::shared_ptr<const DiffuseLightProbeData> mProbeData;
        std::shared_ptr<const std::array<GLLDRTexture3D, 4>> mSphericalHarmonics;
        GLVertexArray<DiffuseLightProbe> mDiffuseProbesVAO;
        GLSLGridLightProbeRendering mGridProbeRenderingShader;
        RenderingSettings mRenderingSettings;

    public:
        DiffuseLightProbeRenderer(const Scene *scene,
                                  std::shared_ptr<const DiffuseLightProbeData> probeData,
                                  std::shared_ptr<const std::array<GLLDRTexture3D, 4>> sphericalHarmonics);

        void setRenderingSettings(const RenderingSettings& settings);

        void render();
    };

}

#endif /* DiffuseLightProbeRenderer_hpp */