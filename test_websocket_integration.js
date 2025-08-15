// 测试WebSocket集成的JavaScript文件
// 用于验证JsonStringify中的WebSocket功能

console.log("开始测试WebSocket集成...");

// 测试用例1: 包含关键字"lhh"的对象
const testObject1 = {
    name: "lhh",
    message: "Hello WebSocket!",
    timestamp: Date.now()
};

console.log("测试1: 包含关键字'lhh'的对象");
const jsonString1 = JSON.stringify(testObject1);
console.log("结果:", jsonString1);

// 测试用例2: 包含关键字"error"的对象
const testObject2 = {
    type: "error",
    message: "Something went wrong",
    code: 500
};

console.log("\n测试2: 包含关键字'error'的对象");
const jsonString2 = JSON.stringify(testObject2);
console.log("结果:", jsonString2);

// 测试用例3: 包含关键字"warning"的对象
const testObject3 = {
    level: "warning",
    message: "This is a warning message",
    source: "test"
};

console.log("\n测试3: 包含关键字'warning'的对象");
const jsonString3 = JSON.stringify(testObject3);
console.log("结果:", jsonString3);

// 测试用例4: 不包含任何关键字的对象（不会触发WebSocket发送）
const testObject4 = {
    name: "John",
    age: 30,
    city: "New York"
};

console.log("\n测试4: 不包含关键字的对象（不会触发WebSocket）");
const jsonString4 = JSON.stringify(testObject4);
console.log("结果:", jsonString4);

// 测试用例5: 复杂嵌套对象包含关键字"important"
const testObject5 = {
    user: {
        name: "Alice",
        preferences: {
            notifications: {
                important: true,
                email: true
            }
        }
    },
    data: [1, 2, 3, 4, 5]
};

console.log("\n测试5: 嵌套对象包含关键字'important'");
const jsonString5 = JSON.stringify(testObject5);
console.log("结果:", jsonString5);

console.log("\n测试完成!");
console.log("如果WebSocket连接成功，应该在服务器端看到相应的消息。");
