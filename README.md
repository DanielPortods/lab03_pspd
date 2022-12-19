### Universidade de Brasília

### Programação para Sistemas Paralelos e Distribuídos

### Prof. Fernando William Cruz – 2022/2 – 08/12/2022 – Turma A

# Construindo aplicações distribuídas usando MPI

**Nome/Matrícula: Daniel Porto de Souza / 180149687 -  Denys Rogeres Leles dos Santos / 180041592**

## Introdução:
Message Passing Interface, ou simplesmente MPI, trata-se de um padrão para comunicação de dados relativo à computação paralela. Ou seja, o MPI permite a comunicação entre processos por meio de uma API de comunicação, tudo isso de forma eficiente e flexível.

## Solução:
Para se monitorar o desempenho da solução, foram utilizadas as estruturas da biblioteca sys/time para registar o tempo de execução da contagem.

- **Solução com Workers utilizando MPI:** Uma rotina em que é utilizado MPI para comunicação entre o master e os workers. Inicialmente é dividido o arquivo conforme o seu tamanho e o número de workers e enviado mensagens fragmentadas para cada worker. Os workers, por sua vez, farão a contagem da fração do arquivo recebido e retornará a quantidade total de palavras, a quantidade de palavras menores de 6 bytes e quantidade de palavras com tamanho entre 6 e 10 bytes. Em seguida, a master fará o somatório dos resultados e apresentará na tela o resultado final.

## Instalação e Uso:
O presente relatório acompanha a rotina:
- **contador.c:** contém a solução paralela e distribuída utilizando o MPI com o objetivo de contagem de palavras menores que 6 bytes, entre 6 e 10 bytes e a quantidade total de palavras.

### Solução com Worker utilizando MPI:
Para testar a solução, será preciso ter a biblioteca mpi instalada. Para isso, basta executar o comando:
>$ sudo apt install mpich
 	
Em seguida, dentro do diretório app_mpi, execute o programa com o comando:
>$ mpicc -o contador contador.c
 	
Por último, execute o programa com o comando:
>$ mpirun -np numeroDeWorkers ./contador arquivo.txt
	
Caso ocorra alguma inconsistência por conta de slots indisponíveis, utilize o comando abaixo:
>$ mpirun -oversubscribe -np numero de workers ./contador arquivo.txt

Também é possível distribuir a solução em mais de um host. Para tanto, deve-se configurar o ssh e executar o app conforme [esse passo a passo](https://mpitutorial.com/tutorials/running-an-mpi-cluster-within-a-lan/). Por conta da indisponibilidade dos equipamentos, o presente relatório abordará os resultados obtidos utilizando 1 host.
	
A saída da aplicação mostrará a quantidade mínima de bytes em cada buffer, mostrando a quantidade de bytes enviada para cada broker seguido de quanto representa no tamanho total do arquivo, e em seguida mostrará uma contabilização parcial referente ao contabilizado pelos workers.

OBS: Como foi definido como 10.000.000 a quantidade de bytes limite que o processo master deve ler, é necessário utilizar o comando abaixo para aumentar o tamanho da stack do SO:

>$ ulimit -s 11000
### Resultados:

Para este trabalho, foram realizados testes com arquivos textos com 100.000 bytes, 1.000.000 bytes e por fim, arquivo com mais de 20.0000.000 bytes. Como os resultados relativos à performance foram parecidos, neste relatório foi mostrado o teste de carga que fizemos utilizando cerca de 22 milhões de palavras.

O único limite encontrado com as diretivas do MPI foi referente ao tamanho da stack do SO que, por default, estava setada em 8.192.000 de bytes. Dessa forma, ao aumentar o limite da stack com o comando ulimit, não houveram mais problemas para enviar o buffer de 10.000.000

Observamos que conforme aumentamos o número de workers, o tempo para realizar a contagem também aumentava. Chegamos a conclusão que tal fato deve-se por, além de se tratar de uma comunicação sincorona, estarmos aumentando o número de processos em uma única máquina, e, como a mesma possui recursos limitados, é fato que isso impactará no desempenho da aplicação. No entanto, talvez obtivéssemos resultados diferentes caso fosse utilizado um conjunto de máquinas diferentes.

De toda forma, a tabela abaixo representa o resultado obtido nos testes utilizando um arquivo com aproximadamente 22 milhões de palavras:


### Aplicação com Workers utilizando MPI

Nº Workers|Tempo médio gasto|Quantidade de Mensagens
-|-|-
2|10.78 segundos|172
4|15.84 segundos|196
6|19.19 segundos|205
8|22.88 segundos|239
10|26.02 segundos|176

## Opinião:

- **Denys Rógeres:** “Foi a primeira vez que realizei uma aplicação utilizando MPI e sem dúvida me trouxe grandes aprendizados que poderei aplicar em futuros projetos. Particularmente eu gostei de realizar este trabalho, mas sinto que seria melhor realizar essa aplicação com um conjunto de máquinas diferentes, fiquei com a vontade de realizar isso. Como participação, acredito que uma nota 7 representa bem meu desempenho, fiquei responsável por realizar os testes e ajustes, bem como construir o relatório.”

- **Daniel Porto:** “Foi um laboratório relativamente tranquilo, mas que muito agregou no meu conhecimento sobre MPI em C. Foi interessante pois consegui diferenciar a comunicação com MPI dos Brokers por fazer uma rotina síncrona. Acredito que os resultados seriam diferentes ao implementar uma rotina assíncrona e testar o desempenho com outros hosts participantes, o que não foi feito por conta da indisponibilidade dos equipamentos. Me empenhei bastante junto ao Denys nas pesquisas iniciais e desenvolvimento do código para a realização desse laboratório, por isso me avalio com 10.”
