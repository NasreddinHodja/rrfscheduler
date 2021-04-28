
# Table of Contents

1.  [Escopo](#orge4de1c4)
2.  [Premissas a serem definidas](#org63a6e4d)



<a id="orge4de1c4"></a>

# Escopo

-   desenvolver um simulador que implementa o algoritmo de escalonamento de processos usando a estratégia de seleção *Round Robin* com *Feedback*
-   preparar um relatório contendo uma descrição sobre:
    -   os objetivos do trabalho
    -   as premissas consideradas no desenvolvimento
    -   a saída e execução do simulador
-   gravar um vídeo de até 10 minutos onde todos os participantes do grupo apresentam o simulador


<a id="org63a6e4d"></a>

# Premissas a serem definidas

-   limite máximo de processos
-   valor da fatia de tempo
-   tempos de serviço de I/O aleatórios para cada processo criado
-   tempos de duração de cada tipo de I/O (disco, fita magnética e impressora)
-   gerência de processos
    -   definição do PID de cada processo
    -   informações do PCB (contexto de software - prioridade, PID, PPID, status)
    -   escalonador (pelo menos 3 filas, sendo uma fila de alta e uma de baixa prioridade para execução na CPU, e 1 fila de I/O, que pode ser implementada com filas diferentes para cada tipo de dispositivo)
-   tipos de I/O
    -   disco - retorna para a fila de **baixa** prioridade
    -   fita magnética - retorna para a fila de **alta** prioridade
    -   impressora - retorna para a fila de **alta** prioridade
-   ordem de entrada na fila de prontos
    -   processos novos - entram na fila de **alta** prioridade
    -   processos que retornam de I/O - dependente do tipo de I/O solicitado
    -   processos que sofreram preempção - retornam na fila de **baixa** prioridade

**OBS**: As premissas estabelecidas devem estar explícitas no relatório

