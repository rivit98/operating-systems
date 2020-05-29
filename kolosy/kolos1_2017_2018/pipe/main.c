#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int pdesk[2];
    if (pipe(pdesk) == -1) {
        perror("PIPE");
        return 1;
    }

    switch (fork()) {
        case -1:
            perror("FORK");
            return 2;
        case 0:
            dup2(pdesk[1], STDOUT_FILENO);
            execvp("ls", argv);
            perror("EXECVP ls");
            exit(1);
        default: {
			/* Zamknij deskryptor do zapisu,
			 * przekieruj deskryptor deskryptor wejścia standardowego na deskryptor końca do odczytu tego potoku,
			 * wykonaj  tr a-z A-Z, w przypadku błędu  obsłuż go i wyjdź, zwracając 3.
			*/
			close(pdesk[1]);
			dup2(pdesk[0], STDIN_FILENO);
			FILE* fp = popen("tr a-z A-Z", "r");
			if(!fp){
				perror("popen");
				return 3;
			}

			char buffer[512];
			while(fread(buffer, sizeof(char), sizeof(buffer), fp) > 0){
				printf("%s", buffer);
			}

			pclose(fp);

        }
    }
    return 0;
}
