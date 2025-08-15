#ifndef V8_JSON_WEBSOCKET_CLIENT_H_
#define V8_JSON_WEBSOCKET_CLIENT_H_

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace v8 {
namespace internal {

class WebSocketClient {
 public:
  WebSocketClient();
  ~WebSocketClient();
  
  // 连接到WebSocket服务器
  bool Connect(const std::string& host, int port, const std::string& path = "/");
  
  // 发送消息
  bool SendMessage(const std::string& message);
  
  // 断开连接
  void Disconnect();
  
  // 检查连接状态
  bool IsConnected() const;
  
 private:
  // WebSocket握手
  bool PerformHandshake(const std::string& host, const std::string& path);
  
  // 发送WebSocket帧
  bool SendFrame(const std::string& data);
  
  // 创建WebSocket key
  std::string CreateWebSocketKey();
  
  // Base64编码
  std::string Base64Encode(const std::string& data);
  
  // SHA1哈希
  std::string Sha1Hash(const std::string& data);
  
  int socket_fd_;
  std::atomic<bool> connected_;
  std::string host_;
  int port_;
  std::mutex send_mutex_;
};

// WebSocket客户端由内部函数管理，不需要外部访问全局实例

// 初始化WebSocket客户端
bool InitializeWebSocketClient(const std::string& host, int port, const std::string& path = "/");

// 发送JSON消息到WebSocket服务器
bool SendJsonToWebSocket(const std::string& json_message);

// 清理WebSocket客户端
void CleanupWebSocketClient();

}  // namespace internal
}  // namespace v8

#endif  // V8_JSON_WEBSOCKET_CLIENT_H_
