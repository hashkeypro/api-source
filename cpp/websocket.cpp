//g++ -o example websocket.cpp -lcpprest -lethash -lboost_system -lcrypto -lssl $(pkg-config --cflags libbitcoin --libs libbitcoin)
#include <iostream>
#include <csignal>
#include <boost/thread.hpp>

#include <cpprest/ws_client.h> // https://github.com/Microsoft/cpprestsdk
#include <bitcoin/bitcoin.hpp> // https://github.com/libbitcoin/libbitcoin
#include <ethash/keccak.hpp> // https://github.com/chfast/ethash
#include <nlohmann/json.hpp> // https://github.com/nlohmann/json/tree/master

#include "eccutils.hpp"

using namespace web;
using namespace web::websockets::client;

volatile sig_atomic_t signaled = 1;

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
    const std::string secret = "VAoASo72TbEYsasQAD64nHlZVyBglPw13kfvlqM1j5Y=";
    const std::string apikey = "15381937145820000000054";
    const std::string sign_msg = "WSS/APITradeWS/v1/messages";
    auto sig = xdaex::ECCSignature(sign_msg, secret);

    const std::string url = "wss://api-test.xdaex.com/APITradeWS/v1/messages";

    // Public Message Flow: Subscribe to ticker requests
    // 公有消息流：订阅逐笔成交信息
    // { "type": "subscribe", "channel": {"ticker":["ETH-BTC", "CYB-BTC"]} }
    nlohmann::json public_subscribe;
    public_subscribe["type"] = "subscribe";
    public_subscribe["channel"]["ticker"] = {"ETH-BTC", "CYB-BTC"};

    // Private Message Flow
    // 私有消息流
    // { "type": "unsubscribe ", "channel": {"user":[API-KEY, signature]} }
    nlohmann::json private_subscribe;
    private_subscribe["type"] = "subscribe";
    private_subscribe["channel"]["user"] = {apikey, sig};

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