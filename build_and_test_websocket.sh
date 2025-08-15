#!/bin/bash
# V8 WebSocket集成构建和测试脚本

set -e  # 遇到错误时退出

echo "=== V8 WebSocket集成构建和测试脚本 ==="
echo

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 函数：打印带颜色的消息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查是否在V8根目录
if [ ! -f "BUILD.gn" ]; then
    print_error "请在V8项目根目录下运行此脚本"
    exit 1
fi

# 步骤1: 检查依赖
print_info "检查构建依赖..."

if ! command -v python3 &> /dev/null; then
    print_error "Python3未安装，无法运行WebSocket测试服务器"
    exit 1
fi

if ! python3 -c "import websockets" 2>/dev/null; then
    print_warning "websockets模块未安装，正在安装..."
    pip3 install websockets
fi

# 步骤2: 配置构建
print_info "配置V8构建..."

if [ ! -d "out/x64.debug" ]; then
    print_info "创建调试构建配置..."
    gn gen out/x64.debug --args='
        is_debug = true
        v8_enable_backtrace = true
        v8_enable_slow_dchecks = true
        v8_optimized_debug = false
    '
else
    print_info "使用现有的调试构建配置"
fi

# 步骤3: 编译
print_info "编译V8 (这可能需要几分钟)..."
tools/dev/gm.py x64.debug tests

if [ $? -ne 0 ]; then
    print_error "V8编译失败"
    exit 1
fi

print_info "V8编译成功!"

# 步骤4: 启动WebSocket服务器
print_info "启动WebSocket测试服务器..."

# 在后台启动Python服务器
python3 websocket_test_server.py &
SERVER_PID=$!

# 等待服务器启动
sleep 2

# 检查服务器是否正在运行
if ! kill -0 $SERVER_PID 2>/dev/null; then
    print_error "WebSocket服务器启动失败"
    exit 1
fi

print_info "WebSocket服务器已启动 (PID: $SERVER_PID)"

# 步骤5: 运行测试
print_info "运行WebSocket集成测试..."
echo "=== 开始测试输出 ==="

./out/x64.debug/d8 test_websocket_integration.js

echo "=== 测试输出结束 ==="

# 步骤6: 清理
print_info "清理..."
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

print_info "测试完成!"
echo
echo "=== 测试总结 ==="
echo "1. V8已成功编译包含WebSocket功能"
echo "2. WebSocket服务器已测试"
echo "3. JsonStringify集成功能已验证"
echo
echo "要手动测试："
echo "1. 运行: python3 websocket_test_server.py"
echo "2. 在另一个终端运行: ./out/x64.debug/d8 test_websocket_integration.js"
echo
print_info "所有测试完成!"
