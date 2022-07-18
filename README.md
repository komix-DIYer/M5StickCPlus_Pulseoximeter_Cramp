# M5StickCPlus_Pulseoximeter_Cramp  
[komix-DIYer/M5Fire_HRSpO2](https://github.com/komix-DIYer/M5Fire_HRSpO2)を改変  
M5StickC Plus + Mini Heart Rate Unit (MAX30100) で心拍数 (HR) と酸素飽和度 (SpO2) を計測/描画/送信  
M5StickC Plusで動作検証済み  
※計測の信頼度はセンサとライブラリの品質そのもの（本コードによる補正は無し）  
  
参考記事：[DIYer クランプ型パルスオキシメータを作ってみた](http://twinklesmile.blog42.fc2.com/blog-entry-476.html)  
<img src="https://user-images.githubusercontent.com/25588291/179397123-c3dec917-65ef-4b4e-8a3a-d44349f6b16d.jpg" width="600">  

# Main functions  
- ボタン機能
  - 電源ボタンは短く押すとリセット，長く押すとシャットダウン．
  - ボタン A はで画面表示の向きを回転．
  - ボタン B はモード変更 （LCD 表示する値：センサ出力値そのもの or 処理したもの）．
- LDC 表示
  - ボタン A 押下で画面表示の向きを回転．
  - LCD 表示の内容は（左上から），モード，Bluetooth接続状況，バッテリ残量，SpO2，HR．
<img src="https://user-images.githubusercontent.com/25588291/179397163-7d8a0444-9eab-47ab-902e-3571e4d545b4.jpg" width="600">  

- サウンド
  - センサで検出された心拍のタイミングで，SpO2 に応じた音階で，ビープ音を発生 （SpO2 < 92% の場合のみ）．
- モード
  - LCD 表示される値の処理を変更する．  
    モード1: センサ出力値そのもの: センサへの指の当たり具合によりしばしば乱高下して0にもなる  
    モード2: 値の範囲を制限して平滑化したもの: 現実的な範囲で滑らかに変化する

# Licence
[GPLv3](https://github.com/komix-DIYer/M5StickCPlus_Pulseoximeter_Cramp/blob/master/LICENSE)

# Author
komix
