# Случайные последовательности
```
xxd -plain -len 32 -cols 32 /dev/random
```
```
openssl rand -hex 32
```

# Test
## Создаем открытый текст
```
seq 1000 > somefile.txt
```

## Создаем 256-битовый ключ шифрования
```
openssl rand -hex 32
9beafdc657a80ae80fed27b785bf7aa5160c971038a8f3129aaa84a074380696
```
## Создаем 128-битовый IV
```
openssl rand -hex 16
747f536cd56ffa68d2620d0b29241e11
```
## Шифруем файл
```
openssl enc \
-aes-256-cbc \
-K 9beafdc657a80ae80fed27b785bf7aa5160c971038a8f3129aaa84a074380696 \
-iv 747f536cd56ffa68d2620d0b29241e11 \
-e \
-in somefile.txt \
-out somefile.txt.encrypted
```
## Дешифруем файл
```
openssl enc \
-aes-256-cbc \
-K 9beafdc657a80ae80fed27b785bf7aa5160c971038a8f3129aaa84a074380696 \
-iv 747f536cd56ffa68d2620d0b29241e11 \
-d \
-in somefile.txt.encrypted \
-out somefile.txt.decrypted
```
## Проверяем чексумму
```
cksum somefile.txt*
```

# Компиляция и компановка с OpenSSL
# OpenSSL init
```
cd init
cc init.c -lssl -lcrypto
./a.out
```
Вариант с makefile
```
make
ldd init
./init
```

# Шифрование с применением AES-256 в режиме GCM из программы
Имя файла: encrypt
Формат файла:

Позиция,| байты Длина,байты|  Содержимое   |
|-------|------------------|---------------| 
|   0   |       12         |     IV        | 
|  12   |       ?          | Шифртекст     |
|   ?   |       16         | Аутентификационный жетон

## Компиляция
```
cc encrypt.c -lssl -lcrypto
```

## Генерируем тестовый файл для шифрования
```
seq 20000 > somefile.txt
```
## Генерируем ключ
```
openssl rand -hex 64
```
## Шифруем файл
```
./a.out somefile.txt somefile.txt.encrypted ba68bb60c17a109907760490f68f94ed3edcb006442486e47b5f62eaa954b62e
```
## Вычисляем проверочную сумму
```
cksum somefile.txt*
```
# Расшивровка
## Компиляция
```
cc decrypt.c -lssl -lcrypto
```
## Расшифровываем
```
./a.out somefile.txt.encrypted  somefile.txt.decrypted ba68bb60c17a109907760490f68f94ed3edcb006442486e47b5f62eaa954b62e
```
## Вычисляем проверочную сумму
```
cksum somefile.txt*
```

# SHA3 256
## Компиляция
```
cd sha3_256
cc digest.c -lssl -lcrypto
```
## Генерируем тестовый файл
```
seq 20000 > somefile.txt
```
## Вычисляем хэш-значение
```
./a.out somefile.txt
```

# MAC и HMAC
## Схемы
- EtM шифрование, затем MAC
- E&M шифрование и MAC
- MtE MAC, затем шифрование

Аутентификация - вычисление MAC
Принцип криптографической обреченности - если вы вынуждены выполнить любую криптографическую операцию до проверки имитовставки полученного сообщения, то это так или иначе, но неизбежно приведет к роковому концу.

## Режимы шифрования с аутентификацией
AES-GCM
ChaCha20-Poly1305

## TLS
сначала MtE
потом EtM
v.1.2 - в протокол были добавлены шифры с аутентификацией
v/1.3 - допустимы шифры только с аутентификацией
Для шифров с аутентификацией отдельная операция вычисления МАС не нужна, НМАС все равно используется в TLS1.3 как основа для псево=дослучайной функции PRF - PseudoRandomFunction

## SSH
1998 вторая версия
исторически E&M
схема аутентификации в SSH зависит от алгоритма вычисления MAC, в новых алгоритмах это EtM

## Вычисление HMAC в командной строке
### Генерируем открытый текст
```
cd hmac
seq 20000 > somefile.txt
```
### Генерируем секретный ключ
```
openssl rand -hex 32
c8bfce35ffa38bf1ce433297bb927d8cc22c897040e040d91d86c08b0e488928
```
### Вычисляем НМАС (Вариант №1)
```
openssl dgst -sha-256 -mac HMAC -macopt \
hexkey:c8bfce35ffa38bf1ce433297bb927d8cc22c897040e040d91d86c08b0e488928 \
somefile.txt
```
результат:
```
HMAC-SHA2-256(somefile.txt)= 36a8ae5ef865fdf253af2216ed4cb5ff3178f6df1a89e19998896a51df57c308
```
### Вычисляем НМАС (Вариант №2)
```
openssl mac -digest SHA-256 -macopt \
hexkey:c8bfce35ffa38bf1ce433297bb927d8cc22c897040e040d91d86c08b0e488928 \
-in somefile.txt \
HMAC
```
результат:
```
36A8AE5EF865FDF253AF2216ED4CB5FF3178F6DF1A89E19998896A51DF57C308
```
## Вычисление програмно
```
cd hmac
cc hmac.c -lssl -lcrypto
```
## Генерируем тестовый файл
```
seq 20000 > somefile.txt
```
## Вычисляем хэш-значение
```
openssl rand -hex 32
2fc498c4a4ba59c76ef9c56525a3aaeb5df2af4d7413317d234f7bf12e3e73c5

./a.out somefile.txt 2fc498c4a4ba59c76ef9c56525a3aaeb5df2af4d7413317d234f7bf12e3e73c5
```