//
// Created by root on 2/10/24.
//

#ifndef NODE_LIBVIRT_ERROR_H
#define NODE_LIBVIRT_ERROR_H


#define virt_error_check(condition) \
    if((condition)){ \
         Napi::Error::New(info.Env(), std::string(virGetLastError()->message)).ThrowAsJavaScriptException(); \
         return info.Env().Undefined(); \
    }

#define virt_error_check_void(condition) \
    if((condition)){ \
        Napi::Error::New(info.Env(), std::string(virGetLastError()->message)).ThrowAsJavaScriptException(); \
        return; \
    }

#define virt_error_check_last_void() \
    auto err = virGetLastError(); \
    if(err != nullptr){ \
        Napi::Error::New(info.Env(), std::string(err->message)).ThrowAsJavaScriptException(); \
        return; \
    }

#define virt_error_check_last() \
    auto err = virGetLastError(); \
    if(err != nullptr){ \
        Napi::Error::New(info.Env(), std::string(err->message)).ThrowAsJavaScriptException(); \
        return env.Undefined(); \
    }

#endif //NODE_LIBVIRT_ERROR_H
