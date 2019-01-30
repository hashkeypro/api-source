//g++ -o example rest.cpp -lcpprest -lethash -lboost_system -lcrypto -lssl $(pkg-config --cflags libbitcoin --libs libbitcoin)
#include <ctime>

#include <bitcoin/bitcoin.hpp> // https://github.com/libbitcoin/libbitcoin
#include <ethash/keccak.hpp> // https://github.com/chfast/ethash
#include <nlohmann/json.hpp> // https://github.com/nlohmann/json/tree/master

#include <cpprest/http_client.h> // https://github.com/Microsoft/cpprestsdk
#include <cpprest/filestream.h> // https://github.com/Microsoft/cpprestsdk

#include "eccutils.hpp"

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

int main(int argc, char* argv[])
{
    const std::string path = "/v1/referenceData/rateLimit";
    const std::string secret = "uvX6WIUzE5jJLMszT7elkTMKgRZEoYkx7X7mTpPWyXo=";
    const std::string apikey = "MTU0MjEwNDAwMTA1NjAwMDAwMDAwNTQ=";
    const std::string base_url = "https://api-preview.pro.hashkey.com/APITrade";

    // generate the message to sign
    // 生成待签消息并签名
    std::stringstream timestamp;
    timestamp << std::time(nullptr) * 1000;
    std::string timestamp_str = timestamp.str();
    std::string msg = timestamp_str + "GET" + path;

    auto sig = hashkey::ECCSignature(msg, secret);

    auto fileStream = std::make_shared<ostream>();
    // Open stream to log file.
    // 准备日志文件
    pplx::task<void> requestTask = fstream::open_ostream(U("api.log")).then([=](ostream logFile)
    {
        *fileStream = logFile;

        // set request header and send
        // 设置请求消息头，发送GET请求
        http_client client(base_url + path);
        http_request request(methods::GET);
        request.headers().add("Content-Type", "application/json");
        request.headers().add("API-KEY", apikey);
        request.headers().add("API-SIGNATURE", sig);
        request.headers().add("API-TIMESTAMP", timestamp_str);
        request.headers().add("AUTH-TYPE", "PUB-PRIV");
        return client.request(request);
    })

    // Handle response headers arriving.
    // 处理收到的请求消息头
    .then([=](http_response response)
    {
        printf("Received response\n %s\n", response.to_string().c_str());
        // Even a response lag occurs, the full response can be found in the log file.
        // 有应答延迟时，在日志文件中总能找到完整应答。
        return response.body().read_to_end(fileStream->streambuf());
    })

    // Close the file stream.
    // 关闭日志文件
    .then([=](size_t)
    {
        return fileStream->close();
    });

    // Wait for all the outstanding I/O to complete and handle any exceptions
    // 等待所有的输入输出完成并处理发生了的异常
    try
    {
        requestTask.wait();
    }
    catch (const std::exception &e)
    {
        printf("Error exception:%s\n", e.what());
    }

    return 0;
}