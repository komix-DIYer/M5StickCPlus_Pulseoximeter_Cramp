/* 
 * 20220102
 * by komix
 * 
 * モード (op_mode)
 *   0: センサ出力値を直接表示
 *   1: センサ出力値を処理して表示
 *   2: 既定値を表示
 * 
 * シリアル通信 (USB / Bluetooth)
 *   想定受信データ: Int 型, カンマ区切り, RCV_DAT_NUM 個, 終端文字 \n
 *   送信データ:    モード %d, 経過時間 %.3f [s], 心拍数 %.2f [bpm], SpO2 %.1f [%] \r\n
 */

#include <M5StickCPlus.h>
#include "MAX30100_PulseOximeter.h"
#include "BluetoothSerial.h"

// HR&SpO2
#define REPORTING_PERIOD_MS 1000
#define SPO2_INI 98.0 // モード 2 で表示される既定値
#define HR_INI 60.0 // モード 2 で表示される既定値
PulseOximeter pox;
uint32_t millis_ini = 0, tsLastReport = 0;
float SpO2 = SPO2_INI, SpO2_old = SPO2_INI, SpO2_ref = SPO2_INI, dSpO2 = random(10, 90)/100.0;
float HR = HR_INI, HR_old = HR_INI, HR_ref = HR_INI, dHR = random(10, 90)/100.0;
float T_SpO2 = 10.0, T_HR = 10.0;

// Bluetooth
BluetoothSerial SerialBT;
#define SIZE_OF_ARRAY(ary)  (sizeof(ary)/sizeof((ary)[0]))
#define RCV_DAT_NUM 1
int val[RCV_DAT_NUM];
bool isBTbegun = false;
PROGMEM const uint8_t btlogo[] = {
  0x40, 0x00, 0xC0, 0x00, 0xC0, 0x03, 0xC0, 0x07, 0xC0, 0x1E, 0xC6, 0x39, 
  0xCE, 0x38, 0xFC, 0x1E, 0xF0, 0x07, 0xE0, 0x03, 0xE0, 0x03, 0xF8, 0x0F, 
  0xFC, 0x1C, 0xCE, 0x79, 0xC2, 0x3C, 0xC0, 0x0E, 0xC0, 0x07, 0xC0, 0x03, 
  0xC0, 0x00, 0x40, 0x00, };

// バッテリ
float MAX_BATTERY_VOLTAGE = 4.2f;
float MIN_BATTERY_VOLTAGE = 3.0f;

// 画面表示
TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
int16_t LCD_HEIGHT, LCD_WIDTH;
int16_t LCD_ROTATION = 3;
int16_t LCD_ROW_STEP = 0;

uint8_t op_mode = 0; // 0:センサ出力値を直接表示, 1:センサ出力値を処理して表示, 2:既定値を表示

void onBeatDetected()
{
  //Serial.println("Beat!");
  
  // ビープ音
  if (SpO2 > 70 && SpO2 < 92)
  {
    int freq = 440 - 30 * (96 - SpO2);
    M5.Beep.tone(freq, 500);
  }
}

int split(String *result, size_t resultsize, String data, char delimiter)
{
  int index = 0;
  int datalength = data.length();
  for (int i = 0; i < datalength; i++)
  {
    char tmp = data.charAt(i);
    if ( tmp == delimiter )
    {
      index++;
      if ( index > (resultsize - 1)) return -1;
    }
    else result[index] += tmp;
  }
  return (index + 1);
}

void serialEvent()
// 想定受信データ: Int 型, カンマ区切り, RCV_DAT_NUM 個, 終端文字 \n
{
  String s = "";
  
  while(1)
  {
    if(Serial.available()==0) break;
    
    char tmp = Serial.read();
    
    if(tmp=='\n') break;
    s += String(tmp);
  }
  
  String v[RCV_DAT_NUM];
  size_t arraysize = SIZE_OF_ARRAY(v);
  char delimiter = ',';
  int index = split(v, arraysize, s, delimiter);
  for(int i=0; i<index; i++) val[i] = v[i].toInt();
}

void serialBTEvent()
// 想定受信データ: Int 型, カンマ区切り, RCV_DAT_NUM 個, 終端文字 \n
{
  String s = "";
  
  while(1)
  {
    if(SerialBT.available()==0) break;
    
    char tmp = SerialBT.read();
    
    if(tmp=='\n') break;
    s += String(tmp);
  }
  
  String v[RCV_DAT_NUM];
  size_t arraysize = SIZE_OF_ARRAY(v);
  char delimiter = ',';
  int index = split(v, arraysize, s, delimiter);
  for(int i=0; i<index; i++) val[i] = v[i].toInt();
}

void checkBattery()
{
  float bat = (M5.Axp.GetBatVoltage() - MIN_BATTERY_VOLTAGE) / (MAX_BATTERY_VOLTAGE - MIN_BATTERY_VOLTAGE) * 100.0f;

  // バッテリアイコン描画
  Disbuff.fillRect(M5.Lcd.width()-80,  5, 4, 20, GREEN);  if (bat < 30) Disbuff.fillRect(M5.Lcd.width()-80,  5, 4, 20, RED);
  Disbuff.fillRect(M5.Lcd.width()-75,  5, 4, 20, GREEN);  if (bat < 50) Disbuff.fillRect(M5.Lcd.width()-75,  5, 4, 20, DARKGREEN);
  Disbuff.fillRect(M5.Lcd.width()-70,  5, 4, 20, GREEN);  if (bat < 70) Disbuff.fillRect(M5.Lcd.width()-70,  5, 4, 20, DARKGREEN);
  Disbuff.fillRect(M5.Lcd.width()-65,  5, 4, 20, GREEN);  if (bat < 90) Disbuff.fillRect(M5.Lcd.width()-65,  5, 4, 20, DARKGREEN);
  Disbuff.fillRect(M5.Lcd.width()-60, 10, 2, 10, GREEN);  if (bat < 95) Disbuff.fillRect(M5.Lcd.width()-60, 10, 2, 10, DARKGREEN);

  // バッテリ残量パーセンテージ表示
  Disbuff.setTextSize(2);
  Disbuff.setTextColor(WHITE);
  Disbuff.setCursor(M5.Lcd.width()-55, 8);
  Disbuff.printf("%3d%%", (uint8_t)bat);
  
  // Bluetooth アイコン表示
  if (isBTbegun)
  {
    Disbuff.drawXBitmap(M5.Lcd.width()-105, 5, btlogo, 15, 20, BLUE); // 元画像サイズを編集 10x20 -> 15x20
  }
  
  // 動作モードアイコン表示
  Disbuff.drawCircle(15, 15, 5, WHITE);
  if (op_mode==1) Disbuff.fillCircle(15, 15, 5, WHITE);
  if (op_mode==2) Disbuff.fillCircle(15, 15, 5, RED);
  
  // 補助線 (240x135)
  //Disbuff.drawRect(30, 30, 75, 75, WHITE);
  //Disbuff.drawRect(M5.Lcd.width()-(30+75), 30+LCD_ROW_STEP, 75, 75, WHITE);
}

void setup() {
  M5.begin();
  // Wire.begin(32, 33); // for M5StickC(Plus) 無くても動作する
  
  // BT シリアル通信開始
  isBTbegun = SerialBT.begin("M5-SpO2"); 
  
  // 画面設定
  M5.Axp.ScreenBreath(9); // 画面の明るさ(7-12)
  M5.Lcd.setRotation(LCD_ROTATION);
  //M5.Lcd.setSwapBytes(false);
  LCD_WIDTH  = M5.Lcd.width();
  LCD_HEIGHT = M5.Lcd.height();
  Disbuff.createSprite(LCD_WIDTH, LCD_HEIGHT);
  Disbuff.setSwapBytes(true);
  
  // 心拍センサユニットの初期化
  if (!pox.begin()) { // PULSEOXIMETER_DEBUGGINGMODE_NONE/RAW_VALUES/AC_VALUES/PULSEDETECT
    // Serial.println("FAILED");
    for(;;);
  } else {
    // Serial.println("SUCCESS");
  }
  pox.setOnBeatDetectedCallback(onBeatDetected);
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  
  millis_ini = millis();
}

void loop()
{
  // 画面全体を黒で塗りつぶし
  Disbuff.fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, BLACK);
  
  M5.update();
  
  if (op_mode==0 || op_mode==1) pox.update();
  
  M5.Beep.update();
  
  // ボタン機能
  int axpButton = M5.Axp.GetBtnPress();
  if ( axpButton == 1 ) {
    // 1 秒以上電源ボタンを押している
    // Serial.println("M5.Axp.GetBtnPress() == 1");
  }
  if ( axpButton == 2 ) {
    // 1 秒未満電源ボタンを押して離した
    // Serial.println("M5.Axp.GetBtnPress() == 2");
    
    // リセット
    esp_restart();
  }
  
  if(M5.BtnA.wasPressed()){
    // A ボタン押下
    //Serial.println("Pressed A button.");
    
    // 画面表示の回転
    LCD_ROTATION++;
    if (LCD_ROTATION > 3) LCD_ROTATION = 0;
    M5.Lcd.setRotation(LCD_ROTATION);
    LCD_HEIGHT = M5.Lcd.height();
    LCD_WIDTH = M5.Lcd.width();
    Disbuff.deleteSprite();
    Disbuff.createSprite(LCD_WIDTH, LCD_HEIGHT);
    Disbuff.fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, BLACK);
    
    if (LCD_ROTATION==0 || LCD_ROTATION==2) LCD_ROW_STEP = 105;
    if (LCD_ROTATION==1 || LCD_ROTATION==3) LCD_ROW_STEP = 0;
    
    while(M5.BtnA.isPressed())
    {
      M5.update();
      delay(10);
    }
  }
  
  if(M5.BtnB.wasPressed()){
    // B ボタン押下
    //Serial.println("Pressed B button.");

    // モード変更
    op_mode++;
    if (op_mode > 2) op_mode = 0;
  }
  
  // シリアル受信
  if (Serial.available())
  {
    serialEvent();
    op_mode = (uint8_t)val[0];
  }
  
  if (SerialBT.available())
  {
    serialBTEvent();
    op_mode = (uint8_t)val[0];
  }
  
  // SpO2, HR の取得
  // モード 0, 1 の場合はセンサ出力，それ以外 (モード 2) の場合は既定値を設定
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    if (op_mode==0 || op_mode==1)
    {
      SpO2 = (float)pox.getSpO2();
      HR   = pox.getHeartRate();
    }
    else
    {
      SpO2 = SPO2_INI;
      HR   = HR_INI;
    }
    
    // モード 0 の場合以外は一次遅れフィルタを適用
    if (op_mode!=0)
    {
      SpO2 = SpO2_old - (SpO2_old - SpO2) / T_SpO2;
      HR   = HR_old   - (HR_old   - HR)   / T_HR;
      
      // SpO2, HR の値域制限
      if (SpO2 <  70) SpO2 =  70;
      if (SpO2 > 100) SpO2 = 100;
      if (HR <  50) HR =  50;
      if (HR > 150) HR = 150;
    }
    
    SpO2_old = SpO2;
    HR_old = HR;

    // シリアル送信: モード, 経過時間 [s], 心拍数 [bpm], SpO2 [%]
    Serial.printf(  "%d, %.3f, %.2f, %.1f\r\n", op_mode, millis()/1000.0f, HR, SpO2);
    SerialBT.printf("%d, %.3f, %.2f, %.1f\r\n", op_mode, millis()/1000.0f, HR, SpO2);
  
    tsLastReport = millis();
  }
  
  // バッテリ残量チェック
  checkBattery();
  
  // 画面表示構成
  Disbuff.setTextSize(2);
  Disbuff.setTextColor(YELLOW);
  Disbuff.setCursor(30+2, 40);
  Disbuff.printf("SpO2 %%");
  Disbuff.setCursor(M5.Lcd.width()-75-30+2, 40+LCD_ROW_STEP);
  Disbuff.printf("PR bpm");
  
  Disbuff.setTextSize(5);
  Disbuff.setTextColor(CYAN);
  Disbuff.setCursor(30-8, 60);
  Disbuff.printf("%3d", (uint8_t)(SpO2+0.5));
  Disbuff.setCursor(M5.Lcd.width()-75-30-8, 60+LCD_ROW_STEP);
  Disbuff.printf("%3d", (uint8_t)(HR+0.5));
  
  // 画面表示
  Disbuff.pushSprite(0,0);
  
  delay(10);
}
