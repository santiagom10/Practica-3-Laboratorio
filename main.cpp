#include <iostream>
#include <string>
using namespace std;

string comprimirRLE(const string& texto) {
    string resultado = "";
    int n = texto.length();

    for (int i = 0; i < n; i++) {
        int contador = 1;

        // contar repeticiones
        while (i < n - 1 && texto[i] == texto[i + 1]) {
            contador++;
            i++;
        }

        resultado += to_string(contador) + texto[i];
    }

    return resultado;
}

string descomprimirRLE(const string& texto) {
    string resultado = "";
    int n = texto.length();

    for (int i = 0; i < n; i++) {
        int numero = 0;

        // construir número (por si es de más de un dígito)
        while (isdigit(texto[i])) {
            numero = numero * 10 + (texto[i] - '0');
            i++;
        }

        char caracter = texto[i];

        // repetir el carácter
        for (int j = 0; j < numero; j++) {
            resultado += caracter;
        }
    }

    return resultado;
}

int main() {
    string texto;

    cout << "Ingrese texto: ";
    cin >> texto;

    string comprimido = comprimirRLE(texto);
    cout << "Comprimido: " << comprimido << endl;

    string descomprimido = descomprimirRLE(comprimido);
    cout << "Descomprimido: " << descomprimido << endl;

    // verificación
    if (texto == descomprimido) {
        cout << "Verificacion correcta" << endl;
    } else {
        cout << "Error en descompresion" << endl;
    }

    return 0;
}
