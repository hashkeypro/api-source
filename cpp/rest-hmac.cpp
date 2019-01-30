// g++ -o hmac rest-hmac.cpp -lcrypto -lboost_system -lcpprest -lssl -pthread
#include <stdio.h>
#include <string.h>

#include <nlohmann/json.hpp> // https://github.com/nlohmann/json/tree/master

#include <openssl/hmac.h> // https://www.openssl.org/
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <cpprest/http_client.h> // https://github.com/Microsoft/cpprestsdk
#include <cpprest/filestream.h> // https://github.com/Microsoft/cpprestsdk

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

char *base64encode(const unsigned char *input, int length)
{
    BIO *bmem, *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    char *buff = (char *)malloc(bptr->length);
    memcpy(buff, bptr->data, bptr->length-1);
    buff[bptr->length-1] = 0;

    BIO_free_all(b64);

    return buff;
}

std::string SHA256HMAC(std::string secret, std::string msg)
{
    unsigned char* digest;
    
    digest = HMAC(EVP_sha256(), secret.c_str(), strlen(secret.c_str()), (unsigned char*)msg.c_str(), strlen(msg.c_str()), NULL, NULL);    
    auto base64digest = base64encode(digest, 32);

    return std::string(base64digest);
}

int main()
{
    const std::string path = "/v1/referenceData/rateLimit";
    const std::string secret = "vprggEasLOksdmut6WcFvuv4oUuAbewdkGJY1fgAvBw=";
    const std::string apikey = "MTU0NDUwODQ0NjI0NTAwMDAwMDAwNTQ=";
    const std::string base_url = "https://api-preview.pro.hashkey.com/APITrade";

    // generate the message to sign
    // 生成待签消息并签名
    std::stringstream timestamp;
    timestamp << std::time(nullptr) * 1000;
    std::string timestamp_str = timestamp.str();
    std::string msg = timestamp_str + "GET" + path;
    
    auto base64digest = SHA256HMAC(secret, msg);

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
        request.headers().add("API-SIGNATURE", base64digest);
        request.headers().add("API-TIMESTAMP", timestamp_str);
        request.headers().add("AUTH-TYPE", "HMAC");
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