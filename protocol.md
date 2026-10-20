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