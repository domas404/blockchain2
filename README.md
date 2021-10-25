# 2-oji užduotis. Supaprastintos blokų grandinės (blockchain) kūrimas.

## Programos veikimas

1. Pirmiausiai programa sugeneruoja **1000** vartotojų, kurie turi šiuos atributus:
    * name;
    * public key;
    * balance.
2. Tada sugeneruojama **10'000** tranzakcijų, tutinčių šiuos atributus:
    * id (hash of sender's and receiver's public keys and sum);
    * sender's public key;
    * receiver's public key;
    * sum;
3. Tikrinamos pirmos **100** tranzakcijų, iš kurių visos sėkmingos bus priskirtos naujam blokui (tikrinama ar siuntėjo
norima persiųsti suma yra ne didesnė už jo sąskaitos balansą).
4. Pradedamas bloko kasimo procesas (ieškoma nonce vertė, pagal nurodytą *difficulty target*).
5. Naujai iškastas blokas pridedamas į blokų grandinę.

**Note:** hash'avimui naudojama *sha256* funkcijos implementacija, nes ankščiau kurta *hash* funkcija nėra pakankamai
efektyvi, kad galėtų būti naudojama blokų kasimui.

### Konsolėje rodoma ši informacija apie iškastą bloką:
* Block number (height);
* Previous block hash;
* Timestamp;
* Version;
* Merkleroot (transactions string line hash);
* Nonce;
* Difficulty;
* Block hash;
* Time since when last block was mined;
* Number of transactions within a block;

PVZ:
![block info](pvz.png)