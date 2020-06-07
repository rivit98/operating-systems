#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/mman.h>
#include <strings.h>

#define SHM_NAME "/kol_shm"
#define MAX_SIZE 1024

int main(int argc, char **argv)
{
    
   if(argc !=2){
    printf("Not a suitable number of program parameters\n");
    return(1);
   }

   /***************************************
   Utworz/otworz posixowy obszar pamieci wspolnej "reprezentowany" przez SHM_NAME
   zapisz tam wartosc przekazana jako parametr wywolania programu
   posprzataj
   *****************************************/

    int shmid = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shmid, MAX_SIZE);

    char *mem = mmap(NULL, MAX_SIZE, PROT_WRITE, MAP_SHARED, shmid, 0);
    bzero(mem, MAX_SIZE);
    strncpy(mem, argv[1], MAX_SIZE);
    munmap(mem, MAX_SIZE);

    return 0;
}
