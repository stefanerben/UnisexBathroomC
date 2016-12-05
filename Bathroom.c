/**
 * Unisex Bathroom Problem - C-Programm
 * by Stefan Erben & Andreas Mattes 5BHEL
 */

//Testcomment ob Gitintegration in IDE funktioniert

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

#define MAXMAENNER 5            //Wieviele Meanner duerfen rein
#define MAXFRAUEN 5     //Wieviele Frauen duerfen rein
#define WAITTIME 2        //Zeit im Bad und zwischen Badbesuche

void * male(void *);    //maenner Methoden Deklaration
void * female(void *);  //Frauen Methoden Deklaration

sem_t crit_sem, warten_maenner_sem, warten_frauen_sem; //verwendete Semaphoren

//Definition der Counter und bool wert ob noch wer rein darf
int maenner_in, frauen_in, maenner_warten, frauen_warten;
bool lassLeuterein;

//ieviele Maenner und Frauen sollen reingehen
int maenner, frauen;

// Main
int main(int argc, char ** argv)
{
    pthread_attr_t attr;
    pthread_t personen[MAXMAENNER + MAXFRAUEN]; //Thread starten
    int index; //Zaehlvariable

    if(argc < 3) // Fehlerabfrage TODO nochverbessern
    {
        printf("ERROR. Falsche parametrisierung\n");
        return EXIT_FAILURE;
    }
    // Maenner und Frauen aus Argumenten lesen 
    else
    {
        maenner = atoi(argv[1]);
        //TODO falls maenner mehr als Max zahl

        // TODO Fehlerabfrage ob es zuviele sind
        frauen = atoi(argv[2]);
        //TODO falls maenner mehr als Max zahl
    }

    //threads parametrisieren
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    //Semaphoren parametrisieren
    sem_init(&crit_sem, 0, 1);            //2. Argument 0 weil sem nicht geshared werden soll
    sem_init(&warten_maenner_sem, 0, 0);        //2. Argument 0 weil sem nicht geshared werden soll
    sem_init(&warten_frauen_sem, 0, 0); //2. Argument 0 weil sem nicht geshared werden soll

    //Counter initialiseren
    maenner_in = 0;
    frauen_in = 0;
    maenner_warten = 0;
    frauen_warten = 0;

    lassLeuterein = false; //Niemanden reinlassen

        for(index = 0; index < maenner; index++) // Maenner erstellen
        {
            pthread_create(&personen[index], &attr, male, NULL);
        }
        for(index = 0; index < frauen; index++) // Frauen erstellen
        {
            pthread_create(&personen[maenner + index], &attr, female, NULL);
        }
        for(index = 0; index < maenner + frauen; index++) 
        {
            pthread_join(personen[index], NULL);
        }
        return EXIT_SUCCESS; 
}

// Gibt den derzeitigen Status aus, also wievile sind im Wartezimmer und wieviele in Bad?
void print_status()
{
    //Ausgabe
    printf("\nIm Bad:\t\t M=%d, F=%d.\nEs warten:\t M=%d, F=%d.\n\n", maenner_in, frauen_in, maenner_warten, frauen_warten);
}

// MAENNER!
void * male(void * input)
{
    int i; //Indexvariale

    // Maenner im Wartezimmer
    int maenner_inWarteschlange;

    // TODO alles einruecken
    
    sleep(WAITTIME);    //Badezimmer wird verwendet

    sem_wait(&crit_sem); //Critical section beginnt!
    maenner_warten++; //ein Mann mehr wartet

    // Ausgabe
    printf("Maenner wollen ins Badezimmer.");
    print_status();

    // Wenn grad frauen drin
    if(frauen_in > 0 || frauen_warten > 0)
    {
        // Ausgabe
        printf("Maenner warten dass sie dran kommen.");
        print_status();

        sem_post(&crit_sem);
        sem_wait(&warten_maenner_sem); // Warten bis Maenner drann sind/
        // baton technique.
        

        // ein mann rein -> einer weniger im Wartezimmer
        maenner_in++;
        maenner_warten--;

        // jetzt wen reinlassen
        if(lassLeuterein == false)
        {
            lassLeuterein = true;

            maenner_inWarteschlange = maenner_warten;

            // Ausgabe
            printf("Maenner sind dran, der erste geht hinein.");    // TODO \n1. Mann geht rein.
            print_status();

            for(i = 0; i < maenner_inWarteschlange; i++)
            {
                sem_post(&warten_maenner_sem);
                sem_wait(&crit_sem);
            }
            // Maenner reinlassen ist ageschlossen
            lassLeuterein = false;
        }
        else
        {
            // Ausgabe
            printf("Mann betritt das Badezimmer." );  // TODO %d. ,maenner_in
            print_status();
        }

    }
    else // wenn maenner bereits drin
    {
        maenner_in++;
        maenner_warten--;

          // Ausgabe
        printf("Mann betritt das Badezimmer.\n");
        print_status();
    }
    sem_post(&crit_sem); //Ende der critical section.

    // warten
    sleep(WAITTIME);

    // badezimmer verlassen
    sem_wait(&crit_sem);    //critical section eginnt
    maenner_in--;         /* Ein mann geht raus */

    /// Ausgabe
    printf("Mann verlaesst das Badezimmer.");
    print_status();

    //Wenn der letzte mann rausgeht

    //TODO dass der letzte das Bad verlassen hat
    if(maenner_in == 0)
    {
        // Ausgabe
        printf("Letzter Mann hat das Badezimmer verlassen.");
        print_status();
        if(frauen_warten > 0)
        {
            sem_post(&warten_frauen_sem);
        }
        else
            sem_post(&crit_sem);
    }
    else // ende der critical session
    {
        sem_post(&crit_sem);
    }  
    pthread_exit(NULL);
}

// FRAUEN
// gleich wie maenner
void * female(void * input)
{
    int i;
    int frauen_inWarteschlange;

    // alles formatieren ( buendig machen )
    {
        // warten
        sleep(WAITTIME);

        // Start der critical section.
        sem_wait(&crit_sem);

        frauen_warten++;

        //Ausgabe
        printf("Frau will ins Badezimmer.");
        print_status();

        // sind gerade maenner drin?
        if(maenner_in > 0 || maenner_warten > 0)
        {
             //Ausgabe
            printf("Frauen warte dass sie dran sind.");
            print_status();

            sem_post(&crit_sem); //ende der critical section
            sem_wait(&warten_frauen_sem); // frauen warten dass sie dran sind
            //baton technique. 
            
            frauen_in++;
            frauen_warten--;

            
            if(lassLeuterein == false)
            {
                lassLeuterein = true; // nur die erste frau soll das machen

                frauen_inWarteschlange = frauen_warten;

                //Ausgabe
                printf("Frauen sind dran, die erste geht hinein.");
                print_status();

                // alle anderen Frauen auch reinlassen*/
                for(i = 0; i < frauen_inWarteschlange; i++)
                {
                    sem_post(&warten_frauen_sem);
                    sem_wait(&crit_sem);
                }
                
                lassLeuterein = false;  //wenn alle drinnen dann niemanden mehr reinlassen
            }
            
            else
            {
                //Ausgabe
                printf("Frau betritt das Badezimmer.");
                print_status();
            }
        }
        else // wenn eh schon frauen drinn sind
        {
                frauen_in++;
                frauen_warten--;
                printf("Frau betritt das Badezimmer.");
                print_status();
        }
        sem_post(&crit_sem); //ende der critical section

        // warten
        sleep(WAITTIME);

        sem_wait(&crit_sem);    //Start der critical section um badezimmer zu verlassen
        frauen_in--;       //eine Frau weniger

        //Ausgabe
        printf("Frau verlaesst das Badezimmer.");
        print_status();

        //letzte Frau
        if(frauen_in == 0)
        {
            //Ausgabe
            printf("Letzte Frau hat das Badezimmer verlassen.");
            print_status();

            if(maenner_warten > 0)
                sem_post(&warten_maenner_sem);
            else
                sem_post(&crit_sem);
        }
        else
            sem_post(&crit_sem);
    }
   
   //pthread_exit
    pthread_exit(NULL);
}


