#include <SoftwareSerial.h>
#include <StepperMotor.h>
#include <Wire.h>
#include <AFMotor.h>
#include <Servo.h> 
// the maximum received command length from an Android system (over the bluetooth)
#define MAX_BTCMDLEN 16
// 建立一個軟體模擬的序列埠; 不要接反了!
// HC-06    Arduino
// TX       RX/Pin2
// RX       TX/Pin13
SoftwareSerial BTSerial(2,13); // Arduino 藍芽 RX/TX
Servo servo1;
Servo servo2;
AF_Stepper motor1(48, 1);
AF_Stepper motor2(48, 2);
StepperMotor motor(14,15,16,17); // y軸步進馬達，建立步進機物件，參數就是4個接腳，IN1, IN2, IN3, IN4


byte cmd[MAX_BTCMDLEN]; // received 128 bytes from an Android system
int len = 0; // received command length
//char str[MAX_BTCMDLEN]={0};
char c=0,n=0;//判斷用字母

int dis1=0;         //移動後位置
int dis2=0;         //移動後位置
int dis1_to_road=0; //x軸移動到走道距離
int dis2_to_road=0; //y軸移動到走道距離
int cache_dis1_to_road=0;
int cache_dis2_to_road=0;
int pos_x=0;  //目前位置
int pos_y=0;  //目前位置
int cache_x=0;//移動前位置
int cache_y=0;//移動前位置

int move_x=50;//x軸移動常數
int move_y=4096;//y軸移動常數

char buf[100]={0};//顯示文字
int motorSpeed = 1; // 速度，數字愈大，速度愈慢，在5v電力下，最快可達15 rpm
int motorSteps = 4096; // 28BYJ-48 轉一圈，就是64步 ( 每步的角度是 5.625°/64 )

void setup() {
    motor.setStepDuration(motorSpeed); // 設定速度
    Serial.begin(9600);   // Arduino起始鮑率：9600
    BTSerial.begin(9600); // HC-06 出廠的鮑率：每個藍牙晶片的鮑率都不太一樣，請務必確認
    servo1.attach(9); // 伺服馬達1接腳
    servo2.attach(10); // 伺服馬達2接腳
    motor1.setSpeed(50);  //X軸步進馬達速度
    motor2.setSpeed(50);  //X軸步進馬達速度
    servo1.write(25); // Z軸伺服馬達開始先歸零
    servo2.write(25); // Z軸伺服馬達開始先歸零
    pinMode(18,INPUT);//x軸限位器
    pinMode(19,INPUT);//y軸限位器
    reset();
}

void loop() {
    char str[MAX_BTCMDLEN]={0};
    int insize, ii;
    int tick=0;
    while ( tick<MAX_BTCMDLEN ) { // 因為包率同為9600, Android送過來的字元可能被切成數份
        if ( (insize=(BTSerial.available()))>0 ){ // 讀取藍牙訊息          
            for ( ii=0; ii<insize; ii++ ){
                cmd[(len++)%MAX_BTCMDLEN]=char(BTSerial.read());                
            }
        } else {
            tick++;
        }
    }
    
    if ( len ) { // 用串列埠顯示從Android手機傳過來的訊息
        sprintf(buf,"目前位置:%d,%d",cache_x,cache_y);
        Serial.println(buf);//顯示目前的位置(座標值)
        sprintf(str,"%s",cmd);
        Serial.println(str);//顯示接收到的指令
        c=str[0];
        n=str[1];
        letter_det();//判斷x軸指令與z軸指令
        number_det();//判斷y軸指令
        Serial.println("判斷完成");
        if ( c != 'Y' && c != 'Z' && c != 'O' ){
          dis_road_x(cache_dis1_to_road);//x軸移動到走道中間
          dis_road_y(cache_dis2_to_road);//y軸移動到走道中間
        }
        pos_x=cache_x-cache_dis1_to_road;
        pos_y=cache_y-cache_dis2_to_road;
        sprintf(buf,"目前位置:%d,%d",pos_x,pos_y);
        Serial.println(buf);//顯示目前的位置(座標值)(應為走道中心)
        dis_x(dis1);//x軸移動副程式
        dis_y(dis2);//y軸移動副程式
        sprintf(buf,"目前位置:%d,%d",dis1,dis2);
        Serial.println(buf);//顯示目前的位置(座標值)(應為走道中心)
        dis_road_x(-dis1_to_road);//x軸移動到走道中間
        dis_road_y(-dis2_to_road);//y軸移動到走道中間
        pos_x=dis1-dis1_to_road;
        pos_y=dis2-dis2_to_road;
        sprintf(buf,"目前位置:%d,%d",pos_x,pos_y);
        Serial.println(buf);//顯示目前的位置(座標值)(應為棋格)
        cache_dis1_to_road=dis1_to_road;
        cache_dis2_to_road=dis2_to_road;
        cmd[0] = '\0';//清空
        dis1=0; //dis1歸零
        dis2=0; //dis2歸零
        dis1_to_road=0;
        dis2_to_road=0;
        Serial.println("移動完成");
    }
    len = 0; //len歸零
    
}

//===================副程式============================

//x軸判斷
void letter_det(){
  int value=100;
  sprintf(buf,"x軸指定位置:%c",c);
  Serial.println(buf);
  switch(c){
    case 'A':
      Serial.println("you press letter A");
      dis1=1;
      dis1_to_road=1;      
      break;
    case 'B':
      Serial.println("you press letter B");
      dis1=1;
      dis1_to_road=-1;
      break;
    case 'C':
      Serial.println("you press letter C");
      dis1=4;
      dis1_to_road=1;
      break;
    case 'D':
      Serial.println("you press letter D");
      dis1=4;
      dis1_to_road=-1;
      break;
    case 'E':
      Serial.println("you press letter E");
      dis1=7;
      dis1_to_road=1;
      break;
    case 'F':
      Serial.println("you press letter F");
      dis1=7;
      dis1_to_road=-1;
      break;
    case 'G':
      Serial.println("you press letter G");
      dis1=10;
      dis1_to_road=1;
      break;
    case 'H':
      Serial.println("you press letter H");
      dis1=10;
      dis1_to_road=-1;
      break;
    case 'Y'://on
      Serial.println("you press on");
      same_degree(); //伺服馬達轉向90度
      break;
    case 'Z'://off
      Serial.println("you press off");
      initial();     // 伺服馬達位置歸零
    break;
      case '0': //歸零
      reset();
      break;
    default:
      Serial.println("NOT LETTER IS PRESSED");
      break;          
    }
  }
//y軸判斷
void number_det(){
  sprintf(buf,"y軸指定位置:%c",n);
  Serial.println(buf);
  switch(n){
    case '1':
    Serial.println("you press letter 1");
    dis2=1;
    dis2_to_road=1;
    break;
    case '2':
    Serial.println("you press letter 2");
    dis2=1;
    dis2_to_road=-1;
    break;
    case '3':
    Serial.println("you press letter 3");
    dis2=4;
    dis2_to_road=1;
    break;
    case '4':
    Serial.println("you press letter 4");
    dis2=4;
    dis2_to_road=-1;
    break;
    case '5':
    Serial.println("you press letter 5");
    dis2=7;
    dis2_to_road=1;
    break;
    case '6':
    Serial.println("you press letter 6");
    dis2=7;
    dis2_to_road=-1;
    break;
    case '7':
    Serial.println("you press letter 7");
    dis2=10;
    dis2_to_road=1;
    break;
    case '8':
    Serial.println("you press letter 8");
    dis2=10;
    dis2_to_road=-1;
    break;
    default:
    Serial.println("NOT LETTER IS PRESSED");
    break;    
    }
  }

//x軸移動
void dis_x(int dis){
  Serial.println(dis);
  Serial.println("x軸開始在走道移動");
  int i=0;
  int cache;
  cache=dis;
  dis=dis-cache_x;
  dis=dis*move_x;
  if(dis>0){//往前移動
    while(i,i++,i<dis){
    Serial.println(i);
    motor1.step(2, FORWARD, SINGLE);
    delay(100);
    motor2.step(2, FORWARD, SINGLE);
    delay(100);
    }
  }
  else{//往後移動
    while(i,i--,i>dis){
    Serial.println(i);
    motor1.step(2, BACKWARD, SINGLE);
    delay(100);
    motor2.step(2, BACKWARD, SINGLE);
    delay(100);  
    }
  }
  cache_x=cache;
  Serial.println("x軸移動到最近走道");
}

//y軸移動
void dis_y(int disy){
  Serial.println("y軸開始在走道移動");
  Serial.println(disy);
  int cache;
  cache=disy;
  disy=disy-cache_y;
  disy=disy*move_y;
  Serial.println(disy);
  motor.step(disy); // 正值正轉，負值負轉
  delay(1000);
  cache_y=cache;
  Serial.println("y軸移動到最近走道");
}

//z軸抬升
void same_degree() {
  Serial.println("z軸上升");
  servo1.write(70);
  servo2.write(95);
}

//z軸下降
void initial(){
  Serial.println("z軸下降");
  servo1.write(25);
  servo2.write(25);
}

//x軸與y軸歸零
void reset(){
  Serial.println("歸零");
  int i=0;
  while(digitalRead(18) == HIGH){
    Serial.println(i);
    motor1.step(5, BACKWARD, SINGLE);
    delay(100);
    motor2.step(5, BACKWARD, SINGLE);
    delay(100);
    i++;
  }
  i=0;
  while(digitalRead(19) == HIGH){
    Serial.println(i);
    motor.step(-10); // 轉一圈
    i++;
  }
  cache_x=0;
  cache_y=0;
  Serial.println("已歸零");
}
//移動到走道或是離開走到
void dis_road_x(int dis){
  Serial.println(dis);
  int i=0;
  dis=dis*move_x;
  if(dis>0){//往前移動
    while(i,i++,i<dis){
    Serial.println(i);
    motor1.step(2, FORWARD, SINGLE);
    delay(100);
    motor2.step(2, FORWARD, SINGLE);
    delay(100);
    }
  }
  else{//往後移動
    while(i,i--,i>dis){
    Serial.println(i);
    motor1.step(2, BACKWARD, SINGLE);
    delay(100);
    motor2.step(2, BACKWARD, SINGLE);
    delay(100);  
    }
  }
  Serial.println("移動到棋格或是離開棋格");
}

void dis_road_y(int dis){
  dis=dis*move_y;
  Serial.println(dis);
  motor.step(dis); // 正值正轉，負值負轉
  delay(1000);
  Serial.println("移動到走道中心或是離開中心");
}
