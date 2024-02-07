//
// Created by root on 2/7/24.
//

#ifndef NODE_LIBVIRT_ASSERT_H
#define NODE_LIBVIRT_ASSERT_H

#define assert(handle, message) \
    if(!(handle)){ \
        Napi::Error::New(info.Env(), message).ThrowAsJavaScriptException(); \
        return info.Env().Undefined(); \
    }

#define assert_void(handle, message) \
    if(!(handle)){ \
        Napi::Error::New(info.Env(), message).ThrowAsJavaScriptException(); \
        return; \
    }
#endif //NODE_LIBVIRT_ASSERT_H
