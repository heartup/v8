#!/usr/bin/env python3
"""
简单的WebSocket测试服务器
用于接收来自V8 JsonStringify的消息
"""

import asyncio
import websockets
import json
from datetime import datetime

# 全局客户端集合
clients = set()

async def register_client(websocket):
    """注册新的客户端连接"""
    clients.add(websocket)
    print(f"客户端已连接: {websocket.remote_address}")
    print(f"当前连接数: {len(clients)}")

async def unregister_client(websocket):
    """注销客户端连接"""
    clients.discard(websocket)
    print(f"客户端已断开: {websocket.remote_address}")
    print(f"当前连接数: {len(clients)}")

async def handle_message(websocket, message):
    """处理接收到的消息"""
    try:
        # 尝试解析JSON消息
        data = json.loads(message)
        
        print(f"\n=== 收到来自V8的消息 ===")
        print(f"时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"消息类型: {data.get('type', 'unknown')}")
        print(f"匹配关键字: {data.get('keyword', 'none')}")
        print(f"时间戳: {data.get('timestamp', 'none')}")
        print(f"JSON内容: {data.get('content', 'none')}")
        print("=" * 40)
        
        # 发送确认消息回复
        response = {
            "type": "ack",
            "status": "received",
            "original_keyword": data.get('keyword'),
            "server_time": datetime.now().isoformat()
        }
        
        await websocket.send(json.dumps(response))
        
    except json.JSONDecodeError:
        print(f"收到非JSON消息: {message}")
    except Exception as e:
        print(f"处理消息时出错: {e}")

async def client_handler(websocket):
    """处理单个客户端连接"""
    print(f"新的连接请求")
    await register_client(websocket)
    try:
        async for message in websocket:
            await handle_message(websocket, message)
    except websockets.exceptions.ConnectionClosed:
        print("客户端连接已关闭")
    except Exception as e:
        print(f"客户端处理错误: {e}")
    finally:
        await unregister_client(websocket)

async def start_server():
    """启动WebSocket服务器"""
    host = 'localhost'
    port = 8080
    
    print(f"启动WebSocket服务器: ws://{host}:{port}")
    print("等待来自V8 JsonStringify的连接...")
    print("按Ctrl+C停止服务器")
    
    # 启动服务器并等待
    async with websockets.serve(client_handler, host, port) as server:
        print(f"服务器已启动在 {host}:{port}")
        await server.serve_forever()

def main():
    """主函数"""
    try:
        asyncio.run(start_server())
    except KeyboardInterrupt:
        print("\n服务器已停止")
    except Exception as e:
        print(f"服务器启动失败: {e}")

if __name__ == "__main__":
    main()