// Scocca della molletta Bianca (Waveshare ESP32-C6-Touch-LCD-1.69 + LiPo 803040).
// Unita' in mm. Si stampa un pezzo alla volta: cambia "pezzo" e esporta l'STL
// (oppure: openscad -D 'pezzo="guscio"' -o guscio.stl scocca.scad).
//
// Le misure segnate con DA MISURARE sono stime: vanno controllate con il calibro.

pezzo = "assieme"; // assieme | guscio | coperchio | stella | cuore

/* [Display e scheda] (disegno Waveshare) */
vetro = [41.13, 33.13];       // vetro del display
vetro_raggio = 4;             // angoli del vetro
spessore_scheda = 7.5;        // DA MISURARE: dal vetro al componente piu' alto sul retro
pcb = [37.12, 29.83];
pcb_dall_alto = 1.75;         // bordo alto del pcb rispetto a quello del vetro
usb_c = [9.4, 3.6];           // apertura della USB-C (larghezza, altezza)
usb_c_profondita = 4.6;       // DA MISURARE: centro della USB-C dietro al vetro
pulsanti_x = [12.0, 20.9, 29.9]; // RST, BOOT, PWR dal bordo sinistro del vetro

/* [Batteria] */
batteria = [40.5, 30.5, 8.3]; // DA MISURARE: 803040 con un po' di margine

/* [Molletta a scatto] */
// si incolla in un incavo a goccia, largo apposta: le misure possono essere approssimative
molletta_lunghezza = 50;      // circa
molletta_larghezza = 12;      // punto piu' largo, circa
molletta_punta = 5;           // vicino alla punta stretta, circa
incavo_margine = 1.0;         // spazio in piu' per lato
incavo_profondita = 0.6;

/* [Stampa FDM] */
gioco = 0.25;                 // tolleranza fra pezzi che si incastrano
parete = 2.2;
bordino = 1.4;                // quanto il bordo davanti copre il vetro
fronte = 1.2;                 // spessore del bordo davanti
fondo = 2.0;                  // spessore del coperchio
tappo = 1.6;                  // parte del coperchio che entra nel guscio
$fn = 48;

interno = [vetro[0] + 2 * gioco, vetro[1] + 2 * gioco];
esterno = [interno[0] + 2 * parete, interno[1] + 2 * parete];
raggio_esterno = vetro_raggio + parete;
profondita = fronte + spessore_scheda + batteria[2] + 0.6 + tappo; // fino al coperchio

module rettangolo_tondo(misure, raggio, altezza, center = false) {
  r = min(raggio, min(misure[0], misure[1]) / 2 - 0.01);
  linear_extrude(altezza, center = center)
    offset(r) offset(-r) square(misure, center = true);
}

module stella_2d(r_fuori, r_dentro) {
  polygon([for (i = [0:9]) let(r = i % 2 ? r_dentro : r_fuori, a = 90 + i * 36) [r * cos(a), r * sin(a)]]);
}

module cuore_2d(lato) {
  translate([0, -lato * 0.25]) rotate(45) {
    square(lato, center = true);
    translate([0, lato / 2]) circle(d = lato);
    translate([-lato / 2, 0]) circle(d = lato);
  }
}

// angolo in alto a sinistra: sulla diagonale dell'arrotondamento esterno la cornice e' piena
// (dietro non c'e' vetro), quindi il perno della stella entra nel materiale
dentro_angolo = (raggio_esterno - 1.9) / sqrt(2);
pos_stella = [-(esterno[0] / 2 - raggio_esterno + dentro_angolo), esterno[1] / 2 - raggio_esterno + dentro_angolo];

module guscio() {
  difference() {
    union() {
      rettangolo_tondo(esterno, raggio_esterno, profondita);
      // linguetta per il ciondolo in alto a destra, a filo col davanti: si stampa sul piatto
      // e l'anellino la attraversa, cosi' il ciondolo pende verso il basso
      translate([esterno[0] / 2 - 1.5, esterno[1] / 4, 0]) difference() {
        hull() {
          translate([4.5, 0, 0]) cylinder(r = 3, h = 2.4);
          translate([0, -3, 0]) cube([1, 6, 2.4]);
        }
        translate([4.5, 0, -1]) cylinder(d = 2.2, h = 5);
      }
    }
    // finestra del display: lascia libero il vetro tranne il bordino
    translate([0, 0, -1]) rettangolo_tondo([vetro[0] - 2 * bordino, vetro[1] - 2 * bordino], vetro_raggio - bordino, fronte + 2);
    // vano per vetro, scheda e batteria
    translate([0, 0, fronte]) rettangolo_tondo(interno, vetro_raggio + gioco, profondita);
    // USB-C al centro del lato basso
    translate([0, -esterno[1] / 2, fronte + usb_c_profondita])
      rotate([90, 0, 0]) rettangolo_tondo(usb_c, usb_c[1] / 2, parete * 3, center = true);
    // aperture dei pulsanti sul lato alto (si premono con la punta di una matita)
    for (x = pulsanti_x)
      translate([x - vetro[0] / 2, esterno[1] / 2, fronte + 4.2]) rotate([90, 0, 0]) cylinder(d = 2.6, h = parete * 3, center = true);
    // foro per il perno della stella
    translate([pos_stella[0], pos_stella[1], -1]) cylinder(d = 2.2 + gioco, h = 1 + 2.6);
  }
}

// si stampa con il tappo sul piatto, l'incavo della molletta resta in alto
module coperchio() {
  alto = tappo + fondo;
  difference() {
    union() {
      rettangolo_tondo([interno[0] - 2 * gioco, interno[1] - 2 * gioco], vetro_raggio, tappo);
      translate([0, 0, tappo]) rettangolo_tondo(esterno, raggio_esterno, fondo);
    }
    // passaggio per il cavo della batteria verso il connettore in basso a destra
    translate([interno[0] / 2 - 10, -interno[1] / 2 + 1, -1]) cube([8, 5, tappo + 1.5]);
    // incavo per incollare la molletta, centrato; se la molletta e' piu' lunga sporge dai lati
    translate([0, 0, alto - incavo_profondita]) linear_extrude(1) intersection() {
      offset(r = -1.5) offset(r = raggio_esterno) offset(r = -raggio_esterno) square(esterno, center = true);
      sagoma_molletta(2 * incavo_margine);
    }
  }
}

module sagoma_molletta(extra = 0) {
  hull() {
    translate([-molletta_lunghezza / 2 + molletta_larghezza / 2, 0]) circle(d = molletta_larghezza + extra);
    translate([molletta_lunghezza / 2 - molletta_punta / 2, 0]) circle(d = molletta_punta + extra);
  }
}

// si stampa con la faccia sul piatto e il perno verso l'alto
module stella() {
  linear_extrude(2.4) stella_2d(6.5, 2.8);
  translate([0, 0, 2.4]) cylinder(d = 2.2, h = 2.4); // perno
}

module cuore() {
  difference() {
    linear_extrude(3) cuore_2d(9);
    translate([0, 5.2, -1]) cylinder(d = 2, h = 5); // foro per l'anellino
  }
}

if (pezzo == "guscio") guscio();
else if (pezzo == "coperchio") coperchio();
else if (pezzo == "stella") stella();
else if (pezzo == "cuore") cuore();
else {
  color("#7e22ce") guscio();
  color("#5b21b6") translate([0, 0, profondita + 8]) mirror([0, 0, 0]) coperchio();
  color("#f0abfc") translate([pos_stella[0], pos_stella[1], -8]) stella();
  color("#f0abfc") translate([esterno[0] / 2 + 3, esterno[1] / 4 - 9, 0]) cuore();
}
