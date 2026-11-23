# SSH latence
## Naměřený přenos

- Meřeno pomicí iperf3 -c 192.168.100.7 -t 20
  - Průměr je jen ~13–14 Mbit/s, ale to není to nejhorší. Rychlost skáče od 0 po 33 Mbit/s.
  - Několik úseků po 1 až 2 s má nulový přenos (u Macu v čase 7–9 s, 15–16 s, 17–18 s; Jetson to potvrzuje). Při použití terminálu to znamená, že se obraz na vteřinu až dvě zasekne.
   - ***Retr 653***: TCP muselo znovu poslat 653 segmentů, tedy pakety se ztrácely.
Cwnd 1,41 KB v 8. až 9. sekundě je propad spojení po timeoutu. To je typický příznak ztrát na radiovém spoji.
   - Cwnd rostlo až na ~3,8 MB, což u spoje s ~13 Mbit/s naznačuje velké fronty někde na cestě. Při zaplnění spoje by to znamenalo zpoždění v řádu sekund. Je to můj odhad z čísel, nepotvrzený měřením.