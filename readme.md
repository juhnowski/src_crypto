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

# KDF - Key Derivation Function 
Функция формирования ключа
IKM(Input Key Material) -> KDF -> OKM (Output Key Material)
PBKDF - Password-Based Key Derivation Function

Входные параметры KDF
- IKM, например пароль
- соль - случайно сгенерированные данные
- информация, зависящая от приложения
- базовая псевдослучайная функция PRF - PseudoRandom Function, например функция вычисления HMAC или блочный шифр
- параметры для функции PFR, стойкие к полному перебору, например счетчик операций
- желаемая длина OKM

Энтропия пароля:
Entropy =log2(nchar^plen)
nchar - количество возможных символов, 80 (большие, маленькие, цифры, символы)
plen - длина пароля
8 символов = 51 бит
12 символов = 76 бит

Если PBKDF формирует 256-битовый ключ из пароля, содержащего 76 бит энтропии, и соль открыта, то выходной ключ все равно будет содержать только 76 бит энтропии

OpenSSL поддерживает:
- PBKDF2 - HMAC-SHA-256, настраиваемое число операций, высокое потребление процессора, OWASP (Open Web Application Security Project) рекомендовано 310 000 итераиций 
- Scrypt - лушая, может быть настроен как на потребление процессора, так и памяти. В 2021 OWASP рекомендовал N=65536, r=8, p=1
- KDF, HKDF - не являются вычислительно трудоемкими
- ANSI X9.42, ANSI X9.63 и TLS1  не являются вычислительно трудоемкими
## Формирование ключа из пароля в командной строке
Документация
```
man openssl-kdf
```
Генерируем 128-битовую соль:
```
openssl rand -hex 16
d6d03beb84500bf2d374a8531c69b363
```
Формируем 256-битовый ключ симметричного шифрования
```
openssl kdf \
-keylen 32 \
-kdfopt pass:SuperPa$$w0rd \
-kdfopt hexsalt:d6d03beb84500bf2d374a8531c69b363 \
-kdfopt n:65536 \
-kdfopt r:8 \
-kdfopt p:1 \
SCRYPT
```
результат
```
D4:C9:30:4C:B7:ED:FE:72:DA:CC:C6:0F:1E:DD:9D:92:02:DE:65:97:32:41:90:7E:3F:12:60:CC:9B:42:C5:B8
```
Пример из документации
```
openssl kdf -keylen 64 -kdfopt pass:password -kdfopt salt:NaCl \
                       -kdfopt n:1024 -kdfopt r:8 -kdfopt p:16 \
                       -kdfopt maxmem_bytes:10485760 SCRYPT
```
результат:
```
FD:BA:BE:1C:9D:34:72:00:78:56:E7:19:0D:01:E9:FE:7C:6A:D7:CB:C8:23:78:30:E7:73:76:63:4B:37:31:62:2E:AF:30:D9:2E:22:A3:88:6F:F1:09:27:9D:98:30:DA:C7:27:AF:B9:4A:83:EE:6D:83:60:CB:DF:A2:CC:06:40
```
## Программная реализация
```
cd kdf
cc kdf.c -lssl -lcrypto
./a.out SuperPa$$w0rd d6d03beb84500bf2d374a8531c69b363
D4:C9:30:4C:B7:ED:FE:72:DA:CC:C6:0F:1E:DD:9D:92:02:DE:65:97:32:41:90:7E:3F:12:60:CC:9B:42:C5:B8
```
# Ассиметрично шифрование
MITM - Man in the Middle - атака, в которой противник прослушивает транзитный трафик и изменяет его, пытаясь выдать себя:
- отправителю за получателя
- получателю за отправителя
Варианты борьбы:
- личная встреча
- встречи для подписания ключей
- проверка цифрового отпечатка ключа по телефону
- разделение ключа
- подписание ключа доверенной третьей стороной
- сеть доверия PGP
- PKI - Public Key Infrastructure 

## Алгоритмы
RSA - Непосредственная шифрация данных

Для обмена ключами в протоколе TLS
DSA - Digital Signature Algorithm
ECDSA - Elliptic Curve Digital Signature Algorithm
ECDH - Elliptic Curve Diffie-Helman

ElGamal - опосредованное шифрование

У каждого алгоритма ассимитричной криптографии своя структура открытого и закрытого ключей.
Поэтому ключи есть разные: RSA, ECDSA...

## Генерируем пару ключей RSA
```
cd rsa

openssl genpkey \
-algorithm RSA \
-pkeyopt rsa_keygen_bits:4096 \
-out rsa_keypair.pem
```
PEM - Privacy Enhanced Mail - почта с повышенной секретностью, обертка (заголовок и концевик), 
DER внутри - Distinguished Encoding Rules, формат хранения ключей и сертификатов 
```
man openssl-pkey
```
Просмотреть пару ключей можно командой
```
openssl pkey -in rsa_keypair.pem -noout -text
```
Извлечь открытый ключ из пары:
```
openssl pkey \
-in rsa_keypair.pem \
-pubout \
-out rsa_public_key.pem
```
Посмотреть содержание открытого ключа:
```
openssl pkey -pubin -in rsa_public_key.pem -noout -text
```
## Шифрация/дешифрация RSA
Генерируем 256-битный сеансовый ключ
```
openssl rand -out session_key.bin 32
```
Шифруем сеансовый ключ
```
openssl pkeyutl \
-encrypt \
-in session_key.bin \
-out session_key.bin.encrypted \
-pubin \
-inkey rsa_public_key.pem \
-pkeyopt rsa_padding_mode:oaep
```
где -pkeyopt rsa_padding_mode:oaep задает использовать оптимальное дополнение ассиметричного шифрования OAEP

```
cksum session_key.bin*
```
Если повторно зашифровать, то размер не изменится, но контрольная сумма всегда будет разная

Генерируем открытый текс
```
seq 20000 > somefile.txt
```

Шифруем его
```
openssl pkeyutl \
-encrypt \
-in somefile.txt \
-out somefile.txt.encrypted \
-pubin \
-inkey rsa_public_key.pem \
-pkeyopt rsa_padding_mode:oaep
```
получаем ошибку что открытый текст длиннее заданного ключа. 

```
openssl pkeyutl \
-decrypt \
-in session_key.bin.encrypted \
-out session_key.bin.decrypted \
-inkey rsa_keypair.pem \
-pkeyopt rsa_padding_mode:oaep
```
Проверяем контрольную сумму:
```
cksum session_key.bin*
```
# Шифрование RSA программно
Выполняем rsa-encrypt и шифруем созданный ранее 32-байтовый сеансовый ключ:
```
./rsa-encrypt \
session_key.bin \
session_key.bin.encrypted \
rsa_public_key.pem
```
Проверяем контрольную сумму:
```
cksum session_key.bin*
```
Проверяем, что программа интероперабельная
```
openssl pkeyutl \
-decrypt \
-in session_key.bin.encrypted \
-out session_key.bin.decrypted \
-inkey rsa_keypair.pem \
-pkeyopt rsa_padding_mode:oaep
```
Проверяем контрольную сумму:
```
cksum session_key.bin*
```