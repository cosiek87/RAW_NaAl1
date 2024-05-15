#include <iostream>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TMath.h>

void join() {
    // Inicjalizacja łańcucha i dodawanie plików ROOT
    TChain chain("Data_R");
    for (int i = 0; i < 8; ++i) {
        chain.Add(Form("DataR_CH%d@DT5730SB_10808_run.root", i)); // %d to miejsce, w którym następuje wpisanie numeru kanału
    }

    // Zmienne do przechowywania danych
    UShort_t energia, energia_other, channel;
    ULong64_t czas;

    // Ustawianie wskaźników na gałęzie
    chain.SetBranchAddress("Timestamp", &czas);
    chain.SetBranchAddress("Channel", &channel);
    chain.SetBranchAddress("Energy", &energia);
    chain.SetBranchAddress("EnergyShort", &energia_other);

    // Pobieranie liczby wpisów
    Long64_t nEntries = chain.GetEntries();
    std::vector<Long64_t> index(nEntries);
    std::vector<Long64_t> timestamps(nEntries);

    // Tymczasowe drzewo do przechowywania nieposortowanych danych
    TTree firstTree("FIRST_Data_R", "Tree with events sorted by time");
    firstTree.Branch("Timestamp", &czas, "Timestamp/l");
    firstTree.Branch("Channel", &channel, "Channel/s");
    firstTree.Branch("Energy", &energia, "Energy/s");
    firstTree.Branch("EnergyShort", &energia_other, "EnergyShort/s");

    // Wypełnianie tymczasowego drzewa i zapisywanie znaczników czasu
    for (Long64_t i = 0; i < nEntries; ++i) {
        chain.GetEntry(i);
        firstTree.Fill();
        timestamps[i] = czas;
    }

    // Sortowanie znaczników czasu
    std::cout << "Get tree index" << std::endl;
    TMath::Sort(nEntries, timestamps.data(), index.data(), false);
    std::cout << "Posortowano" << std::endl;

    // Tworzenie nowego pliku i drzewa do przechowywania posortowanych danych
    TFile f2("DataR_10808_run.root", "RECREATE");
    TTree sortedTree("Data_R", "Tree with events sorted by time");
    sortedTree.Branch("Timestamp", &czas, "Timestamp/l");
    sortedTree.Branch("Channel", &channel, "Channel/s");
    sortedTree.Branch("Energy", &energia, "Energy/s");
    sortedTree.Branch("EnergyShort", &energia_other, "EnergyShort/s");
    sortedTree.SetCacheSize(1000 * 1000 * 2000);

    // Przechodzenie przez posortowane wpisy i zapisywanie do nowego drzewa
    for (Long64_t i = 0; i < nEntries; ++i) {
        if ((i + 1) % 0xfffff == 0) firstTree.DropBaskets(); // Czyszczenie koszyków co pewien czas
        firstTree.GetEntry(index[i]);
        sortedTree.Fill();
    }

    // Zapisanie drzewa do pliku i zwolnienie pamięci
    sortedTree.Write();
    std::cout << "Zapisano posortowane drzewo do pliku DataR_10808_run.root" << std::endl;
}
