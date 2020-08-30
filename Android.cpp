//Android.cpp

#include "Android.h"

Android::Android (){
    dadosEnviados = "T . U L I FH M PD H M O ";
    dadosRecebidos = "T . U L I FH M PD H M ";
}

Android::~Android(){
}

float Android::getTemperaturaModelo (){
    float temperatura;
    temperatura = 10*ConverteCharacterParaInteger(dadosEnviados[1]);
    temperatura += ConverteCharacterParaInteger(dadosEnviados[2]);
    temperatura += 0.1*ConverteCharacterParaInteger(dadosEnviados[4]);
    temperatura += 0.01*ConverteCharacterParaInteger(dadosEnviados[5]);
    return temperatura;
}

void Android::setTemperaturaAtual (float temperatura){
    int intTemp = (int) temperatura;
    int chave1 = intTemp/10;
    int chave2 = intTemp%10;
    int chave3 = (100*(temperatura - intTemp));
    int chave4 = chave3%10;
    chave3 = chave3 - chave4;
    dadosRecebidos[1] = ConverteIntegerParaChar(chave1);
    dadosRecebidos[2] = ConverteIntegerParaChar(chave2);
    dadosRecebidos[4] = ConverteIntegerParaChar(chave3);
    dadosRecebidos[5] = ConverteIntegerParaChar(chave4);
}

float Android::getUmidadeModelo (){
    float umidade;
    umidade = 10*ConverteCharacterParaInteger(dadosEnviados[7]);
    umidade += ConverteCharacterParaInteger(dadosEnviados[8]);
    umidade += 0.1*ConverteCharacterParaInteger(dadosEnviados[9]);
    return umidade;
}

void Android::setUmidadeAtual (int umidade){
    int chave1, chave2, chave3;
    chave3 = umidade%10;
    chave2 = umidade%100 - chave3;
    chave1 = umidade/100;
    dadosRecebidos [7] = ConverteIntegerParaChar(chave1);
    dadosRecebidos[8] = ConverteIntegerParaChar(chave2);
    dadosRecebidos[9] = ConverteIntegerParaChar(chave3);
}

bool Android::getLuminosidadeModelo (){
    if (dadosEnviados[11] == 'T')
        return true;
    else
        return false;
}

void Android::setLuminosidadeAtual (bool luminosidade){
    if (luminosidade)
        dadosRecebidos[11] = 'V';
    else 
        dadosRecebidos[11] = 'F';
}

int Android::getIntensidadeModelo (){
    int intensidade = 1000*ConverteCharacterParaInteger(dadosEnviados[13]);
    intensidade += 100*ConverteCharacterParaInteger(dadosEnviados[14]);
    intensidade += 10*ConverteCharacterParaInteger(dadosEnviados[15]);
    intensidade += ConverteCharacterParaInteger(dadosEnviados[16]);
    return intensidade;
}

void Android::setIntensidadeAtual (int intensidade){
    int chave1, chave2, chave3, chave4;
    chave4 = intensidade%10;
    chave3 = intensidade%100 - chave4;
    chave2 = intensidade%1000 - chave3;
    chave1 = intensidade/1000;
    dadosRecebidos [13] = ConverteIntegerParaChar(chave1);
    dadosRecebidos[14] = ConverteIntegerParaChar(chave2);
    dadosRecebidos [15] = ConverteIntegerParaChar(chave3);
    dadosRecebidos [16] = ConverteIntegerParaChar(chave4);
}


//Periodo do ano
fotoPeriod Android::getFotoperiodo(){
    fotoPeriod value;
    //HORAS
    value.hours = 10*ConverteCharacterParaInteger(dadosEnviados[19]);
    horas += ConverteCharacterParaInteger(dadosEnviados[20]);
    //MINUTOS
    value.min = 10*ConverteCharacterParaInteger(dadosEnviados[22]);
    minutos += ConverteCharacterParaInteger(dadosEnviados[23]);
    return value;
}

void Android::setFotoperiodoSegundos (int segundos){
    int minutos = segundos/60;
    int horas = minutos/60;
    minutos -= horas*60;
    dadosRecebidos [19] = ConverteIntegerParaChar(horas/10);
    dadosRecebidos[20] = ConverteIntegerParaChar(horas%10);
    dadosRecebidos[22] = ConverteIntegerParaChar(minutos/10);
    dadosRecebidos[23] = ConverteIntegerParaChar(minutos%10);
}

bool Android::deveEstarEmOperacao (){
    if (dadosEnviados[36] == 'F')
        return false;
    else 
        return true;
}

int* Android::getPeriodoDeOperacao (){
    int minutos = ConverteCharacterParaInteger(dadosEnviados[34]);
    minutos += 10*ConverteCharacterParaInteger(dadosEnviados[33]);
    int horas = ConverteCharacterParaInteger(dadosEnviados[31]);
    horas += 10*ConverteCharacterParaInteger(dadosEnviados[30]);
    int dias = ConverteCharacterParaInteger(dadosEnviados[28]);
    dias += 10*ConverteCharacterParaInteger(dadosEnviados [27]);
    dias += 100*ConverteCharacterParaInteger(dadosEnviados[26]);
    int vector [3] = {dias, horas, minutos};
    return vector;
}

void Android::setPeriodoEmOperacao (int* vector){
    dadosRecebidos [26] = ConverteIntegerParaChar (vector[0]/100);
    dadosRecebidos [27] = ConverteIntegerParaChar ((vector[0]%100)/10);
    dadosRecebidos [28] = ConverteIntegerParaChar (vector[0]%10);
    dadosRecebidos [30] = ConverteIntegerParaChar (vector[1]/10);
    dadosRecebidos [31] = ConverteIntegerParaChar (vector[1]%10);
    dadosRecebidos [33] = ConverteIntegerParaChar (vector[2]/10);
    dadosRecebidos [34] = ConverteIntegerParaChar (vector[2]%10);
}

void Android::sendDados (){
    string data = this->dadosRecebidos;
    data += "\r\n";
    bluetooth.printf(data.c_str());
}

bool Android::dadosAReceber (){
    if (this->readable()){
        string rxStr = "";

        //Criando um Buffer
        while (true){
            rxStr += (char) bluetooth.getc();
            if (!(this->readable()))
                break;
        }

        this->dadosEnviados = rxStr;
        return true;
    }
    else{
        return false;
    }
}
char Android::ConverteIntegerParaChar (int numero){
    return (char) (numero + 48);
}

int Android::ConverteCharacterParaInteger (char caracter){
    return (int) (caracter - 48);
}

bool Android::readable (){
    return bluetooth.avaliable();
}

int Android::_putc (int valor){
    bluetooth.putc(valor);
    return valor;
}

int Android::_getc (){
    return bluetooth.getc();
}