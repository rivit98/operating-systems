#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <strings.h>
#include <pthread.h>

void *hello(void *arg) {

    sleep(1);
    while (1) {
        printf("Hello world from thread number %d\n", *(int *) arg);
        /****************************************************
            przerwij dzialanie watku jesli bylo takie zadanie
        *****************************************************/

        pthread_testcancel();

        printf("Hello again world from thread number %d\n", *(int *) arg);
        sleep(2);
    }
    return NULL;
}


int main(int argc, char **args) {

    if (argc != 3) {
        printf("Not a suitable number of program parameters\n");
        return (1);
    }

    int n = atoi(args[1]);
    int i;
    pthread_t *tids = calloc(n, sizeof(pthread_t));
    int *ids = calloc(n, sizeof(int));
    bzero(tids, n);
    bzero(ids, n);

    /**************************************************
        Utworz n watkow realizujacych funkcje hello
    **************************************************/

    for(i = 0; i < n; i++){
        ids[i] = i+1;
        pthread_create(&tids[i], NULL, hello, (void *)&ids[i]);
    }


    i = 0;
    while (i++ < atoi(args[2])) {
        printf("Hello from main %d\n", i);
        sleep(2);
    }

    i = 0;

    /*******************************************
        "Skasuj" wszystke uruchomione watki i poczekaj na ich zakonczenie
    *******************************************/
    for(i = 0; i < n; i++){
        pthread_cancel(tids[i]);
    }

    for(i = 0; i < n; i++){
        pthread_join(tids[i], NULL);
    }

    printf("DONE");

    free(tids);
    free(ids);

    return 0;
}

