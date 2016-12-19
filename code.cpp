/* OPTYMALIZACJA KOMBINATORYCZNA
 * Temat: Metaheurystyki w problemie szeregowania zadań
 * Autor: Bartosz Górka, Mateusz Kruszyna
 * Data: Grudzień 2016r.
*/

// Biblioteki używane w programie
#include <iostream> // I/O stream
#include <vector> // Vector table
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <climits> // INT_MAX do generatora losowego
#include <algorithm> // Sortowanie przerwań

using namespace std;

// DEFINICJE GLOBALNE
#define DEBUG true // TRYB DEBUGOWANIA [true / false]
#define MIN_TASK_COUNTER 30 // PO ilu iteracjach sprawdzać zapętlenie [Wartość liczbowa > 0]

#define LOWER_TIME_TASK_LIMIT 5 // Dolne ograniczenie długości zadania [Wartość liczbowa > 0]
#define UPPER_TIME_TASK_LIMIT 15 // Górne ograniczenie długości zadania [Wartość liczbowa > 0]

#define MAINTENANCE_FIRST_PROCESSOR 3 // Ilość przerwań na pierwszej maszynie [Wartość liczbowa >= 0]
#define MAINTENANCE_SECOND_PROCESSOR 3 // Ilość przerwań na drugiej maszynie [Wartość liczbowa >= 0]

#define LOWER_TIME_MAINTENANCE_LIMIT 5 // Dolne ograniczenie długości przerwania [Wartość liczbowa >= 0]
#define UPPER_TIME_MAINTENANCE_LIMIT 20 // Górne ograniczenie długości przerwania [Wartość liczbowa > 0]

#define LOWER_READY_TIME_MAINTENANCE_LIMIT 0 // Dolne ograniczenie czasu gotowości przerwania [Wartość liczbowa >= 0]
#define UPPER_READY_TIME_MAINTENANCE_LIMIT 200 // Górne ograniczenie czasu gotowości przerwania [Wartość liczbowa > 0]

#define INSTANCE_SIZE 5 // Rozmiar instancji problemu
#define INSTANCE_NUMBER 1 // Numer instancji problemu (może być zmieniana przy odczycie danych z pliku)
#define MAX_SOLUTIONS 3 // Ilość rozwiązań jakie chcemy wygenerować
#define MAX_SOLUTION_AFTER_MUTATION 9 // Ilość rozwiązań po mutacji (ile ta mutacja ma stworzyc rozwiazan w sumie)

#define MAX_DURATION_PROGRAM_TIME 0.3 // Maksymalna długość trwania programu w SEKUNDACH
#define PROBABILTY_OF_RANDOM_GENERATION 30 // Prawdopodobieństwo stworzenia rozwiązań przez los (dopełnienie to przez macierz feromonową

#define PROCENT_ZANIKANIA 5 // Ile procennt śladu feromonowego ma znikać co iterację
#define WYKLADNIK_POTEGI 3.5 // Potrzebne do funkcji spłaszczającej
#define CO_ILE_ITERACJI_WIERSZ 2 // Co ile iteracji przeskakujemy do kolejnego wiersza [>=1 - gdy 1 to cyklicznie przechodzimy]

ofstream debugFile; // Zmienna globalna używana przy DEBUG mode
long int firstSolutionValue; // Zmienna globalna najlepszego rozwiązania wygenerowanego przez generator losowy
double MacierzFeromonowa[INSTANCE_SIZE][INSTANCE_SIZE]; // Macierz feromonowa w programie

// Struktura danych w pamięci
struct Task{
	int ID; // ID zadania
	int part; // Numer części zadania [0, 1]
	int assigment; // Przydział zadania do maszyny [0, 1]
	int duration; // Długość zadania
	int endTime; // Moment zakończenia
	Task *anotherPart; // Wskaźnik na komplementarne zadanie
};

struct Maintenance {
	int assigment; // Numer maszyny
	int readyTime; // Czas gotowości (pojawienia się)
	int duration; // Czas trwania przerwania
};

// Funkcja pomocnicza używana w sortowaniu przerwań
bool sortMaintenance(Maintenance * i, Maintenance * j) {return (i->readyTime < j->readyTime); }

// Pomocnicze funkcje używane przy sortowaniu zadań
bool sortTask(Task *i, Task *j) { return (i->endTime < j->endTime); }
bool sortTaskByID(Task *i, Task *j) { return (i->ID < j->ID); } // Po wartości ID

void KopiujDaneOperacji(vector<Task*> &listaWejsciowa, vector<Task*> &listaWyjsciowa);

// Generator przestojów na maszynie
void GeneratorPrzestojow(vector<Maintenance*> &lista, int liczbaPrzerwanFirstProcessor, int liczbaPrzerwanSecondProcessor, int lowerTimeLimit, int upperTimeLimit, int lowerReadyTime, int upperReadyTime) {
	int size = (upperReadyTime - lowerReadyTime) + (upperTimeLimit - lowerTimeLimit);
	bool * maintenanceTimeTable = new bool[size]; // Jedna tablica bo przerwania na maszynach nie mogą się nakładać na siebie

	for(int i = 0; i < size; i++) {
		maintenanceTimeTable[i] = false;
	}

	int liczbaPrzerwan = liczbaPrzerwanFirstProcessor + liczbaPrzerwanSecondProcessor;

	for(int i = 0; i < liczbaPrzerwan; i++) {
		Maintenance * przerwa = new Maintenance;

		// Losowanie przerwy na którą maszynę ma trafić
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

		// Random punkt startu + sprawdzenie czy jest to możliwe
			int readyTime = 0;
			int startTimeCheck, stopTimeCheck = 0;

			while(true) {
				readyTime = lowerReadyTime + (int)(rand() / (RAND_MAX + 1.0) * upperReadyTime);

				startTimeCheck = readyTime - lowerReadyTime;
				stopTimeCheck = startTimeCheck + duration;
				// Sprawdzenie czy można dać przerwanie od readyTime
					bool repeatCheck = false;
					for(int j = startTimeCheck; j < stopTimeCheck; j++) {
						if(maintenanceTimeTable[j]) {
							repeatCheck = true;
							break; // Konieczne jest ponowne losowanie czasu rozpoczęcia
						}
					}

					if(!repeatCheck) {
						break; // Można opuścić pętle while - znaleziono konfigurację dla przerwania
					}
			}

			// Zapis przerwania w tablicy pomocniczej
				for(int j = startTimeCheck; j < stopTimeCheck; j++) {
					maintenanceTimeTable[j] = true;
				}

			// Uzupełnienie danych o przerwaniu
				przerwa->readyTime = readyTime;
				przerwa->duration = duration;

			// Dodanie przestoju do listy
				lista.push_back(przerwa);
	}

	// Czyszczenie pamięci - zwalnianie niepotrzebnych zasobów
		delete maintenanceTimeTable;
}

// Sortowanie przerwań według rosnącego czasu rozpoczęcia
void SortujPrzerwania(vector<Maintenance*> &listaPrzerwan) {
	// Używamy algorytmicznej funkcji sort z ustawionym trybem sortowania aby przyspieszyć pracę
		sort(listaPrzerwan.begin(), listaPrzerwan.end(), sortMaintenance);
}

// Sortowanie zadań według wzrastającego ID
void SortujZadaniaPoID(vector<Task*> &listaZadan) {
	sort(listaZadan.begin(), listaZadan.end(), sortTaskByID);
}

// Sortowanie zadań według rosnącego czasu zakończenia pracy
void SortujZadaniaPoEndTime(vector<Task*> &listaZadan) {
	sort(listaZadan.begin(), listaZadan.end(), sortTask);
}

void SortujListeZadanPoEndTime(vector< vector<Task*> > &listaRozwiazan){
    int sizeListaRozwiazan = listaRozwiazan.size();
    for(int i = 0; i < sizeListaRozwiazan; i++) {
        SortujZadaniaPoEndTime(listaRozwiazan[i]);
    }
}
// Generator instancji problemu
void GeneratorInstancji(vector<Task*> &lista, int maxTask, int lowerTimeLimit, int upperTimeLimit) {
	int assigment = 0;

	for(int i = 0; i < maxTask; i++) {
		Task * taskFirst = new Task;
		Task * taskSecond = new Task;

		// Powiązujemy między sobą zadania
			taskFirst->anotherPart = taskSecond;
			taskSecond->anotherPart = taskFirst;

		// Przypisujemy pararametr part
			taskFirst->part = 0;
			taskSecond->part = 1;

		// Przypisujemy ID zadania
			taskFirst->ID = i + 1;
			taskSecond->ID = i + 1;

		// Losujemy przydział części pierwszej zadania na maszynę
			assigment = rand() % 2;

		// Uzupełniamy dane przydziałów
			taskFirst->assigment = assigment;
			taskSecond->assigment = 1 - assigment;

		// Randomy na czas trwania zadań
			taskFirst->duration = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			taskSecond->duration = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);

		// Czas zakończenia póki co ustawiony na 0 = zadania nie były jeszcze zakolejkowane
			taskFirst->endTime = 0;
			taskSecond->endTime = 0;

		// Dodanie zadań do listy
			lista.push_back(taskFirst);
			lista.push_back(taskSecond);
	}
}

// Zapis instancji do pliku
void ZapiszInstancjeDoPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, int numerInstancjiProblemu, string nameParam) {
	// Zmienna pliku docelowego
		ofstream file;

	// Utworzenie zmiennej pomocniczej w postaci nazwy pliku aby móc parametryzować zapis danych
		string fileName = "instancje_" + nameParam + ".txt";
		file.open(fileName.c_str());

	if(file.is_open()) {
		file << "**** " << numerInstancjiProblemu << " ****" << endl;

		// Obliczenie ilości zadań w otrzymanym wektorze
			int iloscZadan = listaZadan.size();

		// Posortowanie wektora po wartości ID aby mieć obok siebie operacje z tego samego zadania
			SortujZadaniaPoID(listaZadan);

		// Przypisanie do pliku ilości zadań w instancji
			file << iloscZadan / 2 << endl;

		// Uzupełnienie pliku o wygenerowane czasy pracy
			for(int i = 0; i < iloscZadan; i += 2) {
				// Dodanie linii z opisem zadania do pliku instancji
					if(listaZadan[i]->part == 0) { // Pod i mamy zadanie będące Part I
						file << listaZadan[i]->duration << ":" << listaZadan[i]->anotherPart->duration << ":" << listaZadan[i]->assigment << ":" << listaZadan[i]->anotherPart->assigment << ";" << endl;
					} else {
						file << listaZadan[i]->anotherPart->duration << ":" << listaZadan[i]->duration << ":" << listaZadan[i]->anotherPart->assigment << ":" << listaZadan[i]->assigment << ";" << endl;
					}
			}

		// Uzupełnienie pliku o czasy przestojów maszyn
			int iloscPrzestojow = listaPrzerwan.size();
			for(int i = 0; i < iloscPrzestojow; i++) {
				file << i << ":" << listaPrzerwan[i]->assigment << ":" << listaPrzerwan[i]->duration << ":" << listaPrzerwan[i]->readyTime << ";" << endl;
			}

			file << "**** EOF ****" << endl;
	}

	file.close();
}

// Wczytywanie instancji z pliku do pamięci
void WczytajDaneZPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, int &numerInstancjiProblemu, string nameParam) {
	FILE *file;
	string fileName = "instancje_" + nameParam + ".txt";
	file = fopen(fileName.c_str(), "r");

	if(file != NULL) {
		// Pobranie numeru instancji problemu
			fscanf(file, "**** %d ****", &numerInstancjiProblemu);

		// Pobranie liczby zadań
			int liczbaZadan = 0;
			fscanf(file, "%d", &liczbaZadan);

		// Zmienne pomocnicze w tworzeniu zadania
			int assigmentFirstPart, assigmentSecondPart, durationFirstPart, durationSecondPart;

		// Pobranie wartości zadania z pliku instancji
			for(int i = 0; i < liczbaZadan; i++) {
				// Odczyt wpisu
					fscanf(file, "%d:%d:%d:%d;", &durationFirstPart, &durationSecondPart, &assigmentFirstPart, &assigmentSecondPart);

				// Utworzenie zadania
					Task * taskFirst = new Task;
					Task * taskSecond = new Task;

				// Powiązanie zadań
					taskFirst->anotherPart = taskSecond;
					taskSecond->anotherPart = taskFirst;

				// Ustawienie wartości zadań
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

				// Dodanie zadania do wektora zadań
					listaZadan.push_back(taskFirst);
					listaZadan.push_back(taskSecond);
			}

		// Zestaw zmiennych używanych przy odczycie przerwań na maszynach
			int assigment, duration, readyTime, numer;
			int oldNumber = -1;

		// Pobranie wartości dotyczących przerwań
			while(fscanf(file, "%d:%d:%d:%d;", &numer, &assigment, &duration, &readyTime)) {
				// Sprawdzenie czy nie mamy zapętlenia
					if(oldNumber == numer)
						break;

				// Utworzenie przerwy
					Maintenance * przerwa = new Maintenance;

				// Ustawienie wartości zadania
					przerwa->assigment = assigment;
					przerwa->duration = duration;
					przerwa->readyTime = readyTime;

				// Dodanie zadania do wektora zadań
					listaPrzerwan.push_back(przerwa);

				// Zmienna pomocnicza do eliminacji zapętleń przy odczycie
					oldNumber = numer;
			}
	}
}

// Odczyt przerwań na maszynach na ekran
void OdczytPrzerwan(vector<Maintenance*> &listaPrzerwan) {
	int size = listaPrzerwan.size();
	for(int i = 0; i < size; i++) {
		cout << "Maszyna = " << listaPrzerwan[i]->assigment << " | Start = " << listaPrzerwan[i]->readyTime << " | Czas trwania = " << listaPrzerwan[i]->duration << endl;
	}
}

// Generator rozwiązań losowych
vector<Task*> GeneratorLosowy(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor) {
	// Utworzenie kopii zadań aby móc tworzyć swoje rozwiązanie
		vector<Task*> zadaniaLokalne;
		KopiujDaneOperacji(listaZadan, zadaniaLokalne);

	// Zmienne używane w przebiegu pracy Generatora Losowego
		int iloscZadan = listaZadan.size() / 2;	// Ilość zadań (ilość operacji / 2)
		Task * currentTask = NULL; // Zmmienna operacyjna aby uprościć zapis
		int numerPrzerwaniaFirstProcessor = 0; // Numer aktualnego przerwania na procesorze pierwszym
		int numerPrzerwaniaSecondProcessor = 0; // Numer aktualnego przerwania na procesorze drugim
		int count = 0; // Licznik przeliczonych już zadań
		int najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime; // Czas momentu ROZPOCZÄCIA przerwania na procesorze pierwszym
		int najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime; // Czas momentu ROZPOCZÄCIA przerwania na procesorze drugim
		int timeFirstProcessor = 0; // Zmienna czasowa - procesor pierwszy
		int timeSecondProcessor = 0; // Zmienna czasowa - procesor drugi
		int maxCount = 2 * iloscZadan; // Ilość koniecznych edycji w zadaniach (part I + part II w każdym zadaniu)
		int listaPrzerwanFirstProcessorSize = listaPrzerwanFirstProcessor.size(); // Ilość przerwań dla pierwszego procesora - aby nie liczyć za każdym razem tej wartości
		int listaPrzerwanSecondProcessorSize = listaPrzerwanSecondProcessor.size(); // Ilość przerwań dla drugiej maszyny - podobnie jak wyżej, unikamy niepotrzebnego, wielokrotnego liczenia tej wartości
		int taskID = 0; // Numer zadania
		int pozycja = 0; // Numer aktualnie rozpatrywanego zadania (losowa wartość z z przedziału 0 - ilosc zadan*2)

	// Tworzymy dwie tablice pomocnicze do sprawdzania czy zadanie było już uwzględnione
		bool * firstPart = new bool[iloscZadan]; // Część I zadania - czy była uwzględniona (jeśli tak to true)
		bool * secondPart = new bool[iloscZadan]; // Część II zadania - czy była uwzględniona (jeśli tak to true)

	// Licznik odwiedzin w każdym z zadań
		int * licznikOdwiedzonych = new int[iloscZadan]; // Licznik odwiedzeń w danym zadaniu aby unikać pętli

	// Pętla startowa zerująca tablice
		for(int i = 0; i < iloscZadan; i++) {
			firstPart[i] = false;
			secondPart[i] = false;
			licznikOdwiedzonych[i] = 0;
		}

		while(count < maxCount) {
			// Losujemy pozycję w tablicy zadań
				pozycja = rand() % maxCount;

			// Sprawdzamy zadanie odpowiadające wylosowanej pozycji nie było już przypadkiem użyte całe - w takim przypadku losujemy na nowo numer pozycji w tablicy
				taskID = zadaniaLokalne[pozycja]->ID - 1;
				if(firstPart[taskID] && secondPart[taskID])
					continue; // Skok do kolejnej iteracji

				if(DEBUG) {
					debugFile << "Wylosowano = " << pozycja << " Zadanie nr " << taskID << " (Part " << zadaniaLokalne[pozycja]->part + 1 << ")"
					<< " Parametry zadania = " << zadaniaLokalne[pozycja]->assigment << "|" << zadaniaLokalne[pozycja]->duration << "|" << zadaniaLokalne[pozycja]->endTime
					<< " Parametry komplementarnej części = " << zadaniaLokalne[pozycja]->anotherPart->assigment << "|" << zadaniaLokalne[pozycja]->anotherPart->duration << "|" << zadaniaLokalne[pozycja]->anotherPart->endTime
					<< " czasy: " << timeFirstProcessor << "|" << timeSecondProcessor << " przerwy: " << najblizszyMaintenanceFirstProcessor << "|" << najblizszyMaintenanceSecondProcessor << endl;
				}

			// Zadanie nie było jeszcze używane
				if(!firstPart[taskID]) {
					// Sprawdzamy typ zadania - jeżeli jest zero to podstawiamy pod zmienną pomocniczą
						if(zadaniaLokalne[pozycja]->part == 0) {
							currentTask = zadaniaLokalne[pozycja];
						} else { // Jeżeli nie - konieczne jest podstawienie części komplementarnej wylosowanego zadania
							currentTask = zadaniaLokalne[pozycja]->anotherPart;
						}

					// Sprawdzamy czy zadanie powinno trafić na maszynę 0
						if(currentTask->assigment == 0) {
							// Sprawdzamy czy zadanie uda się ustawić przed najblizszym maintenance na maszynie
								if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
									// Ustawiamy czas na maszynie pierwszej
										timeFirstProcessor += currentTask->duration;

									if(DEBUG)
										debugFile << "Czas FM: " << timeFirstProcessor << endl;

									// Ustawiamy czas zakończenia Part I
										currentTask->endTime = timeFirstProcessor;

									// Ustawiamy że zadanie zostało użyte (Part I)
										firstPart[taskID] = true;

								} else { // Nie udało się umieścić zadania przed przerwą
									while(true) {
										// Przesuwamy się na chwilę po przerwaniu
											timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;

										// Ustawiamy czas następnego przerwania
											numerPrzerwaniaFirstProcessor++;
											if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
												najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
											else
												najblizszyMaintenanceFirstProcessor = -1;

										// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
											if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
												break;
									}

									// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeFirstProcessor (wystarczy zwiększyć ją o długość zadania)
										timeFirstProcessor += currentTask->duration;

										if(DEBUG)
											debugFile << "I Czas FM " << timeFirstProcessor << endl;

									// Ustawiamy zmienną czasową zakończenia zadania
										currentTask->endTime = timeFirstProcessor;

									// Zaznaczamy w tablicy pomocniczej że część pierwsza zadania była użyta
										firstPart[taskID] = true;
								}

								// Zwiększamy ilość zadań jakie przerobiliśmy
									count++;

						} else { // Przydział zadania na maszynę nr 2
							// Sprawdzamy czy zadanie można umieścić przed maintenance najbliższym (jeżeli jest  on -1 to już nie wystąpi)
								if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
									// Ustawiamy czas na maszynie drugiej
										timeSecondProcessor += currentTask->duration;

										if(DEBUG)
											debugFile << "I Czas SM: " << timeSecondProcessor << endl;

									// Ustawiamy czas zakończenia zadania
										currentTask->endTime = timeSecondProcessor;

									// Ustawiamy że zadanie zostało użyte (part I)
										firstPart[taskID] = true;

								} else { // Nie umieściliśmy zadania przed przerwą
									while(true) {
										// Przesuwamy się na chwilę po przerwaniu
											timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;

										// Ustawiamy czas następnego przerwania
											numerPrzerwaniaSecondProcessor++;
											if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
												najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
											else
												najblizszyMaintenanceSecondProcessor = -1;

											if(DEBUG)
												debugFile << "Druga = " << timeSecondProcessor << " oraz " << najblizszyMaintenanceSecondProcessor << endl;

										// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
											if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
												break;
									}

									// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeSecondProcessor(wystarczy zwiększyć ją o długość zadania)
										timeSecondProcessor += currentTask->duration;

										if(DEBUG)
											debugFile << "Czas SM " << timeSecondProcessor << endl;

									// Ustawiamy zmienną czasową zakończenia zadania
										currentTask->endTime = timeSecondProcessor;

									// Zaznaczamy w tablicy pomocniczej że część pierwsza zadania była użyta
										firstPart[taskID] = true;
								}

								// Zwiększamy ilość zadań jakie przerobiliśmy
									count++;
						}
				} else {
				// PRZYDZIELAMY DRUGÄ„ CZÄĹšÄ† ZADANIA

					// Mogą wystąpić problemy z zapętleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartość
						licznikOdwiedzonych[taskID]++;

					// Sprawdzamy typ zadania - jeżeli jest zero to podstawiamy pod zmienną pomocniczą
						if(zadaniaLokalne[pozycja]->part == 1) {
							currentTask = zadaniaLokalne[pozycja];
						} else { // Jeżeli nie - konieczne jest podstawienie części komplementarnej wylosowanego zadania
							currentTask = zadaniaLokalne[pozycja]->anotherPart;
						}

					// Sprawdzamy typ zadania
						if(currentTask->assigment == 1) { // Przydział na drugą maszynę
							// Sprawdzamy czy czas na maszynie nie jest mniejszy od zakończenia się pierwszej części
							if(timeSecondProcessor < currentTask->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteśmy po raz x w pętli
								if(licznikOdwiedzonych[taskID] >= MIN_TASK_COUNTER) {
									if(DEBUG)
										debugFile << "Przestawiono czas! M1" << endl;
									// Tworzymy pomocniczą zmienną odległości
										int minTime = INT_MAX;
										int tempTime = 0;

									// Resetujemy liczniki i patrzymy na odległości
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

								} else // Jeżeli nie mamy osiągniętej wartości to pomijamy iterację
									continue;
							}

							// Zadanie można umieścić
								// Sprawdzamy czy zadanie można umieścić przed maintenance najbliższym (jeżeli jest  on -1 to już nie wystąpi)
								if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
									// Ustawiamy czas na maszynie pierwszej
										timeSecondProcessor += currentTask->duration;

									// Ustawiamy czas zakończenia zadania
										currentTask->endTime = timeSecondProcessor;

									// Ustawiamy że zadanie zostało użyte (part II)
										secondPart[taskID] = true;

								} else { // Nie umieściliśmy zadania przed przerwą
									while(true) {
										// Przesuwamy się na chwilę po przerwaniu
											timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;

										// Ustawiamy czas następnego przerwania
											numerPrzerwaniaSecondProcessor++;
											if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
												najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
											else
												najblizszyMaintenanceSecondProcessor = -1;

										// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
											if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
												break;
									}

									// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeSecondProcessor (wystarczy zwiększyć ją o długość zadania)
										timeSecondProcessor += currentTask->duration;

									// Ustawiamy zmienną czasową zakończenia zadania
										currentTask->endTime = timeSecondProcessor;

									// Zaznaczamy w tablicy pomocniczej że część pierwsza zadania była użyta
										secondPart[taskID] = true;
								}

								// Zwiększamy ilość zadań jakie przerobiliśmy
									count++;
						} else {
							// Sprawdzamy czy czas na maszynie nie jest mniejszy od zakończenia się pierwszej części
							if(timeFirstProcessor < currentTask->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteśmy po raz x w pętli
								if(licznikOdwiedzonych[taskID] >= MIN_TASK_COUNTER) {
									if(DEBUG)
										debugFile << "Przestawiono czas! M0" << endl;

									// Tworzymy pomocniczą zmienną odległości
										int minTime = INT_MAX;
										int tempTime = 0;

									// Resetujemy liczniki i patrzymy na odległości
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

								} else // Jeżeli nie mamy osiągniętej wartości to pomijamy iterację
									continue;
							}

							// Zadanie można umieścić
								// Sprawdzamy czy zadanie można umieścić przed maintenance najbliższym (jeżeli jest  on -1 to już nie wystąpi)
								if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
									// Ustawiamy czas na maszynie pierwszej
										timeFirstProcessor += currentTask->duration;

									// Ustawiamy czas zakończenia zadania
										currentTask->endTime = timeFirstProcessor;

									// Ustawiamy że zadanie zostało użyte (part II)
										secondPart[taskID] = true;

								} else { // Nie umieściliśmy zadania przed przerwą
									while(true) {
										// Przesuwamy się na chwilę po przerwaniu
											timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;

										// Ustawiamy czas następnego przerwania
											numerPrzerwaniaFirstProcessor++;
											if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
												najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
											else
												najblizszyMaintenanceFirstProcessor = -1;

										// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
											if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
												break;
									}

									// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeSecondProcessor (wystarczy zwiększyć ją o długość zadania)
										timeFirstProcessor += currentTask->duration;

									// Ustawiamy zmienną czasową zakończenia zadania
										currentTask->endTime = timeFirstProcessor;

									// Zaznaczamy w tablicy pomocniczej że część pierwsza zadania była użyta
										secondPart[taskID] = true;
								}

								// Zwiększamy ilość zadań jakie przerobiliśmy
									count++;
						}
				}
		}

		// Czyszczenie pamięci - zwalnianie niepotrzebnych zasobów
			delete firstPart;
			delete secondPart;
			delete licznikOdwiedzonych;

	return zadaniaLokalne;
}

// Odczyt danych zadań na ekran
void OdczytDanychZadan(vector<Task*> &listaZadan) {
	// Przeliczenie ilości operacji do zmienne pomocniczej aby nie liczyć operacji w każdej iteracji
	int size = listaZadan.size();

	// Przesortowanie listy zadań aby mieć obok siebie zadania z tym samym ID
		SortujZadaniaPoID(listaZadan);

	// Pętla odczytu wartości zadań
		for(int i = 0; i < size; i++) {
			cout << "--- ID: " << listaZadan[i]->ID << " (Part " << listaZadan[i]->part << ") przydzial: M" << listaZadan[i]->assigment << " duration = " << listaZadan[i]->duration << " --- zakonczenie = " << listaZadan[i]->endTime << " --- " << endl;
		}
}

// Tworzenie timeline dla obserwacji wyników pracy
void UtworzGraf(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, long int wynik, string nameParam) {
	int iloscZadan = listaZadan.size(); // Ilość zadań w systemie
	int iloscPrzerwan = listaPrzerwan.size(); // Ilość okresów przestojów na maszynach

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

	// Zapis przerwań
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

// Obliczanie wartości funkcji celu
long int ObliczFunkcjeCelu(vector<Task*> &lista) {
	int size = lista.size();
	long int sum = 0;

	for(int i = 0; i < size; i++) {
		sum += lista[i]->endTime;
	}

	return sum;
}

// Podział struktury T na maszyny
template <class T>
void PodzielStrukturyNaMaszyny(vector<T*> &listaWejsciowa, vector<T*> &firstProcessor, vector<T*> &secondProcessor) {
	// Zmienna pomocnicza by skrócić czas pracy (nie trzeba x razy liczyć)
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

// Obliczanie długości Task / Maintenance list
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

	string fileName;
	stringstream ss;
	ss << "wyniki/wyniki_" << numerInstancjiProblemu << "_" << nameParam << ".txt";
	ss >> fileName;
	file.open(fileName.c_str());

	if(file.is_open()) {
			long int optimalSolutionValue = ObliczFunkcjeCelu(listaZadan); // Wartość funkcji celu dla rozwiązania optymalnego
			vector<Task*> taskFirstProcessor, taskSecondProcessor; // Wektory dla podziału zadań na maszyny
			int taskFirstProcessorSize; // Ilość zadań na pierwszym procesorze
			int taskSecondProcessorSize; // Ilość zadań na drugim procesorze
			int numerPrzerwania = 0; // Numer aktualnie rozpatrywanego przerwania
			int najblizszyMaintenance = -1; // Czas momentu ROZPOCZÄCIA przerwania
			int processorTime = 0; // Czas procesora
			int count = 0; // Ilość operacji które zostały już umieszczone w pliku wynikowym
			int maxCount; // Ilość operacji które trzeba umieścić (liczba operacji + przerwania)
			int taskPoint = 0; // Zmienna wskazująca aktualnie rozpatrywane zadanie z listy operacji
			int countIldeFirstProcessor = 0; // Licznik okresów bezczynności dla maszyny pierwszej
			int countIldeSecondProcessor = 0; // Licznik okresów bezczynności dla maszyny drugiej
			int ildeTimeFirstProcessor = 0; // Ogólny czas bezczynności na maszynie pierwszej
			int ildeTimeSecondProcessor = 0; // Ogólny czas bezczynności na maszynie drugiej

		// Podzielenie listy zadań na maszyny i przypisanie ilości do zmiennych pomocniczych
			PodzielStrukturyNaMaszyny<Task>(listaZadan, taskFirstProcessor, taskSecondProcessor);
			taskFirstProcessorSize = taskFirstProcessor.size();
			taskSecondProcessorSize = taskSecondProcessor.size();

		// Sortowanie zadań
			SortujZadaniaPoEndTime(taskFirstProcessor);
			SortujZadaniaPoEndTime(taskSecondProcessor);

		// Przypisanie numeru instancji
			file << "**** " << numerInstancjiProblemu << " ****" << endl;

		// Przypisanie wartości optymalnej oraz wartości początkowej wygenerowanej przez generator losowy
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

					// Skok do kolejnej wartości
						taskPoint++;

					// Musimy sprawdzić czy nie wychodzimy poza zakres
						if(taskPoint >= taskFirstProcessorSize) {
							taskPoint = -1;
						}

					// Zwiększamy licznik odwiedzonych operacji
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

					// Zwiększamy licznik odwiedzonych operacji
						count++;
				} else { // Bezczynność

					// Sprawdzamy które zdarzenie będzie wcześniej - wystąpienie zadania czy maintenance
						int minTime = INT_MAX;
						if(taskPoint >= 0) {
							int temp =  taskFirstProcessor[taskPoint]->endTime - taskFirstProcessor[taskPoint]->duration - processorTime;
							if(temp < minTime)
								minTime = temp;
						}
						if(((najblizszyMaintenance - processorTime) < minTime) && najblizszyMaintenance > -1) {
							minTime = najblizszyMaintenance - processorTime;
						}

					// Zapis do pliku danych o bezczynności
						file << "idle" << countIldeFirstProcessor + 1 << "_M1, " << processorTime << ", " << minTime << "; ";
						countIldeFirstProcessor++;

					// Dodanie do ogólnego licznika bezczynności zapisanego czasu
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

					// Skok do kolejnej wartości
						taskPoint++;

					// Musimy sprawdzić czy nie wychodzimy poza zakres
						if(taskPoint >= taskSecondProcessorSize) {
							taskPoint = -1;
						}

					// Zwiększamy licznik odwiedzonych operacji
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

					// Zwiększamy licznik odwiedzonych operacji
						count++;
				} else { // Bezczynność
					// Sprawdzamy które zdarzenie będzie wcześniej - wystąpienie zadania czy maintenance
						int minTime = INT_MAX;
						if(taskPoint >= 0) {
							int temp =  taskSecondProcessor[taskPoint]->endTime - taskSecondProcessor[taskPoint]->duration - processorTime;
							if(temp < minTime)
								minTime = temp;
						}
						if(((najblizszyMaintenance - processorTime) < minTime) && najblizszyMaintenance > -1) {
							minTime = najblizszyMaintenance - processorTime;
						}

					// Zapis do pliku danych o bezczynności
						file << "ilde" << countIldeSecondProcessor + 1 << "_M2, " << processorTime << ", " << minTime << "; ";

					// Inkrementacja numeru przerwania
						countIldeSecondProcessor++;

					// Dodanie do ogólnego licznika bezczynności zapisanego czasu
						ildeTimeSecondProcessor += minTime;

					// Przestawienie czasu maszyny
						processorTime += minTime;
				}
			}

			// Dopisanie wartości sum
				file << endl << listaPrzerwanFirstProcessor.size() << ", " << ObliczDlugoscOperacji<Maintenance>(listaPrzerwanFirstProcessor) << endl
					 << listaPrzerwanSecondProcessor.size() << ", " << ObliczDlugoscOperacji<Maintenance>(listaPrzerwanSecondProcessor) << endl
					 << countIldeFirstProcessor << ", " << ildeTimeFirstProcessor << endl
					 << countIldeSecondProcessor << ", " << ildeTimeSecondProcessor << endl << "*** EOF ***";

			// Czyszczenie pamięci operacyjnej
				taskFirstProcessor.clear();
				taskSecondProcessor.clear();
	}
    else{
        if(DEBUG) cout<<"Nie utworzono pliku: "<<fileName.c_str()<<endl;
    }
}

// Mutacja jednego rozwiązania z założeniem podzielenia operacji na dwie maszyny
vector<Task*> Mutacja(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor) {
	// Zmienne operacyjne
		vector<Task*> taskListFirstProcessor, taskListSecondProcessor; // Wektory dla podziału zadań na maszyny

	// Podzielenie listy zadań na maszyny i przypisanie ilości do zmiennych pomocniczych
			PodzielStrukturyNaMaszyny<Task>(listaZadan, taskListFirstProcessor, taskListSecondProcessor);

		int iloscZadan = taskListFirstProcessor.size();
		int firstTaskPosition = 0; // Random - pozycja pierwszego zadania
		int secondTaskPosition = 0;  // Random - pozycja drugiego zadania

		int processor = rand() % 2; // Random - wybór maszyny którą dotyczyć będzie mutacja
		int timeFirstProcessor = 0; // Czas na maszynie pierwszej
		int timeSecondProcessor = 0; // Czas na maszynie drugiej
		Task * currentTaskFirstProcessor = NULL; // Zadanie na maszynie pierwszej
		Task * currentTaskSecondProcessor = NULL; // Zadanie na maszynie drugiej
		int numerPrzerwaniaFirstProcessor = 0; // Numer aktualnego przerwania na procesorze pierwszym
		int numerPrzerwaniaSecondProcessor = 0; // Numer aktualnego przerwania na procesorze drugim
		int countTask = 0; // Licznik sprawdzionych już zadań
		int najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime; // Czas momentu ROZPOCZÄCIA przerwania na procesorze pierwszym
		int najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime; // Czas momentu ROZPOCZÄCIA przerwania na procesorze drugim
		int listaPrzerwanFirstProcessorSize = listaPrzerwanFirstProcessor.size(); // Ilość przerwań dla pierwszego procesora - aby nie liczyć za każdym razem tej wartości
		int listaPrzerwanSecondProcessorSize = listaPrzerwanSecondProcessor.size(); // Ilość przerwań dla drugiej maszyny - podobnie jak wyżej, unikamy niepotrzebnego, wielokrotnego liczenia tej wartości
		int taskIDFirstProcessor = 0; // Numer zadania na maszynie pierwszej
		int taskIDSecondProcessor = 0; // Numer zadania na maszynie drugiej

		int iteratorFP = 0; // Numer rozpatrywanego zadania na maszynie pierwszej
		int iteratorSP = 0; // Numer aktualnie rozpatrywanego zadania na maszynie drugiej

	// Wektory kolejności zadań (ID)
		int *taskOrderFirstProcessor = new int[iloscZadan];
		int *taskOrderSecondProcessor = new int[iloscZadan];

	// Licznik odwiedzonych aby nie zapętlić się w czasach bezczynności
		int *licznikOdwiedzonych = new int[iloscZadan];

	// Pomocnicze tablice part I & part II aby przyspieszyć proces sprawdzania
		bool *firstPart = new bool[iloscZadan];
		bool *secondPart = new bool[iloscZadan];

	// Sortowanie zadań w listach
		SortujZadaniaPoEndTime(taskListFirstProcessor);
		SortujZadaniaPoEndTime(taskListSecondProcessor);

	// Tworzymy wektor kolejności zadań i zerujemy tablice pomocnicze
		for(int i = 0; i < iloscZadan; i++) {
			taskOrderFirstProcessor[i] = taskListFirstProcessor[i]->ID - 1;
			taskOrderSecondProcessor[i] = taskListSecondProcessor[i]->ID - 1;
			firstPart[i] = false;
			secondPart[i] = false;
			licznikOdwiedzonych[i] = 0;
		}

		if(DEBUG) {
			debugFile << "Przed mutacją:" << endl;
			for(int i = 0; i < iloscZadan; i++) {
				debugFile << taskOrderFirstProcessor[i] << " | " << taskOrderSecondProcessor[i] << endl;
			}
		}

	// Pętla losowania i zmiany kolejności zadań
		while(true) {
			// Losujemy wartości
				firstTaskPosition = (int)(rand() / (RAND_MAX + 1.0) * iloscZadan);
				secondTaskPosition = (int)(rand() / (RAND_MAX + 1.0) * iloscZadan);

			if(processor == 0) { // Przestawienie kolejności zadań dotyczy maszyny pierwszej
				// Sprawdzamy czy te zadania możemy mutować (założenie - przestawiamy tylko zadania z tym samym wskaĹşnikiem części Part)
					if(secondTaskPosition != firstTaskPosition && taskListFirstProcessor[firstTaskPosition]->part == taskListFirstProcessor[secondTaskPosition]->part) {
					// Zamiana kolejności zadań w liście
						int temp = taskOrderFirstProcessor[firstTaskPosition];
						taskOrderFirstProcessor[firstTaskPosition] = taskOrderFirstProcessor[secondTaskPosition];
						taskOrderFirstProcessor[secondTaskPosition] = temp;

						break;
					} else {
						continue; // Skok do kolejnej iteracji i nowego losowania
					}
			} else { // Zmiany kolejności dla maszynie nr 2
				if(secondTaskPosition != firstTaskPosition && taskListSecondProcessor[firstTaskPosition]->part == taskListSecondProcessor[secondTaskPosition]->part) {
					// Zamiana kolejności zadań w liście
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

		// Posortowanie zadań według ID - aby łatwo odwoływać się poprzez wartość z tablicy kolejności zadań
			SortujZadaniaPoID(taskListFirstProcessor);
			SortujZadaniaPoID(taskListSecondProcessor);

	// Pętla ustawiająca nowe czasy zakończenia dla naszych operacji
		while(countTask < iloscZadan*2) {
			// Sprawdzamy czy nie wyskoczyliśmy na maszynie pierwszej poza zakres vektora
			if(iteratorFP < iloscZadan) {
				// Przypisujemy zadanie do zmiennej pomocniczej
					taskIDFirstProcessor = taskOrderFirstProcessor[iteratorFP];
					currentTaskFirstProcessor = taskListFirstProcessor[taskIDFirstProcessor];

				// Sprawdzamy part zadania - jeżeli jest to I to można wstawiać od razu, jeżeli II trzeba poczekać aż zostanie wstawiona część I na maszynie drugiej
					if(currentTaskFirstProcessor->part == 0) {
						// Sprawdzamy czy zadanie uda się ustawić przed najblizszym maintenance na maszynie
							if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
								// Ustawiamy czas na maszynie pierwszej
									timeFirstProcessor += currentTaskFirstProcessor->duration;

								if(DEBUG)
									debugFile << "Czas FM: " << timeFirstProcessor << endl;

								// Ustawiamy czas zakończenia Part I
									currentTaskFirstProcessor->endTime = timeFirstProcessor;

								// Ustawiamy że zadanie zostało użyte (Part I)
									firstPart[taskIDFirstProcessor] = true;

							} else { // Nie udało się umieścić zadania przed przerwą
								while(true) {
									// Przesuwamy się na chwilę po przerwaniu
										timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;

									// Ustawiamy czas następnego przerwania
										numerPrzerwaniaFirstProcessor++;
										if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
												najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
											else
												najblizszyMaintenanceFirstProcessor = -1;

										// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
											if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
												break;
									}

									// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeFirstProcessor (wystarczy zwiększyć ją o długość zadania)
										timeFirstProcessor += currentTaskFirstProcessor->duration;

										if(DEBUG)
											debugFile << "I Czas FM " << timeFirstProcessor << endl;

									// Ustawiamy zmienną czasową zakończenia zadania
										currentTaskFirstProcessor->endTime = timeFirstProcessor;

									// Zaznaczamy w tablicy pomocniczej że część pierwsza zadania była użyta
										firstPart[taskIDFirstProcessor] = true;
							}

							// Zwiększamy ilość poprawionych zadań
								countTask++;

							// Przestawiamy iterator na pierwszej maszynie
								iteratorFP++;

					} else if(firstPart[taskIDFirstProcessor]) { // Sprawdzamy czy została wstawiona część I zadania (ma ono part == 1)
						// Mogą wystąpić problemy z zapętleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartość
							licznikOdwiedzonych[taskIDFirstProcessor]++;

							if(timeFirstProcessor < currentTaskFirstProcessor->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteśmy po raz x w pętli
								if(licznikOdwiedzonych[taskIDFirstProcessor] >= MIN_TASK_COUNTER) {
									// Tworzymy pomocniczą zmienną odległości
										int minTime = INT_MAX;
										int tempTime = 0;

									// Resetujemy liczniki i patrzymy na odległości
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
								// Zadanie można umieścić
								// Sprawdzamy czy zadanie można umieścić przed maintenance najbliższym (jeżeli jest  on -1 to już nie wystąpi)
									if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
										// Ustawiamy czas na maszynie pierwszej
											timeFirstProcessor += currentTaskFirstProcessor->duration;

										// Ustawiamy czas zakończenia zadania
											currentTaskFirstProcessor->endTime = timeFirstProcessor;

										// Przestawiamy iterator oraz ilość zedytowanych zadań
											iteratorFP++;
											countTask++;

										// Zaznaczamy zadanie jako wykonane w pełni
											secondPart[taskIDFirstProcessor] = true;

									} else { // Nie umieściliśmy zadania przed przerwą
										while(true) {
											// Przesuwamy się na chwilę po przerwaniu
												timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;

											// Ustawiamy czas następnego przerwania
												numerPrzerwaniaFirstProcessor++;
												if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
													najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
												else
													najblizszyMaintenanceFirstProcessor = -1;

											// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
												if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
													break;
										}

										// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeSecondProcessor (wystarczy zwiększyć ją o długość zadania)
											timeFirstProcessor += currentTaskFirstProcessor->duration;

										// Ustawiamy zmienną czasową zakończenia zadania
											currentTaskFirstProcessor->endTime = timeFirstProcessor;

										// Przestawiamy iterator oraz licznik edycji
											iteratorFP++;
											countTask++;

										// Zaznaczamy zadanie jako wykonane w pełni
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

				// Sprawdzamy part zadania - jeżeli jest to I to można wstawiać od razu, jeżeli II trzeba poczekać aż zostanie wstawiona część I na maszynie pierwszej
					if(currentTaskSecondProcessor->part == 0) {
						// Sprawdzamy czy zadanie uda się ustawić przed najblizszym maintenance na maszynie
							if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
								// Ustawiamy czas na maszynie pierwszej
									timeSecondProcessor += currentTaskSecondProcessor->duration;

								// Ustawiamy czas zakończenia Part I
									currentTaskSecondProcessor->endTime = timeSecondProcessor;

								// Ustawiamy że zadanie zostało użyte (Part I)
									firstPart[taskIDSecondProcessor] = true;

							} else { // Nie udało się umieścić zadania przed przerwą
								while(true) {
									// Przesuwamy się na chwilę po przerwaniu
										timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;

									// Ustawiamy czas następnego przerwania
										numerPrzerwaniaSecondProcessor++;
										if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
												najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
											else
												najblizszyMaintenanceSecondProcessor = -1;

										// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
											if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
												break;
									}

									// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeSecondProcessor (wystarczy zwiększyć ją o długość zadania)
										timeSecondProcessor += currentTaskSecondProcessor->duration;

									// Ustawiamy zmienną czasową zakończenia zadania
										currentTaskSecondProcessor->endTime = timeSecondProcessor;

									// Zaznaczamy w tablicy pomocniczej że część pierwsza zadania była użyta
										firstPart[taskIDSecondProcessor] = true;
							}

							// Zwiększamy ilość poprawionych zadań
								countTask++;

							// Przestawiamy iterator na pierwszej maszynie
								iteratorSP++;

					} else if(firstPart[taskIDSecondProcessor]) { // Sprawdzamy czy została wstawiona część I zadania (ma ono part == 1)
						// Mogą wystąpić problemy z zapętleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartość
							licznikOdwiedzonych[taskIDSecondProcessor]++;

							if(timeSecondProcessor < currentTaskSecondProcessor->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteśmy po raz x w pętli
								if(licznikOdwiedzonych[taskIDSecondProcessor] >= MIN_TASK_COUNTER) {
									// Tworzymy pomocniczą zmienną odległości
										int minTime = INT_MAX;
										int tempTime = 0;

									// Resetujemy liczniki i patrzymy na odległości
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
								// Zadanie można umieścić
								// Sprawdzamy czy zadanie można umieścić przed maintenance najbliższym (jeżeli jest  on -1 to już nie wystąpi)
									if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
										// Ustawiamy czas na maszynie pierwszej
											timeSecondProcessor += currentTaskSecondProcessor->duration;

										// Ustawiamy czas zakończenia zadania
											currentTaskSecondProcessor->endTime = timeSecondProcessor;

										// Przestawiamy iterator oraz ilość zedytowanych zadań
											iteratorSP++;
											countTask++;

										// Zaznaczamy zadanie jako wykonane w pełni
											secondPart[taskIDSecondProcessor] = true;

									} else { // Nie umieściliśmy zadania przed przerwą
										while(true) {
											// Przesuwamy się na chwilę po przerwaniu
												timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;

											// Ustawiamy czas następnego przerwania
												numerPrzerwaniaSecondProcessor++;
												if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
													najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
												else
													najblizszyMaintenanceSecondProcessor = -1;

											// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
												if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
													break;
										}

										// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeSecondProcessor (wystarczy zwiększyć ją o długość zadania)
											timeSecondProcessor += currentTaskSecondProcessor->duration;

										// Ustawiamy zmienną czasową zakończenia zadania
											currentTaskSecondProcessor->endTime = timeSecondProcessor;

										// Przestawiamy iterator oraz licznik edycji
											iteratorSP++;
											countTask++;

										// Zaznaczamy zadanie jako wykonane w pełni
											secondPart[taskIDSecondProcessor] = true;
									}
							}
					}
			}
		}

	// Dopisanie zadań ze zmienionymi wartościami
		for(int i = 0; i < iloscZadan; i++) {
			taskListFirstProcessor.push_back(taskListSecondProcessor[i]);
		}

	// Czyszczenie pamięci
		delete firstPart;
		delete secondPart;
		delete licznikOdwiedzonych;

	return taskListFirstProcessor;
}

void Turniej(vector< vector<Task*> > &solutionsList) {
	// Przeliczenie rozmiaru otrzymanej struktury listy rozwiązań
		int size = solutionsList.size();

	// Utworzenie struktry pomocniczej = tabeli przegranych oraz tabeli z wartościami funkcji celu
		int *solutionsValue = new int[size];
		bool *looserSolution = new bool[size];

	// Uzupełniami wartości w tabelach
		for(int i = 0; i < size; i++) {
			looserSolution[i] = false;
			solutionsValue[i] = ObliczFunkcjeCelu(solutionsList[i]);
		}

	// Turniej - wracamy do ilości rozwiązań jakie chcemy wygenerować
		int toKill = size - MAX_SOLUTIONS;
		int first, second;

		if(DEBUG)
			debugFile << "Kill = " << toKill << endl;

	// Pętla operacyjna
		while(toKill > 0) {
			first = (int)(rand() / (RAND_MAX + 1.0) * size);
			second = (int)(rand() / (RAND_MAX + 1.0) * size);

			if(DEBUG)
				debugFile << "First = " << first << " second =" << second << endl;

			if(first != second && !looserSolution[first] && !looserSolution[second]) {
				// Sprawdzamy które z rozwiązań ma mniejszą wartość funkcji celu
				if(solutionsValue[first] < solutionsValue[second])
					looserSolution[second] = true;
				else
					looserSolution[first] = true;
				toKill--;
			} else
				continue; // Ponawiamy iterację - albo to samo zadanie, albo wylosowano rozwiązanie które odpadło
		}

	// Usunięcie wykluczonych rozwiązań
		for(int i = size - 1; i >= 0; i--) {
			if(looserSolution[i]) {
				solutionsList.erase(solutionsList.begin() + i);
			}
		}

	// Czyszczenie pamięci operacyjnej
		delete looserSolution;
		delete solutionsValue;
}

void KopiujDaneOperacji(vector<Task*> &listaWejsciowa, vector<Task*> &listaWyjsciowa) {
	// Zmienna pomocnicza by skrócić czas pracy (nie trzeba x razy liczyć)
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

// Poszukiwanie najlepszego rozwiązania z wektora rozwiązań
vector <Task*> ZnajdzNajlepszeRozwiazanie (vector< vector < Task*> > &listaRozwiazan){
	// Zmienne operacyjne
		int sizeListyRozwiazan = listaRozwiazan.size(); // Rozmiar listy rozwiązań
		int minFunkcjiCelu = INT_MAX; // Zmienna z minimalną wartością funkcji celu
		vector <Task*> najlepszeRozwiazanie; // Najlepsze znalezione rozwiązanie
		int temp = 0; // Zmienna pomocnicza aby nie liczyć dwa razy wartości funkcji celu

	// Obliczanie wartości rozwiązania
		for(int i = 0; i < sizeListyRozwiazan; i++) {
			temp = ObliczFunkcjeCelu(listaRozwiazan[i]);

			if(temp < minFunkcjiCelu) { // Obliczona wartość jest lepsza niż dotychczasowe rozwiązanie
					najlepszeRozwiazanie = listaRozwiazan[i];
					minFunkcjiCelu = temp;
			}
		}

    return najlepszeRozwiazanie;
}

// dodaje do macierzy fermonow wartosci procentowo po turnieju
void DodajDoMacierzyFermonow(vector< vector<Task*> > &listaRozwiazan, int tablicaWartosciFunkcjiCelu[]){
    int sizeListyRozwiazan = listaRozwiazan.size(); //optymalizacja
    double prawdFunkcjiCelu[sizeListyRozwiazan]; // tablica wartosci funkcji celu (prawdopodobienstw)
    double sumaFunkcjiCelu=0.0; // suma wartosci funkcji celu - do wyznaczenia prawdopodobienstw
    for(int i=0;i<sizeListyRozwiazan;i++){
        prawdFunkcjiCelu[i]=(double)tablicaWartosciFunkcjiCelu[i]; // uzupelniamy tablice wartosciami f celu
        sumaFunkcjiCelu+=prawdFunkcjiCelu[i]; // liczymy sume
    }

    for(int i=0;i<sizeListyRozwiazan;i++){
        prawdFunkcjiCelu[i]=sumaFunkcjiCelu/prawdFunkcjiCelu[i]; // przeksztalcamy w prawdopodobienstwo
    }
    for(int i=0;i<sizeListyRozwiazan;i++){ // dla kazdego rozwiazania
        int sizeRozwiazania=listaRozwiazan[i].size(); // optymalizacja
        for(int j=1;j<sizeRozwiazania;j++){ // dla kazdego zadania (operacji)
            macierzFermonowa[listaRozwiazan[i][j-1]->ID-1][listaRozwiazan[i][j]->ID-1]+=prawdFunkcjiCelu[i];
        }
    }
}
// zanika slady fermonowe co iteracje
void zanikMacierzFermonowa(){
    for(int i=0;i<INSTANCE_SIZE;i++){
        for(int j=0;j<INSTANCE_SIZE;j++){
            macierzFermonowa[i][j]*=(double)(100-PROCENT_ZANIKANIA)/100;
        }
    }
}

// wypisuje macierz fermonowa
void wypiszMacierzFermonowa(){
    for(int i=0;i<INSTANCE_SIZE;i++){
        for(int j=0;j<INSTANCE_SIZE;j++){
            cout<<" ";
            cout.width(7);
            cout<<macierzFermonowa[i][j];
        }
        cout<<endl;
    }
    cout<<endl;
}
void utworzTabliceFunkcjiCelu(vector< vector <Task*> > &listarozwiazan, int tablica[]){
    int sizeListyRozwiazan = listarozwiazan.size();
    for(int i=0;i<sizeListyRozwiazan;i++){
        tablica[i]=ObliczFunkcjeCelu(listarozwiazan[i]);
    }
}

// funckja do splaszczania wiersza w macierzy fermonow (moze ten pow() to nie najlepsza funckja ale innej nie wymyslilem
void FunckjaSplaszczajace(int wiersz){
    for(int i=0;i<INSTANCE_SIZE;i++){
        macierzFermonowa[wiersz][i]=pow(macierzFermonowa[wiersz][i]*INSTANCE_SIZE,1/WYKLADNIK_POTEGI);
    }
}
// Główna pętla metaheurestyki
void GlownaPetlaMety (vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor, int numerInstancjiProblemu){
    clockid_t czasStart = clock(); // czas startu mety
    int numerIteracji = 0;
    int aktualnyWiersz =0; // dla funckji splaszczajacej
    vector <Task*> najlepszeRozwiazanie;
    vector < vector <Task*> > listaRozwiazan; // vector ze wszystkimi aktualnymi rozwiazaniami
     vector <Task*> tempTask;
    while ((clock()-czasStart)<MAX_DURATION_PROGRAM_TIME*CLOCKS_PER_SEC){ // warunek by meta nie działała dłużej niz MAX_DURATION_PROGRAM_TIME
        numerIteracji++;

        for(int i=0;i<MAX_SOLUTIONS;){
            if((rand()+1.0)<RAND_MAX*PROBABILTY_OF_RANDOM_GENERATION/100){ // tworzy totalnie losowe z prawdopodobienstwem
            tempTask=GeneratorLosowy(listaZadan,listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor);
            if(numerIteracji==1 && i==0) {// pierwsze napotkane rozwiazanie jest najlepszym
                    KopiujDaneOperacji(tempTask,najlepszeRozwiazanie);
            }
            listaRozwiazan.push_back(tempTask);
            i++; // dodanie rozwiazania do puli
            // zapis do pliku wszystkich wygenerowanch losowa rozwizan dla sprawdzenia czy sie robia
            /*
            string nazwa = "ITER_" + to_string(numerIteracji) + "_LOS_" + to_string(i);
            ZapiszWynikiDoPliku(tempTask,listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor, firstSolutionValue,numerInstancjiProblemu,nazwa);
            */
            tempTask.clear(); // czyszczenie vectora by nie bylo problemow
            }
            else { // tworzy za pomoca tablicy fermonow
                ///TODO
            }
        }
        // zrobienie mutacji

        for(int i=0;i<MAX_SOLUTION_AFTER_MUTATION-MAX_SOLUTIONS;i++){ // musimy dodac roznice tych wartosci
            tempTask = Mutacja(listaRozwiazan[i%MAX_SOLUTIONS],listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor);
            // zapis do pliku wszystkich mutacji dla sprawdzenia czy sie robia
            /*
            string nazwa = "ITER_" + to_string(numerIteracji) + "_MUT_" + to_string(i);
            ZapiszWynikiDoPliku(tempTask,listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor, firstSolutionValue,numerInstancjiProblemu,nazwa);
            */
            listaRozwiazan.push_back(tempTask);
            tempTask.clear();
        }
        // turniej
        Turniej(listaRozwiazan);

        // sortowanie po czasie zakonczenia aby bylo bez problemu w macierzy fermonow i tabeli wartosci funkcji celu
        SortujListeZadanPoEndTime(listaRozwiazan);

        // tworzy tablice funkcji celu
        int tablicaWartosciFunkcjiCelu[listaRozwiazan.size()];
        utworzTabliceFunkcjiCelu(listaRozwiazan,tablicaWartosciFunkcjiCelu);

        // uzupelnia macierz fermonowa
        DodajDoMacierzyFermonow(listaRozwiazan,tablicaWartosciFunkcjiCelu);

        // zanika slad fermonowy
        zanikMacierzFermonowa();

        //funkcja splaszczajaca
        if(!(numerIteracji%CO_ILE_ITERACJI_WIERSZ)){
                FunckjaSplaszczajace(aktualnyWiersz);
                aktualnyWiersz=(aktualnyWiersz+1)%INSTANCE_SIZE; //wylicza aktualny wiersz do funkcji
        }

        // zapis do pliku rozwiazan ktore przeszly turniej (po to by sprawdzic czy najlepsze rozwiazanie aktualnie to jest najlepsze
        for(int i=0;i<listaRozwiazan.size();i++){
            string nazwa = "ITER_" + to_string(numerIteracji) + "_TUR_" + to_string(i);
            ZapiszWynikiDoPliku(listaRozwiazan[i],listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor, firstSolutionValue,numerInstancjiProblemu,nazwa);
        }

        // zapamietywanie najlepszego rozwiazania
        tempTask = ZnajdzNajlepszeRozwiazanie(listaRozwiazan); // najlepsze z tej iteracji
       if(ObliczFunkcjeCelu(tempTask)<ObliczFunkcjeCelu(najlepszeRozwiazanie)) { // porownanie z najlepszym globalnie
            najlepszeRozwiazanie.clear(); // czyszcze dla pewnosci
            KopiujDaneOperacji(tempTask,najlepszeRozwiazanie); // kopia do najlepszego
       }
       tempTask.clear();
    }
    // zapisz najlepszego rozwiazania do pliku
    string nazwa = "NAJLEPSZE";
    ZapiszWynikiDoPliku(najlepszeRozwiazanie,listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor, firstSolutionValue,numerInstancjiProblemu,nazwa);

}
int main() {
	srand(time(NULL)); // Ino roz reset

	debugFile.open("debug.txt");
	int rozmiarInstancji = INSTANCE_SIZE;
	int numerInstancjiProblemu = INSTANCE_NUMBER;

	// Utworzenie wektora na n zadań
		vector<Task*> zadania;

	// Wektor przerwań pracy na maszynach
		vector<Maintenance*> listaPrzerwan;

	// Wygenerowanie zadań
		GeneratorInstancji(zadania, rozmiarInstancji, LOWER_TIME_TASK_LIMIT, UPPER_TIME_TASK_LIMIT);

	// Wygenerowanie przerwań
		GeneratorPrzestojow(listaPrzerwan, MAINTENANCE_FIRST_PROCESSOR, MAINTENANCE_SECOND_PROCESSOR, LOWER_TIME_MAINTENANCE_LIMIT, UPPER_TIME_MAINTENANCE_LIMIT, LOWER_READY_TIME_MAINTENANCE_LIMIT, UPPER_READY_TIME_MAINTENANCE_LIMIT);
//		OdczytPrzerwan(listaPrzerwan);

	// Zapis danych do pliku
		string nameParam;
		stringstream ss;
    	ss << numerInstancjiProblemu;
    	ss >> nameParam; // Parametr przez stringstream, funkcja to_string odmówiła posłuszeństwa
		ZapiszInstancjeDoPliku(zadania, listaPrzerwan, numerInstancjiProblemu, nameParam);

	// Wczytanie danych z pliku
//		WczytajDaneZPliku(listaZadan, listaPrzerwan, numerInstancjiProblemu, nameParam);

		vector<Maintenance*> przerwaniaFirstProcessor;
		vector<Maintenance*> przerwaniaSecondProcessor;
		SortujPrzerwania(listaPrzerwan);
		PodzielStrukturyNaMaszyny<Maintenance>(listaPrzerwan, przerwaniaFirstProcessor, przerwaniaSecondProcessor);

		vector<Task*> listaZadan;
		listaZadan = GeneratorLosowy(zadania, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
//		OdczytDanychZadan(listaZadan);

		ZapiszWynikiDoPliku(listaZadan, przerwaniaFirstProcessor, przerwaniaSecondProcessor, firstSolutionValue, numerInstancjiProblemu, nameParam);

		long int wynik = ObliczFunkcjeCelu(listaZadan);
//		OdczytPrzerwan(listaPrzerwan);
//		OdczytDanychZadan(listaZadan);
		UtworzGraf(listaZadan, listaPrzerwan, wynik, nameParam);
		nameParam += "w";

		vector<Task*> nowe = GeneratorLosowy(zadania, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
		nowe = Mutacja(nowe, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
//		OdczytDanychZadan(listaZadan);
		vector< vector<Task*> > solution;


		solution.push_back(nowe);
		solution.push_back(listaZadan);
		cout << "S1 " << ObliczFunkcjeCelu(nowe) << endl;
		OdczytDanychZadan(nowe);
		cout << "S2 " << ObliczFunkcjeCelu(listaZadan) << endl;
		OdczytDanychZadan(listaZadan);
		UtworzGraf(nowe, listaPrzerwan, wynik, nameParam);

        //GlownaPetlaMety
        GlownaPetlaMety(zadania,przerwaniaFirstProcessor,przerwaniaSecondProcessor,numerInstancjiProblemu);
		Turniej(solution);

		for(int i = 0; i < solution.size(); i++) {
			cout << "Zapis dla i = " << i << endl;
			long int wyn = ObliczFunkcjeCelu(solution[i]);
			string newNameParam;
			stringstream ss;
			ss << nameParam << "_" << i + 1;
			ss >> newNameParam;
			UtworzGraf(solution[i], listaPrzerwan, wyn, newNameParam);
		}

	// Czyszczenie pamięci - zwalnianie niepotrzebnych zasobów
		przerwaniaFirstProcessor.clear();
		przerwaniaSecondProcessor.clear();
		listaPrzerwan.clear();
		listaZadan.clear();

	return 0;
}
