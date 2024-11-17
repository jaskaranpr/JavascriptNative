#pragma once

#include <quickjs.h>
#include <string>
#include <functional>
#include <memory>

class QuickJSWrapper {
public:
    QuickJSWrapper() {
        runtime_ = JS_NewRuntime();
        context_ = JS_NewContext(runtime_);
    }
    
    ~QuickJSWrapper() {
        if (context_) {
            JS_FreeContext(context_);
        }
        if (runtime_) {
            JS_FreeRuntime(runtime_);
        }
    }

    // Add global function
    void addFunction(const char* name, JSCFunction func, int length) {
        JSValue global = JS_GetGlobalObject(context_);
        JS_SetPropertyStr(context_, global, name, 
            JS_NewCFunction(context_, func, name, length));
        JS_FreeValue(context_, global);
    }

    // Add global number constant
    void addConstant(const char* name, double value) {
        JSValue global = JS_GetGlobalObject(context_);
        JS_SetPropertyStr(context_, global, name, JS_NewFloat64(context_, value));
        JS_FreeValue(context_, global);
    }

    // Evaluate JavaScript code
    bool evaluate(const std::string& code, std::string* error = nullptr) {
        JSValue result = JS_Eval(context_, code.c_str(), code.length(), 
                               "<input>", JS_EVAL_TYPE_GLOBAL);
        
        bool success = !JS_IsException(result);
        if (!success && error) {
            JSValue exception = JS_GetException(context_);
            const char* str = JS_ToCString(context_, exception);
            if (str) {
                *error = str;
                JS_FreeCString(context_, str);
            }
            JS_FreeValue(context_, exception);
        }
        
        JS_FreeValue(context_, result);
        return success;
    }

    JSContext* context() { return context_; }
    JSRuntime* runtime() { return runtime_; }

private:
    JSRuntime* runtime_;
    JSContext* context_;
};