//Main.ino

// Includes secction
// Includes secction

// Interface Android
#include "Android.h"

// Serial Conection
#include <SoftwareSerial.h> //
SoftwareSerial BT(10, 11); // Serial Bluetooth Conection at pins 10 and 11

// Bluetooth Conection
#include "dht11.h" //
dht11 DHT11; // Humidity Sensor Instance

// Hardware Definitions
#define Vi 5.0 // Voltage Input
#define R 1.0 // LRD Resistor voltage divider


// Enumerations 
// Enumeration of the possible Control Methods
enum {
    SIMPLE_ON_OFF,
    HISTERESIS_ON_OFF,
    DIFERENTIAL_PROPORTIONAL,
    INTEGRAL_PROPORTIONAL
} ControlTypeSelector;


// Static definitions
static const int    pin_SL1 = A2,   // Light Sensor 1 pwm pin
                    pin_SL2 = A3,   // Light Sensor 2 pwm pin
                    pin_LEDR = 6,   // Red LED pwm pin
                    pin_LEDG = 5,   // Green LED pwm pin
                    pin_LEDB = 3,   // Blue LED pwm pin
                    pin_Fan = 9,    // Fan pwm pin
                    pin_POT = A0,   // Potenciometer pwm pin
                    pin_U = 8;      // Humidity bus pin

static int filterSample = 30;

static unsigned char    T1MODE = (1 << INTEGRAL_PROPORTIONAL), 
                        T2MODE = (1 << INTEGRAL_PROPORTIONAL); // Initial Mode Types

static bool logIn = false; // Logger flag

static char a;
static String   Input_Mode,
                msg;

static float    T1,
                T2,
                L1,
                L2,
                U1;

// PID Control Parameters
static double Kp = 0.4, Ki = 0.4, Kd = 0.1;

static double   Fan_1 = 0.0,
                Fan_2 = 0.0;

static double   T1target = 25.0,
                T1error = 0.0,
                T1_f = 0.0,
                prev_T1error = 0.0,
                T1threshold = 0.0,
                prev_T1action = 0.0,
                T1_Integral = 0.0,
                T2target = 25.0,
                T2error = 0.0,
                T2_f = 0.0,
                prev_T2error = 0.0,
                T2threshold = 0.0,
                prev_T2action = 0.0,
                T2_Integral = 0.0,
                setPoint;

static float    U1target = 55.0,
                U1error = 0,
                U1threshold = 0;

static int  SUerror = 0,
            SUTime = 0;

static float    L1target = 0,
                L1error = 0,
                L1threshold = 0,
                L2target = 0,
                L2error = 0,
                L2threshold = 0,
                R_Target = 0.0,
                G_Target = 0.0,
                B_Target = 0.0;

static int  SL1,
            SL2;

//Functions definitions
void SetupLog();
void SetupParameters();
void DisplayInfo();
void SerialEventManager();
void BluetoothEventManager();
void getTemperature();
void getHumidity();
void getLight();

void setup()
{
    cli(); // Stop interrupts

    // set timer1 interrupt at 2Hz //
    TCCR1A = 0; // Set entire TCCR1A register to 0
    TCCR1B = 0; // Same for TCCR1B
    TCNT1 = 0; // Initialize counter value to 0

    // set timer count for 2hz increments //
    OCR1A = (16*10^6) / (1*2048) - 1; //

    // turn on CTC mode //
    TCCR1B |= (1 << WGM12); //

    // Set CS11 bit for 8 prescaler //
    TCCR1B |= (1 << CS11); //

    // enable timer compare interrupt //
    TIMSK1 |= (1 << OCIE1A); //

    sei(); // Allow interrupts


    // set the data rate for the SoftwareSerial port and Serial pc comunication
    BT.begin(9600);
    Serial.begin(9600);


    // Send configure message to other device
    BT.println("Arduino Bluetooth Connection Established ");
    BT.println("//--// Arduino Thermal Chamber Terminal Control (MK.2) //--// ");

    setPoint = 10.0;
    R_Target = 2.55*100.0;
    G_Target = 2.55*100.0;
    B_Target = 2.55*100.0;
}


void loop()
{
    // Update global variables
    getTemperature();
    getLight();
    getHumidity();

    // Serial Event Catcher
    if(Serial.available()){
        SerialEventManager();
    }

    // Bluetooth Event Catcher
    if (BT.available()){
        BluetoothEventManager();
    }

    Serial.print (setPoint);
    Serial.print (",");
    Serial.print ((T1 + T2)/2.0);
    Serial.print (",");
    Serial.print (3600*Fan_1);
    Serial.print (",");
    Serial.print (R_Target/255);
    Serial.print (",");
    Serial.print (G_Target/255);
    Serial.print (",");
    Serial.print (B_Target/255);
    Serial.print (",");
    Serial.print (DHT11.humidity);
    Serial.print (",");
    Serial.println ((analogRead((A2)/1024) + analogRead((A3)/1024))/2);
    delay(2000);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

// Interrupt Flag catcher function
ISR(TIMER1_COMPA_vect){ 
    if (logIn == true){ //
        for(int i=0;i < Input_Mode.length(); i++){
            if(Input_Mode.charAt(i) == '1'){
                switch(i){
                case 4:
                    Serial.print((T1+T2)/2.0);
                    Serial.print(",(");
                    Serial.print(T1);
                    Serial.print("),");
                    Serial.println (3600*Fan_1);
                    Serial.println(",");
                    break;
                case 3:
                    Serial.print((T1+T2)/2.0);
                    Serial.print(",(");
                    Serial.print(T2);
                    Serial.print("),");
                    Serial.println (3600*Fan_1);
                    Serial.println(",");
                    break;
                case 2:
                    Serial.print(analogRead((pin_SL1)/1024));
                    Serial.println(",");
                    break;
                case 1:
                    Serial.print(analogRead((pin_SL2)/1024));
                    Serial.println(",");
                    break;
                case 0:
                    SUTime += 1;
                    if(SUerror == DHTLIB_OK){
                        Serial.print(DHT11.humidity);
                        Serial.println(",");
                        SUTime = 0;
                    }
                    break;
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void SerialEventManager()
{
    msg = Serial.readString(); // Read serial buffer string
    BT.println(msg); // Print echo back to bluetooth paired device
}

void BluetoothEventManager()
{
    msg = BT.readString();
    //This code is based on this: http://www.instructables.com/id/Arduino-AND-Bluetooth-HC-05-Connecting-easily/step3/Arduino-Code/
    if(msg == "Start log"){
        BT.println("Starting the Analog input log");
        logIn = true;
    }
    else if(msg == "Stop log"){
        logIn = false;
        BT.println("Input Log halted");
    }
    else if(msg == "Help"){
        BT.println("Boo!");
        // Create help function
    }
    else if(msg == "Log setup"){
        SetupLog();
        BT.println("log succesfully configured");
    }
    else if(msg == "Display"){
        DisplayInfo();
    }else if(msg == "Parameters setup"){
        SetupParameters();
        BT.println("Parameters succesfully configured");
    }else{
        BT.print("Command: ");
        BT.print(msg);
        BT.println(" could not be recognized. Send help for more info!");
    }
    // you can add more "if" statements with other characters to add more commands
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetupLog(){
    BT.println("Starting Input Log Setup");
    BT.println("Choose the desired measures:");
    BT.println("Temp 1 (00001)\nTemp 2 (00010)\nLum 1 (00100)\nLum 2 (01000)\nHum (10000)");
    
    while(!BT.available()){}

    msg = BT.readString();
    Input_Mode = msg;
    BT.print("Chosen Inputs: ");
    
    int j=0;
    for(int i=0;i < msg.length(); i++){
        if(msg.charAt(i) == '1'){
            j+=1;
            switch(i){
                case 4:
                    BT.println(") Temp 1");
                    break;
                case 3:
                    BT.println(") Temp 2");
                    break;
                case 2:
                    BT.println(") Lum 1");
                    break;
                case 1:
                    BT.println(") Lum 2");
                    break;
                case 0:
                    BT.println(") Hum");
                    break;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetupParameters(){
    BT.println("Parameters Setup\nThreshoolds:");
    BT.println("Temperature 1:");
    String temp;

    while(!BT.available()){
    }

    temp = BT.readString();
    T1target = temp.toFloat();
    BT.println("Temperature 2:");

    while(!BT.available()){
    }

    temp = BT.readString();
    T2target = temp.toFloat();
    setPoint = (T1target+T2target)/2;
    BT.println("Lighing 1:");

    while(!BT.available()){
    }

    temp = BT.readString();
    L1 = temp.toFloat();
    BT.println("Lightng 2:");

    while(!BT.available()){
    }

    temp = BT.readString();
    L2 = temp.toFloat();
    BT.println("Humidity:");

    while(!BT.available()){
    }

    temp = BT.readString();
    U1 = temp.toInt();
    BT.println("R:");

    while(!BT.available()){
    }

    temp = BT.readString();
    R_Target = temp.toInt()*2.55;
    Serial.println(R_Target);
    analogWrite(pin_LEDR, R_Target);
    BT.println("B:");

    while(!BT.available()){
    }

    temp = BT.readString();
    B_Target = temp.toInt()*2.55;
    Serial.println(B_Target);
    analogWrite(pin_LEDB, B_Target);
    BT.println("G:");

    while(!BT.available()){
    }

    temp = BT.readString();
    G_Target = temp.toInt()*2.55;
    Serial.println(G_Target);
    analogWrite(pin_LEDG, G_Target);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisplayInfo(){
    int j=0;
    BT.println("Display ----- Logger:\r");
    for(int i=0;i < Input_Mode.length(); i++){
        if(Input_Mode.charAt(i) == '1'){
        j+=1;
        BT.print(j);
            switch(i){
                case 4:
                    BT.println(") Temp 1");
                    break;
                case 3:
                    BT.println(") Temp 2");
                    break;
                case 2:
                    BT.println(") Lum 1");
                    break;
                case 1:
                    BT.println(") Lum 2");
                    break;
                case 0:
                    BT.println(") Hum");
                    break;
            }
        }
    }

    BT.println("Display ----- Parameters:");
    BT.print("T1 = ");
    BT.println(T1);
    BT.print("T2 = ");
    BT.println(T2);
    BT.print("L1 = ");
    BT.println(L1);
    BT.print("L2 = ");
    BT.println(L2);
    BT.print("U1 = ");
    BT.println(U1);
    BT.print("(R,G,B) = ");
    BT.print(R_Target);
    BT.print(",");
    BT.print(G_Target);
    BT.print(",");
    BT.println(B_Target);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void getTemperature(){

    // Private Variables
    double T1Array[filterSample],
    T2Array[filterSample];
    int j;

    // Median Noise Filtering for T1
    for(int i=0; i < filterSample; i++){
        T1 = analogRead (A0)*0.5;
        if((T1_f-T1 >= 0.3) || (T1_f-T1 <= 0.3)){ // Check for anomalies peaks (temperature in this ambient WILL NOT peak)
            T1 = (T1+T1_f)/2;
        }
        if(i==0){
            j=0;
        }else{
            for(j=1; j<i; j++){ // Find location of this temperature measure
                if(T1Array[j-1]<=T1 && T1Array[j+1]>=T1){
                    break;
                }
            }
        }
        for(int k=i; k>j; k--){ // Shift of the array
            T1Array[k] = T1Array[k-1];
        }
        T1Array[j] = T1;
    }

    // Find the mean value of a 5-width band between he center
    T1_f = 0.0;
    for(int i=((filterSample/2)-5); i < ((filterSample/2)+5); i++){
        T1_f += T1Array[i];
    }
    T1_f /= 10;

    // Median Noise Filtering for T2
    for(int i=0; i < filterSample; i++){
        T2 = analogRead (A1)*0.5;
        // Check for anomalies peaks (temperature in this ambient WILL NOT peak)
        if((T2_f-T2 >= 0.3) || (T2_f-T2 <= 0.3)){ 
            T2 = (T2+T2_f)/2;
        }
        if(i==0){
            j=0;
        }else{
            // Find location of this temperature measure
            for(j=1; j<i; j++){ 
                if(T2Array[j-1]<=T2 && T2Array[j+1]>=T2){
                    break;
                }
            }
        }
        
        // Shift of the array
        for(int k=i; k>j; k--){ 
            T2Array[k] = T2Array[k-1];
        }
        T2Array[j] = T2;
    }

    // Find the mean value of a 5-width band between he center
    T2_f = 0.0;
    for(int i=((filterSample/2)-5); i < ((filterSample/2)+5); i++){ 
        T2_f += T2Array[i];
    }
    T2_f /= 10;
    T1_f = (T1 + T2)/2;
    T1target = setPoint;


    // Error in degrees
    T1error = (T1_f - T1target);

    if(T1MODE & (1 << SIMPLE_ON_OFF)){
        if(T1error > 0.0){
            Fan_1 = 1.0;
        } 
        else {
            Fan_1 = 0.0;
        }
    }
    else if(T1MODE & (1 << HISTERESIS_ON_OFF)){
        if(T1error > T1threshold){
            Fan_1 = 1.0;
        } 
        else if(T1error < -T1threshold){
            Fan_1 = 0.0;
        }
    }
    else if(T1MODE & (1 << DIFERENTIAL_PROPORTIONAL)){
        Fan_1 = Kd * T1error;
        if(Fan_1 > 1.0){
            Fan_1 = 1.0;
        } 
        else if(Fan_1 < 0){
            Fan_1 = 0.0;
        } 
        else if(Fan_1 > 0 && Fan_1 < 0.3){
            Fan_1 = 0.3;
        }
    }
    else if(T1MODE & (1 << INTEGRAL_PROPORTIONAL)){
        //Fan_1 = prev_T1action + Kp_I*(T1error - prev_T1error) + Ki*0.02*T1error;
        T1_Integral += Ki*T1error;
        if(T1_Integral > 1.0){
            T1_Integral = 1.0;
        } 
        else if (T1_Integral < 0.0){
            T1_Integral = 0.0;
        }
        Fan_2 = Kp*T1error + T1_Integral - Kd*(Fan_2 - prev_T1action);
        if(Fan_2 > 1.0){
            Fan_2 = 1.0;
        } 
        else if(Fan_2 < 0){
            Fan_2 = 0.0;
        } 
        else if(Fan_2 > 0 && Fan_2 < 0.3){
            Fan_2 = 0.3;
        }
    }
    Fan_1 = Fan_2;
    
    
    //pc.printf("(STone)%3.1f,%3.1f,%3.1f,%3.1f\r", T1_f, T1target, Fan_2.read(), Fan_1.read());
    prev_T1error = T1error;
    prev_T1action = Fan_1;


    analogWrite(pin_Fan, 255*Fan_1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void getLight(){
    // Estamos controlando o tempo em que esta ligada
    // Presumimos que a estufa sera usada em lugares fechados sem iluminação externa, seria trabalhoso equação
    // Ver a composição RGB segundo as estações
    // Formula for conversion from voltage to lux from ldr
    //
    // Vo = Vi*(Rl/(Rl + R)) -- Vo: Voltage divider || Vi = input voltage || R = resistor
    // Rl = 500/lux -- Rl: LDR resistance
    // lux = ((Vi*500/Vo) - 500)/R;
    
    SL1 = analogRead (pin_SL1) / 1023;
    
    SL2 = analogRead (pin_SL2) / 1023;
    
    L1 = ((Vi*500/SL1) - 500)/R;
    L2 = ((Vi*500/SL2) - 500)/R;
    
    analogWrite(pin_LEDR, R_Target);
    analogWrite(pin_LEDG, G_Target);
    analogWrite(pin_LEDB, B_Target);
    
    if (R_Target + G_Target + B_Target > 0){
    //isOn = true;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void getHumidity(){
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////