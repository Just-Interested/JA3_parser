# JA3_parser
Тестовый проект.

Задача обрабатывать файлы pcap и pcapng в указанной директории, извлекать JA3 хэши из SSL соединений и сохранять в файл в виде:
timestamp,src_ip,src_port,client_JA3,dst_ip,dst_port,server_JA3,server_name

Дополнительные требования: 
1. Программа должна запускаться на ОС Centos 7.
2. Программа должна иметь возможность запускаться в режиме демона.

Статические библиотеки (libPcap++.a, libPacket++.a, libCommon++.a) и заголовочные файлы взяты отсюда:
https://github.com/seladb/PcapPlusPlus/releases/download/v22.11/pcapplusplus-22.11-centos-7-gcc-4.8.5.tar.gz

Работоспособность проверялась в docker-контейнере, запускаемом из образа centos:7
Пример команды для запуска:
docker run --rm -it --entrypoint=/bin/bash -v "/working/directory":/data centos:7
, где /working/directory - каталог содержащий исполняемый файл, файл конфигурации, и и каталог captured с файлами для обработки


Замечание:
Существуют проблемы со статической линковкой библиотеки libm.a. Для решения проблемы в файл main.cpp было добавлено определение структуры cpu_features.
Подробнее: https://stackoverflow.com/questions/56415996/linking-error-selective-static-linking-of-libm-a-in-gcc
