# Wildcard support for HelenOS - MSoC Project by Patrik Pritrsky

V tomto projekte som pridal podporu pre zástupné znaky (wildcards) v HelenOS shelle. Teda, ak používateľ bude chcieť vymazať všetky textové súbory v priečinku, tak mu stačí napísať 'rm *.txt', namiesto toho, aby ich všetky vymenoval.

## Funkcie

### Štandardný zástupný znak/wildcard *

Expanduje sa na nula alebo viacero znakov, vyhodnocovanie prebieha rekurzívne na všetkých úrovniach, kde sa nachádza.

Teda napríklad 'priecinok*/subor*.txt', nájde všetky textové súbory začínajúce sa na 'subor', vo všetkých podprečinkoch aktuálneho priečinku začínajúcich sa na 'priecinok'.

### Rekurzívny zástupný znak/wildcard **

Funguje na nájdenie súborov, ktoré sú ľubovoľne hlboko.

Teda napríklad '**/*.txt', nájde všetky textové súbory, ľubovoľne hlboko v aktuálnom priečinku.

## Zoznam zmien do HelenOS

- Pridanie automatizovaných testov pre zástupné znaky
- Vytvorenie funkcie na detekovanie, či reťazec obsahuje zástupný znak
- Vytvorenie funkcie na porovnanie, či sa zástupný znak pattern zhoduje s názvom súboru/priečinku
- Vytvorenie funkcie na rekurzívne expandovanie a nájdenie všetkých výskytov súborov/priečinkov, ktoré sa zhodujú s cestou/názvom súboru obsahujúcim zástupné znaky
- Zmena v tokenizátore HelenOS shellu, tak, aby podporoval expanziu zástupných znakov