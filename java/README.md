### Java Code Sample ^Java代码示例：
* Java
    + download the Java SDK by the following link ^下载 Java SDK， 链接为：https://github.com/hashkeypro/api-src/raw/master/java/hashkeypro-sdk-1.0.jar

    + REST API request example including signing ^包含签名步骤的 REST 请求示例
        ```JAVA
        public class Test2 {
            public static void main(String[] args) throws Exception{

                String BASE_URL = "https://pro.hashkey.com/APITrade";
                String API_KEY = "1232144332323423";
                String PRIVATE_KEY = "Bh9tSghsfhjdFHKJAfajk78fa980kaFDda78sa/fda=";

                /**
                * Get the difference between server time and local time
                * 计算服务器时间和本地时间的差
                 */
                //Get local time stamp
                //获取本地时间
                String timestamp = String.valueOf((new Date()).getTime());
                //Compile original data 
                //拼装原文
                String originData = timestamp + "GET" + "/v1/marketData/time";
                //Get original data summary
                //对原文摘要
                String hashedData = ECCUtils.SHA256(originData);
                //Get summary signature
                //对摘要签名
                String signData = ECCUtils.signData(hashedData.getBytes(), PRIVATE_KEY);
                //Send a GET request, the setting of the header details for the request is enclosed in the Http client class
                //发送一个GET请求，请求消息头的细节已经封装在Http client类中
                String url = BASE_URL + "/v1/marketData/time";
                String timeResult = HttpClient.sendGet(url, API_KEY, signData, timeStamp);
                Long timeDiff = Long.valueOf(JSON.parseObject(timeResult).getLong("timestamp").longValue() - new Date().getTime());

                /**
                * Create order
                * 创建订单
                */
                //Enclose order request message
                //封装请求信息
                OrderInsertRequest orderInsertRequest = new OrderInsertRequest("buy", "CYB-ETH", "12345678901234", "0.0001", "0.001");
                //Convert the creatOrder request to JSON string
                //将封装好的请求转化为JSON字符串
                String jsonData = JSON.toJSONString(orderInsertRequest);
                //Get the timestamp and synchronizes with the server time
                //得到时间戳并和服务器时间同步
                String timestampCreate = String.valueOf((new Date()).getTime() + timeDiff.longValue());
                //Compile original data
                //拼装原文
                String originDataCreate = timestampCreate + "POST" + "/v1/order/create" + jsonData;
                //Get the original data summary
                //对原文摘要
                String hashedOriginDataCreate = ECCUtils.SHA256(originDataCreate);
                //Use ECC algorithm to sign for summary
                //对摘要签名
                String signDataCreate = ECCUtils.signData(hashedOriginDataCreate.getBytes(), PRIVATE_KEY);
                //Send a POST request, the setting of the header details for the request is enclosed in the Http Client 
                //发送一个POST请求，请求消息头的细节已经封装在Http client类中
                String createResult = HttpClient.sendPost(BASE_URL + "/v1/order/create",
                        API_KEY, signDataCreate, timeStamp, jsonData);
            }
        }
        ```
    + For user's convenience, SDK provides a full enclosure of each request, there is no need for signature and other pre-operational measures. It directly performs the corresponding service, refer to the sample codes below ^为方便用户，SDK对每一个请求都有完整的封装，用户不需要额外进行签名等操作。SDK直接调用相关服务，如下:
        ```JAVA
        // Please download and use Java SDK first
        // 请下载使用Java SDK
        public class Sample {

            static final String BASE_URL = "https://pro.hashkey.com/APITrade";
            static final String API_KEY = "12345678901230987654321";
            static final String PRIVATE_KEY = "Bh9tSghsfhjdkaFDda78sa/fda=";

            /**
            * Account asset inquiry
            * 账户资产查询
            */
            public static void funds() throws Exception{
                // 1. Create an accountService connection object
                // 1. 创建一个accountService连接对象
                AccountService accountService = new AccountService();
                // 2. Query account asset information
                // 2. 查询账户资产信息
                String result = accountService.funds();
                // 3. Print out result json string
                // 3. 结果打印为json字符串
                System.out.println(result);
            }

            /**
            * Create order
            * 创建订单
            */
            public static void createOrder() throws Exception{
                // 1. Add an Orderclient Connection object
                // 1. 增加一个Orderclient连接对象
                OrderService orderService = new OrderService(BASE_URL, API_KEY, PRIVATE_KEY);
                // 2. Create order
                // 2. 创建订单
                String result= orderService.createOrder(new OrderInsertRequest("buy","CYB-ETH","12345677809843","0.0001","0.001"));
                // 3. Print out result json string. 
                // 3. 结果打印为json字符串
                System.out.println(result);
            }

            /**
            * Order inquiry
            * 查询订单
            */
            public static void searchOrders() throws Exception{
                // 1. Add an Orderclient Connection object
                // 1. 增加一个Orderclient连接对象
                OrderService orderService = new OrderService(BASE_URL, API_KEY, PRIVATE_KEY);
                // 2. Inquire executed orders
                // 2. 查询订单
                String result = orderService.searchOrder(new GetOrderRequest("startDate","endDate","statuc","instrumentId","orderId"));
                // 3. Print out result json string. 
                // 3. 结果打印为json字符串
                System.out.println(result);
            }

            /**
            * Cancel order
            * 取消订单
            */
            public static void cancelOrder() throws Exception{
                // 1. Add an Orderclient Connection object
                // 1. 增加一个Orderclient连接对象
                OrderService orderService = new OrderService(BASE_URL, API_KEY, PRIVATE_KEY);
                // 2. Cancel order
                // 2. 取消订单
                String result = orderService.cancelOrder(new OrderCancelRequest("orderId"));
                // 3. Print out result json string. 
                // 3. 结果打印为json字符串
                System.out.println(result);
            }

            public static void main(String[] args) throws Exception{
                funds();
                createOrder();
                searchOrders();
                cancelOrder();
            }

        }
        ```