Необходимо реализовать прокси-сервер, соответствующий стандарту SOCKS версии 5.
В параметрах программе передаётся только порт, на котором прокси будет ждать входящих подключений от клиентов.
Из трёх доступных в протоколе команд, обязательной является только реализация команды 1 (establish a TCP/IP stream connection)
Поддержку аутентификации и IPv6-адресов реализовывать не требуется.
Реализация должна быть основана на неблокирующихся сокетах.
Прокси должен поддерживать резолвинг доменных имён (значение 0x03 в поле address). Резолвинг тоже должен быть неблокирующимся.
На старте программы создать новый UDP-сокет и добавить его в селектор на чтение
Когда необходимо отрезолвить доменное имя, отправлять через этот сокет DNS-запрос A-записи на адрес рекурсивного DNS-резолвера
В обработчике чтения из сокета обрабатывать случай, когда получен ответ на DNS-запрос, и продолжать работу с полученным адресом
