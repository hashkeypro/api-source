package main

import (
	"encoding/json"
	"log"
	"net/url"
	"os"
	"os/signal"
	"time"

	"github.com/gorilla/websocket"
	"github.com/hashkeypro/api-src/golang/hashkey"
)

type RequestMessage struct {
	Type    string              `json:"type"`
	Channel map[string][]string `json:"channel"`
}

func main() {
	// key pair and signature
	// 密钥对和签名

	msg := "WSS/APITradeWS/v1/messages"

	// authType := "PUB-PRIV"
	// apiKey := "MTU0MjEwNDAwMTA1NjAwMDAwMDAwNTQ="
	// privateKey := "uvX6WIUzE5jJLMszT7elkTMKgRZEoYkx7X7mTpPWyXo="
	// sig, err := hashkey.ECCSignature([]byte(msg), privateKey)

	authType := "HMAC"
	apiKeyHMAC := "MTU0NDUwODQ0NjI0NTAwMDAwMDAwNTQ="
	secretKey := "vprggEasLOksdmut6WcFvuv4oUuAbewdkGJY1fgAvBw="
	hmacStr := hashkey.SHA256HMAC([]byte(msg), secretKey)

	host := "api-preview.pro.hashkey.com"
	path := "/APITradeWS/v1/messages"

	// Public Message Flow: Subscribe to ticker requests
	// 公有消息流：订阅逐笔成交信息
	// { "type": "subscribe", "channel": {"ticker":["ETH-BTC", "CYB-BTC"]} }
	publicChannel := make(map[string][]string)
	publicChannel["ticker"] = []string{"ETH-BTC", "CYB-BTC"}
	publicMessage := RequestMessage{
		Type:    "subscribe",
		Channel: publicChannel,
	}
	publicMessageBytes, err := json.Marshal(publicMessage)

	// Private Message Flow
	// 私有消息流
	// { "type": "subscribe", "channel": {"user":[API-KEY, API-SIGNATURE, AUTHTYPE]} }
	privateChannel := make(map[string][]string)

	// privateChannel["user"] = []string{apiKey, sig, authType}
	privateChannel["user"] = []string{apiKeyHMAC, hmacStr, authType}

	privateMessage := RequestMessage{
		Type:    "subscribe",
		Channel: privateChannel,
	}
	privateMessageBytes, err := json.Marshal(privateMessage)

	// interrupt (Ctrl + C) notification setup
	// 中断 (Ctrl + C) 处理准备
	interrupt := make(chan os.Signal, 1)
	signal.Notify(interrupt, os.Interrupt)

	// handshake
	// 握手
	u := url.URL{Scheme: "wss", Host: host, Path: path}
	log.Printf("connecting to %s", u.String())
	c, _, err := websocket.DefaultDialer.Dial(u.String(), nil)
	if err != nil {
		log.Fatal("dial:", err)
	}
	defer c.Close()

	// subcribe
	// 订阅
	log.Printf("sending: %s", publicMessageBytes)
	err = c.WriteMessage(websocket.TextMessage, publicMessageBytes)
	if err != nil {
		log.Println("write:", err)
		return
	}
	log.Printf("sending: %s", privateMessageBytes)
	err = c.WriteMessage(websocket.TextMessage, privateMessageBytes)
	if err != nil {
		log.Println("write:", err)
		return
	}

	done := make(chan struct{})
	// goroutine keep listening
	// goroutine 持续接收信息
	go func() {
		defer close(done)
		for {
			_, message, err := c.ReadMessage()
			if err != nil {
				log.Println("read:", err)
				return
			}
			log.Printf("recv: %s", message)
		}
	}()

	ticker := time.NewTicker(15 * time.Second)
	defer ticker.Stop()

	for {
		select {
		case <-done:
			return
		case <-ticker.C:
			// heartbeat mechanism
			// 心跳机制
			err = c.WriteMessage(websocket.PingMessage, []byte{})
			if err != nil {
				log.Println("ping:", err)
				return
			}

		case <-interrupt:
			// unsubscribe
			// 退订
			log.Println("interrupt")
			publicMessage.Type = "unsubscribe"
			publicMessageBytes, err = json.Marshal(publicMessage)
			log.Printf("sending: %s", publicMessageBytes)
			err = c.WriteMessage(websocket.TextMessage, publicMessageBytes)
			if err != nil {
				log.Println("write:", err)
				return
			}
			privateMessage.Type = "unsubscribe"
			privateMessageBytes, err = json.Marshal(privateMessage)
			log.Printf("sending: %s", privateMessageBytes)
			err = c.WriteMessage(websocket.TextMessage, privateMessageBytes)
			if err != nil {
				log.Println("write:", err)
				return
			}
			// Cleanly close the connection by sending a close message and then
			// waiting (with timeout) for the server to close the connection.
			// 发送关闭信息干净地终止连接并等待服务器终止这次连接
			err = c.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.CloseNormalClosure, ""))
			if err != nil {
				log.Println("write close:", err)
				return
			}
			select {
			case <-done:
			case <-time.After(time.Second):
			}
			return
		}
	}
}
