#include "src/json/websocket-client.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <sstream>
#include <random>
#include <iomanip>
#include <vector>

namespace v8 {
namespace internal {

// 全局WebSocket客户端管理
static WebSocketClient*& GetGlobalWebSocketClientRef() {
  static WebSocketClient* g_websocket_client = nullptr;
  return g_websocket_client;
}

WebSocketClient::WebSocketClient() 
    : socket_fd_(-1), connected_(false), port_(0) {}

WebSocketClient::~WebSocketClient() {
  Disconnect();
}

bool WebSocketClient::Connect(const std::string& host, int port, const std::string& path) {
  if (connected_.load()) {
    return true;
  }
  
  host_ = host;
  port_ = port;
  
  // 创建socket
  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ < 0) {
    return false;
  }
  
  // 解析主机地址
  struct hostent* server = gethostbyname(host.c_str());
  if (server == nullptr) {
    close(socket_fd_);
    socket_fd_ = -1;
    return false;
  }
  
  // 设置服务器地址
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
  
  // 连接到服务器
  if (connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    close(socket_fd_);
    socket_fd_ = -1;
    return false;
  }
  
  // 执行WebSocket握手
  if (!PerformHandshake(host, path)) {
    close(socket_fd_);
    socket_fd_ = -1;
    return false;
  }
  
  connected_ = true;
  return true;
}

bool WebSocketClient::PerformHandshake(const std::string& host, const std::string& path) {
  std::string websocket_key = CreateWebSocketKey();
  
  std::ostringstream request;
  request << "GET " << path << " HTTP/1.1\r\n"
          << "Host: " << host << ":" << port_ << "\r\n"
          << "Upgrade: websocket\r\n"
          << "Connection: Upgrade\r\n"
          << "Sec-WebSocket-Key: " << websocket_key << "\r\n"
          << "Sec-WebSocket-Version: 13\r\n"
          << "User-Agent: V8-WebSocket-Client/1.0\r\n"
          << "Origin: http://" << host << ":" << port_ << "\r\n"
          << "Cache-Control: no-cache\r\n"
          << "Pragma: no-cache\r\n"
          << "\r\n";
  
  std::string request_str = request.str();
  
  // 添加调试输出
  printf("WebSocket handshake request:\n%s", request_str.c_str());
  
  if (send(socket_fd_, request_str.c_str(), request_str.length(), 0) < 0) {
    printf("Failed to send handshake request\n");
    return false;
  }
  
  // 读取响应
  char buffer[1024];
  ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
  if (bytes_received <= 0) {
    printf("Failed to receive handshake response\n");
    return false;
  }
  
  buffer[static_cast<size_t>(bytes_received)] = '\0';
  std::string response(buffer);
  
  // 添加调试输出
  printf("WebSocket handshake response:\n%s\n", response.c_str());
  
  // 检查是否包含升级确认
  bool has_101 = response.find("HTTP/1.1 101") != std::string::npos;
  bool has_upgrade = response.find("Upgrade: websocket") != std::string::npos || 
                     response.find("upgrade: websocket") != std::string::npos;
  
  printf("Handshake check - 101: %s, Upgrade: %s\n", 
         has_101 ? "YES" : "NO", has_upgrade ? "YES" : "NO");
  
  return has_101 && has_upgrade;
}

bool WebSocketClient::SendMessage(const std::string& message) {
  if (!connected_.load()) {
    return false;
  }
  
  std::lock_guard<std::mutex> lock(send_mutex_);
  return SendFrame(message);
}

bool WebSocketClient::SendFrame(const std::string& data) {
  size_t data_length = data.length();
  std::vector<uint8_t> frame;
  
  // WebSocket帧头
  frame.push_back(0x81); // FIN=1, opcode=1 (text frame)
  
  // 生成掩码键
  uint32_t mask_key = 0;
  std::random_device rd;
  std::mt19937 gen(rd());
  mask_key = static_cast<uint32_t>(gen());
  
  // 设置payload长度和掩码位
  if (data_length < 126) {
    frame.push_back(0x80 | static_cast<uint8_t>(data_length)); // MASK=1, payload length
  } else if (data_length < 65536) {
    frame.push_back(0x80 | 126); // MASK=1, extended payload length
    frame.push_back((data_length >> 8) & 0xFF);
    frame.push_back(data_length & 0xFF);
  } else {
    frame.push_back(0x80 | 127); // MASK=1, extended payload length
    for (int i = 7; i >= 0; i--) {
      frame.push_back((data_length >> (i * 8)) & 0xFF);
    }
  }
  
  // 添加掩码键
  frame.push_back((mask_key >> 24) & 0xFF);
  frame.push_back((mask_key >> 16) & 0xFF);
  frame.push_back((mask_key >> 8) & 0xFF);
  frame.push_back(mask_key & 0xFF);
  
  // 添加掩码后的payload
  for (size_t i = 0; i < data_length; i++) {
    uint8_t mask_byte = (mask_key >> ((3 - (i % 4)) * 8)) & 0xFF;
    frame.push_back(data[i] ^ mask_byte);
  }
  
  // 发送帧
  if (send(socket_fd_, frame.data(), frame.size(), 0) < 0) {
    connected_ = false;
    return false;
  }
  
  return true;
}

void WebSocketClient::Disconnect() {
  if (socket_fd_ >= 0) {
    close(socket_fd_);
    socket_fd_ = -1;
  }
  connected_ = false;
}

bool WebSocketClient::IsConnected() const {
  return connected_.load();
}

std::string WebSocketClient::CreateWebSocketKey() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 255);
  
  std::string key;
  for (int i = 0; i < 16; i++) {
    key += static_cast<char>(dis(gen));
  }
  
  return Base64Encode(key);
}

std::string WebSocketClient::Base64Encode(const std::string& data) {
  const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string result;
  int val = 0, valb = -6;
  for (unsigned char c : data) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      result.push_back(chars[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6) result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
  while (result.size() % 4) result.push_back('=');
  return result;
}

// 全局函数实现
bool InitializeWebSocketClient(const std::string& host, int port, const std::string& path) {
  WebSocketClient*& client = GetGlobalWebSocketClientRef();
  if (client == nullptr) {
    client = new WebSocketClient();
  }
  
  return client->Connect(host, port, path);
}

bool SendJsonToWebSocket(const std::string& json_message) {
  WebSocketClient* client = GetGlobalWebSocketClientRef();
  if (client == nullptr || !client->IsConnected()) {
    return false;
  }
  
  return client->SendMessage(json_message);
}

void CleanupWebSocketClient() {
  WebSocketClient*& client = GetGlobalWebSocketClientRef();
  if (client != nullptr) {
    client->Disconnect();
    delete client;
    client = nullptr;
  }
}

}  // namespace internal
}  // namespace v8
