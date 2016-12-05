/* OPTYMALIZACJA KOMBINATORYCZNA
 * Temat: Metaheurystyki w problemie szeregowania zadań
 * Autor: Bartosz Górka, Mateusz Kruszyna
 * Data: Listopad 2016r.
*/

#define TEST

#include <iostream> // I/O stream
#include <vector> // Vector table
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream> 

using namespace std;

// Struktura danych w pamięci
struct Task {
	int part_1_assign; // Przydział zadania pierwszego do maszyny x = [0, 1]
	int time_part_1; // Czas części 1 zadania
	int time_part_2; // Czas części 2 zadania
	int end_time_1; // Moment zakończenia części 1
	int end_time_2; // Moment zakończenia części 2
};

// Generator przestojów na maszynie

// Generator instancji problemu
void GeneratorInstancji(vector<Task*> &lista, int n, int lower_limit, int upper_limit) {
	srand(time(NULL));
	
	for(int i = 0; i < n; i++) {
		Task * zadanie = new Task;
		
		// Przydział maszyny do części 1 zadania
			zadanie->part_1_assign = rand() % 2;
		
		// Randomy na time
			zadanie->time_part_1 = lower_limit + (int)(rand() / (RAND_MAX + 1.0) * upper_limit);
			zadanie->time_part_2 = lower_limit + (int)(rand() / (RAND_MAX + 1.0) * upper_limit);
			
		// Czas zakończenia póki co 0 = jeszcze nie było używane
			zadanie->end_time_1 = 0;
			zadanie->end_time_2 = 0;
		
		// Dodanie zadania do listy
			lista.push_back(zadanie);	
	}
}

// Zapis instancji do pliku
void ZapiszInstancjeDoPliku(vector<Task*> &lista, int numer_instancji_problemu) {
	ofstream file;
	file.open("instancje.txt");	
	
	if(file.is_open()) {
		file << "**** " << numer_instancji_problemu << "  ****" << endl;
		
		// Uzupełnienie pliku o wygenerowane czasy pracy
			int ilosc_zadan = lista.size();
			for(int i = 0; i< ilosc_zadan; i++) {
				file << lista[i]->time_part_1 << ":" << lista[i]->time_part_2 << ":"
				<< lista[i]->part_1_assign << ":";
				
				if(lista[i]->part_1_assign == 0) {
					file << "1;" << endl;
				} else {
					file << "0;" << endl;
				}
			}
		
		// Uzupełnienie pliku o czasy przestojów maszyn
	}	
	
	file.close();
}

int main() {
	int rozmiar_instancji = 50;
	int numer_instancji_problemu = 1;
	
	// Utworzenie wektora na n zadań
		vector<Task*> lista_zadan; 
	
	// Wygenerowanie zadań
		GeneratorInstancji(lista_zadan, rozmiar_instancji, 5, 15);
		
	// Zapis danych do pliku
		ZapiszInstancjeDoPliku(lista_zadan, numer_instancji_problemu);

	return 0;
}
