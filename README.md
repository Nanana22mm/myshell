# README

## 目的
- シェルの内部の仕組みを勉強するために自作シェルを作った．
- 一般的なシェルと同様，ユーザの入力に応じてフ，コマンドの実行を行う．

## 機能
制作したシェルは以下の機能を持つ．
- 入力で与えられたコマンド（例えば ls, cd pwd など）を実行する．
- 多段パイプやリダイレクトも実行可能である．
- ctrl-C で実行中のコマンドを終了する．
- ユーザが "exit" と入力すると終了する．

## 実行した時の様子
![shell](https://github.com/Nanana22mm/myshell/assets/126635893/ad37c426-143f-4871-9ca8-495bf56a6adf)

## 反省
- 時間の都合上，＋αの機能，例えば入力の補完機能や予測，ヒストリの管理などの機能を追加することができなかった．
- プロセスの管理に苦戦し，バックグラウンド実行を実現できなかった．

## 苦労した点
- リダイレクトや多段パイプに苦労した．課題解決にあたって，問題を細かく切り分けること，段階的に開発することを意識した．
- コマンドの実行の際に execve を使用したが，その実行のための引数を求めるのが難しかった．
- 開発全体を通してバグや動作不良に何度も直面したが，諦めずに try & error を繰り返した．またどうしてもうまくいかない時は一人で抱え込まずに友人に相談することで課題解決の糸口を探った．