//
// Created by root on 1/14/24.
//

#ifndef NODE_LIBVIRT_PROMISEWORKER_H
#define NODE_LIBVIRT_PROMISEWORKER_H

#include <napi.h>
#include <libvirt/libvirt.h>
#include <stdexcept>

class PromiseWorker : public Napi::AsyncWorker {
public:
    PromiseWorker(const Napi::Promise::Deferred &deferred, std::function<void(PromiseWorker *)> &&asyncFunction)
            : Napi::AsyncWorker(deferred.Env()), deferred_(deferred), asyncFunction_(std::move(asyncFunction)) {}

    void Execute() override {
        try {
            asyncFunction_(this);
        } catch (const std::exception &e) {
            SetError(e.what());
        }
    }

    void SetResult(Napi::Value val) {
        val_ = val;
    }


    void OnOK() override {
        Napi::HandleScope scope(Env());
        deferred_.Resolve(val_);
    }

    void OnError(const Napi::Error &e) override {
        Napi::HandleScope scope(Env());
        deferred_.Reject(e.Value());
    }

private:
    Napi::Promise::Deferred deferred_;
    std::function<void(PromiseWorker *)> asyncFunction_;
    Napi::Value val_;
};


#endif //NODE_LIBVIRT_PROMISEWORKER_H
