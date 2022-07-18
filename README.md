# M5StickCPlus_Pulseoximeter_Cramp  
[komix-DIYer/M5Fire_HRSpO2](https://github.com/komix-DIYer/M5Fire_HRSpO2)を改変  
M5StickC Plus + Mini Heart Rate Unit (MAX30100) で心拍数 (HR) と酸素飽和度 (SpO2) を計測/表示/送信  
M5StickC Plusで動作検証済み  
※計測の信頼度はセンサとライブラリの品質そのもの（本コードによる補正は無し）  
  
参考記事：[DIYer クランプ型パルスオキシメータを作ってみた](http://twinklesmile.blog42.fc2.com/blog-entry-476.html)  
<img src="https://user-images.githubusercontent.com/25588291/179397123-c3dec917-65ef-4b4e-8a3a-d44349f6b16d.jpg" width="600">  

# Main functions  
- ボタン機能
  - 電源ボタンは短く押すとリセット，長く押すとシャットダウン．
  - ボタン A はで画面表示の向きを回転．
  - ボタン B はモード変更（詳細は下記）．
- LDC 表示
  - ボタン A 押下で画面表示の向きを回転．
  - LCD 表示の内容は（左上から），モード，Bluetooth接続状況，バッテリ残量，SpO2，HR．
<img src="https://user-images.githubusercontent.com/25588291/179397163-7d8a0444-9eab-47ab-902e-3571e4d545b4.jpg" width="600">  

- サウンド
  - センサで検出された心拍のタイミングで，SpO2 に応じた音階で，ビープ音を発生 （SpO2 < 92% の場合のみ）．
- モード
  - LCD 表示される値の処理を変更する．  
    モード0（黒抜き白丸アイコン）：センサ出力値そのものを表示．センサへの指の当たり具合によりしばしば0になったり乱高下する．  
    モード1（白塗り白丸アイコン）：値の範囲を制限して平滑化したものを表示．現実的な範囲で滑らかに変化する．  
    モード2（赤丸アイコン）：HR=120，SpO2=70を目指して滑らかに変化する．サウンド機能の確認のため異常状態を模擬する．
- 通信  
  USB接続によるシリアル通信のほかに，Bluetooth接続（デバイス名：M5-SpO2）してのBluetoothシリアル通信も可能．  
  送信されるデータはカンマ区切りで順に，モード，時間，HR，SpO2．

# Requirements
- [M5StickC Plus](https://www.switch-science.com/catalog/6470/)
- [M5Stack用心拍センサユニット](https://www.switch-science.com/catalog/5695/)
  [Mini Heart Rate Unit (MAX30100) Pulse Oximeter](https://m5stack-store.myshopify.com/products/mini-heart-unit)
- [oxullo/Arduino-MAX30100 library](https://github.com/oxullo/Arduino-MAX30100)
- [クランプ型パルスオキシメータDIYキット（M5StickC Plus + 心拍センサユニット使用）（STLデータ） - First DIY：まずは何か作ってみる - BOOTH](https://first-diy.booth.pm/items/4010143)

# Licence
[GPLv3](https://github.com/komix-DIYer/M5StickCPlus_Pulseoximeter_Cramp/blob/master/LICENSE)

# Author
komix
