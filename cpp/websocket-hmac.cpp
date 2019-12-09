// g++ -o hmac websocket-hmac.cpp -lcrypto -lboost_system -lboost_thread -lcpprest -lssl -pthread
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <csignal>
#include <boost/thread.hpp>

#include <nlohmann/json.hpp> // https://github.com/nlohmann/json/tree/master

#include <openssl/hmac.h> // https://www.openssl.org/
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <cpprest/ws_client.h> // https://github.com/Microsoft/cpprestsdk

using namespace web;
using namespace web::websockets::client;

volatile sig_atomic_t signaled = 1;

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

void listen(websocket_client *client)
{
    while (true) {
        client->receive().then([](websocket_incoming_message in_msg) {
            return in_msg.extract_string();
        }).then([](std::string body) {
            std::cout << body << std::endl; 
        }).wait();
        boost::this_thread::interruption_point();
    }
}

void int_handler (int signum)
{
    std::cout << "closing...\n";
    signaled = 0;
}

int main()
{
    // key pair and signature
    // 密钥对和签名
    const std::string secret = "vprggEasLOksdmut6WcFvuv4oUuAbewdkGJY1fgAvBw=";
    const std::string apikey = "MTU0NDUwODQ0NjI0NTAwMDAwMDAwNTQ=";
    const std::string sign_msg = "WSS/APITradeWS/v1/messages";
    auto sig = SHA256HMAC(secret, sign_msg);
    
    const std::string url = "wss://preview-pro.hashkey.com/APITradeWS/v1/messages";

    // Public Message Flow: Subscribe to ticker requests
    // 公有消息流：订阅逐笔成交信息
    // { "type": "subscribe", "channel": {"ticker":["ETH-BTC", "CYB-BTC"]} }
    nlohmann::json public_subscribe;
    public_subscribe["type"] = "subscribe";
    public_subscribe["channel"]["ticker"] = {"ETH-BTC", "CYB-BTC"};

    // Private Message Flow
    // 私有消息流
    // { "type": "unsubscribe ", "channel": {"user":[API-KEY, signature, HMAC]} }
    nlohmann::json private_subscribe;
    private_subscribe["type"] = "subscribe";
    private_subscribe["channel"]["user"] = {apikey, sig, "HMAC"};

    // handshake
    // 握手
    websocket_client client;
    client.connect(url).wait();

    // keep listening
    // 持续接受信息
    boost::thread listen_thread(boost::bind(listen, &client));

    // subcribe
    // 订阅
    websocket_outgoing_message out_msg;

    std::cout << public_subscribe.dump() << std::endl;
    out_msg.set_utf8_message(public_subscribe.dump());
    client.send(out_msg).wait();
    std::cout << private_subscribe.dump() << std::endl;
    out_msg.set_utf8_message(private_subscribe.dump());
    client.send(out_msg).wait();

    // interrupt (Ctrl + C) handler
    // 中断 (Ctrl + C) 处理
    signal(SIGINT, int_handler);

    // heartbeat mechanism
    // 心跳机制
    websocket_outgoing_message ping;
    ping.set_pong_message();
    while (signaled) {
        client.send(ping).wait();
        boost::this_thread::sleep(boost::posix_time::seconds(15));
    }

    // unsubscribe
    // 退订
    public_subscribe["type"] = "unsubscribe";
    std::cout << public_subscribe.dump() << std::endl;
    private_subscribe["type"] = "unsubscribe";
    std::cout << private_subscribe.dump() << std::endl;

    out_msg.set_utf8_message(public_subscribe.dump());
    client.send(out_msg).wait();
    out_msg.set_utf8_message(private_subscribe.dump());
    client.send(out_msg).wait();

    // interrupt listen_thread and wait for it to close
    // 中断listen_thread并等待其关闭
    listen_thread.interrupt();
    listen_thread.join();

    client.close().wait();
    return 0;
}