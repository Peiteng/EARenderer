//
//  DeferredSceneRenderer.cpp
//  EARenderer
//
//  Created by Pavlo Muratov on 30.06.2018.
//  Copyright © 2018 MPO. All rights reserved.
//

#include "DeferredSceneRenderer.hpp"
#include "GLShader.hpp"
#include "ResourcePool.hpp"
#include "Vertex1P4.hpp"
#include "Collision.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace EARenderer {

#pragma mark - Lifecycle

    DeferredSceneRenderer::DeferredSceneRenderer(const Scene* scene,
                                                 const DefaultRenderComponentsProviding *provider,
                                                 const RenderingSettings& settings,
                                                 std::shared_ptr<const SurfelData> surfelData,
                                                 std::shared_ptr<const DiffuseLightProbeData> diffuseProbeData,
                                                 std::shared_ptr<const GLHDRTexture2DArray> GBuffer)
    :
    mScene(scene),
    mDefaultRenderComponentsProvider(provider),
    mSettings(settings),
    mSurfelData(surfelData),
    mDiffuseProbeData(diffuseProbeData),
    mGBuffer(GBuffer),
    mProbeGridResolution(scene->preferredProbeGridResolution()),

    // Shadow maps
    mDepthRenderbuffer(Size2D(1500)),
    mDirectionalExponentialShadowMap(Size2D(1500)),
    mDirectionalShadowFramebuffer(Size2D(1500)),

    // Surfels and surfel clusters
    mSurfelsLuminanceMap(surfelData->surfelsGBuffer()->size(), GLTexture::Filter::None),
    mSurfelClustersLuminanceMap(surfelData->surfelClustersGBuffer()->size(), GLTexture::Filter::None),
    mSurfelsLuminanceFramebuffer(mSurfelsLuminanceMap.size()),
    mSurfelClustersLuminanceFramebuffer(mSurfelClustersLuminanceMap.size()),

    // Diffuse light probes
    mGridProbesSHMaps {
        GLLDRTexture3D(Size2D(mProbeGridResolution.x, mProbeGridResolution.y), mProbeGridResolution.z),
        GLLDRTexture3D(Size2D(mProbeGridResolution.x, mProbeGridResolution.y), mProbeGridResolution.z),
        GLLDRTexture3D(Size2D(mProbeGridResolution.x, mProbeGridResolution.y), mProbeGridResolution.z),
        GLLDRTexture3D(Size2D(mProbeGridResolution.x, mProbeGridResolution.y), mProbeGridResolution.z)
    },
    mGridProbesSHFramebuffer(Size2D(mProbeGridResolution.x, mProbeGridResolution.y)),

    // Output frame
    mOutputFrame(settings.resolution),
    mThresholdFilteredOutputFrame(mOutputFrame.size()),
    mOutputDepthRenderbuffer(mOutputFrame.size()),
    mOutputFramebuffer(mOutputFrame.size()),

    // Effects
    mShadowBlurEffect(std::shared_ptr<const GLHDRTexture2D>(&mDirectionalExponentialShadowMap)),
    mBloomEffect(std::shared_ptr<const GLHDRTexture2D>(&mOutputFrame),
                 std::shared_ptr<const GLHDRTexture2D>(&mThresholdFilteredOutputFrame)),
    mToneMappingEffect(mBloomEffect.outputImage())
    {
        setupGLState();
        setupFramebuffers();
    }

#pragma mark - Setters

    void DeferredSceneRenderer::setRenderingSettings(const RenderingSettings& settings) {
        mSettings = settings;
    }

#pragma mark - Getters

    const FrustumCascades& DeferredSceneRenderer::shadowCascades() const {
        return mShadowCascades;
    }

#pragma mark - Initial setup

    void DeferredSceneRenderer::setupGLState() {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glDisable(GL_BLEND);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClearDepth(1.0);
        glDepthFunc(GL_LEQUAL);
    }

    void DeferredSceneRenderer::setupFramebuffers() {
        mDirectionalShadowFramebuffer.attachTexture(mDirectionalExponentialShadowMap);
        mDirectionalShadowFramebuffer.attachRenderbuffer(mDepthRenderbuffer);

        mSurfelsLuminanceFramebuffer.attachTexture(mSurfelsLuminanceMap);
        mSurfelClustersLuminanceFramebuffer.attachTexture(mSurfelClustersLuminanceMap);

        mGridProbesSHFramebuffer.attachTexture(mGridProbesSHMaps[0], GLFramebuffer::ColorAttachment::Attachment0);
        mGridProbesSHFramebuffer.attachTexture(mGridProbesSHMaps[1], GLFramebuffer::ColorAttachment::Attachment1);
        mGridProbesSHFramebuffer.attachTexture(mGridProbesSHMaps[2], GLFramebuffer::ColorAttachment::Attachment2);
        mGridProbesSHFramebuffer.attachTexture(mGridProbesSHMaps[3], GLFramebuffer::ColorAttachment::Attachment3);

        mOutputFramebuffer.attachTexture(mOutputFrame, GLFramebuffer::ColorAttachment::Attachment0);
        mOutputFramebuffer.attachTexture(mThresholdFilteredOutputFrame, GLFramebuffer::ColorAttachment::Attachment1);
        mOutputFramebuffer.attachRenderbuffer(mOutputDepthRenderbuffer);
    }

#pragma mark - Rendering
#pragma mark - Runtime

    void DeferredSceneRenderer::bindDefaultFramebuffer() {
        if (mDefaultRenderComponentsProvider) {
            mDefaultRenderComponentsProvider->bindSystemFramebuffer();
            mDefaultRenderComponentsProvider->defaultViewport().apply();
        }
    }

    void DeferredSceneRenderer::performDepthPrepass() {
        bindDefaultFramebuffer();
        glClear(GL_DEPTH_BUFFER_BIT);

        mDepthPrepassShader.bind();
        mDepthPrepassShader.setCamera(*mScene->camera());

        for (ID instanceID : mScene->meshInstances()) {
            auto& instance = mScene->meshInstances()[instanceID];
            auto& subMeshes = ResourcePool::shared().meshes[instance.meshID()].subMeshes();

            mDepthPrepassShader.setModelMatrix(instance.transformation().modelMatrix());

            for (ID subMeshID : subMeshes) {
                auto& subMesh = subMeshes[subMeshID];
                subMesh.draw();
            }
        }
    }

    void DeferredSceneRenderer::renderExponentialShadowMapsForDirectionalLight() {
        mDirectionalESMShader.bind();
        mDirectionalShadowFramebuffer.bind();
        mDirectionalShadowFramebuffer.viewport().apply();

        mDirectionalESMShader.setESMFactor(mSettings.meshSettings.ESMFactor);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (size_t cascade = 0; cascade < mShadowCascades.amount; cascade++) {
            // Ensure only one texture channel will be written to for each respective cascade
            glColorMask(cascade == 0, cascade == 1, cascade == 2, cascade == 3);

            mDirectionalESMShader.setLightMatrix(mShadowCascades.lightViewProjections[cascade]);

            for (ID meshInstanceID : mScene->meshInstances()) {
                auto& instance = mScene->meshInstances()[meshInstanceID];
                auto& subMeshes = ResourcePool::shared().meshes[instance.meshID()].subMeshes();

                mDirectionalESMShader.setModelMatrix(instance.transformation().modelMatrix());

                for (ID subMeshID : subMeshes) {
                    auto& subMesh = subMeshes[subMeshID];
                    subMesh.draw();
                }
            }
        }

        glColorMask(true, true, true, true);

        mShadowBlurEffect.blur(mSettings.meshSettings.shadowBlur);
    }

    void DeferredSceneRenderer::relightSurfels() {
        const DirectionalLight& directionalLight = mScene->directionalLight();

        mSurfelsLuminanceFramebuffer.bind();
        mSurfelsLuminanceFramebuffer.viewport().apply();

        mSurfelLightingShader.bind();
        mSurfelLightingShader.setLight(directionalLight);
        mSurfelLightingShader.setMultibounceEnabled(mSettings.meshSettings.lightMultibounceEnabled);
        mSurfelLightingShader.ensureSamplerValidity([&]() {
            mSurfelLightingShader.setShadowCascades(mShadowCascades);
            mSurfelLightingShader.setExponentialShadowMap(mDirectionalExponentialShadowMap);
            mSurfelLightingShader.setSurfelsGBuffer(*mSurfelData->surfelsGBuffer());
            mSurfelLightingShader.setGridProbesSHTextures(mGridProbesSHMaps);
            mSurfelLightingShader.setWorldBoundingBox(mScene->lightBakingVolume());
            mSurfelLightingShader.setProbePositions(*mDiffuseProbeData->probePositionsBufferTexture());
            mSurfelLightingShader.setSettings(mSettings);
        });

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void DeferredSceneRenderer::averageSurfelClusterLuminances() {
        mSurfelClusterAveragingShader.bind();
        mSurfelClustersLuminanceFramebuffer.bind();
        mSurfelClustersLuminanceFramebuffer.viewport().apply();

        mSurfelClusterAveragingShader.ensureSamplerValidity([&]() {
            mSurfelClusterAveragingShader.setSurfelClustersGBuffer(*mSurfelData->surfelClustersGBuffer());
            mSurfelClusterAveragingShader.setSurfelsLuminaceMap(mSurfelsLuminanceMap);
        });

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void DeferredSceneRenderer::updateGridProbes() {
        mGridProbesUpdateShader.bind();
        mGridProbesUpdateShader.ensureSamplerValidity([&] {
            mGridProbesUpdateShader.setSurfelClustersLuminaceMap(mSurfelClustersLuminanceMap);
            mGridProbesUpdateShader.setProbeProjectionsMetadata(*mDiffuseProbeData->probeClusterProjectionsMetadataBufferTexture());
            mGridProbesUpdateShader.setProjectionClusterIndices(*mDiffuseProbeData->projectionClusterIndicesBufferTexture());
            mGridProbesUpdateShader.setProjectionClusterSphericalHarmonics(*mDiffuseProbeData->projectionClusterSHsBufferTexture());
            mGridProbesUpdateShader.setProbesGridResolution(mProbeGridResolution);
        });

        mGridProbesSHFramebuffer.bind();
        mGridProbesSHFramebuffer.viewport().apply();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)mProbeGridResolution.z);
    }

#pragma mark - Public interface

    void DeferredSceneRenderer::renderMeshes() {
        mShadowCascades = mScene->directionalLight().cascadesForWorldBoundingBox(mScene->boundingBox());

        renderExponentialShadowMapsForDirectionalLight();
        relightSurfels();
        averageSurfelClusterLuminances();
        updateGridProbes();

        mCookTorranceShader.bind();

        mCookTorranceShader.setSettings(mSettings);
        mCookTorranceShader.setCamera(*(mScene->camera()));
        mCookTorranceShader.setLight(mScene->directionalLight());
        mCookTorranceShader.setWorldBoundingBox(mScene->lightBakingVolume());
        mCookTorranceShader.setProbePositions(*mDiffuseProbeData->probePositionsBufferTexture());
        mCookTorranceShader.setFrustumCascades(mShadowCascades);

        mCookTorranceShader.ensureSamplerValidity([&]() {
            mCookTorranceShader.setGBuffer(*mGBuffer);
            mCookTorranceShader.setExponentialShadowMap(*mShadowBlurEffect.outputImage());
            mCookTorranceShader.setGridProbesSHTextures(mGridProbesSHMaps);
        });

        //        mOutputFramebuffer.bind();
        //        mOutputFramebuffer.viewport().apply();
        bindDefaultFramebuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void DeferredSceneRenderer::renderSkybox() {
        bindDefaultFramebuffer();
        mSkyboxShader.bind();
        mSkyboxShader.ensureSamplerValidity([this]() {
            mSkyboxShader.setViewMatrix(mScene->camera()->viewMatrix());
            mSkyboxShader.setProjectionMatrix(mScene->camera()->projectionMatrix());
            mSkyboxShader.setEquirectangularMap(mScene->skybox()->equirectangularMap());
        });
        mScene->skybox()->draw();
    }

}