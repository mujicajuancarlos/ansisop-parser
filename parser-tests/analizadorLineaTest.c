#include <parser/parser.h>
#include <cspecs/cspec.h>
#include "utils/parserUtils.h"

AnSISOP_funciones *funciones = NULL;
AnSISOP_kernel *kernel = NULL;
bool imprimirEnPantalla = true;

context (parser) {

    void setup(){
        funciones = malloc(sizeof(AnSISOP_funciones));
        kernel = malloc(sizeof(AnSISOP_kernel));
        parserUtilSetup(tmpnam(NULL), imprimirEnPantalla);

        funciones->AnSISOP_definirVariable = definirVariable;
        funciones->AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable;
        funciones->AnSISOP_dereferenciar = dereferenciar;
        funciones->AnSISOP_asignar = asignar;
        funciones->AnSISOP_irAlLabel = irAlLabel;

        kernel->AnSISOP_reservar = alocar;
        kernel->AnSISOP_liberar = liberar;

        kernel->AnSISOP_abrir = abrir;
        kernel->AnSISOP_borrar = borrar;
        kernel->AnSISOP_escribir = escribir;
    };

    setup();

    before {
        limpiarElContextoDeEjecucion();
    } end

    describe("Si al parser") {

        it("definicion de variables") {
            analizadorLinea("variables x, a, g", funciones, kernel);
                assertDefinirVariable('x');
                assertDefinirVariable('a');
                assertDefinirVariable('g');
        } end

         it("asignacion de variables") {
            analizadorLinea("x = a+3", funciones, kernel);
                t_puntero posicionA = assertObtenerPosicion('a');
                t_valor_variable valorA = assertDereferenciar(posicionA);
                t_puntero posicionX = assertObtenerPosicion('x');
                assertAsignar(posicionX, valorA+3);
        } end

        it("ir a etiqueta") {
            analizadorLinea("goto inicio", funciones, kernel);
            assertIrAlLabel("inicio");
        } end

        it("alocar") {
            analizadorLinea("alocar x 6666 ", funciones, kernel);
                t_puntero punteroAlocar = assertMalloc(6666);
                t_puntero posicionX = assertObtenerPosicion('x');
                assertAsignar(posicionX, punteroAlocar);
        } end

        it("liberar") {
            analizadorLinea("liberar x", funciones, kernel);
                t_puntero posicionX = assertObtenerPosicion('x');
                assertLiberar(posicionX);
        } end

        it("imprimir un valor") {
            analizadorLinea("prints n x+53-&b", funciones, kernel);
                t_valor_variable valorX = assertDereferenciar(assertObtenerPosicion('x'));
                t_puntero posicionB = assertObtenerPosicion('b');
                char *valorAImprimir = string_itoa(valorX + 53 - posicionB);
                assertEscribir(DESCRIPTOR_SALIDA, valorAImprimir, string_length(valorAImprimir)+1);
        } end

        it("imprimir un literal") {
            analizadorLinea("prints l Holitas", funciones, kernel);
            assertEscribir(DESCRIPTOR_SALIDA, "Holitas", 8);
        } end

        it("imprimir un string en memoria") {
            //setup un dereferenciar de mentira
            funciones->AnSISOP_obtenerPosicionVariable = ({
                t_puntero __obtenerDeMentira(t_nombre_variable _){
                    return 0;
                } __obtenerDeMentira; });
            funciones->AnSISOP_dereferenciar = ({
                t_valor_variable __dereferenciarMentira(t_puntero puntero){
                    char stringEnMemoria[] = { 'H', 'o', 'l', 'a', '\0' };
                    return stringEnMemoria[puntero];
                } __dereferenciarMentira; });

            analizadorLinea("prints s x", funciones, kernel);
            assertEscribir(DESCRIPTOR_SALIDA, "Hola", 5);

            //Rollback
            funciones->AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable;
            funciones->AnSISOP_dereferenciar = dereferenciar;
        } end

         it("abrir un archivo") {
            analizadorLinea("abrir LC /utn/so/archivo", funciones, kernel);
            assertAbrir("/utn/so/archivo", (t_banderas){.lectura = true, .escritura = false, .creacion = true}); 
         } end

         it("abrir un archivo, los flags importan") {
            analizadorLinea("abrir LE /utn/so/archivo", funciones, kernel);
            assertAbrir("/utn/so/archivo", (t_banderas){.lectura = true, .escritura = true, .creacion = false});
         } end
            
         it("borrar un archivo") {
            analizadorLinea("borrar t", funciones, kernel);
            t_puntero posicionT = assertObtenerPosicion('t');
            t_valor_variable descriptor = assertDereferenciar(posicionT);
            assertBorrar(descriptor);
         } end

         it("borrar un descriptor cualquiera") {
             analizadorLinea("borrar 5+2", funciones, kernel);
             assertBorrar(5+2);
         } end
    } end

    parserUtilTearDown();

}