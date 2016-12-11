/* OPTYMALIZACJA KOMBINATORYCZNA
 * Temat: Metaheurystyki w problemie szeregowania zadañ
 * Autor: Bartosz Górka, Mateusz Kruszyna
 * Data: Listopad 2016r.
*/

// Biblioteki u¿ywane w programie
#include <iostream> // I/O stream
#include <vector> // Vector table
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream> 

using namespace std;

// Struktura danych w pamiêci
struct Task {
	int part_1_assign; // PrzydziaÅ‚ zadania pierwszego do maszyny x = [0, 1]
	int time_part_1; // Czas czêœci 1 zadania
	int time_part_2; // Czas czêœci 2 zadania
	int end_time_1; // Moment zakoñczenia czêœci 1
	int end_time_2; // Moment zakoñczenia czêœci 2
};

struct Maintenance {
	int assigment; // Numer maszyny
	int readyTime; // Czas gotowoœci (pojawienia siê)
	int duration; // Czas trwania przerwania
};

// Generator przestojóww na maszynie
void GeneratorPrzestojow(vector<Maintenance*> &lista, int liczbaPrzerwanFirstProcessor, int liczbaPrzerwanSecondProcessor, int lowerTimeLimit, int upperTimeLimit, int lowerReadyTime, int upperReadyTime) {
	srand(time(NULL));
	
	int size = (upperReadyTime - lowerReadyTime) + (upperTimeLimit - lowerTimeLimit);
	bool * maintenanceTimeTableFirstProcessor = new bool[size] {}; 
	bool * maintenanceTimeTableSecondProcessor = new bool[size] {};
	
	int liczbaPrzerwan = liczbaPrzerwanFirstProcessor + liczbaPrzerwanSecondProcessor;
	
	for(int i = 0; i < liczbaPrzerwan; i++) {
		Maintenance * przerwa = new Maintenance;
		
		// Losowanie przerwy na któr¹ maszynê ma trafiæ
		if(liczbaPrzerwanFirstProcessor == 0) {
			przerwa->assigment = 1;
			liczbaPrzerwanSecondProcessor--;
		} else if (liczbaPrzerwanSecondProcessor == 0){
			przerwa->assigment = 0;
			liczbaPrzerwanFirstProcessor--;
		} else {
			przerwa->assigment = rand() % 2;
			if(przerwa->assigment == 0)
				liczbaPrzerwanFirstProcessor--;
			else
				liczbaPrzerwanSecondProcessor--;
		}
		
		// Randomowy czas trwania
			int duration = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			przerwa->duration = duration;
			
		// Random punkt startu + sprawdzenie czy jest to mo¿liwe
			int readyTime = 0;
			int startTimeCheck, stopTimeCheck = 0;
			
			while(true) {
				readyTime = lowerReadyTime + (int)(rand() / (RAND_MAX + 1.0) * upperReadyTime);
				
				startTimeCheck = readyTime - lowerReadyTime;
				stopTimeCheck = startTimeCheck + duration;
				// Sprawdzenie
					bool repeatCheck = false;
					for(int j = startTimeCheck; j < stopTimeCheck; j++) {
						if(maintenanceTimeTableSecondProcessor[j] == true || maintenanceTimeTableFirstProcessor[j] == true) {
								repeatCheck = true;
								break;
						}
					}
					
					if(repeatCheck == false) {
						break;
					}
				
			}
			
			// Zapis przerwania w tablicy pomocniczej
				for(int j = startTimeCheck; j < stopTimeCheck; j++) {
					if(przerwa->assigment == 0)
						maintenanceTimeTableFirstProcessor[j] = true;
					else
						maintenanceTimeTableSecondProcessor[j] = true;
				}
			
			// Uzupe³nienie danych o przerwaniu
				przerwa->readyTime = readyTime;
				przerwa->duration = duration;
			
			// Dodanie przestoju do listy
				lista.push_back(przerwa);
	}
}

// Generator instancji problemu
void GeneratorInstancji(vector<Task*> &lista, int n, int lower_limit, int upper_limit) {
	srand(time(NULL));
	
	for(int i = 0; i < n; i++) {
		Task * zadanie = new Task;
		
		// Przydzia³ maszyny do czêœci 1 zadania
			zadanie->part_1_assign = rand() % 2;
		
		// Randomy na czas trwania zadania
			zadanie->time_part_1 = lower_limit + (int)(rand() / (RAND_MAX + 1.0) * upper_limit);
			zadanie->time_part_2 = lower_limit + (int)(rand() / (RAND_MAX + 1.0) * upper_limit);
			
		// Czas zakoñczenia póki co ustawiony na 0 = zadanie nie by³o u¿ywane
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
		
		// Uzupe³nienie pliku o wygenerowane czasy pracy
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
		
		// Uzupe³nienie pliku o czasy przestojów maszyn
	}	
	
	file.close();
}

void OdczytPrzerwan(vector<Maintenance*> &listaPrzerwan) {
	int size = listaPrzerwan.size();
	for(int i = 0; i < size; i++) {
		cout << "Maszyna = " << listaPrzerwan[i]->assigment << " | Start = " << listaPrzerwan[i]->readyTime << " | Czas trwania = " << listaPrzerwan[i]->duration << endl;
	}
}

int main() {
	int rozmiar_instancji = 50;
	int numer_instancji_problemu = 1;
	
	// Utworzenie wektora na n zadañ
		vector<Task*> lista_zadan;
	
	// Wektor przerwañ pracy na maszynach
		vector<Maintenance*> listaPrzerwan; 
	
	// Wygenerowanie zadañ
		GeneratorInstancji(lista_zadan, rozmiar_instancji, 5, 15);
		
	// Wygenerowanie przerwañ	
		GeneratorPrzestojow(listaPrzerwan, 2, 2, 0, 10, 0, 25);
		OdczytPrzerwan(listaPrzerwan);
		
	// Zapis danych do pliku
		ZapiszInstancjeDoPliku(lista_zadan, numer_instancji_problemu);

	return 0;
}
