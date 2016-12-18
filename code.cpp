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
#include <string>
#include <sstream>
#include <fstream> 
#include <climits> // INT_MAX do generatora losowego
#include <algorithm> // Sortowanie przerwañ

using namespace std;

// DEFINICJE GLOBALNE
#define DEBUG true // TRYB DEBUGOWANIA [true / false]
#define MIN_TASK_COUNTER 30 // PO ilu iteracjach sprawdzaæ zapêtlenie [Wartoœæ liczbowa > 0]

#define LOWER_TIME_TASK_LIMIT 5 // Dolne ograniczenie d³ugoœci zadania [Wartoœæ liczbowa > 0]
#define UPPER_TIME_TASK_LIMIT 15 // Górne ograniczenie d³ugoœci zadania [Wartoœæ liczbowa > 0]

#define MAINTENANCE_FIRST_PROCESSOR 3 // Iloœæ przerwañ na pierwszej maszynie [Wartoœæ liczbowa >= 0]
#define MAINTENANCE_SECOND_PROCESSOR 3 // Iloœæ przerwañ na drugiej maszynie [Wartoœæ liczbowa >= 0]

#define LOWER_TIME_MAINTENANCE_LIMIT 5 // Dolne ograniczenie d³ugoœci przerwania [Wartoœæ liczbowa >= 0]
#define UPPER_TIME_MAINTENANCE_LIMIT 20 // Górne ograniczenie d³ugoœci przerwania [Wartoœæ liczbowa > 0]

#define LOWER_READY_TIME_MAINTENANCE_LIMIT 0 // Dolne ograniczenie czasu gotowoœci przerwania [Wartoœæ liczbowa > 0]
#define UPPER_READY_TIME_MAINTENANCE_LIMIT 200 // Górne ograniczenie czasu gotowoœci przerwania [Wartoœæ liczbowa > 0]

#define INSTANCE_SIZE 5 // Rozmiar instancji problemu
#define INSTANCE_NUMBER 1 // Numer instancji problemu (mo¿e byæ zmieniana przy odczycie danych z pliku)

#define MAX_DURATION_PROGRAM_TIME 1000 // Maksymalna d³ugoœæ trwania programu

ofstream debugFile; // Zmienna globalna u¿ywana przy DEBUG mode
long int firstSolutionValue; // Zmienna globalna najlepszego rozwi¹zania wygenerowanego przez generator losowy

// Struktura danych w pamiêci
struct Task{
	int ID; // ID zadania
	int part; // Numer czêœci zadania [0, 1]
	int assigment; // Przydzia³ zadania do maszyny [0, 1]
	int duration; // D³ugoœæ zadania
	int endTime; // Moment zakoñczenia
	Task *anotherPart; // WskaŸnik na komplementarne zadanie
};

struct Maintenance {
	int assigment; // Numer maszyny
	int readyTime; // Czas gotowoœci (pojawienia siê)
	int duration; // Czas trwania przerwania
};

// Funkcja pomocnicza u¿ywana w sortowaniu przerwañ
bool sortMaintenance(Maintenance * i, Maintenance * j) {return (i->readyTime < j->readyTime); }

// Pomocnicze funkcje u¿ywane przy sortowaniu zadañ
bool sortTask(Task *i, Task *j) { return (i->endTime < j->endTime); }
bool sortTaskByID(Task *i, Task *j) { return (i->ID < j->ID); } // Po wartoœci ID

void KopiujDaneOperacji(vector<Task*> &listaWejsciowa, vector<Task*> &listaWyjsciowa);

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

// Sortowanie przerwañ wed³ug rosn¹cego czasu rozpoczêcia
void SortujPrzerwania(vector<Maintenance*> &listaPrzerwan) {
	// U¿ywamy algorytmicznej funkcji sort z ustawionym trybem sortowania aby przyspieszyæ pracê
		sort(listaPrzerwan.begin(), listaPrzerwan.end(), sortMaintenance);
}

// Sortowanie zadañ wed³ug wzrastaj¹cego ID
void SortujZadaniaPoID(vector<Task*> &listaZadan) {
	sort(listaZadan.begin(), listaZadan.end(), sortTaskByID);
}

// Sortowanie zadañ wed³ug rosn¹cego czasu zakoñczenia pracy
void SortujZadaniaPoEndTime(vector<Task*> &listaZadan) {
	sort(listaZadan.begin(), listaZadan.end(), sortTask);
}

// Generator instancji problemu
void GeneratorInstancji(vector<Task*> &lista, int maxTask, int lowerTimeLimit, int upperTimeLimit) {
	srand(time(NULL));
	int assigment = 0;
	
	for(int i = 0; i < maxTask; i++) {
		Task * taskFirst = new Task;
		Task * taskSecond = new Task;
		
		// Powi¹zujemy miêdzy sob¹ zadania
			taskFirst->anotherPart = taskSecond;
			taskSecond->anotherPart = taskFirst;
			
		// Przypisujemy pararametr part
			taskFirst->part = 0;
			taskSecond->part = 1;
		
		// Przypisujemy ID zadania
			taskFirst->ID = i + 1;
			taskSecond->ID = i + 1;
		
		// Losujemy przydzia³ czêœci pierwszej zadania na maszynê
			assigment = rand() % 2;
		
		// Uzupe³niamy dane przydzia³ów
			taskFirst->assigment = assigment;
			taskSecond->assigment = 1 - assigment;
		
		// Randomy na czas trwania zadañ
			taskFirst->duration = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			taskSecond->duration = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			
		// Czas zakoñczenia póki co ustawiony na 0 = zadania nie by³y jeszcze zakolejkowane
			taskFirst->endTime = 0;
			taskSecond->endTime = 0;
		
		// Dodanie zadañ do listy
			lista.push_back(taskFirst);
			lista.push_back(taskSecond);	
	}
}

// Zapis instancji do pliku
void ZapiszInstancjeDoPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, int numerInstancjiProblemu, string nameParam) {
	// Zmienna pliku docelowego
		ofstream file;
	
	// Utworzenie zmiennej pomocniczej w postaci nazwy pliku aby móc parametryzowaæ zapis danych
		string fileName = "instancje_" + nameParam + ".txt";
		file.open(fileName.c_str());	
	
	if(file.is_open()) {
		file << "**** " << numerInstancjiProblemu << " ****" << endl;
		
		// Obliczenie iloœci zadañ w otrzymanym wektorze
			int iloscZadan = listaZadan.size();
		
		// Posortowanie wektora po wartoœci ID aby mieæ obok siebie operacje z tego samego zadania
			SortujZadaniaPoID(listaZadan);
		
		// Przypisanie do pliku iloœci zadañ w instancji
			file << iloscZadan / 2 << endl;
		
		// Uzupe³nienie pliku o wygenerowane czasy pracy
			for(int i = 0; i < iloscZadan; i += 2) {
				// Dodanie linii z opisem zadania do pliku instancji
					if(listaZadan[i]->part == 0) { // Pod i mamy zadanie bêd¹ce Part I
						file << listaZadan[i]->duration << ":" << listaZadan[i]->anotherPart->duration << ":" << listaZadan[i]->assigment << ":" << listaZadan[i]->anotherPart->assigment << ";" << endl;
					} else {
						file << listaZadan[i]->anotherPart->duration << ":" << listaZadan[i]->duration << ":" << listaZadan[i]->anotherPart->assigment << ":" << listaZadan[i]->assigment << ";" << endl;
					}
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
void WczytajDaneZPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, int &numerInstancjiProblemu, string nameParam) {
	FILE *file;
	string fileName = "instancje_" + nameParam + ".txt";
	file = fopen(fileName.c_str(), "r");
	
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
					Task * taskFirst = new Task;
					Task * taskSecond = new Task;
					
				// Powi¹zanie zadañ
					taskFirst->anotherPart = taskSecond;
					taskSecond->anotherPart = taskFirst;
					
				// Ustawienie wartoœci zadañ
					taskFirst->ID = i + 1;
					taskFirst->assigment = assigmentFirstPart;
					taskFirst->duration = durationFirstPart;
					taskFirst->endTime = 0;
					taskFirst->part = 0;
					
					taskSecond->ID = i + 1;
					taskSecond->assigment = assigmentSecondPart;
					taskSecond->duration = durationSecondPart;
					taskSecond->endTime = 0;
					taskSecond->part = 1;
					
				// Dodanie zadania do wektora zadañ
					listaZadan.push_back(taskFirst);
					listaZadan.push_back(taskSecond);
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
vector<Task*> GeneratorLosowy(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor) {
	srand(time(NULL));
	
	// Utworzenie kopii zadañ aby móc tworzyæ swoje rozwi¹zanie
		vector<Task*> zadaniaLokalne;
		KopiujDaneOperacji(listaZadan, zadaniaLokalne);
	
	// Zmienne u¿ywane w przebiegu pracy Generatora Losowego
		int iloscZadan = listaZadan.size() / 2;	// Iloœæ zadañ (iloœæ operacji / 2)
		Task * currentTask = NULL; // Zmmienna operacyjna aby uproœciæ zapis
		int numerPrzerwaniaFirstProcessor = 0; // Numer aktualnego przerwania na procesorze pierwszym
		int numerPrzerwaniaSecondProcessor = 0; // Numer aktualnego przerwania na procesorze drugim
		int count = 0; // Licznik przeliczonych ju¿ zadañ
		int najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime; // Czas momentu ROZPOCZÊCIA przerwania na procesorze pierwszym
		int najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime; // Czas momentu ROZPOCZÊCIA przerwania na procesorze drugim 
		int timeFirstProcessor = 0; // Zmienna czasowa - procesor pierwszy
		int timeSecondProcessor = 0; // Zmienna czasowa - procesor drugi
		int maxCount = 2 * iloscZadan; // Iloœæ koniecznych edycji w zadaniach (part I + part II w ka¿dym zadaniu)
		int listaPrzerwanFirstProcessorSize = listaPrzerwanFirstProcessor.size(); // Iloœæ przerwañ dla pierwszego procesora - aby nie liczyæ za ka¿dym razem tej wartoœci
		int listaPrzerwanSecondProcessorSize = listaPrzerwanSecondProcessor.size(); // Iloœæ przerwañ dla drugiej maszyny - podobnie jak wy¿ej, unikamy niepotrzebnego, wielokrotnego liczenia tej wartoœci
		int taskID = 0; // Numer zadania
		int pozycja = 0; // Numer aktualnie rozpatrywanego zadania (losowa wartoœæ z z przedzia³u 0 - ilosc zadan*2)
		
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
			// Losujemy pozycjê w tablicy zadañ
				pozycja = rand() % maxCount;
			
			// Sprawdzamy zadanie odpowiadaj¹ce wylosowanej pozycji nie by³o ju¿ przypadkiem u¿yte ca³e - w takim przypadku losujemy na nowo numer pozycji w tablicy
				taskID = zadaniaLokalne[pozycja]->ID - 1;
				if(firstPart[taskID] && secondPart[taskID])
					continue; // Skok do kolejnej iteracji
				
				if(DEBUG) {
					debugFile << "Wylosowano = " << pozycja << " Zadanie nr " << taskID << " (Part " << zadaniaLokalne[pozycja]->part + 1 << ")"
					<< " Parametry zadania = " << zadaniaLokalne[pozycja]->assigment << "|" << zadaniaLokalne[pozycja]->duration << "|" << zadaniaLokalne[pozycja]->endTime
					<< " Parametry komplementarnej czêœci = " << zadaniaLokalne[pozycja]->anotherPart->assigment << "|" << zadaniaLokalne[pozycja]->anotherPart->duration << "|" << zadaniaLokalne[pozycja]->anotherPart->endTime
					<< " czasy: " << timeFirstProcessor << "|" << timeSecondProcessor << " przerwy: " << najblizszyMaintenanceFirstProcessor << "|" << najblizszyMaintenanceSecondProcessor << endl;
				}

			// Zadanie nie by³o jeszcze u¿ywane
				if(!firstPart[taskID]) {
					// Sprawdzamy typ zadania - je¿eli jest zero to podstawiamy pod zmienn¹ pomocnicz¹
						if(zadaniaLokalne[pozycja]->part == 0) {
							currentTask = zadaniaLokalne[pozycja];
						} else { // Je¿eli nie - konieczne jest podstawienie czêœci komplementarnej wylosowanego zadania
							currentTask = zadaniaLokalne[pozycja]->anotherPart;
						}
						
					// Sprawdzamy czy zadanie powinno trafiæ na maszynê 0
						if(currentTask->assigment == 0) {
							// Sprawdzamy czy zadanie uda siê ustawiæ przed najblizszym maintenance na maszynie 
								if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
									// Ustawiamy czas na maszynie pierwszej
										timeFirstProcessor += currentTask->duration;
							
									if(DEBUG) 
										debugFile << "Czas FM: " << timeFirstProcessor << endl;
								
									// Ustawiamy czas zakoñczenia Part I
										currentTask->endTime = timeFirstProcessor;
										
									// Ustawiamy ¿e zadanie zosta³o u¿yte (Part I)
										firstPart[taskID] = true;	
										
								} else { // Nie uda³o siê umieœciæ zadania przed przerw¹
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
											if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
												break;
									}
							
									// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeFirstProcessor (wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
										timeFirstProcessor += currentTask->duration;
									
										if(DEBUG)		
											debugFile << "I Czas FM " << timeFirstProcessor << endl;
									
									// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
										currentTask->endTime = timeFirstProcessor;
										
									// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
										firstPart[taskID] = true;									
								}
						
								// Zwiêkszamy iloœæ zadañ jakie przerobiliœmy
									count++;
									
						} else { // Przydzia³ zadania na maszynê nr 2
							// Sprawdzamy czy zadanie mo¿na umieœciæ przed maintenance najbli¿szym (je¿eli jest  on -1 to ju¿ nie wyst¹pi)
								if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
									// Ustawiamy czas na maszynie drugiej
										timeSecondProcessor += currentTask->duration;
									
										if(DEBUG)
											debugFile << "I Czas SM: " << timeSecondProcessor << endl;
									
									// Ustawiamy czas zakoñczenia zadania
										currentTask->endTime = timeSecondProcessor;
										
									// Ustawiamy ¿e zadanie zosta³o u¿yte (part I)
										firstPart[taskID] = true;
											
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
											if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
												break;
									}
									
									// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeSecondProcessor(wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
										timeSecondProcessor += currentTask->duration;
									
										if(DEBUG)		
											debugFile << "Czas SM " << timeSecondProcessor << endl;
									
									// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
										currentTask->endTime = timeSecondProcessor;
										
									// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
										firstPart[taskID] = true;									
								}
								
								// Zwiêkszamy iloœæ zadañ jakie przerobiliœmy
									count++;
						}
				} else {
				// PRZYDZIELAMY DRUG¥ CZÊŒÆ ZADANIA
					
					// Mog¹ wyst¹piæ problemy z zapêtleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartoœæ
						licznikOdwiedzonych[taskID]++;
						
					// Sprawdzamy typ zadania - je¿eli jest zero to podstawiamy pod zmienn¹ pomocnicz¹
						if(zadaniaLokalne[pozycja]->part == 1) {
							currentTask = zadaniaLokalne[pozycja];
						} else { // Je¿eli nie - konieczne jest podstawienie czêœci komplementarnej wylosowanego zadania
							currentTask = zadaniaLokalne[pozycja]->anotherPart;
						}
					
					// Sprawdzamy typ zadania
						if(currentTask->assigment == 1) { // Przydzia³ na drug¹ maszynê
							// Sprawdzamy czy czas na maszynie nie jest mniejszy od zakoñczenia siê pierwszej czêœci
							if(timeSecondProcessor < currentTask->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteœmy po raz x w pêtli
								if(licznikOdwiedzonych[taskID] >= MIN_TASK_COUNTER) {
									if(DEBUG)
										debugFile << "Przestawiono czas! M1" << endl;
									// Tworzymy pomocnicz¹ zmienn¹ odleg³oœci
										int minTime = INT_MAX;
										int tempTime = 0;
										
									// Resetujemy liczniki i patrzymy na odleg³oœci
										for(int i = 0; i < iloscZadan; i++) {
											licznikOdwiedzonych[i] = 0;
											
											if(!secondPart[i]) {
												int tempTime = currentTask->anotherPart->endTime - timeSecondProcessor;
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
								if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
									// Ustawiamy czas na maszynie pierwszej
										timeSecondProcessor += currentTask->duration;
									
									// Ustawiamy czas zakoñczenia zadania
										currentTask->endTime = timeSecondProcessor;
										
									// Ustawiamy ¿e zadanie zosta³o u¿yte (part II)
										secondPart[taskID] = true;	
								
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
											if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
												break;
									}
									
									// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeSecondProcessor (wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
										timeSecondProcessor += currentTask->duration;
											
									// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
										currentTask->endTime = timeSecondProcessor;
										
									// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
										secondPart[taskID] = true;
								}
							
								// Zwiêkszamy iloœæ zadañ jakie przerobiliœmy
									count++;
						} else {
							// Sprawdzamy czy czas na maszynie nie jest mniejszy od zakoñczenia siê pierwszej czêœci
							if(timeFirstProcessor < currentTask->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteœmy po raz x w pêtli
								if(licznikOdwiedzonych[taskID] >= MIN_TASK_COUNTER) {
									if(DEBUG)
										debugFile << "Przestawiono czas! M0" << endl;
										
									// Tworzymy pomocnicz¹ zmienn¹ odleg³oœci
										int minTime = INT_MAX;
										int tempTime = 0;
										
									// Resetujemy liczniki i patrzymy na odleg³oœci
										for(int i = 0; i < iloscZadan; i++) {
											licznikOdwiedzonych[i] = 0;
											
											if(!secondPart[i]) {
												tempTime = currentTask->anotherPart->endTime - timeFirstProcessor;
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
								if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
									// Ustawiamy czas na maszynie pierwszej
										timeFirstProcessor += currentTask->duration;
									
									// Ustawiamy czas zakoñczenia zadania
										currentTask->endTime = timeFirstProcessor;
										
									// Ustawiamy ¿e zadanie zosta³o u¿yte (part II)
										secondPart[taskID] = true;	
								
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
											if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
												break;
									}
									
									// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeSecondProcessor (wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
										timeFirstProcessor += currentTask->duration;
											
									// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
										currentTask->endTime = timeFirstProcessor;
										
									// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
										secondPart[taskID] = true;
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

// Odczyt danych zadañ na ekran
void OdczytDanychZadan(vector<Task*> &listaZadan) {
	// Przeliczenie iloœci operacji do zmienne pomocniczej aby nie liczyæ operacji w ka¿dej iteracji
	int size = listaZadan.size();
	
	// Przesortowanie listy zadañ aby mieæ obok siebie zadania z tym samym ID
		SortujZadaniaPoID(listaZadan);
	
	// Pêtla odczytu wartoœci zadañ
		for(int i = 0; i < size; i++) {
			cout << "--- ID: " << listaZadan[i]->ID << " (Part " << listaZadan[i]->part << ") przydzial: M" << listaZadan[i]->assigment << " duration = " << listaZadan[i]->duration << " --- zakonczenie = " << listaZadan[i]->endTime << " --- " << endl;
		}
}

// Tworzenie timeline dla obserwacji wyników pracy
void UtworzGraf(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, long int wynik, string nameParam) {
	int iloscZadan = listaZadan.size(); // Iloœæ zadañ w systemie
	int iloscPrzerwan = listaPrzerwan.size(); // Iloœæ okresów przestojów na maszynach
	
	ofstream file;
	string fileName = "index_" + nameParam + ".html";
	file.open(fileName.c_str());
	file << "<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" /><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
	file << "<title>OK - Wyniki pracy generatora</title></head><body><script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>";
	file << "<script type=\"text/javascript\">google.charts.load(\"current\", {packages:[\"timeline\"]});google.charts.setOnLoadCallback(drawChart);function drawChart() {";
	file << "var container = document.getElementById('example4.2');var chart = new google.visualization.Timeline(container);var dataTable = new google.visualization.DataTable();";
	file << "dataTable.addColumn({ type: 'string', id: 'Role' });dataTable.addColumn({ type: 'string', id: 'Name' });dataTable.addColumn({ type: 'number', id: 'Start' });dataTable.addColumn({ type: 'number', id: 'End' });dataTable.addRows([";
	
	int timeStart = 0;
	int timeStop = 0;

	// Zapisujemy do pliku nasze zadania
		for(int i = 0; i < iloscZadan; i++) {
			timeStop = listaZadan[i]->endTime;
			timeStart = timeStop - listaZadan[i]->duration;

			file << "[ 'M" << listaZadan[i]->assigment + 1 << "', 'Zadanie " << listaZadan[i]->ID << "', " << timeStart << ", " << timeStop << " ]," << endl;
		}
		
	// Zapis przerwañ
		for(int i = 0; i < iloscPrzerwan; i++) {
			timeStart = listaPrzerwan[i]->readyTime;
			timeStop = timeStart + listaPrzerwan[i]->duration;
			
			if(i + 1 == iloscPrzerwan) { // Ostatnia iteracja
				file << "[ 'M" << listaPrzerwan[i]->assigment + 1 << "', 'PRZERWANIE " << i + 1 << "', " << timeStart << ", " << timeStop << " ]]);" << endl;
				file << "var options = {timeline: { groupByRowLabel: true }};chart.draw(dataTable, options);}</script><div id=\"example4.2\" style=\"height: 200px;\"></div><br><div><span>Wartosc funkcji celu: " << wynik << "</span></div></body></html>" << endl;
			} else {
				file << "[ 'M" << listaPrzerwan[i]->assigment + 1 << "', 'PRZERWANIE " << i + 1 << "', " << timeStart << ", " << timeStop << " ]," << endl;
			}
		}
}

// Obliczanie wartoœci funkcji celu
long int ObliczFunkcjeCelu(vector<Task*> &lista) {
	int size = lista.size();
	long int sum = 0;
	
	for(int i = 0; i < size; i++) {
		sum += lista[i]->endTime;
	}
	
	return sum;
}

// Podzia³ struktury T na maszyny
template <class T>
void PodzielStrukturyNaMaszyny(vector<T*> &listaWejsciowa, vector<T*> &firstProcessor, vector<T*> &secondProcessor) {
	// Zmienna pomocnicza by skróciæ czas pracy (nie trzeba x razy liczyæ)
		int size = listaWejsciowa.size();

	//Sprawdzamy do jakiej maszyny przypisana jest struktura	
		for(int i = 0; i < size; i++) {
			T * operacja = listaWejsciowa[i];
			
			if(operacja->assigment == 0) {
				firstProcessor.push_back(operacja);
			} else {
				secondProcessor.push_back(operacja);
			}
		}	
}

// Obliczanie d³ugoœci Task / Maintenance list
template <class T>
long int ObliczDlugoscOperacji(vector<T*> &lista) {
	int size = lista.size();
	long int sum = 0;
	
	for(int i = 0; i < size; i++) {
		sum += lista[i]->duration;
	}
	
	return sum;
}

// Zapis wyników do pliku tekstowego
void ZapiszWynikiDoPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor, long int firstSolutionValue, int numerInstancjiProblemu, string nameParam) {
	ofstream file;
	string fileName = "wyniki_" + nameParam + ".txt";
	file.open(fileName.c_str());
	
	if(file.is_open()) {
			long int optimalSolutionValue = ObliczFunkcjeCelu(listaZadan); // Wartoœæ funkcji celu dla rozwi¹zania optymalnego
			vector<Task*> taskFirstProcessor, taskSecondProcessor; // Wektory dla podzia³u zadañ na maszyny
			int taskFirstProcessorSize; // Iloœæ zadañ na pierwszym procesorze
			int taskSecondProcessorSize; // Iloœæ zadañ na drugim procesorze
			int numerPrzerwania = 0; // Numer aktualnie rozpatrywanego przerwania
			int najblizszyMaintenance = -1; // Czas momentu ROZPOCZÊCIA przerwania
			int processorTime = 0; // Czas procesora
			int count = 0; // Iloœæ operacji które zosta³y ju¿ umieszczone w pliku wynikowym
			int maxCount; // Iloœæ operacji które trzeba umieœciæ (liczba operacji + przerwania)
			int taskPoint = 0; // Zmienna wskazuj¹ca aktualnie rozpatrywane zadanie z listy operacji
			int countIldeFirstProcessor = 0; // Licznik okresów bezczynnoœci dla maszyny pierwszej
			int countIldeSecondProcessor = 0; // Licznik okresów bezczynnoœci dla maszyny drugiej
			int ildeTimeFirstProcessor = 0; // Ogólny czas bezczynnoœci na maszynie pierwszej
			int ildeTimeSecondProcessor = 0; // Ogólny czas bezczynnoœci na maszynie drugiej
			
		// Podzielenie listy zadañ na maszyny i przypisanie iloœci do zmiennych pomocniczych
			PodzielStrukturyNaMaszyny<Task>(listaZadan, taskFirstProcessor, taskSecondProcessor);
			taskFirstProcessorSize = taskFirstProcessor.size();
			taskSecondProcessorSize = taskSecondProcessor.size();
		
		// Sortowanie zadañ
			SortujZadaniaPoEndTime(taskFirstProcessor);
			SortujZadaniaPoEndTime(taskSecondProcessor);
			
		// Przypisanie numeru instancji
			file << "**** " << numerInstancjiProblemu << " ****" << endl;
		
		// Przypisanie wartoœci optymalnej oraz wartoœci pocz¹tkowej wygenerowanej przez generator losowy
			file << optimalSolutionValue << ", " << firstSolutionValue << endl;
		
		// Przypisanie do pliku utworzonej instancji
			file << "M1:";
			
			if(listaPrzerwanFirstProcessor.size() > 0)
				najblizszyMaintenance = listaPrzerwanFirstProcessor[0]->readyTime; 
			maxCount = taskFirstProcessorSize + listaPrzerwanFirstProcessor.size(); // maxCount dla pierwszej maszyny
			while(count < maxCount) {
				if(taskPoint >= 0 && processorTime == (taskFirstProcessor[taskPoint]->endTime - taskFirstProcessor[taskPoint]->duration)) {
					// Zapis do pliku
						file << "op" << taskFirstProcessor[taskPoint]->part + 1 << "_" << taskFirstProcessor[taskPoint]->ID << ", " << taskFirstProcessor[taskPoint]->endTime - taskFirstProcessor[taskPoint]->duration
							 << ", " << taskFirstProcessor[taskPoint]->duration << "; ";
					
					// Przestawienie czasu
						processorTime = taskFirstProcessor[taskPoint]->endTime;
					
					// Skok do kolejnej wartoœci
						taskPoint++;
				
					// Musimy sprawdziæ czy nie wychodzimy poza zakres
						if(taskPoint >= taskFirstProcessorSize) {
							taskPoint = -1;
						}
					
					// Zwiêkszamy licznik odwiedzonych operacji
						count++;
				} else if (processorTime == najblizszyMaintenance) {
					// Zapis do pliku
						file << "maint" << numerPrzerwania + 1 << "_M1, " << listaPrzerwanFirstProcessor[numerPrzerwania]->readyTime << ", "
							 << listaPrzerwanFirstProcessor[numerPrzerwania]->duration << "; ";
					
					// Przestawienie czasu
						processorTime = listaPrzerwanFirstProcessor[numerPrzerwania]->readyTime + listaPrzerwanFirstProcessor[numerPrzerwania]->duration;
					
					// Konieczne jest sprawdzenie czy nie wychodzimi poza zakres
						numerPrzerwania++;
						if(numerPrzerwania >= listaPrzerwanFirstProcessor.size()) {
							najblizszyMaintenance = -1;
						} else {
							najblizszyMaintenance = listaPrzerwanFirstProcessor[numerPrzerwania]->readyTime;
						}
					
					// Zwiêkszamy licznik odwiedzonych operacji
						count++;
				} else { // Bezczynnoœæ
				
					// Sprawdzamy które zdarzenie bêdzie wczeœniej - wyst¹pienie zadania czy maintenance
						int minTime = INT_MAX;
						if(taskPoint >= 0) {
							int temp =  taskFirstProcessor[taskPoint]->endTime - taskFirstProcessor[taskPoint]->duration - processorTime;
							if(temp < minTime)
								minTime = temp;
						} 
						if(((najblizszyMaintenance - processorTime) < minTime) && najblizszyMaintenance > -1) {
							minTime = najblizszyMaintenance - processorTime;
						}
					
					// Zapis do pliku danych o bezczynnoœci	
						file << "ilde" << countIldeFirstProcessor + 1 << "_M1, " << processorTime << ", " << minTime << "; ";
						countIldeFirstProcessor++;

					// Dodanie do ogólnego licznika bezczynnoœci zapisanego czasu
						ildeTimeFirstProcessor += minTime;
					
					// Przestawienie czasu maszyny
						processorTime += minTime;					 
				}				
			}
			
			file << endl << "M2:";
			
			// Zerowanie zmiennych uniwersalnych
			taskPoint = 0;
			count = 0;
			processorTime = 0;
			numerPrzerwania = 0;
			if(listaPrzerwanSecondProcessor.size() > 0)
				najblizszyMaintenance = listaPrzerwanSecondProcessor[0]->readyTime;
			else
				najblizszyMaintenance = -1;
			maxCount = taskSecondProcessorSize + listaPrzerwanSecondProcessor.size(); // maxCount dla drugiej maszyny
			while(count < maxCount) {
				if(taskPoint >= 0 && processorTime == (taskSecondProcessor[taskPoint]->endTime - taskSecondProcessor[taskPoint]->duration)) {
					// Zapis do pliku
						file << "op" << taskSecondProcessor[taskPoint]->part + 1 << "_" << taskSecondProcessor[taskPoint]->ID << ", " << taskSecondProcessor[taskPoint]->endTime - taskSecondProcessor[taskPoint]->duration
							 << ", " << taskSecondProcessor[taskPoint]->duration << "; ";
					
					// Przestawienie czasu
						processorTime = taskSecondProcessor[taskPoint]->endTime;
						
					// Skok do kolejnej wartoœci
						taskPoint++;
				
					// Musimy sprawdziæ czy nie wychodzimy poza zakres
						if(taskPoint >= taskSecondProcessorSize) {
							taskPoint = -1;
						}
					
					// Zwiêkszamy licznik odwiedzonych operacji
						count++;
				} else if (processorTime == najblizszyMaintenance) {
					// Zapis do pliku
						file << "maint" << numerPrzerwania + 1 << "_M1, " << listaPrzerwanSecondProcessor[numerPrzerwania]->readyTime << ", "
							 << listaPrzerwanSecondProcessor[numerPrzerwania]->duration << "; ";
					
					// Przestawienie czasu
						processorTime = listaPrzerwanSecondProcessor[numerPrzerwania]->readyTime + listaPrzerwanSecondProcessor[numerPrzerwania]->duration;
					
					// Konieczne jest sprawdzenie czy nie wychodzimi poza zakres
						numerPrzerwania++;
						if(numerPrzerwania >= listaPrzerwanSecondProcessor.size()) {
							najblizszyMaintenance = -1;
						} else {
							najblizszyMaintenance = listaPrzerwanSecondProcessor[numerPrzerwania]->readyTime;
						}
					
					// Zwiêkszamy licznik odwiedzonych operacji
						count++;
				} else { // Bezczynnoœæ
					// Sprawdzamy które zdarzenie bêdzie wczeœniej - wyst¹pienie zadania czy maintenance
						int minTime = INT_MAX;
						if(taskPoint >= 0) {
							int temp =  taskSecondProcessor[taskPoint]->endTime - taskSecondProcessor[taskPoint]->duration - processorTime;
							if(temp < minTime)
								minTime = temp;
						}
						if(((najblizszyMaintenance - processorTime) < minTime) && najblizszyMaintenance > -1) {
							minTime = najblizszyMaintenance - processorTime;
						}
					
					// Zapis do pliku danych o bezczynnoœci	
						file << "ilde" << countIldeSecondProcessor + 1 << "_M2, " << processorTime << ", " << minTime << "; ";
						
					// Inkrementacja numeru przerwania
						countIldeSecondProcessor++;

					// Dodanie do ogólnego licznika bezczynnoœci zapisanego czasu
						ildeTimeSecondProcessor += minTime;
					
					// Przestawienie czasu maszyny
						processorTime += minTime;					 
				}				
			}
			
			// Dopisanie wartoœci sum
				file << endl << listaPrzerwanFirstProcessor.size() << ", " << ObliczDlugoscOperacji<Maintenance>(listaPrzerwanFirstProcessor) << endl 
					 << listaPrzerwanSecondProcessor.size() << "," << ObliczDlugoscOperacji<Maintenance>(listaPrzerwanSecondProcessor) << endl
					 << countIldeFirstProcessor << ", " << ildeTimeFirstProcessor << endl 
					 << countIldeSecondProcessor << ", " << ildeTimeSecondProcessor << endl << "*** EOF ***";
					 
			// Czyszczenie pamiêci operacyjnej
				taskFirstProcessor.clear();
				taskSecondProcessor.clear();
	}
}

// Mutacja jednego rozwi¹zania z za³o¿eniem podzielenia operacji na dwie maszyny
vector<Task*> Mutacja(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor) {
	// Zmienne operacyjne
		srand(time(NULL)); // Odœwie¿enie randoma
		vector<Task*> taskListFirstProcessor, taskListSecondProcessor; // Wektory dla podzia³u zadañ na maszyny
			
	// Podzielenie listy zadañ na maszyny i przypisanie iloœci do zmiennych pomocniczych
			PodzielStrukturyNaMaszyny<Task>(listaZadan, taskListFirstProcessor, taskListSecondProcessor);
		
		int iloscZadan = taskListFirstProcessor.size();
		int firstTaskPosition = 0; // Random - pozycja pierwszego zadania
		int secondTaskPosition = 0;  // Random - pozycja drugiego zadania
		
		int processor = rand() % 2; // Random - wybór maszyny któr¹ dotyczyæ bêdzie mutacja
		int timeFirstProcessor = 0; // Czas na maszynie pierwszej
		int timeSecondProcessor = 0; // Czas na maszynie drugiej
		Task * currentTaskFirstProcessor = NULL; // Zadanie na maszynie pierwszej
		Task * currentTaskSecondProcessor = NULL; // Zadanie na maszynie drugiej
		int numerPrzerwaniaFirstProcessor = 0; // Numer aktualnego przerwania na procesorze pierwszym
		int numerPrzerwaniaSecondProcessor = 0; // Numer aktualnego przerwania na procesorze drugim
		int countTask = 0; // Licznik sprawdzionych ju¿ zadañ
		int najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime; // Czas momentu ROZPOCZÊCIA przerwania na procesorze pierwszym
		int najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime; // Czas momentu ROZPOCZÊCIA przerwania na procesorze drugim 
		int listaPrzerwanFirstProcessorSize = listaPrzerwanFirstProcessor.size(); // Iloœæ przerwañ dla pierwszego procesora - aby nie liczyæ za ka¿dym razem tej wartoœci
		int listaPrzerwanSecondProcessorSize = listaPrzerwanSecondProcessor.size(); // Iloœæ przerwañ dla drugiej maszyny - podobnie jak wy¿ej, unikamy niepotrzebnego, wielokrotnego liczenia tej wartoœci
		int taskIDFirstProcessor = 0; // Numer zadania na maszynie pierwszej
		int taskIDSecondProcessor = 0; // Numer zadania na maszynie drugiej
		
		int iteratorFP = 0; // Numer rozpatrywanego zadania na maszynie pierwszej
		int iteratorSP = 0; // Numer aktualnie rozpatrywanego zadania na maszynie drugiej
		
	// Wektory kolejnoœci zadañ (ID)
		int *taskOrderFirstProcessor = new int[iloscZadan];
		int *taskOrderSecondProcessor = new int[iloscZadan];
	
	// Licznik odwiedzonych aby nie zapêtliæ siê w czasach bezczynnoœci
		int *licznikOdwiedzonych = new int[iloscZadan];
	
	// Pomocnicze tablice part I & part II aby przyspieszyæ proces sprawdzania
		bool *firstPart = new bool[iloscZadan];
		bool *secondPart = new bool[iloscZadan];
	
	// Sortowanie zadañ w listach
		SortujZadaniaPoEndTime(taskListFirstProcessor);
		SortujZadaniaPoEndTime(taskListSecondProcessor);	
		
	// Tworzymy wektor kolejnoœci zadañ i zerujemy tablice pomocnicze
		for(int i = 0; i < iloscZadan; i++) {
			taskOrderFirstProcessor[i] = taskListFirstProcessor[i]->ID - 1;
			taskOrderSecondProcessor[i] = taskListSecondProcessor[i]->ID - 1;
			firstPart[i] = false;
			secondPart[i] = false;
			licznikOdwiedzonych[i] = 0;
		}
		
		if(DEBUG) {
			debugFile << "Przed mutacj¹:" << endl;
			for(int i = 0; i < iloscZadan; i++) {
				debugFile << taskOrderFirstProcessor[i] << " | " << taskOrderSecondProcessor[i] << endl;
			}
		}
		
	// Pêtla losowania i zmiany kolejnoœci zadañ
		while(true) {
			// Losujemy wartoœci
				firstTaskPosition = (int)(rand() / (RAND_MAX + 1.0) * iloscZadan);
				secondTaskPosition = (int)(rand() / (RAND_MAX + 1.0) * iloscZadan);	
			
			if(processor == 0) { // Przestawienie kolejnoœci zadañ dotyczy maszyny pierwszej
				// Sprawdzamy czy te zadania mo¿emy mutowaæ (za³o¿enie - przestawiamy tylko zadania z tym samym wskaŸnikiem czêœci Part)
					if(secondTaskPosition != firstTaskPosition && taskListFirstProcessor[firstTaskPosition]->part == taskListFirstProcessor[secondTaskPosition]->part) {						
					// Zamiana kolejnoœci zadañ w liœcie
						int temp = taskOrderFirstProcessor[firstTaskPosition];
						taskOrderFirstProcessor[firstTaskPosition] = taskOrderFirstProcessor[secondTaskPosition];
						taskOrderFirstProcessor[secondTaskPosition] = temp;
											
						break;
					} else {
						continue; // Skok do kolejnej iteracji i nowego losowania
					}
			} else { // Zmiany kolejnoœci dla maszynie nr 2
				if(secondTaskPosition != firstTaskPosition && taskListSecondProcessor[firstTaskPosition]->part == taskListSecondProcessor[secondTaskPosition]->part) {						
					// Zamiana kolejnoœci zadañ w liœcie
						int temp = taskOrderSecondProcessor[firstTaskPosition];
						taskOrderSecondProcessor[firstTaskPosition] = taskOrderSecondProcessor[secondTaskPosition];
						taskOrderSecondProcessor[secondTaskPosition] = temp;
							
						break;
					} else {
						continue; // Skok do kolejnej iteracji i nowego losowania
					}
			}
		}
		
		if(DEBUG) {
			debugFile << "PO mutacji:" << endl;
			for(int i = 0; i < iloscZadan; i++) {
				debugFile << taskOrderFirstProcessor[i] << " | " << taskOrderSecondProcessor[i] << endl;
			}
		}
		
		// Posortowanie zadañ wed³ug ID - aby ³atwo odwo³ywaæ siê poprzez wartoœæ z tablicy kolejnoœci zadañ
			SortujZadaniaPoID(taskListFirstProcessor);
			SortujZadaniaPoID(taskListSecondProcessor);
	
	// Pêtla ustawiaj¹ca nowe czasy zakoñczenia dla naszych operacji
		while(countTask < iloscZadan*2) {
			// Sprawdzamy czy nie wyskoczyliœmy na maszynie pierwszej poza zakres vektora
			if(iteratorFP < iloscZadan) {
				// Przypisujemy zadanie do zmiennej pomocniczej
					taskIDFirstProcessor = taskOrderFirstProcessor[iteratorFP];
					currentTaskFirstProcessor = taskListFirstProcessor[taskIDFirstProcessor];
					
				// Sprawdzamy part zadania - je¿eli jest to I to mo¿na wstawiaæ od razu, je¿eli II trzeba poczekaæ a¿ zostanie wstawiona czêœæ I na maszynie drugiej
					if(currentTaskFirstProcessor->part == 0) {
						// Sprawdzamy czy zadanie uda siê ustawiæ przed najblizszym maintenance na maszynie 
							if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
								// Ustawiamy czas na maszynie pierwszej
									timeFirstProcessor += currentTaskFirstProcessor->duration;
						
								if(DEBUG) 
									debugFile << "Czas FM: " << timeFirstProcessor << endl;
							
								// Ustawiamy czas zakoñczenia Part I
									currentTaskFirstProcessor->endTime = timeFirstProcessor;
									
								// Ustawiamy ¿e zadanie zosta³o u¿yte (Part I)
									firstPart[taskIDFirstProcessor] = true;	
									
							} else { // Nie uda³o siê umieœciæ zadania przed przerw¹
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
											if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
												break;
									}
							
									// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeFirstProcessor (wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
										timeFirstProcessor += currentTaskFirstProcessor->duration;
									
										if(DEBUG)		
											debugFile << "I Czas FM " << timeFirstProcessor << endl;
									
									// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
										currentTaskFirstProcessor->endTime = timeFirstProcessor;
										
									// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
										firstPart[taskIDFirstProcessor] = true;									
							}
							
							// Zwiêkszamy iloœæ poprawionych zadañ
								countTask++;
							
							// Przestawiamy iterator na pierwszej maszynie
								iteratorFP++;
								
					} else if(firstPart[taskIDFirstProcessor]) { // Sprawdzamy czy zosta³a wstawiona czêœæ I zadania (ma ono part == 1)
						// Mog¹ wyst¹piæ problemy z zapêtleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartoœæ
							licznikOdwiedzonych[taskIDFirstProcessor]++;
							
							if(timeFirstProcessor < currentTaskFirstProcessor->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteœmy po raz x w pêtli
								if(licznikOdwiedzonych[taskIDFirstProcessor] >= MIN_TASK_COUNTER) {
									// Tworzymy pomocnicz¹ zmienn¹ odleg³oœci
										int minTime = INT_MAX;
										int tempTime = 0;
										
									// Resetujemy liczniki i patrzymy na odleg³oœci
										for(int i = 0; i < iloscZadan; i++) {
											licznikOdwiedzonych[i] = 0;
											
											if(!secondPart[i]) {
												tempTime = currentTaskFirstProcessor->anotherPart->endTime - timeFirstProcessor;
												if(tempTime < minTime)
													minTime = tempTime;
											}
										}
										
									// Przestawiamy czas na maszynie
										timeFirstProcessor += minTime;
										
								}
							} else {
								// Zadanie mo¿na umieœciæ
								// Sprawdzamy czy zadanie mo¿na umieœciæ przed maintenance najbli¿szym (je¿eli jest  on -1 to ju¿ nie wyst¹pi)
									if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
										// Ustawiamy czas na maszynie pierwszej
											timeFirstProcessor += currentTaskFirstProcessor->duration;
										
										// Ustawiamy czas zakoñczenia zadania
											currentTaskFirstProcessor->endTime = timeFirstProcessor;
											
										// Przestawiamy iterator oraz iloœæ zedytowanych zadañ
											iteratorFP++;
											countTask++;
											
										// Zaznaczamy zadanie jako wykonane w pe³ni
											secondPart[taskIDFirstProcessor] = true;	
								
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
												if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
													break;
										}
									
										// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeSecondProcessor (wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
											timeFirstProcessor += currentTaskFirstProcessor->duration;
												
										// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
											currentTaskFirstProcessor->endTime = timeFirstProcessor;
											
										// Przestawiamy iterator oraz licznik edycji
											iteratorFP++;
											countTask++;
										
										// Zaznaczamy zadanie jako wykonane w pe³ni
											secondPart[taskIDFirstProcessor] = true;
									}	
							}
					}
			}
			
			// Zadania na drugim procesorze
			
			if(iteratorSP < iloscZadan) {
				// Przypisujemy zadanie do zmiennej pomocniczej
					taskIDSecondProcessor = taskOrderSecondProcessor[iteratorSP];
					currentTaskSecondProcessor = taskListSecondProcessor[taskIDSecondProcessor];
					
				// Sprawdzamy part zadania - je¿eli jest to I to mo¿na wstawiaæ od razu, je¿eli II trzeba poczekaæ a¿ zostanie wstawiona czêœæ I na maszynie pierwszej
					if(currentTaskSecondProcessor->part == 0) {
						// Sprawdzamy czy zadanie uda siê ustawiæ przed najblizszym maintenance na maszynie 
							if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
								// Ustawiamy czas na maszynie pierwszej
									timeSecondProcessor += currentTaskSecondProcessor->duration;
						
								// Ustawiamy czas zakoñczenia Part I
									currentTaskSecondProcessor->endTime = timeSecondProcessor;
									
								// Ustawiamy ¿e zadanie zosta³o u¿yte (Part I)
									firstPart[taskIDSecondProcessor] = true;	
									
							} else { // Nie uda³o siê umieœciæ zadania przed przerw¹
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
											if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
												break;
									}
							
									// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeSecondProcessor (wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
										timeSecondProcessor += currentTaskSecondProcessor->duration;
									
									// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
										currentTaskSecondProcessor->endTime = timeSecondProcessor;
										
									// Zaznaczamy w tablicy pomocniczej ¿e czêœæ pierwsza zadania by³a u¿yta
										firstPart[taskIDSecondProcessor] = true;									
							}
							
							// Zwiêkszamy iloœæ poprawionych zadañ
								countTask++;
							
							// Przestawiamy iterator na pierwszej maszynie
								iteratorSP++;
								
					} else if(firstPart[taskIDSecondProcessor]) { // Sprawdzamy czy zosta³a wstawiona czêœæ I zadania (ma ono part == 1)
						// Mog¹ wyst¹piæ problemy z zapêtleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartoœæ
							licznikOdwiedzonych[taskIDSecondProcessor]++;
							
							if(timeSecondProcessor < currentTaskSecondProcessor->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteœmy po raz x w pêtli
								if(licznikOdwiedzonych[taskIDSecondProcessor] >= MIN_TASK_COUNTER) {
									// Tworzymy pomocnicz¹ zmienn¹ odleg³oœci
										int minTime = INT_MAX;
										int tempTime = 0;
										
									// Resetujemy liczniki i patrzymy na odleg³oœci
										for(int i = 0; i < iloscZadan; i++) {
											licznikOdwiedzonych[i] = 0;
											
											if(!secondPart[i]) {
												tempTime = currentTaskSecondProcessor->anotherPart->endTime - timeSecondProcessor;
												if(tempTime < minTime)
													minTime = tempTime;
											}
										}
										
									// Przestawiamy czas na maszynie
										timeSecondProcessor += minTime;
										
								}
							} else {
								// Zadanie mo¿na umieœciæ
								// Sprawdzamy czy zadanie mo¿na umieœciæ przed maintenance najbli¿szym (je¿eli jest  on -1 to ju¿ nie wyst¹pi)
									if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
										// Ustawiamy czas na maszynie pierwszej
											timeSecondProcessor += currentTaskSecondProcessor->duration;
										
										// Ustawiamy czas zakoñczenia zadania
											currentTaskSecondProcessor->endTime = timeSecondProcessor;
											
										// Przestawiamy iterator oraz iloœæ zedytowanych zadañ
											iteratorSP++;
											countTask++;
											
										// Zaznaczamy zadanie jako wykonane w pe³ni
											secondPart[taskIDSecondProcessor] = true;	
								
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
												if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
													break;
										}
									
										// Po opuszczeniu pêtli mamy poprawn¹ wartoœæ w zmiennej timeSecondProcessor (wystarczy zwiêkszyæ j¹ o d³ugoœæ zadania)
											timeSecondProcessor += currentTaskSecondProcessor->duration;
												
										// Ustawiamy zmienn¹ czasow¹ zakoñczenia zadania
											currentTaskSecondProcessor->endTime = timeSecondProcessor;
											
										// Przestawiamy iterator oraz licznik edycji
											iteratorSP++;
											countTask++;
										
										// Zaznaczamy zadanie jako wykonane w pe³ni
											secondPart[taskIDSecondProcessor] = true;
									}	
							}
					}
			}
		}
	
	// Dopisanie zadañ ze zmienionymi wartoœciami
		for(int i = 0; i < iloscZadan; i++) {
			taskListFirstProcessor.push_back(taskListSecondProcessor[i]);
		}
		
	// Czyszczenie pamiêci
		delete firstPart;
		delete secondPart;
		delete licznikOdwiedzonych;
	
	return taskListFirstProcessor;
}

void Turniej(vector< vector<Task*> > &solutionsList) {
	srand(time(NULL));
	// Przeliczenie rozmiaru otrzymanej struktury listy rozwi¹zañ
		int size = solutionsList.size();
	
	// Utworzenie struktry pomocniczej = tabeli przegranych oraz tabeli z wartoœciami funkcji celu
		int *solutionsValue = new int[size];
		bool *looserSolution = new bool[size];
		
	// Uzupe³niami wartoœci w tabelach
		for(int i = 0; i < size; i++) {
			looserSolution[i] = false;
			solutionsValue[i] = ObliczFunkcjeCelu(solutionsList[i]);
		}
	
	// Turniej - wracamy do iloœci wejœciowego rozmiaru instancji
		int toKill = size - INSTANCE_SIZE;
		int first, second;
		
		cout << "Kill = " << toKill << endl;
		toKill = 1;
		
	// Pêtla operacyjna
		while(toKill > 0) {
			first = (int)(rand() / (RAND_MAX + 1.0) * size);
			second = (int)(rand() / (RAND_MAX + 1.0) * size);
			
			cout << "First = " << first << " second =" << second << endl;
			
			if(first != second && !looserSolution[first] && !looserSolution[second]) {
				// Sprawdzamy które z rozwi¹zañ ma mniejsz¹ wartoœæ funkcji celu
				if(solutionsValue[first] < solutionsValue[second])
					looserSolution[second] = true;
				else
					looserSolution[first] = true;
				cout << "Warunek" << endl;
				toKill--;
			} else
				continue; // Ponawiamy iteracjê - albo to samo zadanie, albo wylosowano rozwi¹zanie które odpad³o
		}
	
	// Usuniêcie wykluczonych rozwi¹zañ
		for(int i = 0; i < size; i++) {
			cout << solutionsValue[i] << " = " << looserSolution[i] << " ";
		}
}

void KopiujDaneOperacji(vector<Task*> &listaWejsciowa, vector<Task*> &listaWyjsciowa) {
	// Zmienna pomocnicza by skróciæ czas pracy (nie trzeba x razy liczyæ)
		int size = listaWejsciowa.size();
		
		SortujZadaniaPoID(listaWejsciowa);

	//Sprawdzamy do jakiej maszyny przypisana jest struktura	
		for(int i = 0; i < size; i += 2) {
			Task *operacja = new Task;
			Task *operacjaDruga = new Task;
			operacja->ID = listaWejsciowa[i]->ID;
			operacja->assigment = listaWejsciowa[i]->assigment;
			operacja->duration = listaWejsciowa[i]->duration;
			operacja->endTime = listaWejsciowa[i]->endTime;
			operacja->part = listaWejsciowa[i]->part;
			operacja->anotherPart = operacjaDruga;
			
			operacjaDruga->anotherPart = operacja;
			operacjaDruga->ID = listaWejsciowa[i]->anotherPart->ID;
			operacjaDruga->assigment = listaWejsciowa[i]->anotherPart->assigment;
			operacjaDruga->duration = listaWejsciowa[i]->anotherPart->duration;
			operacjaDruga->endTime = listaWejsciowa[i]->anotherPart->endTime;
			operacjaDruga->part = listaWejsciowa[i]->anotherPart->part;
			
			listaWyjsciowa.push_back(operacja);
			listaWyjsciowa.push_back(operacjaDruga);
		}	
}

int main() {
	debugFile.open("debug.txt");
	int rozmiarInstancji = INSTANCE_SIZE;
	int numerInstancjiProblemu = INSTANCE_NUMBER;

	// Utworzenie wektora na n zadañ
		vector<Task*> zadania;

	// Wektor przerwañ pracy na maszynach
		vector<Maintenance*> listaPrzerwan; 

	// Wygenerowanie zadañ
		GeneratorInstancji(zadania, rozmiarInstancji, LOWER_TIME_TASK_LIMIT, UPPER_TIME_TASK_LIMIT);

	// Wygenerowanie przerwañ	
		GeneratorPrzestojow(listaPrzerwan, MAINTENANCE_FIRST_PROCESSOR, MAINTENANCE_SECOND_PROCESSOR, LOWER_TIME_MAINTENANCE_LIMIT, UPPER_TIME_MAINTENANCE_LIMIT, LOWER_READY_TIME_MAINTENANCE_LIMIT, UPPER_READY_TIME_MAINTENANCE_LIMIT);
//		OdczytPrzerwan(listaPrzerwan);

	// Zapis danych do pliku
		string nameParam;
		stringstream ss;
    	ss << numerInstancjiProblemu;
    	ss >> nameParam; // Parametr przez stringstream, funkcja to_string odmówi³a pos³uszeñstwa
		ZapiszInstancjeDoPliku(zadania, listaPrzerwan, numerInstancjiProblemu, nameParam);

	// Wczytanie danych z pliku
//		WczytajDaneZPliku(listaZadan, listaPrzerwan, numerInstancjiProblemu, nameParam);
		
		vector<Maintenance*> przerwaniaFirstProcessor;
		vector<Maintenance*> przerwaniaSecondProcessor;
		SortujPrzerwania(listaPrzerwan);
		PodzielStrukturyNaMaszyny<Maintenance>(listaPrzerwan, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
		
		vector<Task*> listaZadan = GeneratorLosowy(zadania, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
//		OdczytDanychZadan(listaZadan);
		
		ZapiszWynikiDoPliku(listaZadan, przerwaniaFirstProcessor, przerwaniaSecondProcessor, firstSolutionValue, numerInstancjiProblemu, nameParam);
		
		long int wynik = ObliczFunkcjeCelu(listaZadan);
//		OdczytPrzerwan(listaPrzerwan);
//		OdczytDanychZadan(listaZadan);
		UtworzGraf(listaZadan, listaPrzerwan, wynik, nameParam);
//		nameParam += "w";
		
		vector<Task*> nowe = GeneratorLosowy(zadania, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
//		Mutacja(listaZadan, przerwaniaFirstProcessor, przerwaniaSecondProcessor);		
//		OdczytDanychZadan(listaZadan);
		vector< vector<Task*> > solution;
		
		
		solution.push_back(nowe);
		solution.push_back(listaZadan);
		cout << "S1 " << ObliczFunkcjeCelu(nowe) << endl;
		OdczytDanychZadan(nowe);
		cout << "S2 " << ObliczFunkcjeCelu(listaZadan) << endl;
		OdczytDanychZadan(listaZadan);
		
		Turniej(solution);
		
//		UtworzGraf(listaZadan, listaPrzerwan, wynik, nameParam);
		
		
		
	// Czyszczenie pamiêci - zwalnianie niepotrzebnych zasobów
		przerwaniaFirstProcessor.clear();
		przerwaniaSecondProcessor.clear();
		listaPrzerwan.clear();
		listaZadan.clear();
	
	return 0;
}
