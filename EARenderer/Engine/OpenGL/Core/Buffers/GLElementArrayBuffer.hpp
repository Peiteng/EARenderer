//
//  GLElementArrayBuffer.hpp
//  EARenderer
//
//  Created by Pavlo Muratov on 18.03.17.
//  Copyright © 2017 MPO. All rights reserved.
//

#ifndef GLElementArrayBuffer_hpp
#define GLElementArrayBuffer_hpp

#include <OpenGL/OpenGL.h>
#include "GLBuffer.hpp"

namespace EARenderer {

    class GLElementArrayBuffer : public GLBuffer<GLushort> {
    public:
        GLElementArrayBuffer();
    };

}

#endif /* GLElementArrayBuffer_hpp */