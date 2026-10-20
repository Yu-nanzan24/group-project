# group-project

## 何をするサービスか、メンバー、実行環境・必要な依存関係。
- サービス内容:クライアントから送られてきた文字列の小文字をサーバーで大文字に変えてそれを送り返すプログラム
- 実行環境:Ubuntu
- メンバー:protocol.md担当 西村
        　tcp_server担当  森岡
          tcp_client担当　伊藤

## build方法：実際に利用するコマンドと、実行するディレクトリ。
- コンパイル:`gcc -Wall -o tcp_server server/tcp_server.c`
            `gcc -Wall -o tcp_client client/tcp_client.c`

## server の起動方法：port、引数、設定方法。
- 実行:`./tcp_server`

## client の起動方法：接続先IP、port、送信方法。
- 実行:`./tcp_client "接続先" "ポート番号"`
- 具体例 `./tcp_client 127.0.0.1 5000`

## 実装済みのrequest、未実装のrequest。
- 実装済みリクエスト:小文字から大文字に変える
- 未実装リクエスト:大文字を小文字に変える、数字を除外するなど

## 現在分かっている問題。
送ることのできる文字数に制限がある(1024文字まで)
