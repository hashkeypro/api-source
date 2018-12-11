### WebSocket Subscription Sample Code ^WebSocket订阅示例代码

WebSocket subscriptions include public and private message flows. Here are javascript code samples using those types of message flows. ^WebSocket 消息流包含公有流和私有流，这里提供了 javascript 语言的两种消息流订阅示例。

* Private Flow Subscription Sample ^私有消息流订阅示例
    ```javascript
    const WebSocket = require('ws');
    const ws = new WebSocket('wss://api-test.pro.hashkey.com/APITradeWS/v1/messages', {
        checkServerIdentity: () => {}
    });
    const data = "WSS/APITradeWS/v1/messages";
    const signature = {"r":"FDS/fhdkjaf89YUIfda89fahkjfdaTYUF32IUfda=", "s":"NFMFDsfdsgkj678678431/fahkjaip68fahjKSDh=","v":28};

    const subscribePrivateMsg = {
        type: "subscribe",
        channel: {
            user:["13267813673212", JSON.Stringify(sign)]
        }
    }
    const subscribePrivateMsg = {
        type: "unsubscribe",
        channel: {
            user:["13267813673212", JSON.Stringify(sign)]
        }
    }

    ws.on('open', ()=>{
        console.log('WebSocket open')
        ws.send(JSON.stringify(subscribePrivateMsg));
    })

    //Send ping
    //发送 ping
    setInterval(() => {
        ws.ping('');
    }, 15000);

    ws.on('message', (data) => {
        console.log(data);
    })
    ```

* Public Flow Subscription Sample ^公有消息流订阅示例
    ```javascript
    const WebSocket = require('ws');
    const ws = new WebSocket('wss://api-test.pro.hashkey.com/APITradeWS/v1/messages', {
        checkServerIdentity: () => {}
    });
    const data = "WSS/APITradeWS/v1/messages";
    const signature = {"r":"FDS/fhdkjaf89YUIfda89fahkjfdaTYUF32IUfda=", "s":"NFMFDsfdsgkj678678431/fahkjaip68fahjKSDh=","v":28}；
    ws.on('open', ()=>{
        console.log('WebSocket open')
        ws.send('{"type":"subscribe", "channel":{"ticker":["ETH-BTC"]}}');
        ws.send('{"type":"subscribe", "channel":{"level2":["ETH-BTC"]}}');
    })

    //Send ping
    //发送 ping
    setInterval(() => {
        ws.ping('');
    }, 15000);

    ws.on('message', (data) => {
        console.log(data);
    })
    ```

