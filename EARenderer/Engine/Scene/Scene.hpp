//
//  Scene.hpp
//  EARenderer
//
//  Created by Pavlo Muratov on 24.03.17.
//  Copyright © 2017 MPO. All rights reserved.
//

#ifndef Scene_hpp
#define Scene_hpp

#include "PackedLookupTable.inl"
#include "Mesh.hpp"
#include "SubMesh.hpp"
#include "Transformation.hpp"
#include "Camera.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "Material.hpp"
#include "Skybox.hpp"

namespace EARenderer {
    
    class Scene {
    private:
        PackedLookupTable<Mesh> mMeshes;
        PackedLookupTable<SubMesh> mSubMeshes;
        PackedLookupTable<Transformation> mTransforms;
        PackedLookupTable<DirectionalLight> mDirectionalLights;
        PackedLookupTable<PointLight> mPointLights;
        PackedLookupTable<Material> mMaterials;
        
        Camera *mCamera;
        Skybox *mSkybox;
        
    public:
        Scene();
        
        PackedLookupTable<Mesh>& meshes();
        PackedLookupTable<SubMesh>& subMeshes();
        PackedLookupTable<Transformation>& transforms();
        PackedLookupTable<Camera>& cameras();
        PackedLookupTable<DirectionalLight>& directionalLights();
        PackedLookupTable<PointLight>& pointLights();
        PackedLookupTable<Material>& materials();
        
        void setCamera(Camera* camera);
        Camera* camera() const;
        
        void setSkybox(Skybox* skybox);
        Skybox* skybox() const;
    };
    
}

#endif /* Scene_hpp */