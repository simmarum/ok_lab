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
#include <climits> // INT_MAX do generatora losowego
#include <algorithm> // Sortowanie przerwañ

using namespace std;

// DEFINICJE GLOBALNE
#define DEBUG false // TRYB DEBUGOWANIA [true / false]
#define MIN_TASK_COUNTER 30 // PO ilu iteracjach sprawdzaæ zapêtlenie [Wartoœæ liczbowa > 0]

#define LOWER_TIME_TASK_LIMIT 5 // Dolne ograniczenie d³ugoœci zadania [Wartoœæ liczbowa > 0]
#define UPPER_TIME_TASK_LIMIT 15 // Górne ograniczenie d³ugoœci zadania [Wartoœæ liczbowa > 0]

#define MAINTENANCE_FIRST_PROCESSOR 3 // Iloœæ przerwañ na pierwszej maszynie [Wartoœæ liczbowa >= 0]
#define MAINTENANCE_SECOND_PROCESSOR 3 // Iloœæ przerwañ na drugiej maszynie [Wartoœæ liczbowa >= 0]

#define LOWER_TIME_MAINTENANCE_LIMIT 5 // Dolne ograniczenie d³ugoœci przerwania [Wartoœæ liczbowa >= 0]
#define UPPER_TIME_MAINTENANCE_LIMIT 20 // Górne ograniczenie d³ugoœci przerwania [Wartoœæ liczbowa > 0]

#define LOWER_READY_TIME_MAINTENANCE_LIMIT 0 // Dolne ograniczenie czasu gotowoœci przerwania [Wartoœæ liczbowa > 0]
#define UPPER_READY_TIME_MAINTENANCE_LIMIT 200 // Górne ograniczenie czasu gotowoœci przerwania [Wartoœæ liczbowa > 0]

#define INSTANCE_SIZE 15 // Rozmiar instancji problemu
#define INSTANCE_NUMBER 1 // Numer instancji problemu (mo¿e byæ zmieniana przy odczycie danych z pliku)

ofstream debugFile; // Zmienna globalna u¿ywana przy DEBUG mode

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
	bool * maintenanceTimeTable = new bool[size]; // Jedna tablica bo przerwania na maszynach nie mog¹ siê nak³adaæ na siebie 
	
	for(int i = 0; i < size; i++) {
		maintenanceTimeTable[i] = false;
	}
	
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
	
	// Czyszczenie pamiêci - zwalnianie niepotrzebnych zasobów
		delete maintenanceTimeTable;
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

// Zapis wyników do pliku tekstowego
void ZapiszWynikiDoPliku(vector<Task*> &listaZadan, int numerInstancjiProblemu) {
	ofstream file;
	file.open("wynik.txt");	
	
	if(file.is_open()) {
		file << "**** WYNIKI DLA " << numerInstancjiProblemu << " ****" << endl;
		
		// Uzupe³nienie pliku o rozwi¹zanie
			int iloscZadan = listaZadan.size();			
			for(int i = 0; i < iloscZadan; i++) {
				file << listaZadan[i]->durationFirstPart << ":" << listaZadan[i]->durationSecondPart << ":"
				<< listaZadan[i]->assigment << ":" << 1 - listaZadan[i]->assigment << ":" 
				<< listaZadan[i]->timeEndFirstPart << ":" << listaZadan[i]->timeEndSecondPart << ";" << endl;
			}
	}
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

// Odczyt przerwañ na maszynach na ekran
void OdczytPrzerwan(vector<Maintenance*> &listaPrzerwan) {
	int size = listaPrzerwan.size();
	for(int i = 0; i < size; i++) {
		cout << "Maszyna = " << listaPrzerwan[i]->assigment << " | Start = " << listaPrzerwan[i]->readyTime << " | Czas trwania = " << listaPrzerwan[i]->duration << endl;
	}
}

// Generator rozwi¹zañ losowych
vector<Task*> GeneratorLosowy(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor, int iloscZadan) {
	// Utworzenie kopii zadañ aby móc tworzyæ swoje rozwi¹zanie
		vector<Task*> zadaniaLokalne(listaZadan);
	
	// Zmienne u¿ywane w przebiegu pracy GeneratoraLosowego	
		int numerPrzerwaniaFirstProcessor = 0; // Numer aktualnego przerwania na procesorze pierwszym
		int numerPrzerwaniaSecondProcessor = 0; // Numer aktualnego przerwania na procesorze drugim
		int count = 0; // Licznik przeliczonych ju¿ zadañ
		int numerZadania = 0; // Numer aktualnie rozpatrywanego zadania (losowe)
		int najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime; // Czas momentu ROZPOCZÊCIA przerwania na procesorze pierwszym
		int najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime; // Czas momentu ROZPOCZÊCIA przerwania na procesorze drugim 
		int timeFirstProcessor = 0; // Zmienna czasowa - procesor pierwszy
		int timeSecondProcessor = 0; // Zmienna czasowa - procesor drugi
		int maxCount = 2 * iloscZadan; // Iloœæ koniecznych edycji w zadaniach (part I + part II w ka¿dym zadaniu)
		int listaPrzerwanFirstProcessorSize = listaPrzerwanFirstProcessor.size(); // Iloœæ przerwañ dla pierwszego procesora - aby nie liczyæ za ka¿dym razem tej wartoœci
		int listaPrzerwanSecondProcessorSize = listaPrzerwanSecondProcessor.size(); // Iloœæ przerwañ dla drugiej maszyny - podonie jak wy¿ej, unikamy niepotrzebnego, wielokrotnego liczenia tej wartoœci
		
	// Tworzymy dwie tablice pomocnicze do sprawdzania czy zadanie by³o ju¿ uwzglêdnione
		bool * firstPart = new bool[iloscZadan]; // Czêœæ I zadania - czy by³a uwzglêdniona (jeœli tak to true)
		bool * secondPart = new bool[iloscZadan]; // Czêœæ II zadania - czy by³a uwzglêdniona (jeœli tak to true)
	
	// Licznik odwiedzin w ka¿dym z zadañ
		int * licznikOdwiedzonych = new int[iloscZadan]; // Licznik odwiedzeñ w danym zadaniu aby unikaæ pêtli
		
	// Pêtla startowa zeruj¹ca tablice
		for(int i = 0; i < iloscZadan; i++) {
			firstPart[i] = false;
			secondPart[i] = false;
			licznikOdwiedzonych[i] = 0;
		}
	
		while(count < maxCount) {
			// Losujemy numer zadania
				numerZadania = rand() % iloscZadan;
			
			// Sprawdzamy czy nie by³o ju¿ przypadkiem u¿yte ca³e - w takim przypadku robimy kolejn¹ iteracjê pêtli i losujemy na nowo numer zadania
				if(firstPart[numerZadania] && secondPart[numerZadania])
					continue; // Skok do kolejnej iteracji
				
				if(DEBUG) {
					debugFile << "Numer = " << numerZadania << " M" << zadaniaLokalne[numerZadania]->assigment 
					<< " czasy: " << timeFirstProcessor << "|" << timeSecondProcessor 
					<< " przerwy: " << najblizszyMaintenanceFirstProcessor << "|" << najblizszyMaintenanceSecondProcessor << endl;
				}

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
							
								if(DEBUG) 
									debugFile << "Czas FM: " << timeFirstProcessor << endl;
							
							// Ustawiamy czas zakoñczenia zadania
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeFirstProcessor;
								
							// Ustawiamy ¿e zadanie zosta³o u¿yte (part I)
								firstPart[numerZadania] = true;	
							
						} else { // Nie umieœciliœmy zadania przed przerw¹
							while(true) {
								// Przesuwamy siê na chwilê po przerwaniu
									timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;
								
								// Ustawiamy czas nastêpnego przerwania
									numerPrzerwaniaFirstProcessor++;
									if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
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
							
								if(DEBUG)		
									debugFile << "Czas FM " << timeFirstProcessor << endl;
							
							// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
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
							
								if(DEBUG)
									debugFile << "Czas SM: " << timeSecondProcessor << endl;
							
							// Ustawiamy czas zakoñczenia zadania
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeSecondProcessor;
								
							// Ustawiamy ¿e zadanie zosta³o u¿yte (part I)
								firstPart[numerZadania] = true;	
							
						} else { // Nie umieœciliœmy zadania przed przerw¹
							while(true) {
								// Przesuwamy siê na chwilê po przerwaniu
									timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;
								
								// Ustawiamy czas nastêpnego przerwania
									numerPrzerwaniaSecondProcessor++;
									if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
										najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
									else
										najblizszyMaintenanceSecondProcessor = -1;
										
									if(DEBUG)
										debugFile << "Druga = " << timeSecondProcessor << " oraz " << najblizszyMaintenanceSecondProcessor << endl;
										
								// Musismy sprawdziæ czy uda siê nam wcisn¹æ nasze zadanie
									if((timeSecondProcessor + zadaniaLokalne[numerZadania]->durationFirstPart) <= najblizszyMaintenanceSecondProcessor
										|| (najblizszyMaintenanceSecondProcessor == -1))
										break;
							}
							
							// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeSecondProcessor(wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
								timeSecondProcessor += zadaniaLokalne[numerZadania]->durationFirstPart;
							
								if(DEBUG)		
									debugFile << "Czas SM " << timeSecondProcessor << endl;
							
							// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeSecondProcessor;
								
							// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
								firstPart[numerZadania] = true;									
						}
						
						// Zwiêkszamy iloœæ zadañ jakie przerobiliœmy
							count++;
					}
				} else {
					// PRZYDZIELAMY DRUG¥ CZÊŒÆ ZADANIA
					// Mog¹ wyst¹piæ problemy z zapêtleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartoœæ
					licznikOdwiedzonych[numerZadania]++;
					
					// Sprawdzamy typ zadania
					if(zadaniaLokalne[numerZadania]->assigment == 0) { // Przydzia³ na pierwsz¹ maszynê
						// Sprawdzamy czy czas na maszynie nie jest mniejszy od zakoñczenia siê pierwszej czêœci
						if(timeSecondProcessor < zadaniaLokalne[numerZadania]->timeEndFirstPart) {
							// Sprawdzamy czy nie jesteœmy po raz x w pêtli
							if(licznikOdwiedzonych[numerZadania] >= MIN_TASK_COUNTER) {
								if(DEBUG)
									debugFile << "Przestawiono czas! M1" << endl;
								// Tworzymy pomocnicz¹ zmienn¹ odleg³oœci
									int minTime = INT_MAX;
									int tempTime = 0;
									
								// Resetujemy liczniki i patrzymy na odleg³oœci
									for(int i = 0; i < iloscZadan; i++) {
										licznikOdwiedzonych[i] = 0;
										
										if(!secondPart[i]) {
											int tempTime = zadaniaLokalne[i]->timeEndFirstPart - timeSecondProcessor;
											if(tempTime < minTime)
												minTime = tempTime;
										}
									}
									
								// Przestawiamy czas na maszynie
									timeSecondProcessor += minTime;
									
							} else // Je¿eli nie mamy osi¹gniêtej wartoœci to pomijamy iteracjê
								continue;
						}
							
						// Zadanie mo¿na umieœciæ
							// Sprawdzamy czy zadanie mo¿na umieœciæ przed maintenance najbli¿szym (je¿eli jest  on -1 to ju¿ nie wyst¹pi)
							if((timeSecondProcessor + zadaniaLokalne[numerZadania]->durationSecondPart) <= najblizszyMaintenanceSecondProcessor
								|| (najblizszyMaintenanceSecondProcessor == -1)) {
								// Ustawiamy czas na maszynie pierwszej
									timeSecondProcessor += zadaniaLokalne[numerZadania]->durationSecondPart;
								
								// Ustawiamy czas zakoñczenia zadania
									zadaniaLokalne[numerZadania]->timeEndSecondPart = timeSecondProcessor;
									
								// Ustawiamy ¿e zadanie zosta³o u¿yte (part II)
									secondPart[numerZadania] = true;	
							
							} else { // Nie umieœciliœmy zadania przed przerw¹
								while(true) {
									// Przesuwamy siê na chwilê po przerwaniu
										timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;
									
									// Ustawiamy czas nastêpnego przerwania
										numerPrzerwaniaSecondProcessor++;
										if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
											najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
										else
											najblizszyMaintenanceSecondProcessor = -1;
											
									// Musismy sprawdziæ czy uda siê nam wcisn¹æ nasze zadanie
										if((timeSecondProcessor + zadaniaLokalne[numerZadania]->durationSecondPart) <= najblizszyMaintenanceSecondProcessor
											|| (najblizszyMaintenanceSecondProcessor == -1))
											break;
								}
								
								// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeSecondProcessor (wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
									timeSecondProcessor += zadaniaLokalne[numerZadania]->durationSecondPart;
										
								// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
									zadaniaLokalne[numerZadania]->timeEndSecondPart = timeSecondProcessor;
									
								// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
									secondPart[numerZadania] = true;
							}
						
							// Zwiêkszamy iloœæ zadañ jakie przerobiliœmy
								count++;
					} else {
						// Sprawdzamy czy czas na maszynie nie jest mniejszy od zakoñczenia siê pierwszej czêœci
						if(timeFirstProcessor < zadaniaLokalne[numerZadania]->timeEndFirstPart) {
							// Sprawdzamy czy nie jesteœmy po raz x w pêtli
							if(licznikOdwiedzonych[numerZadania] >= MIN_TASK_COUNTER) {
								if(DEBUG)
									debugFile << "Przestawiono czas! M0" << endl;
									
								// Tworzymy pomocnicz¹ zmienn¹ odleg³oœci
									int minTime = INT_MAX;
									int tempTime = 0;
									
								// Resetujemy liczniki i patrzymy na odleg³oœci
									for(int i = 0; i < iloscZadan; i++) {
										licznikOdwiedzonych[i] = 0;
										
										if(!secondPart[i]) {
											tempTime = zadaniaLokalne[i]->timeEndFirstPart - timeFirstProcessor;
											if(tempTime < minTime)
												minTime = tempTime;
										}
									}
									
								// Przestawiamy czas na maszynie
									timeFirstProcessor += minTime;
									
							} else // Je¿eli nie mamy osi¹gniêtej wartoœci to pomijamy iteracjê
								continue;
						}
							
						// Zadanie mo¿na umieœciæ
							// Sprawdzamy czy zadanie mo¿na umieœciæ przed maintenance najbli¿szym (je¿eli jest  on -1 to ju¿ nie wyst¹pi)
							if((timeFirstProcessor + zadaniaLokalne[numerZadania]->durationSecondPart) <= najblizszyMaintenanceFirstProcessor
								|| (najblizszyMaintenanceFirstProcessor == -1)) {
								// Ustawiamy czas na maszynie pierwszej
									timeFirstProcessor += zadaniaLokalne[numerZadania]->durationSecondPart;
								
								// Ustawiamy czas zakoñczenia zadania
									zadaniaLokalne[numerZadania]->timeEndSecondPart = timeFirstProcessor;
									
								// Ustawiamy ¿e zadanie zosta³o u¿yte (part II)
									secondPart[numerZadania] = true;	
							
							} else { // Nie umieœciliœmy zadania przed przerw¹
								while(true) {
									// Przesuwamy siê na chwilê po przerwaniu
										timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;
									
									// Ustawiamy czas nastêpnego przerwania
										numerPrzerwaniaFirstProcessor++;
										if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
											najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
										else
											najblizszyMaintenanceFirstProcessor = -1;
											
									// Musismy sprawdziæ czy uda siê nam wcisn¹æ nasze zadanie
										if((timeFirstProcessor + zadaniaLokalne[numerZadania]->durationSecondPart) <= najblizszyMaintenanceFirstProcessor
											|| (najblizszyMaintenanceFirstProcessor == -1))
											break;
								}
								
								// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeSecondProcessor (wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
									timeFirstProcessor += zadaniaLokalne[numerZadania]->durationSecondPart;
										
								// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
									zadaniaLokalne[numerZadania]->timeEndSecondPart = timeFirstProcessor;
									
								// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
									secondPart[numerZadania] = true;
							}
						
							// Zwiêkszamy iloœæ zadañ jakie przerobiliœmy
								count++;
					}
				}
		}
		
		// Czyszczenie pamiêci - zwalnianie niepotrzebnych zasobów
			delete firstPart;
			delete secondPart;
			delete licznikOdwiedzonych;
		
		return zadaniaLokalne;
}

// Podzia³ przerwañ na dwie listy - ka¿da maszyna osobno
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

// Odczyt danych zadañ na ekran
void OdczytDanychZadan(vector<Task*> &listaZadan) {
	int size = listaZadan.size();
	for(int i = 0; i < size; i++) {
		cout << "--- ID: " << i << " przydzial: M" << listaZadan[i]->assigment << " duration = " << listaZadan[i]->durationFirstPart << "|" << listaZadan[i]->durationSecondPart 
		<< " --- zakonczenie = " << listaZadan[i]->timeEndFirstPart << "|" << listaZadan[i]->timeEndSecondPart << " --- " << endl;
	}
}

// Sortowanie przerwañ wed³ug rosn¹cego czasu rozpoczêcia
void SortujPrzerwania(vector<Maintenance*> &listaPrzerwan) {
	// U¿ywamy algorytmicznej funkcji sort z ustawionym trybem sortowania aby przyspieszyæ pracê
		sort(listaPrzerwan.begin(), listaPrzerwan.end(), sortMaintenance);
}

// Tworzenie timeline dla obserwacji wyników pracy
void UtworzGraf(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan) {
	int iloscZadan = listaZadan.size(); // Iloœæ zadañ w systemie
	int iloscPrzerwan = listaPrzerwan.size(); // Iloœæ okresów przestojów na maszynach
	
	ofstream file;
	file.open("index.html");
	file << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
	file << "<title>OK - Wyniki pracy generatora</title></head><body><script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>";
	file << "<script type=\"text/javascript\">google.charts.load(\"current\", {packages:[\"timeline\"]});google.charts.setOnLoadCallback(drawChart);function drawChart() {";
	file << "var container = document.getElementById('example4.2');var chart = new google.visualization.Timeline(container);var dataTable = new google.visualization.DataTable();";
	file << "dataTable.addColumn({ type: 'string', id: 'Role' });dataTable.addColumn({ type: 'string', id: 'Name' });dataTable.addColumn({ type: 'number', id: 'Start' });dataTable.addColumn({ type: 'number', id: 'End' });dataTable.addRows([";
	
	int timeStart = 0;
	int timeStop = 0;

	// Zapisujemy do pliku nasze zadania
		for(int i = 0; i < iloscZadan; i++) {
			timeStop = listaZadan[i]->timeEndFirstPart;
			timeStart = timeStop - listaZadan[i]->durationFirstPart;

			file << "[ 'M" << listaZadan[i]->assigment + 1 << "', 'Zadanie " << i + 1 << "', " << timeStart << "," << timeStop << "]," << endl;
			
			timeStop = listaZadan[i]->timeEndSecondPart;
			timeStart = timeStop - listaZadan[i]->durationSecondPart;

			file << "[ 'M" << 2 - listaZadan[i]->assigment << "', 'Zadanie " << i + 1 << "', " << timeStart << ", " << timeStop << " ]," << endl;
		}
		
	// Zapis przerwañ
		for(int i = 0; i < iloscPrzerwan; i++) {
			timeStart = listaPrzerwan[i]->readyTime;
			timeStop = timeStart + listaPrzerwan[i]->duration;
			
			if(i + 1 == iloscPrzerwan) { // Ostatnia iteracja
				file << "[ 'M" << listaPrzerwan[i]->assigment + 1 << "', 'PRZERWANIE " << i + 1 << "', " << timeStart << ", " << timeStop << " ]]);" << endl;
				file << "var options = {timeline: { groupByRowLabel: true }};chart.draw(dataTable, options);}</script><div id=\"example4.2\" style=\"height: 200px;\"></div></body></html>" << endl;
			} else {
				file << "[ 'M" << listaPrzerwan[i]->assigment + 1 << "', 'PRZERWANIE " << i + 1 << "', " << timeStart << ", " << timeStop << " ]," << endl;
			}
		}
}

int main() {
	debugFile.open("debug.txt");
	int rozmiarInstancji = INSTANCE_SIZE;
	int numerInstancjiProblemu = INSTANCE_NUMBER;

	// Utworzenie wektora na n zadañ
		vector<Task*> listaZadan;

	// Wektor przerwañ pracy na maszynach
		vector<Maintenance*> listaPrzerwan; 

	// Wygenerowanie zadañ
		 GeneratorInstancji(listaZadan, rozmiarInstancji, LOWER_TIME_TASK_LIMIT, UPPER_TIME_TASK_LIMIT);

//	 Wygenerowanie przerwañ	
		 GeneratorPrzestojow(listaPrzerwan, MAINTENANCE_FIRST_PROCESSOR, MAINTENANCE_SECOND_PROCESSOR, LOWER_TIME_MAINTENANCE_LIMIT, UPPER_TIME_MAINTENANCE_LIMIT, LOWER_READY_TIME_MAINTENANCE_LIMIT, UPPER_READY_TIME_MAINTENANCE_LIMIT);
		 OdczytPrzerwan(listaPrzerwan);

	// Zapis danych do pliku
		ZapiszInstancjeDoPliku(listaZadan, listaPrzerwan, numerInstancjiProblemu);

	// Wczytanie danych z pliku
//		WczytajDaneZPliku(listaZadan, listaPrzerwan, numerInstancjiProblemu);
//		OdczytPrzerwan(listaPrzerwan);
		
		vector<Maintenance*> przerwaniaFirstProcessor;
		vector<Maintenance*> przerwaniaSecondProcessor;
		SortujPrzerwania(listaPrzerwan);
		PodzielPrzerwyNaMaszyny(listaPrzerwan, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
		
		GeneratorLosowy(listaZadan, przerwaniaFirstProcessor, przerwaniaSecondProcessor, rozmiarInstancji);
		
//		OdczytDanychZadan(listaZadan);
		
		ZapiszWynikiDoPliku(listaZadan, numerInstancjiProblemu);
		
		UtworzGraf(listaZadan, listaPrzerwan);
	
	// Czyszczenie pamiêci - zwalnianie niepotrzebnych zasobów
		przerwaniaFirstProcessor.clear();
		przerwaniaSecondProcessor.clear();
		listaPrzerwan.clear();
		listaZadan.clear();
	
	return 0;
}
