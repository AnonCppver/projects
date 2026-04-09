// #pragma once
// #include <string>

// class RpcController {
// public:
//     void SetFailed(const std::string& reason) {
//         failed_ = true;
//         reason_ = reason;
//     }

//     bool Failed() const { return failed_; }
//     std::string ErrorText() const { return reason_; }

// private:
//     bool failed_ = false;
//     std::string reason_;
// };

// #pragma once
// #include <string>
// #include <google/protobuf/message.h>

// class Service {
// public:
//     virtual ~Service() = default;

//     // 核心：通过方法名调用
//     virtual void CallMethod(const std::string& method,
//                             RpcController* controller,
//                             const google::protobuf::Message* request,
//                             google::protobuf::Message* response) = 0;
// };

#include "../lrpc/RpcServer.h"
#include "../lrpc/RpcConfig.h"
#include <iostream>

int main(int argc, char *argv[])
{

    leef::rpc::RpcConfig config;
    config.load("./rpc_config");
    std::string port = config.getValue("port");
    std::string logFile = config.getValue("log");
    std::cout << "Port: " << port << std::endl;
    std::cout << "Log File: " << logFile << std::endl;
    return 0;
}