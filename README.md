# TransportCatalogue

TransportCatalogue - транспортный справочник. Работает с JSON-запросами. Выдаёт ответ на запрос отрисовки маршрутов строкой SVG формата. Реализован конструктор JSON с использованием цепочки вызовов методом, явные ошибки находятся на этапе компиляции. Для сериализации и десериализации транспортного справочника применяется Google Protocol Buffers(Protobuf). 

## Требования

* C++17 и выше
* Protobuf 3

---

## Установка Protobuf

Для сборки проекта потребуется установить Google Protocol Buffers. Для этого нужно:
1. Скачать Protobuf можно с репозитория на GitHub. Выберите архив protobuf-cpp с исходным кодом последней версии и распакуйте его на своём компьютере. Исходный код содержит CMake-проект.
2. Создать папки build-debug и build-release для сборки двух конфигураций Protobuf. Если будет использоваться Visual Studio, будет достаточно одной папки build. А если CLion или QT Creator, IDE автоматически создаст папки при открытии файла CMakeLists.txt.
3. Создать папку, в которой разместим пакет Protobuf. Будем называть её ***/path/to/protobuf/package***(далее следует указывать абсолютный путь, а не относительный).

Если сборка будет происходить не через IDE, в папке Debug выполните следующие команды:
```
cmake <путь к protobuf>/cmake -DCMAKE_BUILD_TYPE=Debug \
      -Dprotobuf_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=/path/to/protobuf/package
cmake --build . 
```
где:
* ```Dprotobuf_BUILD_TESTS=OFF``` — чтобы не тратить время на сборку тестов,
* ```DCMAKE_INSTALL_PREFIX=/path/to/protobuf/package``` — чтобы сообщить, где нужно будет создать пакет Protobuf.

Для Visual Studio команды немного другие. Конфигурация указывается не при генерации, а при сборке:
```
cmake <путь к protobuf>/cmake ^
      -Dprotobuf_BUILD_TESTS=OFF ^
      -DCMAKE_INSTALL_PREFIX=/path/to/protobuf/package ^
      -Dprotobuf_MSVC_STATIC_RUNTIME=OFF
cmake --build . --config Debug 
```

После сборки библиотеки нужно запустить команду:
```cmake --install . ```
Под Visual Studio нужно указать конфигурацию, поскольку она не задавалась во время генерации:
```cmake --install . --config Debug ```

Далее нужно проделать те же шаги с конфигурацией Release. 

**Важно!** При сборке проекта, нужно указать путь к protobuf:
```DCMAKE_PREFIX_PATH=/path/to/protobuf/package```

---

## Сборка

С помощью CMake собрать файл CMakeLists.txt.

## Аргументы cmd для запуска программы

```make_base``` — создание базы транспортного справочника по запросам и её сериализация в файл с помощью Protobuf.
```process_requests``` — десериализация базы из файла и использование её для ответов на запросы stat_requests.
