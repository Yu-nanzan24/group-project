# protocol.md
## 1. サービス概要：何をするサービスか。clientとserverがそれぞれ何をするか。
- クライアントから送られた小文字をサーバー側で大文字に変換しクライアントに返すプログラム。
## 2. TCP port：待受portとclientの接続先の指定方法。
- 待受portは5000番、クライアントの実行時に、サーバーのIPアドレスを第1引数、ポート番号を第2引数として入力する。
## 3. request format：3種類以上のrequest名、引数、書式、具体例。

- クライアント具体例:text > feS50D2w
　                  converted > FES50D2W
- サーバー具体例:client -> feS50D2w
小文字を大文字に変換する、それ以外はそのまま返す。

## 4. error response：どの入力・状態でどのerrorを返すか。書式と具体例。
- サーバーが待ち受けしていないときに接続しようとした場合、`connect: Connection refused`とメッセージが表示され終了する。
- コマンドライン引数が少ない場合、`Usage: tcp_client IPv4 PORT`と表示され終了する。

#　以下10/20追記分

# protocol更新(10/27発表分)

## 5.TCPportと接続の規格
- 第1引数は接続先のIPアドレスを指定
- 第2引数のport番号は5000番 
- 通信が開始したあと、Ctrl+Dによって通信を終了させるまでrequestを送ることが出来る。
## 6.request終端、argumentの有無・形式・許容値。
- request終端には改行文字\nを用いる。
- request名の後に半角スペースをいれて、処理を行う文字列を指定する。この引数がない場合、エラーが表示される。
- コマンド名は大文字で固定

## 7. success response とerror response の書式・終端・具体例。

### success response
- UPPER <text>
    request: UPPER hello\n
    response:OK HELLO\n
- LOWER <text>
    request: LOWER HELLO\n
    response:OK hello\n
- LENGTH <text>
    request:LENGTH hello\n
    response:OK 5\n

### error response
- hello\n -> ERROR unknown-command\n
- UPPER\n -> ERROR invalid-argument\n