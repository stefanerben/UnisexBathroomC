/**
 * Unisex Bathroom Problem - C-Programm
 * by Stefan Erben & Andreas Mattes 5BHEL
 */

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define MAXMAENNER 5        //Wieviele Meanner duerfen rein
#define MAXFRAUEN 5         //Wieviele Frauen duerfen rein
#define WAITTIME 2          //Zeit im Bad und zwischen Badbesuche

void * male(void * );       //Maenner Methoden Deklaration
void * female(void * );     //Frauen Methoden Deklaration

sem_t crit_sem, warten_maenner_sem, warten_frauen_sem; //Verwendete Semaphoren

//Definition der Counter und bool wert ob noch wer rein darf
int maenner_in, frauen_in, maenner_warten, frauen_warten;
bool lassLeuterein;

//Wieviele Maenner und Frauen sollen reingehen
//ToDo übergabe als Startparameter vom benutzer
int maenner, frauen;

// Main, initialisiert Semaphoren und Counter
int main(int argc, char * * argv) {
    pthread_attr_t attr;
    pthread_t personen[MAXMAENNER + MAXFRAUEN]; //Thread starten
    int index; //Zaehlvariable

    if (argc < 3) // Fehlerabfrage ob inkorrekte Anzahl als Startargumente übergeben wurde
    {
        printf("ERROR. Falsche parametrisierung\n");
        return EXIT_FAILURE;
    }
    // Maenner und Frauen aus Argumenten lesen und überprüfen ob diese im erlaubten Bereich gewählt wurden
    else {
        maenner = atoi(argv[1]);
        if (maenner > MAXMAENNER) {
            printf("\nSie haben eine zu grosse Anzahl eingeben!\nDie Anzahl der Maenner wurde auf die Maximalanzahl (%d) gesetzt!\n\n", MAXMAENNER);
            maenner = MAXMAENNER; //Mehr Maenner als MAXMAENNER Zahl -> Maenner auf Max setzen
        } else if (maenner == 0) {
            printf("\nSie haben eine zu kleine Anzahl eingeben!\nDie Anzahl der Maenner wurde auf die Mindestanzahl (1) gesetzt!\n\n");
            maenner = 1;
        }

        frauen = atoi(argv[2]);
        if (frauen > MAXFRAUEN) //Mehr Frauen als MAXFRAUEN Zahl -> Frauen auf Max setze
        {
            printf("\nSie haben eine zu grosse Anzahl eingeben!\nDie Anzahl der Frauen wurde auf die Maximalanzahl (%d) gesetzt!\n\n", MAXFRAUEN);
            frauen = MAXFRAUEN;
        } else if (frauen == 0) {
            printf("\nSie haben eine zu kleine Anzahl eingeben!\nDie Anzahl der Frauen wurde auf die Mindestanzahl (1) gesetzt!\n\n");
            frauen = 1;
        }

        //Ausgabe der eingegeben Informationen
        printf("\n****************************************\n");
        printf("* folgende Parameter werden verwendet: *\n");
        printf("* Maenner: %d \t\t Frauen: %d     *\n", maenner, frauen);
        printf("****************************************\n\n");
    }

    //threads parametrisieren
    pthread_attr_init( & attr);
    pthread_attr_setscope( & attr, PTHREAD_SCOPE_SYSTEM);

    //Semaphoren parametrisieren
    sem_init( & crit_sem, 0, 1); //Argument 0 weil Semaphore nicht geshared werden soll
    sem_init( & warten_maenner_sem, 0, 0); //Argument 0 weil Semaphore nicht geshared werden soll
    sem_init( & warten_frauen_sem, 0, 0); //Argument 0 weil Semaphore nicht geshared werden soll

    //Counter initialiseren
    maenner_in = 0;
    frauen_in = 0;
    maenner_warten = 0;
    frauen_warten = 0;

    lassLeuterein = false; //anfangs niemanden reinlassen

    for (index = 0; index < maenner; index++) // Maenner erstellen
    {
        pthread_create( & personen[index], & attr, male, NULL);
    }
    for (index = 0; index < frauen; index++) // Frauen erstellen
    {
        pthread_create( & personen[maenner + index], & attr, female, NULL);
    }
    for (index = 0; index < maenner + frauen; index++) {
        pthread_join(personen[index], NULL);
    }
    return EXIT_SUCCESS;
}

// Gibt den derzeitigen Status aus, also wievile sind im Wartezimmer und wieviele in Bad?
void print_status() {
    // Status ausgeben
    printf("\nIm Bad:\t\t M=%d, F=%d.\nEs warten:\t M=%d, F=%d.\n\n", maenner_in, frauen_in, maenner_warten, frauen_warten);
}

// MAENNER!
void * male(void * input) {
    int i; //Indexvariable für Benachrichtigung der Männer

    // Maenner im Wartezimmer
    int maenner_inWarteschlange;

    sleep(WAITTIME); //Zeit wird simuliert während das Bad benutz wird/bis jemand ins Bad muss

    sem_wait( & crit_sem); //Critical section beginnt!
    maenner_warten++; //Anzahl der wartenden Männer erhöhen

    // Status ausgeben
    printf("Mann will ins Badezimmer.");
    print_status();

    // Wenn grad Frauen im Bad sind oder warten
    if (frauen_in > 0 || frauen_warten > 0) {
        // Status ausgeben
        printf("Maenner warten dass sie dran kommen.\n\n");
        //print_status();

        sem_post( & crit_sem);
        sem_wait( & warten_maenner_sem); // Warten bis Maenner drann sind/

        // ein mann rein -> einer weniger im Wartezimmer
        maenner_in++;
        maenner_warten--;

        // jetzt wen reinlassen
        if (lassLeuterein == false) {
            lassLeuterein = true; //Soll nur einmal passieren

            maenner_inWarteschlange = maenner_warten;

            // Status ausgeben
            printf("Maenner sind dran.\n\n%d. Mann betritt das Badezimmer.", maenner_in);
            print_status();

            // Andere Maenner vom Warteraum reinlassen
            for (i = 0; i < maenner_inWarteschlange; i++) {
                sem_post( & warten_maenner_sem);
                sem_wait( & crit_sem);
            }
            // Keine Männer mehr reinlassen
            lassLeuterein = false;
        } else {
            // Status ausgeben
            printf("%d. Mann betritt das Badezimmer.", maenner_in);
            print_status();
        }
    } else // Wenn maenner bereits drin
    {
        maenner_in++;
        maenner_warten--;

        // Status ausgeben
        printf("Mann betritt das Badezimmer.\n");
        print_status();
    }
    sem_post( & crit_sem); //Ende der critical section.

    // Zeit für Toilette simulieren
    sleep(WAITTIME);

    // Badezimmer verlassen
    sem_wait( & crit_sem); // Critical section eginnt
    maenner_in--; /* Ein mann geht raus */

    // Status ausgeben
    printf("%d. Mann verlaesst das Badezimmer.", maenner_in + 1);
    print_status();

    //Wenn der letzte mann rausgeht

    if (maenner_in == 0) {
        // Ausgabe
        printf("Letzter Mann hat das Badezimmer verlassen.\n");
        //print_status();
        if (frauen_warten > 0)
            sem_post( & warten_frauen_sem);
        else
            sem_post( & crit_sem);
    } else // ende der critical session
    {
        sem_post( & crit_sem);
    }
    pthread_exit(NULL);
}

// FRAUEN
// Prinzipiell gleich wie maenner
void * female(void * input) {
    int i;
    int frauen_inWarteschlange; //Anzahl der Frauen in Warteschlange
    {
        sleep(WAITTIME); // Zeit für Toilettenbesuch simulieren
        sem_wait( & crit_sem); // Start der critical section.
        frauen_warten++; //Anzahl der wartenden Frauen erhöhen

        // Status ausgeben
        printf("Frau will ins Badezimmer.");
        print_status();

        // sind gerade maenner drin?
        if (maenner_in > 0 || maenner_warten > 0) {
            // Status ausgeben
            printf("Frauen warte dass sie dran sind.\n\n");

            sem_post( & crit_sem); //ende der critical section
            sem_wait( & warten_frauen_sem); // frauen warten dass sie dran sind

            // Status aktualisieren
            frauen_in++;
            frauen_warten--;

            if (lassLeuterein == false) {
                lassLeuterein = true; // nur die erste frau soll das machen

                frauen_inWarteschlange = frauen_warten;

                // Status ausgeben
                printf("Frauen sind dran, die erste geht hinein.");
                print_status();

                // alle anderen Frauen auch reinlassen*/
                for (i = 0; i < frauen_inWarteschlange; i++) {
                    sem_post( & warten_frauen_sem);
                    sem_wait( & crit_sem);
                }
                lassLeuterein = false; //wenn alle drinnen dann niemanden mehr reinlassen
            } else {
                //Ausgabe
                printf("%d. Frau betritt das Badezimmer.", frauen_in);
                print_status();
            }
        } else // wenn bereits frauen drinnen sind
        {
            frauen_in++;
            frauen_warten--;
            printf("%d. Frau betritt das Badezimmer.", frauen_in);
            print_status();
        }
        sem_post( & crit_sem); //ende der critical section

        // warten
        sleep(WAITTIME);

        sem_wait( & crit_sem); //Start der critical section um badezimmer zu verlassen
        frauen_in--; //eine Frau weniger

        //Ausgabe
        printf("%d. Frau verlaesst das Badezimmer.", frauen_in + 1);
        print_status();

        //letzte Frau
        if (frauen_in == 0) {
            //Ausgabe
            printf("Letzte Frau hat das Badezimmer verlassen.\n\n");
            //print_status();

            if (maenner_warten > 0)
                sem_post( & warten_maenner_sem);
            else
                sem_post( & crit_sem);
        } else
            sem_post( & crit_sem);
    }

    //pthread_exit
    pthread_exit(NULL);
}