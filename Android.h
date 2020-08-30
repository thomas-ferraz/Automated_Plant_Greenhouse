//Android.h
#ifndef ANDROID_H_INCLUDED
#define ANDROID_H_INCLUDED
#include "Arduino.h"
#include <iostream>
#include <string>


extern SoftwareSerial bluetooth;


class Android {
    public:
        Android ();
        virtual ~Android();
        float getTemperaturaModelo ();
        void setTemperaturaAtual (float temperatura);
        float getUmidadeModelo ();
        void setUmidadeAtual (int umidade);
        bool getLuminosidadeModelo ();
        void setLuminosidadeAtual (bool luminosidade);
        int getIntensidadeModelo ();
        void setIntensidadeAtual (int intensidade);

        //Periodo do ano
        int getFotoperiodoMinutos ();
        void setFotoperiodoSegundos (int segundos);
        bool deveEstarEmOperacao ();
        int* getPeriodoDeOperacao ();
        void setPeriodoEmOperacao (int* segundos);
        void sendDados ();
        bool dadosAReceber ();
        
    protected:
        virtual int _putc (int valor);
        virtual int _getc ();

    private:
        string dadosRecebidos; // Pelo android dos sensores
        string dadosEnviados; // Do android para os atuadores
        char ConverteIntegerParaChar (int numero);
        int ConverteCharacterParaInteger (char caracter);
        bool readable ();
};
#endif // ANDROID_H_INCLUDED