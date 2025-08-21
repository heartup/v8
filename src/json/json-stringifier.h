// Copyright 2012 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_JSON_JSON_STRINGIFIER_H_
#define V8_JSON_JSON_STRINGIFIER_H_

#include "src/objects/objects.h"
#include <string>

namespace v8 {
namespace internal {

    // 添加回调函数类型定义
typedef void (*JSONStringifyCallback)(const std::string& json_content, void* user_data);

// 设置回调的函数
void SetJSONStringifyCallback(JSONStringifyCallback callback, void* user_data);

V8_WARN_UNUSED_RESULT MaybeDirectHandle<Object> JsonStringify(
    Isolate* isolate, Handle<JSAny> object, Handle<JSAny> replacer,
    Handle<Object> gap);
}  // namespace internal
}  // namespace v8

#endif  // V8_JSON_JSON_STRINGIFIER_H_
