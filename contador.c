/*
    COMPILA COM: mpicc -o contador contador.c
    EXECUTA COM: mpirun -np <numero de workers> ./contador <arquivo.txt>
*/

#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long int ull;

double calc_time(__time_t sec_ini, __time_t sec_end, __suseconds_t usec_ini, __suseconds_t usec_end){
	return (sec_end + (double) usec_end/1000000) - (sec_ini + (double) usec_ini/1000000);
}

void close_workers(int n){
    ull msg = 0;
    for (int i = 1; i < n; i++)
    {
        MPI_Send(&msg, 1, MPI_UNSIGNED_LONG_LONG, i, 0, MPI_COMM_WORLD);
    }    
}

int main(int argc, char **argv){
    int meurank, nprocs;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meurank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Status status;

    if(!meurank){
        FILE * names = fopen(argv[1], "r");

        struct timeval total_ini, total_end;

        fseek(names, 0, SEEK_END);
        ull file_size = ftell(names);
        fseek(names, 0, SEEK_SET);

        int file_procs_amount_part,
            min_buffer_size = 128,
            max_buffer = 128,
            num_msgs = 0,
            enter = 1;

        ull n = 0, n_l_six = 0, n_g_six = 0, pos_ant = 0;

        gettimeofday(&total_ini, 0);
        for(int i=0; pos_ant<file_size; i++){
            printf("\n            MIN BUFFER: %d bytes\n", min_buffer_size);

            int o=0;
            for(int j=1; j<nprocs && pos_ant<file_size; j++){
                ull relative_pos = pos_ant + min_buffer_size;

                relative_pos = (relative_pos > file_size) ? file_size : relative_pos;

                fseek(names, relative_pos, SEEK_SET);

                if(relative_pos != file_size){
                    int k=0;
                    while(!feof(names) && fgetc(names) != ' ') k++;
                    relative_pos+=k;
                }        

                ull buffer_size = relative_pos - pos_ant;

                char *buffer = malloc(buffer_size + 1);

                fseek(names, pos_ant, SEEK_SET);

                for(ull j = 0; j < buffer_size; j++){
                    buffer[j] = fgetc(names);
                }

                buffer[buffer_size] = '\0';
                buffer_size++;

                max_buffer = (buffer_size > max_buffer) ? buffer_size : max_buffer;
        
                pos_ant = ftell(names) + 1;
                    
                if(MPI_Send(&buffer_size, 1, MPI_UNSIGNED_LONG_LONG, j, 0, MPI_COMM_WORLD)){
                    fprintf(stderr, "Erro: %s\n", strerror(errno));
                    close_workers(nprocs);
                    return errno;
                }

                if(MPI_Send(buffer, buffer_size, MPI_CHAR, j, 0, MPI_COMM_WORLD)){
                    fprintf(stderr, "Erro: %s\n", strerror(errno));
                    close_workers(nprocs);
                    return errno;
                }                

                free(buffer);

                num_msgs++;
                o++;
                printf("MSG %d: Enviado buffer de %llu bytes ao worker %d (%llu/%llu)\n", num_msgs, buffer_size, j, pos_ant-1, file_size);
            }
            

            for(int i = 1; i <= o && o; i++){
                ull res[3];
                MPI_Recv(&res[0], 1, MPI_UNSIGNED_LONG_LONG, i, 1, MPI_COMM_WORLD, &status);
                MPI_Recv(&res[1], 1, MPI_UNSIGNED_LONG_LONG, i, 2, MPI_COMM_WORLD, &status);
                MPI_Recv(&res[2], 1, MPI_UNSIGNED_LONG_LONG, i, 3, MPI_COMM_WORLD, &status);
                n += res[0], n_l_six += res[1], n_g_six += res[2];
            }

            printf("    PARCIAL: Total de %llu palavras: %llu menores de 6 bytes e %llu entre 6 e 10 bytes\n", n, n_l_six, n_g_six);
            
            //Divisão mais balanceada do arquivo para os workers - envia mais msgs com tamanho parecido
            //file_procs_amount_part = ceil((file_size - pos_ant)/nprocs);

            //Seta um limite estático - Envia bem menos msgs
            // !!! se esse valor passar de 8192000, pode gerar erro por conta do limite da stack do sistema
            // !!! dá pra checar o limit pelo terminal com o comando ulimit -s
            // !!! pra enviar msgs de 10000000 bytes, tem que ajustar esse limite nas configurações do SO
            // !!! para testar com o limite maior, basta rodar o comando ulimit -s 11000 no terminal
            file_procs_amount_part = 10000000;
            
            min_buffer_size*=2;

            if(min_buffer_size > 10000000 && enter) {
                min_buffer_size = 10000000;
                enter = 0;
            }
            else {
                min_buffer_size = (min_buffer_size > file_procs_amount_part) ? 128 : min_buffer_size;
                enter = 1;
            }
        }
        gettimeofday(&total_end, 0);

        double duration = calc_time(total_ini.tv_sec, total_end.tv_sec, total_ini.tv_usec, total_end.tv_usec);
        printf("\n[%.8lf s] Total de %llu palavras: %llu menores de 6 bytes e %llu entre 6 e 10 bytes\n     Tamanho do maior buffer enviado: %d\n\n", duration, n, n_l_six, n_g_six, max_buffer);
        close_workers(nprocs);
    }
    else{
        ull tam;

        while(1){
            MPI_Recv(&tam, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if(!tam) break;

            char buffer [tam];

            MPI_Recv(buffer, tam, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            ull worker_n = 0, worker_n_l_six = 0, worker_n_g_six = 0, worker_pos_ant = 0;

            for(ull i = 0; i < tam; i++){
                if(buffer[i] == ' ' || buffer[i] == '\0'){
                    int word_size = i - worker_pos_ant;    

                    if(!word_size){
                        worker_pos_ant = i + 1;
                        continue;
                    }
                    else if(word_size < 6) worker_n_l_six++;
                    else if(word_size < 10) worker_n_g_six++;

                    worker_pos_ant = i+1;
                    worker_n++;
                }
            }

            MPI_Send(&worker_n, 1, MPI_UNSIGNED_LONG_LONG, 0, 1, MPI_COMM_WORLD);
            MPI_Send(&worker_n_l_six, 1, MPI_UNSIGNED_LONG_LONG, 0, 2, MPI_COMM_WORLD);
            MPI_Send(&worker_n_g_six, 1, MPI_UNSIGNED_LONG_LONG, 0, 3, MPI_COMM_WORLD);
        }

        printf("    Worker %d se dispede! 0/\n", meurank);      
    }

    MPI_Finalize();

    return 0 ;
}