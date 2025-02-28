#include <cmath>

/*
pin 0 - krok x
pin 1 - kierunek x
pin 2 - krok y
pin 3 - kierunek y
pin 5 - krok z
pin 6 - kierunek z

pin 4 - masa

pin 7 - krancowka x musi byc pullup
pin 8 - krancowka y musi byc pullup
pin 9 - krancowka z musi byc pullup
*/

const uint8_t krokowe[6]={0,2,5,1,3,6};
const uint8_t krancowki[3]={7,8,9};
const bool domkier[3]={false,true,false};//jeśli true to podanie na pin high powoduje ruch negatywny
const float kranpol[3]={200,0,0};
const int wymiary[6]={0,0,0,200,150,200};//wymiary placu roboczego w mm
double aktualne[3];
const int stepspermm[3]={640,640,640};
const double mmperstep[3]={0.0015625,0.0015625,0.0015625};
String otrzymane[2];

void setup() {
  pinMode(4,OUTPUT);
  digitalWrite(4,LOW);
  for(int i =0;i<6;i++){
    pinMode(krokowe[i],OUTPUT);
  }
  for(int i=0;i<3;i++){
    pinMode(krancowki[i],INPUT_PULLUP);
  }

  Serial.begin(115200);
  Serial.setTimeout(10);
  
  while(otrzymane[0]!="START"){
    if(Serial.available()){
      otrzymane[0] = Serial.readStringUntil('\n');
    }
  }
  Serial.println("Starting...");
}

void loop() {
  if(Serial.available()){
    otrzymane[0] = Serial.readStringUntil('\n');
    if(otrzymane[0]=="ok"){
      wykonaj(otrzymane[1]);
      Serial.println("gotowe");
    }
    else{
      Serial.println(otrzymane[0]);
    }
    otrzymane[1]=otrzymane[0];
  }
}

void G28(bool xbaz, bool ybaz, bool zbaz){
  bool czybaz[3]={xbaz,ybaz,zbaz};
  bool stan[3];

  bool kier=(domkier[2]==true)&&(kranpol[2]==0)||(domkier[2]==false)&&(kranpol[2]!=0);
  if(!kier){
    digitalWrite(krokowe[2+3],HIGH);
  }
  else{
    digitalWrite(krokowe[2+3],LOW);
  }
  for(int j=0;j<stepspermm[2]*40;j++){
    puls(krokowe[2],100);
  }

  for(int i=0;i<3;i++){
    if(czybaz[i]){
      stan[0]=1;
      stan[1]=1;
      stan[2]=1;
      bool kier=(domkier[i]==true)&&(kranpol[i]==0)||(domkier[i]==false)&&(kranpol[i]!=0);
      
      if(kier){
        digitalWrite(krokowe[i+3],HIGH);
      }
      else{
        digitalWrite(krokowe[i+3],LOW);
      }

      while(0!=stan[0]||0!=stan[1]||0!=stan[2]){
        puls(krokowe[i],200);
        stan[2]=stan[1];
        stan[1]=stan[0];
        stan[0]=digitalRead(krancowki[i]);
      }
      stan[0]=1;
      stan[1]=1;
      stan[2]=1;
      if(!kier){
        digitalWrite(krokowe[i+3],HIGH);
      }
      else{
        digitalWrite(krokowe[i+3],LOW);
      }

      for(int j=0;j<stepspermm[i]*8;j++){
        puls(krokowe[i],100);
      }

      if(kier){
        digitalWrite(krokowe[i+3],HIGH);
      }
      else{
        digitalWrite(krokowe[i+3],LOW);
      }

      while(0!=stan[0]||0!=stan[1]||0!=stan[2]){
        puls(krokowe[i],500);
        stan[2]=stan[1];
        stan[1]=stan[0];
        stan[0]=digitalRead(krancowki[i]);
      }
      aktualne[i]=kranpol[i];
    }
  }
}

void ruchdo(double gotox, double gotoy, double gotoz,int okres){
  double docpoz[3]={gotox,gotoy,gotoz};
  double delta[3]={docpoz[0]-aktualne[0],docpoz[1]-aktualne[1],docpoz[2]-aktualne[2]};
  double pokonane[3]={0,0,0};
  int os=2;
  double wspkier[3];
  double odch[3];

  if(gotox<wymiary[0]||gotox>wymiary[3]||gotoy<wymiary[1]||gotoy>wymiary[4]||gotoz<wymiary[2]||gotoz>wymiary[5]){
    Serial.println("OUT OF RANGE!!!");
    while(1){}//sprawdzanie zakresu
    }

  if((fabs(delta[0])>=fabs(delta[1]))&&(fabs(delta[0])>=fabs(delta[2]))){os=0;}
  else if((fabs(delta[1])>=fabs(delta[0]))&&(fabs(delta[1])>=fabs(delta[2]))){os=1;}//wyznaczanie osi 0-x,1-y,2-z

  if(delta[os]!=0){
    for(int i=0;i<3;i++){
      wspkier[i]=delta[i]/delta[os];
    }//wyznaczanie współczynnikow kierunkowych

    if((delta[0]>0)^domkier[0]){digitalWrite(krokowe[3],HIGH);}
    else{digitalWrite(krokowe[3],LOW);}
    if((delta[1]>0)^domkier[1]){digitalWrite(krokowe[4],HIGH);}
    else{digitalWrite(krokowe[4],LOW);}
    if((delta[2]>0)^domkier[2]){digitalWrite(krokowe[5],HIGH);}
    else{digitalWrite(krokowe[5],LOW);}//ustawianie kierunkow
    
    while(((delta[os]<=0)&&(aktualne[os]>docpoz[os]))||((delta[os]>0)&&(aktualne[os]<docpoz[os]))){
      for(int i=0;i<3;i++){
        if(i!=os&&(fabs(wspkier[i]*(aktualne[os]-docpoz[os])+docpoz[i]-aktualne[i])>=fabs(wspkier[i]*(aktualne[os]-docpoz[os])+docpoz[i]-aktualne[i]-mmperstep[i]))){
          puls(krokowe[i],okres);
          if(delta[i]<0){aktualne[i]=aktualne[i]-mmperstep[i];}
          else{aktualne[i]=aktualne[i]+mmperstep[i];}
        }
      }
      if(delta[os]<0){aktualne[os]=aktualne[os]-mmperstep[os];}
      else{{aktualne[os]=aktualne[os]+mmperstep[os];}}
      puls(krokowe[os],okres);
    }
    Serial.print(aktualne[0]);
    Serial.print(" ");
    Serial.print(aktualne[1]);
    Serial.print(" ");
    Serial.println(aktualne[2]);
  }
}
void puls(uint8_t pin, float czas){
  digitalWrite(pin,HIGH);
  delayMicroseconds(static_cast<int>(czas/2));
  digitalWrite(pin,LOW);
  delayMicroseconds(static_cast<int>(czas/2));
}
void wykonaj(String polecenie){
  if(polecenie.startsWith("G28")){
    bool osie[3]={false,false,false};
    for(int i=0;i<polecenie.length();i++){
      switch(polecenie[i]){
        case 'X':
          osie[0]=true;
          break;
        case 'Y':
          osie[1]=true;
          break;
        case 'Z':
          osie[2]=true;
          break;
      }
    }
    if(!(osie[0]||osie[1]||osie[2])){G28(true,true,true);}
    else{G28(osie[0],osie[1],osie[2]);}
  }
  if(polecenie.startsWith("G0")||polecenie.startsWith("G1")){
    int okres;
    if(polecenie.startsWith("G0")){okres=100;}
    else{okres=500;}
    double koordynaty[3]={aktualne[0],aktualne[1],aktualne[2]};
    for(int i=0;i<polecenie.length()&&polecenie[i]!=';';i++){
      if(polecenie[i]=='X'){
        String liczba="";
        int j=i;
        for(int j=i+1;j<polecenie.length()&&polecenie[j]!=' ';j++){
          liczba.concat(polecenie[j]);
        }
        koordynaty[0]=liczba.toDouble();
      }
      if(polecenie[i]=='Y'){
        String liczba="";
        int j=i;
        for(int j=i+1;j<polecenie.length()&&polecenie[j]!=' ';j++){
          liczba.concat(polecenie[j]);
        }
        koordynaty[1]=liczba.toDouble();
      }
      if(polecenie[i]=='Z'){
        String liczba="";
        int j=i;
        for(int j=i+1;j<polecenie.length()&&polecenie[j]!=' ';j++){
          liczba.concat(polecenie[j]);
        }
        koordynaty[2]=liczba.toDouble();
      }
    }
    ruchdo(koordynaty[0],koordynaty[1],koordynaty[2],okres);
  }
}
