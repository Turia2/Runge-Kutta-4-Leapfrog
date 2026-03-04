#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <optional>
#include <cmath>
#include <fstream>
#include <vector>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <string>
#include <iostream>

using namespace std; 
using namespace sf; 

//Usaremos unidades astronómicas, años y masas solares.
const double G = 4*M_PI*M_PI; 


struct Estado{
    double x; 
    double y; 
    double vx;
    double vy;
};
/*hay que enseñarle al código a multiplicar y sumar estados*/
Estado operator+(const Estado& a, const Estado& b){
    return{
        a.x + b.x,
        a.y + b.y,
        a.vx + b.vx,
        a.vy + b.vy
    };
}
Estado operator*(double escalar, const Estado& s){
    return{
        escalar*s.x,
        escalar*s.y,
        escalar*s.vx,
        escalar*s.vy
    };
}

//PANTALLA
Vector2f pixelespantalla(double x, double y){
    //convertir puntos (x,y) del espacio a píxeles. Es el centro de la pantalla
    /*
    ESCALA 1UA = 200px
    Centro X = 600
    Centro Y = 500*/

    return Vector2f((x*200.0f) + 600.f, -y*200.0f + 500.0f);
}

Estado EspacioReal(int px, int py){
    //hace lo contrario a la funcion pixelespantalla
    double x = (px - 600.0)/200.0;
    double y = -(py - 500.0)/200.0;
    return{x, y, 0.5, 1.0};
}
//cuerpos celestes
class Cuerpo{
        
    public: 

        double masa;
        CircleShape forma; 
        Color colorTrayectoria; 
        Estado actual; 
        vector<Vector2f> trayectoria;
        
        Cuerpo(double m, Estado inicial, Color color): masa(m), actual(inicial){
            
            forma.setRadius(5.0f);
            forma.setOrigin({5.0f, 5.0f});
            forma.setFillColor(color);
            colorTrayectoria = color; 
            colorTrayectoria.a = 100; 
            
        }
        Estado derivadas(const Estado& estado_temp, const vector<Cuerpo>& sistema, int id){
            double ax_total = 0.0;
            double ay_total = 0.0; 
            
            for(int i = 0; i < sistema.size(); i++){
                if(i == id){
                    continue;
                }

                double x_otro = sistema[i].actual.x;
                double y_otro = sistema[i].actual.y;
                double m_otro = sistema[i].masa;

                double dx = x_otro - estado_temp.x;
                double dy = y_otro - estado_temp.y;
                double r2 = dx*dx + dy*dy + 0.05;
                double r = sqrt(r2);
                double r3 = r*r*r;

                ax_total = ax_total + m_otro*G*dx/r3;
                ay_total = ay_total + m_otro*G*dy/r3;
            }

            return{estado_temp.vx, estado_temp.vy, ax_total, ay_total};
        }


        void dibujar(RenderWindow& ventana){
            if(trayectoria.size() > 1){
                VertexArray lineas(PrimitiveType::LineStrip, trayectoria.size());
                for(int i = 0; i < trayectoria.size(); i++){
                    lineas[i].position = trayectoria[i];
                    lineas[i].color = colorTrayectoria;
                }
                ventana.draw(lineas);
            }
            forma.setPosition(pixelespantalla(actual.x, actual.y));
            ventana.draw(forma);
        }
};

double Energia(const vector<Cuerpo>& sistema){
    double K = 0.0;
    double U = 0.0; 

    for(int i = 0; i < sistema.size(); i++){
        double v2 = sistema[i].actual.vx*sistema[i].actual.vx + sistema[i].actual.vy*sistema[i].actual.vy;
        K = K + 0.5*sistema[i].masa*v2;

        for(int j = i + 1; j < sistema.size(); j++){
            double dx = sistema[j].actual.x - sistema[i].actual.x;
            double dy = sistema[j].actual.y - sistema[i].actual.y;

            double r = sqrt(dx*dx + dy*dy + 0.05);

            U = U - (G*sistema[i].masa*sistema[j].masa)/r;
        }
    }

    return K + U;
}

void actualizarRK4(vector<Cuerpo>& sistema, double dt){
    int n = sistema.size();
    if(n == 0){
        return;
    }

    //guardar los cálculos de todos los planetas
    vector<Estado> k1(n), k2(n), k3(n), k4(n);
    vector<Estado> inicial(n);

    //calcular el futuro 
    vector<Cuerpo> temp = sistema;

    //para cierto t, se guardan los datos del presente
    for(int i = 0; i < n; i++){
        inicial[i] = sistema[i].actual;
    }

    //cálculo de k1 para todos
    for(int i = 0; i < n; i++){
        k1[i] = temp[i].derivadas(inicial[i], temp, i);
    }

    //cálculo de k2 usando k1 con t+dt/2
    for(int i = 0; i < n; i++){
        temp[i].actual = inicial[i] + (0.5*dt)*k1[i];
    }
    for(int i = 0; i < n; i++){
        k2[i] = temp[i].derivadas(temp[i].actual, temp, i);
    }

    //cálculo de k3 usando k2 con t + dt/2
    for(int i = 0; i < n; i++){
        temp[i].actual = inicial[i] + (0.5 * dt) * k2[i];
    }
    for(int i = 0; i < n; i++){
        k3[i] = temp[i].derivadas(temp[i].actual, temp, i);
    }

    //cálculo de k4 usando k3
    for(int i = 0; i < n; i++){
        temp[i].actual = inicial[i] + dt * k3[i];
    }
    for(int i = 0; i < n; i++){
        k4[i] = temp[i].derivadas(temp[i].actual, temp, i);
    }

    for(int i = 0; i < n; i++) {
        sistema[i].actual = inicial[i] + (dt / 6.0) * (k1[i] + 2.0*k2[i] + 2.0*k3[i] + k4[i]);
        sistema[i].trayectoria.push_back(pixelespantalla(sistema[i].actual.x, sistema[i].actual.y));
        if(sistema[i].trayectoria.size() > 1000) {
            sistema[i].trayectoria.erase(sistema[i].trayectoria.begin()); 
        }
    }
}

void actualizarLF(vector<Cuerpo>& sistema, double dt){
    //kick
    if(sistema.size() == 0){
        return;
    }

    vector<Estado> derivadas_i(sistema.size());

    for(int i = 0; i < sistema.size(); i++){
        derivadas_i[i] = sistema[i].derivadas(sistema[i].actual, sistema, i);
    }

    for(int i = 0; i < sistema.size(); i++){
        double ax = derivadas_i[i].vx;
        double ay = derivadas_i[i].vy;
        sistema[i].actual.vx = sistema[i].actual.vx + ax*(dt/2.0);
        sistema[i].actual.vy = sistema[i].actual.vy + ay*(dt/2.0);
    }

    //drift
    for(int i = 0; i < sistema.size(); i++){
        sistema[i].actual.x = sistema[i].actual.x + sistema[i].actual.vx*dt;
        sistema[i].actual.y = sistema[i].actual.y + sistema[i].actual.vy*dt;
    }

    //kick 2
    vector<Estado> derivadas_f(sistema.size());
    for(int i = 0; i < sistema.size(); i++){
        derivadas_f[i] = sistema[i].derivadas(sistema[i].actual, sistema, i);
    }

    for(int i = 0; i < sistema.size(); i++){
        double ax = derivadas_f[i].vx;
        double ay = derivadas_f[i].vy;
        sistema[i].actual.vx = sistema[i].actual.vx + ax*(dt/2.0);
        sistema[i].actual.vy = sistema[i].actual.vy + ay*(dt/2.0);

        sistema[i].trayectoria.push_back(pixelespantalla(sistema[i].actual.x, sistema[i].actual.y));
        if(sistema[i].trayectoria.size() > 500){
            sistema[i].trayectoria.erase(sistema[i].trayectoria.begin());
        }
    }
}

vector<Cuerpo> Excentrico(){
    vector<Cuerpo> sistema;
    sistema.push_back(Cuerpo(1.0, {0.0, 0.0, 0.0, 0.0}, Color::Yellow));
    sistema.push_back(Cuerpo(0.00003, {1.0, 0.0, 0.0, 2.0}, Color::Cyan));
    return sistema;
}

void Lab(){

    int pasos = 50000;
    double dt = 0.001;

    cout << "Calculando con RK4" << endl;
    vector<Cuerpo> sistemaRK4 = Excentrico();
    double energ0_RK4 = Energia(sistemaRK4);

    ofstream archRK4("rk4.csv");
    archRK4 << "Tiempo,X,Y,Energia,Error\n";

    for(int i = 0; i < pasos; i++){
        actualizarRK4(sistemaRK4, dt);
        if(i%10 == 0){
            double e_actual = Energia(sistemaRK4);
            double error = abs((e_actual - energ0_RK4)/energ0_RK4);
            archRK4 << (i*dt) << "," << sistemaRK4[1].actual.x << "," << sistemaRK4[1].actual.y << "," << e_actual << "," << error << "\n";
        }
    }
    archRK4.close();

    cout << "Calculando con Leapfrog" << endl;
    vector<Cuerpo> sistemaLF = Excentrico();
    double energ0_LF = Energia(sistemaLF);

    ofstream archLF("leapfrog.csv");
    archLF << "Tiempo,X,Y,Energia,Error\n";
    
    for(int i = 0; i < pasos; i++){
        actualizarLF(sistemaLF, dt);
        if(i%10 == 0){
            double e_actual = Energia(sistemaLF);
            double error = abs((e_actual - energ0_LF)/energ0_LF);
            archLF << (i*dt) << "," << sistemaLF[1].actual.x << "," << sistemaLF[1].actual.y << "," << e_actual << "," << error << "\n";
        }
    }
    archLF.close();
}
int main()
{
    int opcion;
    cout << "1. Sistema interactivo (SFML)" << endl;
    cout << "2. Experimento energia (CSV)" << endl;
    cin >> opcion;

    if(opcion == 2){
        Lab();
        cout << "Enter para salir";
        cin.ignore();
        cin.get();
        return 0; 
    }

    RenderWindow ventana(VideoMode({1200, 1000}),"Jeroni Munyos");
    ventana.setFramerateLimit(60);

    Font fuente;
    if(!fuente.openFromFile("arial.ttf")){
        return -1;
    }
    Text textoM(fuente);
    textoM.setCharacterSize(24);
    textoM.setFillColor(Color::White);
    textoM.setPosition({10.0f, 10.0f});


    vector<Color> paleta = {Color::Cyan, Color::Magenta, Color::Red, Color::Green, Color::Yellow, Color::White, Color(255, 165, 0), Color(128, 0, 128)};
    int contcol = 0;
    

    string masaEntr = "1.0";
    double masaNum = 1.0;
    
    vector<Cuerpo> sistema;
    double dt = 0.001; 

    while(ventana.isOpen()){
        while(const auto event = ventana.pollEvent()){
            if(event->is<Event::Closed>()){
                ventana.close();
            }

            if(const auto* textEvent = event->getIf<Event::TextEntered>()){
                char tecla = static_cast<char>(textEvent->unicode);

                if((tecla >= '0' && tecla <= '9' || tecla == '.')){
                    masaEntr = masaEntr + tecla;
                } else if(tecla == 8){
                    if(!masaEntr.empty()){
                        masaEntr.pop_back();
                    }
                }
            }
            if(const auto* mousePress = event->getIf<Event::MouseButtonPressed>()){
                if(mousePress->button == Mouse::Button::Left){
                    try{
                        if(masaEntr.empty()){
                            masaNum = 1.0;
                        } else {
                            masaNum = stod(masaEntr);
                        }
                    }
                    catch(...){
                        masaNum = 1.0;
                    }

                    Vector2i pos = mousePress->position;
                    Estado n_estado = EspacioReal(pos.x, pos.y);

                    Color col_actual = paleta[contcol];
                    contcol++;
                    if(contcol >= paleta.size()){
                        contcol = 0; 
                    }

                    sistema.push_back(Cuerpo(masaNum, n_estado, col_actual));

                }
            }
            

        }
        ventana.clear(Color::Black);
        actualizarRK4(sistema, dt);
        for(int i = 0; i < sistema.size(); i++){
            sistema[i].dibujar(ventana);
        }
        
        textoM.setString("Masa: " + masaEntr + "_");
        ventana.draw(textoM);
        ventana.display();
    }



    return 0;

}
