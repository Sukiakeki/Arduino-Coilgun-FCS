#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

unsigned long time = 0;  //millis() 用
Servo xserv;   //伺服馬達達
Servo yserv;
Servo zserv;
Servo sserv;
int trigP = 52;   //超音波模組
int echoP = 53;
int yservA = 35; 
int xservA = 97; 
int zservA = 160;
int sservA = 107;
int Vt = 0;
double B = 0;  //超音波計算後砲台轉動角度
double xjoy = 0;  //A9  joystick
int yjoy = 0;  //A8
int bled = 13; //板載LED   sda 20   scl 21
int Ran = 0;   //同步亂數用
int Tset[50];  //開關用
const byte R = 4; //列
const byte L = 4;
byte rowPins[R] = {10,9,8,7}; //定義鍵盤腳位
byte colPins[L] = {6,5,4,3};
//平射模式數據
int DisFT[100] = { 0,0,0	,30	,50	,60	,70	,70	,100	,110	,140	,155 ,160	,165,	165,	170	,390	,390	,390	,390	,390,	390	,390	
,390,390,	390,	300,	390	,390	,390,	390	,390	,390	,390	,390	,390,220,	260	,275	,285	,319	,385,240,275};
int DisFA[100] = { 0,0,0,	0	,0,	0,	0	,2,	2,	2	,2,	2 	,2	,2	,2	,2	,2	,0	,0	,0	,0	,0,	0	,0	,0	,0	,2	,2	,2	,2	,2	,2	,4	,4	,4,	4	,6	,6,	6,	6	,6	,6,8,8};
//仰射模式數據
int DisHT[25] = {  };
int DisHA[25] = {  };

char keys[R][L] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, R, L );
LiquidCrystal_I2C lcd(0x27,20,4);  // 設定i2c位址

int key (void){    //按鍵
  while(1){
    char k = keypad.getKey();  
    if(k>0){
      switch(k){
        case '1':
          return 1;
        case '2':
          return 2;
        case '3':
          return 3;
        case '4':
          return 4;
        case '5':
          return 5;
        case '6':
          return 6;
        case '7':
          return 7;
        case '8':
          return 8;
        case '9':
          return 9;
        case '0':
          return 10;
        case '#':
          return 11;
        case '*':
          return 12;
        case 'A':
          return 13;
        case 'B':
          return 14;
        case 'C':
          return 15;
        case 'D':
          return 16;          
      }
    }
  }
}

int enter (void){  //輸入
  int K = 0;
  int re = 0;  
  while(K!=16){
    int K = key();
    if(re==0){
      if(K==0){
        continue;
      }
      re = K;
    }
    else if(K==11){        //按#後退
      if(re==0){
        continue;
      }
      re = (re-(re%10))/10;
    }
    else{
      if(K>10){
        if(K==15){  
          break;
        }  
        continue;
      } 
      else if(K==10){
        re *= 10;
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print(re);
        Serial.println(re);
        continue;
      }
      re = re*10 + K;
    }
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print(re);
    Serial.println(re);
  }
  return re;
}

void rotate (void){
  while(1){
      //yserv.write(yjoy);
    Serial.print("X : ");
    Serial.println(xjoy);
    Serial.print("Y : ");
    Serial.println(yjoy); 
    Serial.println(xservA);

    xjoy = analogRead(9);  
    yjoy = analogRead(8);
    xjoy = map(xjoy,0,1023,-25,25);  //搖桿轉換角度
    //yjoy = map(yjoy,0,1023,5,-5);    
    if(xjoy>7 or xjoy<-7){
      xservA += xjoy;
      xserv.write(xservA);
      delay(10);
    }    
    char kkk = keypad.getKey();    //按C結束
    if(kkk>0){
      if(kkk == 'C'){
        return 0;
      }
    }
  }
}

void reload(void){
  zserv.write(zservA);
  delay(350);
  zserv.write(0);
  delay(350);
  zserv.write(zservA);
}

double Srch(void){  //超音波距離
  digitalWrite(trigP, 0);
  delayMicroseconds(5);
  digitalWrite(trigP, 1);     // trig 高電位 10us
  delayMicroseconds(10);
  digitalWrite(trigP, 0);

  pinMode(echoP, 0);             // 讀取 echo 的時間
  double dura = pulseIn(echoP, 1); 
  double dis = dura/58.2; //29.1us 1cm
  return dis;
}

int Search(void){  //找尋目標
  double dis,Max[10];
  int i=0;
  int virt0 = sservA-90;
  Max[0] = 1000000;
  Max[2] = 1000000;
  sserv.write(virt0);
  delay(1000);
  for(i=0;i<140;i++){   //左右掃一次找最近
    sserv.write(i + virt0);
    dis = Srch();
    if(dis < Max[0]){
      Max[0] = dis;
      Max[1] = i;
    }
    Serial.println(i);
    Serial.print("dis:");
    Serial.println(dis);
    Serial.print("Min-ang:");
    Serial.println(Max[1]);
    Serial.print("Min-dis:");
    Serial.println(Max[0]);
  }
  for( i=140;i>0;i--){
    sserv.write(i + virt0);
    dis = Srch();
    if(dis < Max[2]){
      Max[2] = dis;
      Max[3] = i;
    }
    Serial.println(i);
    Serial.print("dis:");
    Serial.println(dis);
    Serial.print("Min-ang:");
    Serial.println(Max[3]);
    Serial.print("Min-dis:");
    Serial.println(Max[2]);
  }
  Serial.print("Min-ang:");
  Serial.println(Max[3]);
  Serial.print("Min-dis:");
  Serial.println(Max[2]);

  if(/*abs(Max[0]-Max[2])  <= 3*/1){  //確認數據無誤
    double An_ = (Max[1]+Max[3])/2
    int dismin = Max[0] = (Max[0]+Max[2])/2;
    double r = //sqrt( pow((Max[0]*sin(An_)+4.5),2) + pow((Max[0]*cos(An_)+16),2) );
    //B = 1/tan((Max[0]*sin(An_)+4.5)/(Max[0]*cos(An_)+16));
    return r;
  }
  else{
    //return Search();
  }
}

int SC (void){  //輸入距離用射控系統 
  int dis = 0;
  int an = 0;
  int ann =0;
  Serial.println("SCing...");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter Distance :");   //輸入距離
  Serial.println("Enter Distance :");  
  lcd.setCursor(0,1);  
  dis = enter();
  Serial.print("Dis :");
  Serial.println(dis);  
  lcd.clear();
  lcd.print("Set angle");  //切換模式
  lcd.setCursor(0,1);
  lcd.print(" L -> 1   H -> 2 ");
  while(1){
    ann = key();
    if(ann<=2){
      if(ann==1){
        lcd.setCursor(0,1);
        an=ann;
      }
      else{
        lcd.setCursor(9,1);
        an=ann;
      } 
    }
    else if(ann==15 || an!=0){
      lcd.clear();
      break;
    }
    else{
      continue;
    }
  }
  lcd.print("Rotate");   //旋轉平台瞄準
  ///rotate();
  lcd.clear();
  if(an==1){  //平直射擊
    lcd.print("Flat Mode");
    int dd = dis/25;
    int ddd = dis%25;
    int dddd ;
    char hhh;
    Serial.print("SC flat dis : ");
    Serial.println(dis);
    Serial.println(dd);
    Serial.println(ddd);
    if( ddd>14 ){
      digitalWrite(28,0);
      delay( DisFT[dd] *10);
      digitalWrite(28,1);
      reload();
      yserv.write( yservA + DisFA[dd] );
    }
    else{
      digitalWrite(28,0);
      delay( DisFT[dd-1] *10);
      digitalWrite(28,1);
      reload();
      yserv.write( yservA + DisFA[dd-1] );
    }
    while(1){       //按A發射
      hhh = keypad.getKey();
      if( hhh == 'A' ){
        lcd.clear();
        lcd.print("Shot");      
        Serial.println("shot");
        digitalWrite(26,0);
        delay(1000);
        digitalWrite(26,1);
        lcd.clear();
        break;
      }
    }
    while(1){
      lcd.clear();
      lcd.print("Angle error fix");  //誤差調整
      lcd.setCursor(0,1);
      lcd.print("1 to 10  '5'=mid");
      dddd = enter();
      lcd.clear();
      digitalWrite(28,0);
      delay( DisFT[dd-1]+(dddd-5)*50 );
      digitalWrite(28,1);
      reload();
      while(1){       //按A發射
        hhh = keypad.getKey();
        if( hhh == 'A' ){
          lcd.clear();
          lcd.print("Shot");      
          Serial.println("shot");
          digitalWrite(26,0);
          delay(1000);
          digitalWrite(26,1);
          lcd.clear();
          return 0;
          break;
        }
      }
      /*while(1){       //按c結束誤差模式
        hhh = keypad.getKey();
        if( hhh == 'C' ){
          return 0;
        }
        else{
          break;
        }
      }*/
   }
  }
  else{   //高拋物線射擊
    //lcd.print("Mortar Mode");

  }
}

int SC2(void){  //雷達射控
  Serial.println("SC2----------------");
  lcd.clear();
  int dis = Search();
  int realB = B+90;
  while(1){
    lcd.print("Flat Mode");
    Serial.print("an:");
    Serial.println(realB);
    Serial.print("dis:");
    Serial.println(dis);
    xserv.write(realB);

    if( dis%25 >14 ){   //判定強度區間
      digitalWrite(28,0);
      delay( DisFT[dis/25] *10);
      digitalWrite(28,1);
      reload();
      yserv.write( yservA + DisFA[dis/25] );
    }
    else{
      digitalWrite(28,0);
      delay( DisFT[dis/25 -1] *10);
      digitalWrite(28,1);
      reload();
      yserv.write( yservA + DisFA[dis/25 -1] );
    }

    while(1){       //按A發射
      char hhh = keypad.getKey();
      if( hhh == 'A' ){
        lcd.clear();
        lcd.print("Shot");      
        Serial.println("shot");
        digitalWrite(26,0);
        delay(1000);
        digitalWrite(26,1);
        lcd.clear();
        return 0;
      }
    }
  }
}
/*
int SC2 (void){  //廢棄雷達射控系統 
  int dis = 0;
  int an = 1;
  int ann =0;
  Serial.println("SC2 ing...");
  lcd.clear();
  dis = Search();
  if(an==1){  //平直射擊
    lcd.print("Flat Mode");
    Serial.print("an:");
    Serial.println(B);
    int dd = dis/25;
    int ddd = dis%25;
    int dddd ;
    int An = -B + xservA;
    char hhh;
    Serial.print("SC flat dis : ");
    Serial.println(dis);
    Serial.println(dd);
    xserv.write(An);
    Serial.println(ddd);
    if( ddd>14 ){
      digitalWrite(28,0);
      delay( DisFT[dd] *10);
      digitalWrite(28,1);
      reload();
      yserv.write( yservA + DisFA[dd] );
    }
    else{
      digitalWrite(28,0);
      delay( DisFT[dd-1] *10);
      digitalWrite(28,1);
      reload();
      yserv.write( yservA + DisFA[dd-1] );
    }
    while(1){       //按A發射
      hhh = keypad.getKey();
      if( hhh == 'A' ){
        lcd.clear();
        lcd.print("Shot");      
        Serial.println("shot");
        digitalWrite(26,0);
        delay(1000);
        digitalWrite(26,1);
        lcd.clear();
        return 0;
      }
    }
    /*
    while(1){
      lcd.clear();
      lcd.print("Angle error fix");  //誤差調整
      lcd.setCursor(0,1);
      lcd.print("1 to 10  '5'=mid");
      dddd = enter();
      lcd.clear();
      digitalWrite(28,0);
      delay( DisFT[dd-1]+(dddd-5)*50 );
      digitalWrite(28,1);
      reload();
      while(1){       //按A發射
        hhh = keypad.getKey();
        if( hhh == 'A' ){
          lcd.clear();
          lcd.print("Shot");      
          Serial.println("shot");
          digitalWrite(26,0);
          delay(1000);
          digitalWrite(26,1);
          lcd.clear();
          return 0;
          break;
        }  
      }
      /*while(1){       //按c結束誤差模式
        hhh = keypad.getKey();
        if( hhh == 'C' ){
          return 0;
        }
        else{
          break;
        }
      }
   }
  }
  else{   //高拋物線射擊
    //lcd.print("Mortar Mode");

  }
}
*/

int swi(int a){     //開關邏輯 輸入腳位/編號
  if(Tset[a] == 0){ 
      Tset[a] = 1;
      return 0;
  }
  else{
    Tset[a] = 0;
    return 1;
  }  
}

//------------------------------------------------------------------------------------------------
void setup() {

  pinMode(bled,1);
  pinMode(26,1);  //26 shot   繼電器
  pinMode(28,1);  //28 charge
  pinMode(30,0);  //joystick press
  pinMode(trigP, 1);   //超音波接腳
  pinMode(echoP, 0);

  xserv.attach(12);
  yserv.attach(13);
  zserv.attach(11);
  sserv.attach(2);
  yserv.write(yservA); //舵機歸0
  xserv.write(xservA);
  zserv.write(zservA);
  sserv.write(sservA);
  digitalWrite(26,1);
  digitalWrite(28,1);
  Serial.begin(115200);
  randomSeed(analogRead(0));
  Ran = random(999);

  lcd.begin(16, 2); // 初始化LCD
  lcd.init();
  lcd.init();
  lcd.blink();  //lcd.noblink();
  lcd.setBacklight(255);
  lcd.clear();
  lcd.setCursor(0, 0);  //設定游標位置 (字,行)
  lcd.print("setup complete");
  lcd.setCursor(0, 1);
  lcd.print(Ran);

  Serial.print("setup complete");
  Serial.println(Ran);

}

//-------------------------------------------------------------------------------------
/*
void loop(){
  while(1){
  xserv.write(90);
  //yserv.write(90);
  //zserv.write(90);
  sserv.write(90);
  delay(4000);
  xserv.write(0);
  //yserv.write(0);
  //zserv.write(0);
  sserv.write(0);
  delay(4000);
  }
}
*/
void loop() {  
  char kk = keypad.getKey();  
  if(kk>0){
    Serial.print("keyboard :");
    Serial.println(kk);
    switch(kk){
      case 'A':   //發射
        lcd.clear();
        lcd.print("Shot");      
        Serial.println("shot");
        digitalWrite(26,0);
        delay(1000);
        digitalWrite(26,1);
        lcd.clear();
        lcd.print("setup complete");
        lcd.println(Ran);
        break;
        
      case 'B':   //充電
        lcd.clear();
        lcd.print("Charge");
        Serial.println("charge");
        if(swi(28)==1){
          digitalWrite(28,1);
          lcd.clear();
        }
        else{
          digitalWrite(28,0);
        }
        lcd.print("setup complete");
        lcd.println(Ran);
        break;

      case 'C':  //進入射控
        SC();
        lcd.print("setup complete");
        lcd.println(Ran);
        break;

      case 'D':  //測試用
        lcd.clear();
        lcd.print("charge time:");
        int ji = enter();
        digitalWrite(28,0);        
        delay( 10 * ji );
        digitalWrite(28,1);
        reload();
        lcd.print("setup complete");
        lcd.println(Ran);
        break;
    }
    if(kk=='1'){
      Serial.println("yserv +");
      yservA += 2;
      yserv.write(yservA);
      delay(25);
    }
    if(kk=='2'){
      Serial.println("yserv -");
      yservA -= 2;
      yserv.write(yservA); 
      delay(25);
    }
    if(kk=='3'){
      lcd.clear();
      lcd.print("rotate");
      rotate();
      lcd.clear();
      lcd.print("setup complete");
      lcd.println(Ran);
    }
    if(kk=='4'){
      lcd.clear();
      lcd.print("test");
      for(int g = 0; g < 5; g++ ){
        //xserv.write(90);
        yserv.write(90);
        zserv.write(90);
        sserv.write(90);
        delay(4000);
        //xserv.write(0);
        yserv.write(0);
        zserv.write(0);
        sserv.write(0);
        delay(4000);
      } 
      lcd.print("setup complete");
      lcd.println(Ran);
    }
    if(kk=='5'){
      lcd.clear();
      lcd.print("charge time:");
      int ji = enter();
      digitalWrite(28,0);        
      delay( 100 * ji );
      digitalWrite(28,1);
      yserv.write(10);
      delay(200);
      reload();
      lcd.print("setup complete");
      lcd.println(Ran);
    }
    if(kk=='6'){
      reload();
    }
    if(kk=='7'){
      lcd.clear();
      lcd.print("angle:");
      int annn = enter();
      lcd.clear();
      lcd.print("charge time:");
      int ji = enter();
      digitalWrite(28,0);        
      delay( 10 * ji );
      digitalWrite(28,1);
      yserv.write(10);
      delay(200);
      reload();
      delay(100);
      yserv.write(annn + yservA);
      lcd.print("setup complete");
      lcd.println(Ran);
    }
    if(kk=='8'){
      Serial.println("yserv +");
      yservA += 2;
      yserv.write(yservA);
      delay(25);
    }
    if(kk=='9'){
      Serial.println("yserv -");
      yservA -= 2;
      yserv.write(yservA);  
      delay(25);
    }
    if(kk=='*'){
      SC2();
      lcd.print("setup complete");
      lcd.println(Ran);
    }
  }
        

  /*
  if((millis()-time)>=1000){   //延時閃爍
    digitalWrite(28,swi(1));
    digitalWrite(bled,swi(2));
    time = millis();
  }
  
  
  /*digitalWrite(bled,1);
  delay(1000);
  digitalWrite(bled,0);
  delay(1000);*/
  
}
