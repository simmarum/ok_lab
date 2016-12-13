/* OPTYMALIZACJA KOMBINATORYCZNA
 * Temat: Metaheurystyki w problemie szeregowania zadañ
 * Autor: Bartosz Górka, Mateusz Kruszyna
 * Data: Grudzieñ 2016r.
*/

// Biblioteki u¿ywane w programie
#include <iostream> // I/O stream
#include <vector> // Vector table
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream> 
#include <algorithm> // Sortowanie przerwañ

using namespace std;
// Struktura danych w pamiêci
struct Task {
	int assigment; // PrzydziaÅ‚ zadania pierwszego do maszyny x = [0, 1]
	int durationFirstPart; // Czas czêœci 1 zadania
	int durationSecondPart; // Czas czêœci 2 zadania
	int timeEndFirstPart; // Moment zakoñczenia czêœci 1
	int timeEndSecondPart; // Moment zakoñczenia czêœci 2
};

struct Maintenance {
	int assigment; // Numer maszyny
	int readyTime; // Czas gotowoœci (pojawienia siê)
	int duration; // Czas trwania przerwania
};

// Funkcja pomocnicza u¿ywana w sortowaniu przerwañ
bool sortMaintenance(Maintenance * i, Maintenance * j) {return (i->readyTime < j->readyTime); }

// Generator przestojóww na maszynie
void GeneratorPrzestojow(vector<Maintenance*> &lista, int liczbaPrzerwanFirstProcessor, int liczbaPrzerwanSecondProcessor, int lowerTimeLimit, int upperTimeLimit, int lowerReadyTime, int upperReadyTime) {
	srand(time(NULL));
	
	int size = (upperReadyTime - lowerReadyTime) + (upperTimeLimit - lowerTimeLimit);
	bool * maintenanceTimeTable = new bool[size] {}; // Jedna tablica bo przerwania na maszynach nie mog¹ siê nak³adaæ na siebie 
	
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
				// Sprawdzenie czy mo¿na daæ przerwanie od readyTime
					bool repeatCheck = false;
					for(int j = startTimeCheck; j < stopTimeCheck; j++) {
						if(maintenanceTimeTable[j]) {
							repeatCheck = true;
							break; // Konieczne jest ponowne losowanie czasu rozpoczêcia
						}
					}
					
					if(!repeatCheck) {
						break; // Mo¿na opuœciæ pêtle while - znaleziono konfiguracjê dla przerwania
					}
			}
			
			// Zapis przerwania w tablicy pomocniczej
				for(int j = startTimeCheck; j < stopTimeCheck; j++) {
					maintenanceTimeTable[j] = true;
				}
			
			// Uzupe³nienie danych o przerwaniu
				przerwa->readyTime = readyTime;
				przerwa->duration = duration;
			
			// Dodanie przestoju do listy
				lista.push_back(przerwa);
	}
}

// Generator instancji problemu
void GeneratorInstancji(vector<Task*> &lista, int n, int lowerTimeLimit, int upperTimeLimit) {
	srand(time(NULL));
	
	for(int i = 0; i < n; i++) {
		Task * zadanie = new Task;
		
		// Przydzia³ maszyny do czêœci 1 zadania
			zadanie->assigment = rand() % 2;
		
		// Randomy na czas trwania zadania
			zadanie->durationFirstPart = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			zadanie->durationSecondPart = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			
		// Czas zakoñczenia póki co ustawiony na 0 = zadanie nie by³o u¿ywane
			zadanie->timeEndFirstPart = 0;
			zadanie->timeEndSecondPart = 0;
		
		// Dodanie zadania do listy
			lista.push_back(zadanie);	
	}
}

// Zapis instancji do pliku
void ZapiszInstancjeDoPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, int numerInstancjiProblemu) {
	ofstream file;
	file.open("instancje.txt");	
	
	if(file.is_open()) {
		file << "**** " << numerInstancjiProblemu << " ****" << endl;
		
		// Uzupe³nienie pliku o wygenerowane czasy pracy
			int iloscZadan = listaZadan.size();
			file << iloscZadan << endl;
			
			for(int i = 0; i < iloscZadan; i++) {
				file << listaZadan[i]->durationFirstPart << ":" << listaZadan[i]->durationSecondPart << ":"
				<< listaZadan[i]->assigment << ":" << 1 - listaZadan[i]->assigment << ";" << endl;
			}
		
		// Uzupe³nienie pliku o czasy przestojów maszyn
			int iloscPrzestojow = listaPrzerwan.size();
			for(int i = 0; i < iloscPrzestojow; i++) {
				file << i << ":" << listaPrzerwan[i]->assigment << ":" << listaPrzerwan[i]->duration << ":" << listaPrzerwan[i]->readyTime << ";" << endl;
			}
			
			file << "**** EOF ****" << endl;
	}	
	
	file.close();
}

// Wczytywanie instancji z pliku do pamiêci
void WczytajDaneZPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, int &numerInstancjiProblemu) {
	FILE *file;
	file = fopen("instancje.txt", "r");
	
	if(file != NULL) {
		// Pobranie numeru instancji problemu
			fscanf(file, "**** %d ****", &numerInstancjiProblemu);
			
		// Pobranie liczby zadañ
			int liczbaZadan = 0;
			fscanf(file, "%d", &liczbaZadan);
		
		// Zmienne pomocnicze w tworzeniu zadania
			int assigmentFirstPart, assigmentSecondPart, durationFirstPart, durationSecondPart;
		
		// Pobranie wartoœci zadania z pliku instancji
			for(int i = 0; i < liczbaZadan; i++) {
				// Odczyt wpisu
					fscanf(file, "%d:%d:%d:%d;", &durationFirstPart, &durationSecondPart, &assigmentFirstPart, &assigmentSecondPart);		
				
				// Utworzenie zadania
					Task * zadanie = new Task;
					
				// Ustawienie wartoœci zadania
					zadanie->assigment = assigmentFirstPart;
					zadanie->durationFirstPart = durationFirstPart;
					zadanie->durationSecondPart = durationSecondPart;
					zadanie->timeEndFirstPart = 0;
					zadanie->timeEndSecondPart = 0;
					
				// Dodanie zadania do wektora zadañ
					listaZadan.push_back(zadanie);
			}
			
		// Zestaw zmiennych u¿ywanych przy odczycie przerwañ na maszynach
			int assigment, duration, readyTime, numer;
			int oldNumber = -1;
			
		// Pobranie wartoœci dotycz¹cych przerwañ
			while(fscanf(file, "%d:%d:%d:%d;", &numer, &assigment, &duration, &readyTime)) {
				// Sprawdzenie czy nie mamy zapêtlenia
					if(oldNumber == numer)
						break;
						
				// Utworzenie przerwy
					Maintenance * przerwa = new Maintenance;
					
				// Ustawienie wartoœci zadania
					przerwa->assigment = assigment;
					przerwa->duration = duration;
					przerwa->readyTime = readyTime;
					
				// Dodanie zadania do wektora zadañ
					listaPrzerwan.push_back(przerwa);
					
				// Zmienna pomocnicza do eliminacji zapêtleñ przy odczycie
					oldNumber = numer;
			}
	}
}

void OdczytPrzerwan(vector<Maintenance*> &listaPrzerwan) {
	int size = listaPrzerwan.size();
	for(int i = 0; i < size; i++) {
		cout << "Maszyna = " << listaPrzerwan[i]->assigment << " | Start = " << listaPrzerwan[i]->readyTime << " | Czas trwania = " << listaPrzerwan[i]->duration << endl;
	}
}

vector<Task*> GeneratorLosowy(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor, int iloscZadan) {
	// Utworzenie kopii zadañ aby móc tworzyæ swoje rozwi¹zanie
		vector<Task*> zadaniaLokalne(listaZadan);
	
	// Pêtla operacyjna tworzenia losowego rozwi¹zania	
		int numerPrzerwaniaFirstProcessor = 0;
		int numerPrzerwaniaSecondProcessor = 0;
		int count = 0;
		int numerZadania = 0;
		int najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
		int najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
		int timeFirstProcessor = 0;
		int timeSecondProcessor = 0;
		
	// Tworzymy dwie tablice pomocnicze do sprawdzania czy zadanie by³o ju¿ uwzglêdnione
		bool * firstPart = new bool[iloscZadan] {};
		bool * secondPart = new bool[iloscZadan] {};
		
	// 	iloscZadan *= 2;
		
		while(count < iloscZadan) {
			// Losujemy numer zadania
				numerZadania = rand() % iloscZadan;
			
			// Sprawdzamy czy nie by³o ju¿ przypadkiem u¿yte ca³e - w takim przypadku robimy kolejn¹ iteracjê pêtli i losujemy na nowo numer zadania
				if(firstPart[numerZadania] && secondPart[numerZadania])
					break;
				cout << "Numer = " << numerZadania << " M" << zadaniaLokalne[numerZadania]->assigment 
				<< " czasy: " << timeFirstProcessor << "|" << timeSecondProcessor 
				<< " przerwy: " << najblizszyMaintenanceFirstProcessor << "|" << najblizszyMaintenanceSecondProcessor << endl;
			// Zadanie nie by³o jeszcze u¿ywane
				if(!firstPart[numerZadania]) {
					// Sprawdzamy typ zadania
					if(zadaniaLokalne[numerZadania]->assigment == 0) { // Przydzia³ na pierwsz¹ maszynê
						// Dwa mo¿liwe przypadki - zadanie umieszczamy przed przerw¹ lub po niej
						
						// Sprawdzamy czy zadanie mo¿na umieœciæ przed maintenance najbli¿szym (je¿eli jest  on -1 to ju¿ nie wyst¹pi)
						if((timeFirstProcessor + zadaniaLokalne[numerZadania]->durationFirstPart) <= najblizszyMaintenanceFirstProcessor
							|| (najblizszyMaintenanceFirstProcessor == -1)) {
							// Ustawiamy czas na maszynie pierwszej
								timeFirstProcessor += zadaniaLokalne[numerZadania]->durationFirstPart;
							
							// Ustawiamy czas zakoñczenia zadania
								cout << "Czas FM: " << timeFirstProcessor << endl;
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeFirstProcessor;
								
							// Ustawiamy ¿e zadanie zosta³o u¿yte (part I)
								firstPart[numerZadania] = true;	
							
						} else { // Nie umieœciliœmy zadania przed przerw¹
							while(true) {
								// Przesuwamy siê na chwilê po przerwaniu
									timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;
								
								// Ustawiamy czas nastêpnego przerwania
									numerPrzerwaniaFirstProcessor++;
									if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessor.size())
										najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
									else
										najblizszyMaintenanceFirstProcessor = -1;
										
								// Musismy sprawdziæ czy uda siê nam wcisn¹æ nasze zadanie
									if((timeFirstProcessor + zadaniaLokalne[numerZadania]->durationFirstPart) <= najblizszyMaintenanceFirstProcessor
										|| (najblizszyMaintenanceFirstProcessor == -1))
										break;
							}
							
							// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeFirstProcessor (wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
								timeFirstProcessor += zadaniaLokalne[numerZadania]->durationFirstPart;
									
							// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
								cout << "Czas FM " << timeFirstProcessor << endl;
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeFirstProcessor;
								
							// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
								firstPart[numerZadania] = true;									
						}
						
						// Zwiêkszamy iloœæ zadañ jakie przerobiliœmy
							count++;
							
					} else { // Przydzia³ zadania na maszynê nr 2
						
						// Sprawdzamy czy zadanie mo¿na umieœciæ przed maintenance najbli¿szym (je¿eli jest  on -1 to ju¿ nie wyst¹pi)
						if((timeSecondProcessor + zadaniaLokalne[numerZadania]->durationFirstPart) <= najblizszyMaintenanceSecondProcessor
							|| (najblizszyMaintenanceSecondProcessor == -1)) {
							// Ustawiamy czas na maszynie drugiej
								timeSecondProcessor += zadaniaLokalne[numerZadania]->durationFirstPart;
							
							// Ustawiamy czas zakoñczenia zadania
								cout << "Czas SM: " << timeSecondProcessor << endl;
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeSecondProcessor;
								
							// Ustawiamy ¿e zadanie zosta³o u¿yte (part I)
								firstPart[numerZadania] = true;	
							
						} else { // Nie umieœciliœmy zadania przed przerw¹
							while(true) {
								// Przesuwamy siê na chwilê po przerwaniu
									timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;
								
								// Ustawiamy czas nastêpnego przerwania
									numerPrzerwaniaSecondProcessor++;
									if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessor.size())
										najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
									else
										najblizszyMaintenanceSecondProcessor = -1;
										
									cout << "Druga = " << timeSecondProcessor << " oraz " << najblizszyMaintenanceSecondProcessor << endl;
										
								// Musismy sprawdziæ czy uda siê nam wcisn¹æ nasze zadanie
									if((timeSecondProcessor + zadaniaLokalne[numerZadania]->durationFirstPart) <= najblizszyMaintenanceSecondProcessor
										|| (najblizszyMaintenanceSecondProcessor == -1))
										break;
							}
							
							// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeSecondProcessor(wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
								timeSecondProcessor += zadaniaLokalne[numerZadania]->durationFirstPart;
									
							// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
								cout << "Czas SM " << timeSecondProcessor << endl;
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeSecondProcessor;
								
							// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
								firstPart[numerZadania] = true;									
						}
						
						// Zwiêkszamy iloœæ zadañ jakie przerobiliœmy
							count++;
					}
				} else {
					cout << "Pomin linie nad tym wpisem!" << endl;
				}
		}
		
		return zadaniaLokalne;
}

void PodzielPrzerwyNaMaszyny(vector<Maintenance*> &listaPrzerwan, vector<Maintenance*> &przerwaniaFirstProcessor, vector<Maintenance*> &przerwaniaSecondProcessor) {
	// Zmienna pomocnicza by skróciæ czas pracy (nie trzeba x razy liczyæ)
		int size = listaPrzerwan.size();

	//Sprawdzamy do jakiej maszyny przypisane mamy przerwanie	
		for(int i = 0; i < size; i++) {
			Maintenance * przerwa = listaPrzerwan[i];
			if(przerwa->assigment == 0) {
				przerwaniaFirstProcessor.push_back(przerwa);
			} else {
				przerwaniaSecondProcessor.push_back(przerwa);
			}
		}	
}

void OdczytDanychZadan(vector<Task*> &listaZadan) {
	int size = listaZadan.size();
	for(int i = 0; i < size; i++) {
		cout << "--- ID: " << i << " przydzial: M" << listaZadan[i]->assigment << " duration = " << listaZadan[i]->durationFirstPart << "|" << listaZadan[i]->durationSecondPart 
		<< " --- zakonczenie = " << listaZadan[i]->timeEndFirstPart << "|" << listaZadan[i]->timeEndSecondPart << " --- " << endl;
	}
}

void SortujPrzerwania(vector<Maintenance*> &listaPrzerwan) {
	// U¿ywamy algorytmicznej funkcji sort z ustawionym trybem sortowania aby przyspieszyæ pracê
		sort(listaPrzerwan.begin(), listaPrzerwan.end(), sortMaintenance);
}

int main() {
	int rozmiarInstancji = 5;
	int numerInstancjiProblemu = 0;

	// Utworzenie wektora na n zadañ
		vector<Task*> listaZadan;

	// Wektor przerwañ pracy na maszynach
		vector<Maintenance*> listaPrzerwan; 

	// Wygenerowanie zadañ
		// GeneratorInstancji(listaZadan, rozmiarInstancji, 5, 15);

	// Wygenerowanie przerwañ	
		// GeneratorPrzestojow(listaPrzerwan, 2, 2, 1, 10, 0, 50);
		// OdczytPrzerwan(listaPrzerwan);

	// Zapis danych do pliku
		// ZapiszInstancjeDoPliku(listaZadan, listaPrzerwan, numerInstancjiProblemu);

	// Wczytanie danych z pliku
		WczytajDaneZPliku(listaZadan, listaPrzerwan, numerInstancjiProblemu);
		OdczytPrzerwan(listaPrzerwan);
		
		vector<Maintenance*> przerwaniaFirstProcessor;
		vector<Maintenance*> przerwaniaSecondProcessor;
		SortujPrzerwania(listaPrzerwan);
		PodzielPrzerwyNaMaszyny(listaPrzerwan, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
		
		GeneratorLosowy(listaZadan, przerwaniaFirstProcessor, przerwaniaSecondProcessor, rozmiarInstancji);
		
		OdczytDanychZadan(listaZadan);
	return 0;
}
