/* Программа TCP-сервера с параллельной обработкой запросов*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

void main()
{
    int socklisten, sockaccept;   /* Дескрипторы для слушающего и присоединенного сокетов */
    int clilen;                   /* Длина адреса клиента */
    int n;                        /* Количество принятых символов */
    char line[1000];              /* Буфер для приема информации */
    struct sockaddr_in servaddr, cliaddr; /* Структуры для размещения полных адресов сервера и клиента */

    /* Создаем TCP-сокет */
    if ((socklisten = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror(NULL);
        exit(1);
    }

    /* Заполняем структуру для адреса сервера*/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(51000);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Настраиваем адрес сокета */
    if (bind(socklisten, (struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
        perror(NULL);
        close(socklisten);
        exit(1);
    }

    /* Переводим созданный сокет в пассивное состояние */
    if (listen(socklisten, 5) < 0)
    {
        perror(NULL);
        close(socklisten);
        exit(1);
    }

    clilen = sizeof(cliaddr); // В переменную clilen заносим максимальную длину ожидаемого адреса клиента 

    /* Основной цикл сервера */
    
    while (1)
    {
        /* Ожидаем полностью установленного соединения на слушающем сокете. */
        if ((sockaccept = accept(socklisten, (struct sockaddr *)&cliaddr, &clilen)) < 0)
        {
            perror(NULL);
            close(socklisten);
            exit(1);
        }
        switch (fork())
        {
        case -1:
        {
            // Не получилось создать дочерний процесс
            perror(NULL);
            close(socklisten);
            close(sockaccept);
            exit(1);
            break;
        }
        case 0:
        {
            close(socklisten); // Закрываем дескриптор слушающего сокета в дочернем процессе

            /* В цикле принимаем информацию от клиента до тех пор, пока не произойдет ошибки */
            while ((n = read(sockaccept, line, 999)) > 0)
            {
                /* Принятые данные отправляем обратно */
                if ((n = write(sockaccept, line, strlen(line) + 1)) < 0)
                {
                    perror(NULL);
                    close(socklisten);
                    close(sockaccept);
                    exit(1);
                }
            }
            /* Если при чтении возникла ошибка – завершаем работу */
            if (n < 0)
            {
                perror(NULL);
                close(socklisten);
                close(sockaccept);
                exit(1);
            }
            // Обработка запроса дочерним процессом завершена, прекращаем его работу
            exit(0);
        }
        default:
        {
            // Закрываем дескриптор присоединенного сокета в родительском процессе
            close(sockaccept);
        }
        }
       
    }
}