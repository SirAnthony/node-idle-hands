#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include <node.h>
#include <uv.h>

using namespace v8;

Persistent<Function> cb;
uv_idle_t idler;

/**
 * callback for idle watcher
 **/
void idle_event(uv_idle_t* handle) {
    const unsigned argc = 1;
    Isolate* isolate = Isolate::GetCurrent();
    Local<Value> argv[argc] = {};
    Local<Function> callback = Local<Function>::New(isolate, cb);
    callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
}

/**
 * start idle watcher
 * usage: start(js_callback)
 **/
void start(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (!args.Length() || !args[0]->IsFunction()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
            isolate, "First argument must be a function")));
        return;
    }

    // Don't start if already running
    if (!cb.IsEmpty()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
            isolate, "Already started")));
        return;
    }

    cb.Reset(isolate, Local<Function>::Cast(args[0]));

    uv_idle_init(uv_default_loop(), &idler);
    uv_idle_start(&idler, idle_event);
}

/**
 * stop idle watcher
 **/
void stop(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (cb.IsEmpty()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
            isolate, "Already stopped")));
        return;
    }

    cb.Reset();

    uv_idle_stop(&idler);
}

void Init(Handle<Object> exports) {
    NODE_SET_METHOD(exports, "start", start);
    NODE_SET_METHOD(exports, "stop", stop);
}

NODE_MODULE(idle_hands, Init)
