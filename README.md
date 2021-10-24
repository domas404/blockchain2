# 2-oji užduotis. Supaprastintos blokų grandinės (blockchain) kūrimas.

## Programos veikimas

1. Pirmiausiai programa sugeneruoja **1000** vartotojų, kurie turi šiuos atributus:
    * name;
    * public key;
    * balance.
2. Tada sugeneruojama **10'000** tranzakcijų.
3. Tikrinamos pirmos **100** tranzakcijų, iš kurių visos sėkmingos bus priskirtos naujam blokui.
4. Pradedamas bloko kasimo procesas.
5. Naujai iškastas blokas pridedamas į blokų grandinę.

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