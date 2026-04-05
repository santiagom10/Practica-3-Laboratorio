#include <iostream>
#include <string>
#include <stdexcept>
using namespace std;

// Comprime una cadena usando Run-Length Encoding (RLE)
// Ejemplo: "AAAABBBCC" -> "4A3B2C"
string comprimirRLE(const string& cadena) {
    if (cadena.empty()) {
        throw invalid_argument("Error: La cadena de entrada esta vacia.");
    }

    string resultado = "";
    int tamano = cadena.size();
    int i = 0;

    while (i < tamano) {
        char caracterActual = cadena[i];
        int contador = 1;

        while (i + contador < tamano && cadena[i + contador] == caracterActual) {
            contador++;
        }

        if (contador > 1) {
            resultado += to_string(contador);
        }
        resultado += caracterActual;

        i += contador;
    }

    return resultado;
}

// Descomprime una cadena codificada en RLE
// Ejemplo: "4A3B2C" -> "AAAABBBCC"
string descomprimirRLE(const string& cadenaComprimida) {
    if (cadenaComprimida.empty()) {
        throw invalid_argument("Error: La cadena comprimida esta vacia.");
    }

    string resultado = "";
    int tamano = cadenaComprimida.size();
    int i = 0;

    while (i < tamano) {
        string numeroTexto = "";
        while (i < tamano && isdigit(cadenaComprimida[i])) {
            numeroTexto += cadenaComprimida[i];
            i++;
        }

        if (i >= tamano) {
            throw runtime_error("Error: Formato RLE invalido. Numero sin caracter al final.");
        }

        char caracterActual = cadenaComprimida[i];
        i++;

        int repeticiones = numeroTexto.empty() ? 1 : stoi(numeroTexto);

        for (int j = 0; j < repeticiones; j++) {
            resultado += caracterActual;
        }
    }

    return resultado;
}

// Cada entrada del diccionario es un par (indicePrefijo, caracter)
// indicePrefijo = 0 significa que no tiene prefijo (entrada raiz)
struct EntradaDiccionario {
    int  indicePrefijo;
    char caracter;
};

// Cada par de salida de la compresion es (indice, caracter)
// Si esUltimo es true, el caracter es \0 (fin de texto sin caracter extra)
struct ParSalida {
    int  indice;
    char caracter;
    bool esUltimo; // true cuando el texto termina exactamente en una frase del diccionario
};

// Calcula la longitud de la frase almacenada en la posicion 'indice'
int longitudFrase(EntradaDiccionario* diccionario, int indice) {
    int longitud = 0;
    int cursor   = indice;
    while (cursor >= 0) {
        longitud++;
        int prefijo = diccionario[cursor].indicePrefijo;
        cursor = (prefijo == 0) ? -1 : prefijo - 1;
    }
    return longitud;
}

// Verifica si la frase del diccionario en 'indice' coincide con el texto
// a partir de 'posicion' con longitud 'longitud'
bool coincideFrase(EntradaDiccionario* diccionario, int indice,
                   const char* texto, int posicion, int longitud) {
    char* frase = new char[longitud];
    int cursor  = indice;
    for (int i = longitud - 1; i >= 0; i--) {
        frase[i] = diccionario[cursor].caracter;
        int prefijo = diccionario[cursor].indicePrefijo;
        cursor = (prefijo == 0) ? -1 : prefijo - 1;
    }

    bool igual = true;
    for (int i = 0; i < longitud; i++) {
        if (frase[i] != texto[posicion + i]) {
            igual = false;
            break;
        }
    }

    delete[] frase;
    return igual;
}

// Comprime un texto usando LZ78
// Retorna arreglo de pares y guarda la cantidad en 'cantidadPares'
ParSalida* comprimirLZ78(const char* texto, int tamanoTexto, int& cantidadPares) {
    if (tamanoTexto <= 0) {
        throw invalid_argument("Error: El texto de entrada esta vacio.");
    }

    int capacidadDiccionario    = 10;
    int tamanoDiccionario       = 0;
    EntradaDiccionario* diccionario = new EntradaDiccionario[capacidadDiccionario];

    int capacidadPares = 10;
    cantidadPares      = 0;
    ParSalida* pares   = new ParSalida[capacidadPares];

    int posicion = 0;

    while (posicion < tamanoTexto) {
        int mejorIndice   = 0;
        int mejorLongitud = 0;

        // Buscar la frase mas larga del diccionario que coincida
        for (int i = 0; i < tamanoDiccionario; i++) {
            int longitud = longitudFrase(diccionario, i);

            if (posicion + longitud <= tamanoTexto &&
                longitud > mejorLongitud &&
                coincideFrase(diccionario, i, texto, posicion, longitud)) {
                mejorIndice   = i + 1;
                mejorLongitud = longitud;
            }
        }

        // Verificar si quedaron caracteres despues de la mejor frase
        bool hayCaracterExtra = (posicion + mejorLongitud) < tamanoTexto;

        // Agrandar el arreglo de pares si es necesario
        if (cantidadPares >= capacidadPares) {
            capacidadPares *= 2;
            ParSalida* paresNuevo = new ParSalida[capacidadPares];
            for (int i = 0; i < cantidadPares; i++) paresNuevo[i] = pares[i];
            delete[] pares;
            pares = paresNuevo;
        }

        if (hayCaracterExtra) {
            // Caso normal: hay un caracter que extiende la frase
            char caracterNuevo = texto[posicion + mejorLongitud];

            pares[cantidadPares].indice   = mejorIndice;
            pares[cantidadPares].caracter = caracterNuevo;
            pares[cantidadPares].esUltimo = false;
            cantidadPares++;

            // Agregar nueva entrada al diccionario
            if (tamanoDiccionario >= capacidadDiccionario) {
                capacidadDiccionario *= 2;
                EntradaDiccionario* diccionarioNuevo = new EntradaDiccionario[capacidadDiccionario];
                for (int i = 0; i < tamanoDiccionario; i++) diccionarioNuevo[i] = diccionario[i];
                delete[] diccionario;
                diccionario = diccionarioNuevo;
            }
            diccionario[tamanoDiccionario].indicePrefijo = mejorIndice;
            diccionario[tamanoDiccionario].caracter      = caracterNuevo;
            tamanoDiccionario++;

            posicion += mejorLongitud + 1;

        } else {
            // Caso especial: el texto termina exactamente en una frase del diccionario
            // Se emite el par (mejorIndice, \0) marcado como ultimo
            pares[cantidadPares].indice   = mejorIndice;
            pares[cantidadPares].caracter = '\0';
            pares[cantidadPares].esUltimo = true;
            cantidadPares++;

            posicion += mejorLongitud;
        }
    }

    delete[] diccionario;
    return pares;
}

// Descomprime un arreglo de pares LZ78 y retorna el texto original
char* descomprimirLZ78(ParSalida* pares, int cantidadPares, int& tamanoSalida) {
    if (cantidadPares <= 0) {
        throw invalid_argument("Error: No hay pares para descomprimir.");
    }

    int capacidadDiccionario    = 10;
    int tamanoDiccionario       = 0;
    EntradaDiccionario* diccionario = new EntradaDiccionario[capacidadDiccionario];

    int capacidadSalida = 10;
    tamanoSalida        = 0;
    char* salida        = new char[capacidadSalida];

    for (int p = 0; p < cantidadPares; p++) {
        int  indice   = pares[p].indice;
        char caracter = pares[p].caracter;
        bool esUltimo = pares[p].esUltimo;

        // Calcular longitud de la frase completa
        int longitudPrefijo = 0;
        if (indice > 0) {
            longitudPrefijo = longitudFrase(diccionario, indice - 1);
        }

        // Si es el ultimo par especial, no se agrega el caracter extra
        int longitudTotal = esUltimo ? longitudPrefijo : longitudPrefijo + 1;

        // Reconstruir la frase en un arreglo temporal
        char* fraseReconstruida = new char[longitudTotal];

        if (indice > 0) {
            int cursor = indice - 1;
            for (int i = longitudPrefijo - 1; i >= 0; i--) {
                fraseReconstruida[i] = diccionario[cursor].caracter;
                int prefijo = diccionario[cursor].indicePrefijo;
                cursor = (prefijo == 0) ? -1 : prefijo - 1;
            }
        }

        if (!esUltimo) {
            fraseReconstruida[longitudTotal - 1] = caracter;
        }

        // Agregar la frase a la salida
        for (int i = 0; i < longitudTotal; i++) {
            if (tamanoSalida >= capacidadSalida) {
                capacidadSalida *= 2;
                char* salidaNueva = new char[capacidadSalida];
                for (int j = 0; j < tamanoSalida; j++) salidaNueva[j] = salida[j];
                delete[] salida;
                salida = salidaNueva;
            }
            salida[tamanoSalida++] = fraseReconstruida[i];
        }

        // Agregar nueva entrada al diccionario (solo si no es el ultimo especial)
        if (!esUltimo) {
            if (tamanoDiccionario >= capacidadDiccionario) {
                capacidadDiccionario *= 2;
                EntradaDiccionario* diccionarioNuevo = new EntradaDiccionario[capacidadDiccionario];
                for (int i = 0; i < tamanoDiccionario; i++) diccionarioNuevo[i] = diccionario[i];
                delete[] diccionario;
                diccionario = diccionarioNuevo;
            }
            diccionario[tamanoDiccionario].indicePrefijo = indice;
            diccionario[tamanoDiccionario].caracter      = caracter;
            tamanoDiccionario++;
        }

        delete[] fraseReconstruida;
    }

    delete[] diccionario;
    return salida;
}

int main() {

    int opcion;
    cout << "=========================================" << endl;
    cout << "               PRACTICA 3                " << endl;
    cout << "=========================================" << endl;
    cout << "Seleccione el modulo a probar:" << endl;
    cout << "1. RLE" << endl;
    cout << "2. LZ78" << endl;
    cout << "Opcion: ";
    cin >> opcion;
    cout << endl;

    // ── MODULO RLE ───────────────────────────────────────────
    if (opcion == 1) {
        cout << "=========================================" << endl;
        cout << "                MODULO RLE               " << endl;
        cout << "=========================================" << endl;

        string cadenaOriginal;
        cout << "Ingrese una cadena de texto: ";
        cin >> cadenaOriginal;

        try {
            string cadenaComprimida    = comprimirRLE(cadenaOriginal);
            string cadenaDescomprimida = descomprimirRLE(cadenaComprimida);

            cout << "Cadena comprimida   : " << cadenaComprimida    << endl;
            cout << "Cadena descomprimida: " << cadenaDescomprimida << endl;

            if (cadenaDescomprimida == cadenaOriginal) {
                cout << "Verificacion: OK [PASS] - coincide con la original." << endl;
            } else {
                cout << "Verificacion: [FAIL] - NO coincide." << endl;
            }

        } catch (const invalid_argument& error) {
            cout << "Error de argumento: " << error.what() << endl;
        } catch (const runtime_error& error) {
            cout << "Error en ejecucion: " << error.what() << endl;
        }

        // ── MODULO LZ78 ──────────────────────────────────────────
    } else if (opcion == 2) {
        cout << "=========================================" << endl;
        cout << "               MODULO LZ78               " << endl;
        cout << "=========================================" << endl;

        string entrada;
        cout << "Ingrese una cadena de texto: ";
        cin >> entrada;

        int tamanoTexto = entrada.size();
        char* texto = new char[tamanoTexto];
        for (int i = 0; i < tamanoTexto; i++) {
            texto[i] = entrada[i];
        }

        try {
            // 1. Comprimir
            int cantidadPares = 0;
            ParSalida* pares  = comprimirLZ78(texto, tamanoTexto, cantidadPares);

            cout << "Pares generados (indice, caracter):" << endl;
            for (int i = 0; i < cantidadPares; i++) {
                if (pares[i].esUltimo) {
                    cout << "  (" << pares[i].indice << ", [fin])" << endl;
                } else {
                    cout << "  (" << pares[i].indice << ", " << pares[i].caracter << ")" << endl;
                }
            }

            // 2. Descomprimir
            int tamanoSalida      = 0;
            char* textoRecuperado = descomprimirLZ78(pares, cantidadPares, tamanoSalida);

            cout << "Texto original    : ";
            for (int i = 0; i < tamanoTexto; i++) cout << texto[i];
            cout << endl;

            cout << "Texto recuperado  : ";
            for (int i = 0; i < tamanoSalida; i++) cout << textoRecuperado[i];
            cout << endl;

            // 3. Verificar
            bool coincide = (tamanoTexto == tamanoSalida);
            if (coincide) {
                for (int i = 0; i < tamanoTexto; i++) {
                    if (texto[i] != textoRecuperado[i]) {
                        coincide = false;
                        break;
                    }
                }
            }

            if (coincide) {
                cout << "Verificacion: OK [PASS] - coincide con el original." << endl;
            } else {
                cout << "Verificacion: [FAIL] - NO coincide." << endl;
            }

            delete[] pares;
            delete[] textoRecuperado;

        } catch (const invalid_argument& error) {
            cout << "Error de argumento: " << error.what() << endl;
        } catch (const runtime_error& error) {
            cout << "Error en ejecucion: " << error.what() << endl;
        }

        delete[] texto;

    } else {
        cout << "Opcion no valida." << endl;
    }

    return 0;
}
