//
//  Scene.cpp
//  EARenderer
//
//  Created by Pavlo Muratov on 24.03.17.
//  Copyright © 2017 MPO. All rights reserved.
//

#include "Scene.hpp"
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>

namespace EARenderer {
    
    Scene::Scene()
    :
    mMeshes(1000),
    mSubMeshes(1000),
    mTransforms(1000),
    mDirectionalLights(10),
    mPointLights(10),
    mMaterials(1000)
    { }
    
    PackedLookupTable<Mesh>& Scene::meshes() {
        return mMeshes;
    }
    
    PackedLookupTable<SubMesh>& Scene::subMeshes() {
        return mSubMeshes;
    }
    
    PackedLookupTable<Transformation>& Scene::transforms() {
        return mTransforms;
    }

    PackedLookupTable<DirectionalLight>& Scene::directionalLights() {
        return mDirectionalLights;
    }
    
    PackedLookupTable<PointLight>& Scene::pointLights() {
        return mPointLights;
    }

    PackedLookupTable<Material>& Scene::materials() {
        return mMaterials;
    }
    
    void Scene::setCamera(Camera* camera) {
        mCamera = camera;
    }
    
    Camera* Scene::camera() const {
        return mCamera;
    }
    
    void Scene::setSkybox(Skybox* skybox) {
        mSkybox = skybox;
    }
    
    Skybox* Scene::skybox() const {
        return mSkybox;
    }
    
}