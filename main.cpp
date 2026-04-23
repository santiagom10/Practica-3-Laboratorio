#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
using namespace std;

// ============================================================
//  MODULO RLE - Seccion 5.1
// ============================================================

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

// ============================================================
//  MODULO LZ78 - Seccion 5.2
// ============================================================

struct EntradaDiccionario {
    int  indicePrefijo;
    char caracter;
};

struct ParSalida {
    int  indice;
    char caracter;
    bool esUltimo;
};

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

ParSalida* comprimirLZ78(const char* texto, int tamanoTexto, int& cantidadPares) {
    if (tamanoTexto <= 0) {
        throw invalid_argument("Error: El texto de entrada esta vacio.");
    }

    int capacidadDiccionario        = 10;
    int tamanoDiccionario           = 0;
    EntradaDiccionario* diccionario = new EntradaDiccionario[capacidadDiccionario];

    int capacidadPares = 10;
    cantidadPares      = 0;
    ParSalida* pares   = new ParSalida[capacidadPares];

    int posicion = 0;

    while (posicion < tamanoTexto) {
        int mejorIndice   = 0;
        int mejorLongitud = 0;

        for (int i = 0; i < tamanoDiccionario; i++) {
            int longitud = longitudFrase(diccionario, i);

            if (posicion + longitud <= tamanoTexto &&
                longitud > mejorLongitud &&
                coincideFrase(diccionario, i, texto, posicion, longitud)) {
                mejorIndice   = i + 1;
                mejorLongitud = longitud;
            }
        }

        bool hayCaracterExtra = (posicion + mejorLongitud) < tamanoTexto;

        if (cantidadPares >= capacidadPares) {
            capacidadPares *= 2;
            ParSalida* paresNuevo = new ParSalida[capacidadPares];
            for (int i = 0; i < cantidadPares; i++) paresNuevo[i] = pares[i];
            delete[] pares;
            pares = paresNuevo;
        }

        if (hayCaracterExtra) {
            char caracterNuevo = texto[posicion + mejorLongitud];

            pares[cantidadPares].indice   = mejorIndice;
            pares[cantidadPares].caracter = caracterNuevo;
            pares[cantidadPares].esUltimo = false;
            cantidadPares++;

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

char* descomprimirLZ78(ParSalida* pares, int cantidadPares, int& tamanoSalida) {
    if (cantidadPares <= 0) {
        throw invalid_argument("Error: No hay pares para descomprimir.");
    }

    int capacidadDiccionario        = 10;
    int tamanoDiccionario           = 0;
    EntradaDiccionario* diccionario = new EntradaDiccionario[capacidadDiccionario];

    int capacidadSalida = 10;
    tamanoSalida        = 0;
    char* salida        = new char[capacidadSalida];

    for (int p = 0; p < cantidadPares; p++) {
        int  indice   = pares[p].indice;
        char caracter = pares[p].caracter;
        bool esUltimo = pares[p].esUltimo;

        int longitudPrefijo = 0;
        if (indice > 0) {
            longitudPrefijo = longitudFrase(diccionario, indice - 1);
        }

        int longitudTotal = esUltimo ? longitudPrefijo : longitudPrefijo + 1;

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

// ============================================================
//  MODULO ENCRIPTACION POR ROTACION
// ============================================================

// Rota los bits de un byte hacia la izquierda n posiciones
unsigned char rotarIzquierda(unsigned char byte, int n) {
    return (byte << n) | (byte >> (8 - n));
}

// Rota los bits de un byte hacia la derecha n posiciones (inverso)
unsigned char rotarDerecha(unsigned char byte, int n) {
    return (byte >> n) | (byte << (8 - n));
}

// Encripta aplicando solo rotacion de bits a la izquierda a cada byte
unsigned char* encriptarRotacion(const unsigned char* datos, int tamano, int n) {
    if (tamano <= 0) {
        throw invalid_argument("Error: No hay datos para encriptar.");
    }
    if (n <= 0 || n >= 8) {
        throw invalid_argument("Error: El valor de n debe estar entre 1 y 7.");
    }

    unsigned char* resultado = new unsigned char[tamano];
    for (int i = 0; i < tamano; i++) {
        resultado[i] = rotarIzquierda(datos[i], n);
    }
    return resultado;
}

// Desencripta invirtiendo la rotacion (rotacion a la derecha)
unsigned char* desencriptarRotacion(const unsigned char* datos, int tamano, int n) {
    if (tamano <= 0) {
        throw invalid_argument("Error: No hay datos para desencriptar.");
    }
    if (n <= 0 || n >= 8) {
        throw invalid_argument("Error: El valor de n debe estar entre 1 y 7.");
    }

    unsigned char* resultado = new unsigned char[tamano];
    for (int i = 0; i < tamano; i++) {
        resultado[i] = rotarDerecha(datos[i], n);
    }
    return resultado;
}

// ============================================================
//  MODULO ENCRIPTACION POR XOR - Seccion 5.3b
// ============================================================

// Encripta aplicando solo XOR con la clave K a cada byte
// La misma funcion sirve para desencriptar (A XOR K XOR K = A)
unsigned char* encriptarXOR(const unsigned char* datos, int tamano, unsigned char clave) {
    if (tamano <= 0) {
        throw invalid_argument("Error: No hay datos para encriptar.");
    }

    unsigned char* resultado = new unsigned char[tamano];
    for (int i = 0; i < tamano; i++) {
        resultado[i] = datos[i] ^ clave;
    }
    return resultado;
}

// ============================================================
//  MODULO ARCHIVOS
// ============================================================

string leerArchivo(const string& nombreArchivo) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        throw runtime_error("Error: No se pudo abrir el archivo '" + nombreArchivo + "'.");
    }

    string contenido = "";
    string linea;
    bool primeraLinea = true;

    while (getline(archivo, linea)) {
        if (!primeraLinea) contenido += "\n";
        contenido += linea;
        primeraLinea = false;
    }

    archivo.close();
    return contenido;
}

void escribirArchivo(const string& nombreArchivo, const string& contenido) {
    ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        throw runtime_error("Error: No se pudo crear el archivo '" + nombreArchivo + "'.");
    }
    archivo << contenido;
    archivo.close();
}

// ============================================================
//  MAIN
// ============================================================

int main() {

    int opcion;
    cout << "=========================================" << endl;
    cout << "               PRACTICA 3                " << endl;
    cout << "=========================================" << endl;
    cout << "Seleccione el modulo a probar:"           << endl;
    cout << "1. RLE"                                   << endl;
    cout << "2. LZ78"                                  << endl;
    cout << "3. Encriptacion por rotacion de bits"     << endl;
    cout << "4. Encriptacion por XOR"                  << endl;
    cout << "5. Integracion"                           << endl;
    cout << "Opcion: ";
    cin >> opcion;
    cin.ignore(10000, '\n');
    cout << endl;

    // ── MODULO RLE ───────────────────────────────────────────
    if (opcion == 1) {
        cout << "=========================================" << endl;
        cout << "                MODULO RLE               " << endl;
        cout << "=========================================" << endl;

        string cadenaOriginal;
        cout << "Ingrese una cadena de texto: ";
        getline(cin, cadenaOriginal);

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
        getline(cin, entrada);

        int tamanoTexto = entrada.size();
        char* texto = new char[tamanoTexto];
        for (int i = 0; i < tamanoTexto; i++) texto[i] = entrada[i];

        try {
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

            int tamanoSalida      = 0;
            char* textoRecuperado = descomprimirLZ78(pares, cantidadPares, tamanoSalida);

            cout << "Texto original    : ";
            for (int i = 0; i < tamanoTexto; i++) cout << texto[i];
            cout << endl;

            cout << "Texto recuperado  : ";
            for (int i = 0; i < tamanoSalida; i++) cout << textoRecuperado[i];
            cout << endl;

            bool coincide = (tamanoTexto == tamanoSalida);
            if (coincide) {
                for (int i = 0; i < tamanoTexto; i++) {
                    if (texto[i] != textoRecuperado[i]) { coincide = false; break; }
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

        // ── ENCRIPTACION POR ROTACION ─────────────────────────────
    } else if (opcion == 3) {
        cout << "=========================================" << endl;
        cout << "      ENCRIPTACION POR ROTACION          " << endl;
        cout << "=========================================" << endl;

        string entrada;
        cout << "Ingrese una cadena de texto: ";
        getline(cin, entrada);

        int n;
        cout << "Ingrese el valor de rotacion n (1-7): ";

        if (!(cin >> n) || n < 1 || n > 7) {
            cin.clear();
            cin.ignore(10000, '\n');
            throw invalid_argument("Error: n debe estar entre 1 y 7.");
        }

        int tamano           = entrada.size();
        unsigned char* datos = new unsigned char[tamano];
        for (int i = 0; i < tamano; i++) datos[i] = (unsigned char) entrada[i];

        try {
            // 1. Encriptar con rotacion
            unsigned char* datosEncriptados = encriptarRotacion(datos, tamano, n);

            cout << "Bytes encriptados (decimal): ";
            for (int i = 0; i < tamano; i++) {
                cout << (int) datosEncriptados[i];
                if (i < tamano - 1) cout << " ";
            }
            cout << endl;

            // 2. Desencriptar con rotacion inversa
            unsigned char* datosRecuperados = desencriptarRotacion(datosEncriptados, tamano, n);

            cout << "Texto recuperado  : ";
            for (int i = 0; i < tamano; i++) cout << (char) datosRecuperados[i];
            cout << endl;

            // 3. Verificar
            bool coincide = true;
            for (int i = 0; i < tamano; i++) {
                if (datos[i] != datosRecuperados[i]) { coincide = false; break; }
            }

            if (coincide) {
                cout << "Verificacion: OK [PASS] - el texto recuperado coincide con el original." << endl;
            } else {
                cout << "Verificacion: [FAIL] - NO coincide." << endl;
            }

            delete[] datosEncriptados;
            delete[] datosRecuperados;

        } catch (const invalid_argument& error) {
            cout << "Error de argumento: " << error.what() << endl;
        }

        delete[] datos;

        // ── ENCRIPTACION POR XOR ──────────────────────────────────
    } else if (opcion == 4) {
        cout << "=========================================" << endl;
        cout << "          ENCRIPTACION POR XOR           " << endl;
        cout << "=========================================" << endl;

        string entrada;
        cout << "Ingrese una cadena de texto: ";
        getline(cin, entrada);

        int valorClave;
        cout << "Ingrese la clave K (0-255): ";

        if (!(cin >> valorClave) || valorClave < 0 || valorClave > 255) {
            cin.clear();
            cin.ignore(10000, '\n');
            throw invalid_argument("Error: La clave K debe ser un numero entero entre 0 y 255.");
        }

        unsigned char clave = static_cast<unsigned char>(valorClave);

        int tamano           = entrada.size();
        unsigned char* datos = new unsigned char[tamano];
        for (int i = 0; i < tamano; i++) datos[i] = (unsigned char) entrada[i];

        try {
            // 1. Encriptar con XOR
            unsigned char* datosEncriptados = encriptarXOR(datos, tamano, clave);

            cout << "Bytes encriptados (decimal): ";
            for (int i = 0; i < tamano; i++) {
                cout << (int) datosEncriptados[i];
                if (i < tamano - 1) cout << " ";
            }
            cout << endl;

            // 2. Desencriptar aplicando XOR de nuevo con la misma clave
            unsigned char* datosRecuperados = encriptarXOR(datosEncriptados, tamano, clave);

            cout << "Texto recuperado  : ";
            for (int i = 0; i < tamano; i++) cout << (char) datosRecuperados[i];
            cout << endl;

            // 3. Verificar
            bool coincide = true;
            for (int i = 0; i < tamano; i++) {
                if (datos[i] != datosRecuperados[i]) { coincide = false; break; }
            }

            if (coincide) {
                cout << "Verificacion: OK [PASS] - el texto recuperado coincide con el original." << endl;
            } else {
                cout << "Verificacion: [FAIL] - NO coincide." << endl;
            }

            delete[] datosEncriptados;
            delete[] datosRecuperados;

        } catch (const invalid_argument& error) {
            cout << "Error de argumento: " << error.what() << endl;
        }

        delete[] datos;

        // ── INTEGRACION ───────────────────────────────────────────
    } else if (opcion == 5) {
        cout << "=========================================" << endl;
        cout << "             INTEGRACION                 " << endl;
        cout << "=========================================" << endl;

        int metodo;
        cout << "Seleccione el metodo de compresion:" << endl;
        cout << "1. RLE"  << endl;
        cout << "2. LZ78" << endl;
        cout << "Opcion: ";
        cin >> metodo;

        string nombreEntrada;
        cout << "Nombre del archivo de entrada: ";
        cin.ignore();
        getline(cin, nombreEntrada);

        int n;
        cout << "Ingrese el valor de rotacion n (1-7): ";
        cin >> n;

        int valorClave;
        cout << "Ingrese la clave K (0-255): ";
        cin >> valorClave;
        unsigned char clave = (unsigned char) valorClave;

        try {
            cout << "Intentando abrir: [" << nombreEntrada << "]" << endl;
            string textoOriginal = leerArchivo(nombreEntrada);
            cout << "\n[1] Texto leido (" << textoOriginal.size() << " caracteres)." << endl;

            // PASO 2: Comprimir
            int tamanoComprimido          = 0;
            unsigned char* bytesComprimidos = nullptr;
            ParSalida* pares              = nullptr;
            int cantidadPares             = 0;
            char* textoLZ78               = nullptr;

            if (metodo == 1) {
                string comprimidoRLE = comprimirRLE(textoOriginal);
                tamanoComprimido     = comprimidoRLE.size();
                bytesComprimidos     = new unsigned char[tamanoComprimido];
                for (int i = 0; i < tamanoComprimido; i++)
                    bytesComprimidos[i] = (unsigned char) comprimidoRLE[i];
                cout << "[2] Comprimido con RLE (" << tamanoComprimido << " caracteres)." << endl;

            } else if (metodo == 2) {
                int tamanoTexto = textoOriginal.size();
                textoLZ78       = new char[tamanoTexto];
                for (int i = 0; i < tamanoTexto; i++) textoLZ78[i] = textoOriginal[i];
                pares            = comprimirLZ78(textoLZ78, tamanoTexto, cantidadPares);
                tamanoComprimido = cantidadPares * 6;
                bytesComprimidos = new unsigned char[tamanoComprimido];
                for (int i = 0; i < cantidadPares; i++) {
                    int base = i * 6;
                    bytesComprimidos[base]     = (pares[i].indice >> 24) & 0xFF;
                    bytesComprimidos[base + 1] = (pares[i].indice >> 16) & 0xFF;
                    bytesComprimidos[base + 2] = (pares[i].indice >> 8)  & 0xFF;
                    bytesComprimidos[base + 3] = (pares[i].indice)       & 0xFF;
                    bytesComprimidos[base + 4] = (unsigned char) pares[i].caracter;
                    bytesComprimidos[base + 5] = (unsigned char) pares[i].esUltimo;
                }
                cout << "[2] Comprimido con LZ78 (" << cantidadPares << " pares)." << endl;

            } else {
                throw invalid_argument("Error: Metodo de compresion no valido.");
            }

            // PASO 3: Encriptar (rotacion + XOR)
            unsigned char* bytesRotados     = encriptarRotacion(bytesComprimidos, tamanoComprimido, n);
            unsigned char* bytesEncriptados = encriptarXOR(bytesRotados, tamanoComprimido, clave);

            cout << "[3.1] Bytes despues de rotacion: ";
            for (int i = 0; i < tamanoComprimido; i++) {
                cout << (int)bytesRotados[i] << " ";
            }
            cout << endl;

            cout << "[3.2] Datos encriptados (decimal): ";
            for (int i = 0; i < tamanoComprimido; i++) {
                cout << (int)bytesEncriptados[i] << " ";
            }
            cout << endl;

            // 🔼 FIN

            cout << "[3] Datos encriptados (rotacion + XOR)." << endl;

            // PASO 4: Desencriptar (XOR + rotacion inversa)
            unsigned char* bytesSinXor        = encriptarXOR(bytesEncriptados, tamanoComprimido, clave);
            unsigned char* bytesDesencriptados = desencriptarRotacion(bytesSinXor, tamanoComprimido, n);
            cout << "[4] Datos desencriptados." << endl;

            // PASO 5: Descomprimir
            string textoFinal = "";

            if (metodo == 1) {
                string comprimidoRLE = "";
                for (int i = 0; i < tamanoComprimido; i++)
                    comprimidoRLE += (char) bytesDesencriptados[i];
                textoFinal = descomprimirRLE(comprimidoRLE);

            } else if (metodo == 2) {
                ParSalida* paresRecuperados = new ParSalida[cantidadPares];
                for (int i = 0; i < cantidadPares; i++) {
                    int base = i * 6;
                    paresRecuperados[i].indice =
                        (bytesDesencriptados[base]     << 24) |
                        (bytesDesencriptados[base + 1] << 16) |
                        (bytesDesencriptados[base + 2] << 8)  |
                        (bytesDesencriptados[base + 3]);
                    paresRecuperados[i].caracter = (char) bytesDesencriptados[base + 4];
                    paresRecuperados[i].esUltimo = (bool) bytesDesencriptados[base + 5];
                }
                int tamanoSalida      = 0;
                char* textoRecuperado = descomprimirLZ78(paresRecuperados, cantidadPares, tamanoSalida);
                for (int i = 0; i < tamanoSalida; i++) textoFinal += textoRecuperado[i];
                delete[] paresRecuperados;
                delete[] textoRecuperado;
            }

            cout << "[5] Texto descomprimido." << endl;

            // PASO 6: Escribir archivo de salida
            escribirArchivo("C:\\Qt\\Tools\\QtCreator\\bin\\salida.txt", textoFinal);
            cout << "[6] Resultado guardado en 'salida.txt'." << endl;

            // PASO 7: Verificar
            if (textoFinal == textoOriginal) {
                cout << "\nVerificacion: OK [PASS] - el texto final coincide con el original." << endl;
            } else {
                cout << "\nVerificacion: [FAIL] - el texto final NO coincide." << endl;
            }

            delete[] bytesComprimidos;
            delete[] bytesRotados;
            delete[] bytesEncriptados;
            delete[] bytesSinXor;
            delete[] bytesDesencriptados;
            if (pares)     delete[] pares;
            if (textoLZ78) delete[] textoLZ78;

        } catch (const invalid_argument& error) {
            cout << "Error de argumento: " << error.what() << endl;
        } catch (const runtime_error& error) {
            cout << "Error en ejecucion: " << error.what() << endl;
        }

    } else {
        cout << "Opcion no valida." << endl;
    }

    return 0;
}
